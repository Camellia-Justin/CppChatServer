#include "client.h"
#include <iostream>
#include <google/protobuf/util/time_util.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

Client::Client(asio::io_context& io_context)
    : io_context(io_context), socket(io_context), work_guard(asio::make_work_guard(io_context)) {}

//void Client::connect(const std::string& host, unsigned short port) {
//    auto self = shared_from_this();
//    asio::ip::tcp::resolver resolver(io_context);
//    resolver.async_resolve(host, std::to_string(port),
//        [this, self](const asio::error_code& ec, asio::ip::tcp::resolver::results_type endpoints) {
//            if (!ec) {
//                asio::async_connect(socket, endpoints,
//                    [this, self](const asio::error_code& ec, const asio::ip::tcp::endpoint& endpoint) {
//                        if (!ec) {
//                            std::cout << "[System] Successfully connected to " 
//                                      << endpoint << std::endl;
//                            // 连接成功后，启动读循环
//                            do_read_header();
//                        } else {
//                            handle_error("connect", ec);
//                        }
//                    });
//            } else {
//                handle_error("resolve", ec);
//            }
//        });
//}
// client/src/ChatClient.cpp

void Client::connect(const std::string& host, unsigned short port, std::function<void(const asio::error_code&)> handler) {
    auto self = shared_from_this();
    asio::error_code ec;

    // 1. 将 host 字符串（比如 "127.0.0.1"）转换成 Asio 的地址对象
    asio::ip::address address = asio::ip::make_address(host, ec);

    if (ec) {
        // 如果转换失败（比如 host 不是一个合法的 IP 地址字符串），立即报错
        std::cerr << "[FATAL] Invalid IP address format: " << host << std::endl;
        handle_error("make_address", ec);
        return;
    }

    // 2. 使用地址和端口创建一个 endpoint
    asio::ip::tcp::endpoint endpoint(address, port);

    // 3. 直接在 socket 上调用 async_connect
    socket.async_connect(endpoint,
        [this, self,handler](const asio::error_code& ec) {
			handler(ec); 
            if (!ec) {
                std::cout << "[System] Successfully initiated connection to "
                    << socket.remote_endpoint() << std::endl;
                do_read_header();
            }
            else {
                handle_error("connect", ec);
            }
        });
}

void Client::close() {
    asio::post(io_context, [this]() { socket.close(); work_guard.reset(); });
}

