#include "client.h" 
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>


std::atomic<int> connected_clients = 0;
std::atomic<int> successful_logins = 0;
std::atomic<long long> messages_sent = 0;
std::atomic<long long> messages_received = 0;

class TestClient : public Client {
public:
    TestClient(asio::io_context& io_context)
        : Client(io_context), m_timer(io_context) {}

    void onConnect_register() {
        ++connected_clients;
        std::string username = "testuser_" + std::to_string(reinterpret_cast<uintptr_t>(this));

        Envelope register_envelope;
        auto* req = register_envelope.mutable_registration_request();
        req->set_username(username);
        req->set_password("123456");

        send(register_envelope);
    }
    void onConnect_login() {
        ++connected_clients;
        std::string username = "testuser_" + std::to_string(reinterpret_cast<uintptr_t>(this));

        Envelope login_envelope;
        auto* req = login_envelope.mutable_login_request();
        req->set_username(username);
        req->set_password("123456");

        send(login_envelope);
    }
    void start_sending() {

        schedule_send();
    }

protected:
    void handle_server_message(const Envelope& envelope) override {
        Client::handle_server_message(envelope);
        ++messages_received;
        switch(envelope.payload_case()) {
            case chat::Envelope::kRegistrationResponse: {
                const auto& reg_resp = envelope.registration_response();
                if (reg_resp.success()) {
                    std::cout << "register success" << std::endl;
                    onConnect_login();
                }
                break;
            }
            case chat::Envelope::kLoginResponse: {
                const auto& login_resp = envelope.login_response();
                if (login_resp.success()) {
                    std::cout << "logged in success" << std::endl;
                    successful_logins++;
                }
                break;
            }
            default:
                break;
        }
    }
private:

    void schedule_send() {
        std::uniform_int_distribution<int> dist(1000, 5000);
        int random_delay_ms = dist(m_rng);

        m_timer.expires_after(std::chrono::milliseconds(random_delay_ms));

        auto self = std::static_pointer_cast<TestClient>(shared_from_this());
        m_timer.async_wait([this, self](const asio::error_code& ec) {
            if (ec) { return; } 

            Envelope public_envelope;
            std::string content = "Automatic message, time is " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
            public_envelope.mutable_public_message()->set_content(content);

            send(public_envelope);
            messages_sent++;

            schedule_send();
            });
    }

    std::mt19937 m_rng{ std::random_device{}() };
    asio::steady_timer m_timer;
};


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: chat_client <host> <port>\n";
        return 1;
    }

    const std::string host = argv[1];
    const unsigned short port = std::stoi(argv[2]);
    const int num_clients = std::stoi(argv[3]);
    int num_threads = (argc > 4) ? std::stoi(argv[4]) : std::thread::hardware_concurrency();

    std::cout << "Starting stress test with " << num_clients << " clients on "
        << num_threads << " threads...\n";

    asio::io_context io_context;

    std::cout << "Starting stress test with " << num_clients << " clients on "
        << num_threads << " threads...\n";

    std::vector<std::thread> threads;
    auto work_guard = asio::make_work_guard(io_context);
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() { io_context.run(); });
    }

    std::vector<std::shared_ptr<TestClient>> clients;
    for (int i = 0; i < num_clients; ++i) {
        auto client = std::make_shared<TestClient>(io_context);
        clients.push_back(client);

        client->connect(host, port, [client](const asio::error_code& ec) {
            if (!ec) {
                client->onConnect_register();
            }
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    for(const auto& client : clients) {
        chat::Envelope join_envelope;
        join_envelope.mutable_room_operation_request()->set_operation(chat::RoomOperation::JOIN);
        join_envelope.mutable_room_operation_request()->set_room_name("room1");
        client->send(join_envelope);
        client->start_sending();
	}
    std::cout << "All clients initiated. Running test for 60 seconds...\n";
    for (int i = 0; i < 60; ++i) {
        std::cout << "Connected: " << connected_clients
            << ", Logged in: " << successful_logins
            << ", Messages Sent: " << messages_sent << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "--- Test Summary ---\n"
        << "Total Connections: " << connected_clients << "\n"
        << "Total Logins: " << successful_logins << "\n"
        << "Total Messages: " << messages_sent << "\n"
        << "---------------------\n";

    std::cout << "Test finished. Closing all connections...\n";
    for (const auto& client : clients) {
        client->close();
    }

    work_guard.reset();
    for (auto& t : threads) {
        t.join();
    }

    

    return 0;
}