
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include "ServerConf.h"
#include "../common/file_system_helper.h"
#include "ConnectionPool.h"
#include "SingleUserConnection.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>



namespace po = boost::program_options;

int parse_options(int argc, char** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Shows this help menu")
        ("port", po::value<int>()->required(), "Specify the port")
        ("dest", po::value<std::string>()->required(), "The path that will contain files")
    ;
    po::variables_map vm;
    try {
        ServerConf& sc = ServerConf::get_instance();
        auto clp = po::command_line_parser(argc, argv).options(desc);
        po::store(clp.run(), vm);
        po::notify(vm);
        sc.port = std::move(vm["port"].as<int>());
        sc.dest = std::move(vm["dest"].as<std::string>());
        if (!check_dest_dir(sc.dest)) 
            die("La cartella " + sc.dest + " non esiste o non è possibile aprirla in scrittura.");
        return 0;
    } catch (po::error& e) {
        if (!vm.count("help")) {
            std::cout << "Comando non valido " << std::endl;
            std::cout << e.what() << std::endl;
        }
        std::cout << desc << std::endl;
        return -1;
    } catch (std::exception& e) {
        std::cout << "Si è verificato un errore: " << std::endl;
        std::cout << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cout << "uncaught" << std::endl;
        std::cout << *std::current_exception << std::endl;
        return -1;
    }
}

void handle_message(SingleUserConnection& user_connection, const std::string& message) {
    std::cout << "Messaggio ricevuto: " << message << std::endl;
    user_connection.send_response(message + "!!");
}

int main(int argc, char** argv) {
    if (parse_options(argc, argv)) return 0;

    try {
        // boost::asio::io_context io_context;
        boost::asio::thread_pool pool(4); // TODO: spostare la gestione in un'altra classe
        // proposta: MessageDispatcher.h

        ConnectionPool server{pool, [&pool] (std::shared_ptr<SingleUserConnection> user_connection, const std::string& message) { // TODO: rimuovere callback
            boost::asio::post(pool, [user_connection, message]() {
                if (message == "ciao\n") {
                    user_connection->send_response("Ciao a te!\n");
                } else {
                    std::cout << "Sto calcolando la risposta per " << message << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    std::cout << "Risposta calcolata per " << message << std::endl;

                    user_connection->send_response(message + "!!\n");
                }
            });
        }};
        // io_context.run();
        pool.join();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