void Client::do_read_header(){
    auto self=shared_from_this();
    asio::async_read(socket,asio::buffer(read_header,header),
        [this,self](const asio::error_code& ec,size_t bytes_transferred){
            if(!ec){
                    const uint32_t body_length=ntohl(*reinterpret_cast<uint32_t*>(read_header.data()));
                    if(body_length>0 && body_length<max_body_length)do_read_body(body_length);
                    else{
                        std::cerr << "Invalid body length received: " << body_length << std::endl;
                        handle_error("Invalid length", asio::error_code());
                    }
            }else{
                std::cerr << ">>> Actual error code: " << ec.value() << " (" << ec.message() << ")" << std::endl;
                if (ec == asio::error::eof ||
                    ec == asio::error::connection_reset ||
                    ec == asio::error::operation_aborted ||
                    ec.value() == 1236 ||
                    ec.value() == 10053 ||
					ec.value() == 10054 ||
					ec.value() == 104 ||
                    ec.value() == 103)   {
                    handle_error("Connection closed by peer", ec);
                    return;
                }
                std::cerr << "Read header error: " << ec.message() << std::endl;
                handle_error("Read header",ec);
            }
        });
}
void Client::do_read_body(const uint32_t body_length){
    read_body.resize(body_length);
    auto self=shared_from_this();
    asio::async_read(socket,asio::buffer(read_body.data(),body_length),
        [this,self](const asio::error_code& ec,size_t bytes_transferred){
            if(!ec){
                chat::Envelope envelope;
                if(envelope.ParseFromArray(read_body.data(),read_body.size())){
                    handle_server_message(envelope); // 调用消息处理器
                    do_read_header();
                }else{
                    std::cerr<<"Failed to parse message body."<<std::endl;
                    handle_error("Failed to parse", asio::error_code());
                }
            }else{
                std::cerr<<"Read body error: "<<ec.message()<<std::endl;
                handle_error("Read body",ec);
            }
        });
}    
void Client::send(const chat::Envelope& envelope){
    std::string body_data;
    envelope.SerializeToString(&body_data);
    uint32_t body_length=htonl(body_data.size());
    std::string write_buf;
    write_buf.reserve(body_data.size()+sizeof(body_length));
    write_buf.append(reinterpret_cast<const char*>(&body_length),sizeof(body_length));
    write_buf.append(body_data);
    auto self=shared_from_this();
    asio::post(socket.get_executor(),
        [this,self,buffer=std::move(write_buf)](){
            bool write_in_progress=!write_queue.empty();//队列为空，说明没有正在发送的操作
            write_queue.push_back(std::move(buffer));//将消息加入队列
            if(!write_in_progress)do_write();//如果为空，就启动新的发送操作，如果不为空，说明有正在发送的操作，等待其完成后会继续发送队列中的消息
        });
}
void Client::do_write(){
    const std::string& write_buf=write_queue.front();
    asio::async_write(socket,asio::buffer(write_buf),
        [this,self=shared_from_this()](const asio::error_code& ec,size_t bytes_transferred){
            handle_write(ec,bytes_transferred);
        });
}
void Client::handle_write(const asio::error_code& ec,size_t bytes_transferred){
    if(!ec){
        write_queue.pop_front();
        if(!write_queue.empty())do_write();
    }else{
        std::cerr<<"Write error: "<<ec.message()<<std::endl;
        handle_error("Write",ec);
    }
}
// 消息处理器
void Client::handle_server_message(const Envelope& envelope) {
    switch (envelope.payload_case()) {
        case Envelope::kMessageBroadcast: {
            const auto& msg = envelope.message_broadcast();
            std::string time_str = google::protobuf::util::TimeUtil::ToString(msg.timestamp());
                std::cout << "[" << msg.room_name() << " | " << msg.from_username() << " at " << time_str << "]: " 
                          << msg.content() << std::endl;
            break;
        }
        case Envelope::kLoginResponse: {
            const auto& resp = envelope.login_response();
            std::cout << "[System] Login response: " << resp.message() << std::endl;
            break;
        }
        case Envelope::kRegistrationResponse: {
            const auto& resp = envelope.registration_response();
            if (resp.success()) {
                std::cout << "[System] Registration successful! You can now log in." << std::endl;
            }
            else {
                std::cout << "[System] Registration failed: " << resp.message() << std::endl;
            }
            break;
        }
        case Envelope::kChangePasswordResponse: {
            const auto& resp = envelope.change_password_response();
            std::cout << "[System] Change password response: " << resp.message() << std::endl;
            break;
        }
        case Envelope::kChangeUsernameResponse: {
            const auto& resp = envelope.change_username_response();
            std::cout << "[System] Change username response: " << resp.message() << std::endl;
            break;
        }
        case Envelope::kRoomOperationResponse: {
            const auto& resp = envelope.room_operation_response();
            if (resp.success() && (resp.operation() == chat::RoomOperation::JOIN)) {
                setCurrentRoom(resp.room_name());
				chat::Envelope historyReq;
				historyReq.mutable_history_message_request()->set_room_name(resp.room_name());
				historyReq.mutable_history_message_request()->set_limit(20);
                send(historyReq);
            }
            else if (resp.operation() == chat::RoomOperation::CREATE) {
                setCurrentRoom(resp.room_name());
            }
            else if (resp.success() && (resp.operation() == chat::RoomOperation::LEAVE)) {
                setCurrentRoom("");
			}
            std::cout << "[System] Room operation response: " << resp.message() << std::endl;
            break;
        }
        case Envelope::kHistoryMessageResponse: {
            const auto& resp = envelope.history_message_response();
            std::cout << "[System] History messages:" << std::endl;
            for (const auto& msg : resp.messages()) {
                std::string time_str = google::protobuf::util::TimeUtil::ToString(msg.timestamp());
                std::cout << "  [" << msg.room_name() << " | " << msg.from_username() << " at " << time_str << "]: " 
                          << msg.content() << std::endl;
            }
            break;
        }
        case Envelope::kServerNotification: {
            const auto& event = envelope.server_notification();
            std::cout << event.message() << std::endl;
            break;
        }
        case Envelope::kErrorResponse: {
            const auto& err = envelope.error_response();
            std::cout << "[Error] " << err.error_message() << " (Code: " << err.error_code() << ")" << std::endl;
            break;
        }
        default:
            std::cout << "[System] Received an unhandled message type from server." << std::endl;
            break;
    }
}
void Client::handle_error(const std::string& where, const std::error_code& ec) {
    if (ec == asio::error::eof) {
        std::cout << "[System] Connection closed by server (" << where << ")." << std::endl;
    } else if (ec) {
        std::cerr << "[Error] in " << where << ": " << ec.message() << std::endl;
    }
    else if (ec == asio::error::operation_aborted)
    {
        return;
    }
    else if (ec == asio::error::connection_reset) {
        std::cout << "[INFO] Connection reset by peer (" << where << ")." << std::endl;
    }
    else if (ec == asio::error::timed_out)
    {
        std::cout << "[INFO] Session timed out (" << where << ")." << std::endl;
    }
    else if (ec)
    {
        std::cerr << "[ERROR] Session error in " << where << ": " << ec.message() << std::endl;
    }
}