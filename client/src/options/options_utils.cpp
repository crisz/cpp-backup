//
// Created by Andrea Vara on 27/11/20.
//

#include "options_utils.h"

// Funzione che si occupa della stampa a video e della successiva gestione delle opzioni inserite dall'utente
// nel caso in cui il comando inserito sia "sync"
int parse_sync_options(int argc, char** argv, UserSession& us) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "Shows this help menu")
            ("dir", po::value<std::string>()->required(), "The directory you want to sync")
            ("username", po::value<std::string>()->required(), "The username you want to login with")
            ("password", po::value<std::string>()->required(), "The password you want to login with")
            ("address", po::value<std::string>()->default_value("0.0.0.0"), "Address of the server")
            ("port", po::value<int>()->default_value(3333), "Port of the server")
            ;
    po::variables_map vm;
    try {
        auto clp = po::command_line_parser(argc, argv).options(desc);
        po::store(clp.run(), vm);
        po::notify(vm);
        std::string username = vm["username"].as<std::string>();
        std::string password = vm["password"].as<std::string>();
        std::string dir = vm["dir"].as<std::string>();
        std::string address = vm["address"].as<std::string>();
        int port = vm["port"].as<int>();
        us.username = std::move(username);
        us.password = std::move(password);
        if (dir.substr(0,1)!="." && dir.substr(0,1)!="/") {
            std::cout << "ERRORE: Il percorso della cartella è ambiguo. "
                         "Specifica un percorso assoluto o relativo!" << std::endl;
            return -1;
        }
        if(!boost::filesystem::exists(dir)) {
            std::cout << "ERRORE: La cartella specificata non esiste!" << std::endl;
            return -1;
        }
        us.dir = std::move(dir);
        us.address = std::move(address);
        us.port = port;
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
        std::cout << "Si è verificato un errore." << std::endl;
        return -1;
    }
}

// Funzione che si occupa della stampa a video e della successiva gestione delle opzioni inserite dall'utente
// nel caso in cui il comando inserito sia "restore"
int parse_restore_options(int argc, char** argv, UserSession& us) {
    return parse_sync_options(argc, argv, us);
}

// Funzione che si occupa della stampa a video e della successiva gestione delle opzioni inserite dall'utente
// nel caso in cui il comando inserito sia "signup"
int parse_signup_options(int argc, char** argv, UserSession& us) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "Shows this help menu")
            ("username", po::value<std::string>()->required(), "The username you want to login with")
            ("password", po::value<std::string>()->required(), "The password you want to login with")
            ("address", po::value<std::string>()->default_value("0.0.0.0"), "Address of the server")
            ("port", po::value<int>()->default_value(3333), "Port of the server")
            ;
    po::variables_map vm;
    try {
        auto clp = po::command_line_parser(argc, argv).options(desc);
        po::store(clp.run(), vm);
        po::notify(vm);
        std::string username = vm["username"].as<std::string>();
        std::string password = vm["password"].as<std::string>();
        std::string address = vm["address"].as<std::string>();
        int port = vm["port"].as<int>();
        us.username = std::move(username);
        us.password = std::move(password);
        us.address = std::move(address);
        us.port = port;
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
        std::cout << "Si è verificato un errore." << std::endl;
        return -1;
    }
}