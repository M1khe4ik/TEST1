// src/main.cpp

#include <iostream>
#include <unordered_map>
#include <string>
#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

using asio::ip::tcp;
std::unordered_map<std::string, std::string> store;
std::mutex store_mutex;

void handle_client(tcp::socket socket) {
    try {
        asio::streambuf buffer;
        while (asio::read_until(socket, buffer, "\n")) {
            std::istream is(&buffer);
            std::string line;
            std::getline(is, line);

            std::istringstream iss(line);
            std::string command;
            iss >> command;

            if (command == "PUT") {
                std::string key, value;
                iss >> key >> value;
                std::lock_guard<std::mutex> guard(store_mutex);
                auto it = store.find(key);
                std::string response = "OK";
                if (it != store.end()) {
                    response += " " + it->second;
                }
                store[key] = value;
                asio::write(socket, asio::buffer(response + "\n"));
                spdlog::info("PUT {} {}", key, value);
            } else if (command == "GET") {
                std::string key;
                iss >> key;
                std::lock_guard<std::mutex> guard(store_mutex);
                auto it = store.find(key);
                if (it != store.end()) {
                    asio::write(socket, asio::buffer("OK " + it->second + "\n"));
                } else {
                    asio::write(socket, asio::buffer("NE\n"));
                }
                spdlog::info("GET {}", key);
            } else if (command == "DEL") {
                std::string key;
                iss >> key;
                std::lock_guard<std::mutex> guard(store_mutex);
                auto it = store.find(key);
                if (it != store.end()) {
                    asio::write(socket, asio::buffer("OK " + it->second + "\n"));
                    store.erase(it);
                } else {
                    asio::write(socket, asio::buffer("NE\n"));
                }
                spdlog::info("DEL {}", key);
            } else if (command == "COUNT") {
                std::lock_guard<std::mutex> guard(store_mutex);
                asio::write(socket, asio::buffer("OK " + std::to_string(store.size()) + "\n"));
                spdlog::info("COUNT");
            } else if (command == "DUMP") {
                std::string filename;
                iss >> filename;
                std::lock_guard<std::mutex> guard(store_mutex);
                std::ofstream dump_file(filename);
                for (const auto& kv : store) {
                    dump_file << kv.first << " " << kv.second << "\n";
                }
                asio::write(socket, asio::buffer("OK\n"));
                spdlog::info("DUMP {}", filename);
            } else {
                asio::write(socket, asio::buffer("ERROR\n"));
                spdlog::error("Unknown command: {}", command);
            }
        }
    } catch (std::exception& e) {
        spdlog::error("Exception in handle_client: {}", e.what());
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 5) {
            std::cerr << "Usage: kvstore --port <port> --max-clients <number_of_clients>\n";
            return 1;
        }

        int port = std::stoi(argv[2]);
        int max_clients = std::stoi(argv[4]);

        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
        spdlog::set_default_logger(spdlog::basic_logger_mt("basic_logger", "logs/kvstore.log"));

        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

        std::vector<std::thread> threads;
        for (int i = 0; i < max_clients; ++i) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            threads.emplace_back(std::thread(handle_client, std::move(socket)));
        }

        for (auto& th : threads) {
            th.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        spdlog::critical("Exception: {}", e.what());
    }

    return 0;
}
