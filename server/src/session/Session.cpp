#include "Session.h"
#include "core/Server.h"
Session::Session(std::shared_ptr<asio::ip::tcp::socket> sock, Server &srv) : socket_ptr(sock), server(srv) {}
void Session::start()
{
    do_read_header();
}
void Session::do_read_header()
{
    auto self = shared_from_this();
    asio::async_read(*socket_ptr, asio::buffer(header_buf, header_length),
                     [this, self](const asio::error_code &ec, size_t bytes_transferred)
                     {
                         if (!ec)
                         {
                             const uint32_t body_length = ntohl(*reinterpret_cast<uint32_t *>(header_buf.data()));
                             if (body_length > 0 && body_length < max_body_length)
                                 do_read_body(body_length);
                             else
                             {
                                 std::cerr << "Invalid body length received: " << body_length << std::endl;
                                 handle_error("Invalid length", asio::error_code());
                             }
                         }
                         else
                         {
                             std::cerr << "Read header error: " << ec.message() << std::endl;
                             handle_error("Read header", ec);
                         }
                     });
}
void Session::do_read_body(const uint32_t body_length)
{
    body_buf.resize(body_length);
    auto self = shared_from_this();
    asio::async_read(*socket_ptr, asio::buffer(body_buf.data(), body_length),
                     [this, self](const asio::error_code &ec, size_t bytes_transferred)
                     {
                         if (!ec)
                         {
                             chat::Envelope envelope;
                             if (envelope.ParseFromArray(body_buf.data(), body_buf.size()))
                             {
                                 server.onMessage(shared_from_this(), envelope);
                                 do_read_header();
                             }
                             else
                             {
                                 std::cerr << "Failed to parse message body." << std::endl;
                                 handle_error("Failed to parse", asio::error_code());
                             }
                         }
                         else
                         {
                             std::cerr << "Read body error: " << ec.message() << std::endl;
                             handle_error("Read body", ec);
                         }
                     });
}
void Session::send(const chat::Envelope &envelope)
{
    std::string body_data;
    envelope.SerializeToString(&body_data);
    uint32_t body_length = htonl(body_data.size());
    std::string write_buf;
    write_buf.reserve(body_data.size() + sizeof(body_length));
    write_buf.append(reinterpret_cast<const char *>(&body_length), sizeof(body_length));
    write_buf.append(body_data);
    auto self = shared_from_this();
    asio::post(socket_ptr->get_executor(),
               [this, self, buffer = std::move(write_buf)]() mutable
               {
                   bool write_in_progress = !message_queue.empty();
                   message_queue.push(std::move(buffer));
                   if (!write_in_progress)
                       do_write();
               });
}
void Session::do_write()
{
    const std::string &write_buf = message_queue.front();
    auto self = shared_from_this();
    asio::async_write(*socket_ptr, asio::buffer(write_buf),
                      [this, self](const asio::error_code &ec, size_t bytes_transferred)
                      {
                          handle_write(ec, bytes_transferred);
                      });
}
void Session::handle_write(const asio::error_code &ec, size_t bytes_transferred)
{
    if (!ec)
    {
        message_queue.pop();
        if (!message_queue.empty())
            do_write();
    }
    else
    {
        std::cerr << "Write error: " << ec.message() << std::endl;
        handle_error("Write", ec);
    }
}
void Session::handle_error(const std::string &what, const asio::error_code &ec)
{
    if (is_closed)
    {
        return;
    }
    if (ec == asio::error::eof)
    {
        std::cout << "[INFO] Session closed by peer (" << what << ")." << std::endl;
    }
    else if (ec == asio::error::operation_aborted)
    {
        return;
    }
    else if (ec)
    {
        std::cerr << "[ERROR] Session error in " << what << ": " << ec.message() << std::endl;
    }
    is_closed = true;
    server.onDisconnect(shared_from_this());
}
void Session::setAuthenticated(long long userId, const std::string &username)
{
    this->userId = userId;
    this->username = username;
    this->Authenticated = true;
}

bool Session::isAuthenticated() const
{
    return Authenticated;
}

long long Session::getUserId() const
{
    if (!userId.has_value())
    {
        throw std::logic_error("User ID not set");
    }
    return userId.value();
}

std::string Session::getUsername() const
{
    return username.value();
}

void Session::setUsername(const std::string &newUsername)
{
    this->username = newUsername;
}

void Session::clearAuthentication()
{
    userId.reset();
    username.reset();
    Authenticated = false;
}