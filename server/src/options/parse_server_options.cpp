//
// Created by Cristian Traina on 10/12/2020.
//

#include "parse_server_options.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include "server/src/pool/ServerConf.h"
#include "common/file_system_helper.h"
#include "server/src/pool/ConnectionPool.h"
#include "server/src/user/UserSocket.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

namespace po = boost::program_options;


int parse_options(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "Shows this help menu")
            ("port", po::value<int>()->required(), "Specify the port")
            ("dest", po::value<std::string>()->required(), "The path that will contain files")
            ;
    po::variables_map vm;
    try {
        //Di seguito si ottiene l'istanza di ServerConf e si settano la porta e la cartella dest
        //tramite i valori passati come argomento al main
        ServerConf& sc = ServerConf::get_instance();
        auto clp = po::command_line_parser(argc, argv).options(desc);
        po::store(clp.run(), vm);
        po::notify(vm);
        sc.port = std::move(vm["port"].as<int>());
        sc.dest = std::move(vm["dest"].as<std::string>());

        if (sc.dest.back() != '/') {
            sc.dest = sc.dest + '/';
        }

        //Se la cartella specificata non esiste o non è possibile aprirla in scrittura, il server temina.
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
