#include "util/ConfigManager.h"
#include "data/ConnectionPool.h"
#include "core/Server.h"
#include <iostream>

int main() {
    if (!ConfigManager::getInstance().load("config.example.json")) {
        return 1;
    }
    try {
        const auto& config = ConfigManager::getInstance().getConfig();
        const auto& db_config = config.at("database");
        std::string conn_str = 
        "db=" + db_config.at("dbname").get<std::string>() + " " +
        "user=" + db_config.at("user").get<std::string>() + " " +
        "password=" + db_config.at("password").get<std::string>() + " " +
        "host=" + db_config.at("host").get<std::string>() + " " +
        "port=" + std::to_string(db_config.at("port").get<int>());
        
        asio::io_context io_context;
		ConnectionPool::initInstance(io_context);
        ConnectionPool::getInstance().init(conn_str, 10);
        auto work_guard = asio::make_work_guard(io_context.get_executor());
        unsigned short port = config.at("server").at("port").get<unsigned short>();
        Server server(io_context, port);
        server.run();
        unsigned int thread_count = config.at("server").value("threads", 0);
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&io_context]() {
                try {
                    io_context.run();
                } catch (const std::exception& e) {
                    std::cerr << "Thread exception: " << e.what() << std::endl;
                }
            });
        }

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](const asio::error_code&, int) {
            std::cout << "\n[INFO] Shutdown signal received. Stopping server..." << std::endl;
            work_guard.reset();
        });

        io_context.run();
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    } catch (const json::exception& e) {
        std::cerr << "Config Access Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "[INFO] Server has shut down gracefully." << std::endl;
    return 0;
}