#include <iostream>
#include <exception>
#include <stdexcept>
#include "FileWatcher.h"
#include <boost/program_options.hpp>
#include "UserSession.h"
#include "ServerConnectionAsio.h"
#include "Command.h"
#include <iostream>
#include "TreesComparator.h"
#include "FileMetadata.h"
#include "../common/hash_file.h"
#include "../common/file_system_helper.h" // TODO: spostare in common


#define SYNCH_INTERVAL 1000

namespace po = boost::program_options;

void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

void init_file_watcher(FileWatcher &fw) {
    // std::shared_ptr<ServerConnectionAsio> sc = ServerConnectionAsio::get_instance();
    fw.on_file_changed([](std::string path_to_watch, FileStatus status) -> void {
        if (!boost::filesystem::is_regular_file(boost::filesystem::path(path_to_watch)) && status != FileStatus::erased) {
            return;
        }

        switch (status) {
            case FileStatus::created:
                std::cout << "File created: " << path_to_watch << std::endl;
                break;
            case FileStatus::modified:
                std::cout << "File modified: " << path_to_watch << std::endl;
                break;
            case FileStatus::erased:
                std::cout << "File erased: " << path_to_watch << '\n';
                break;
            default:
                std::cout << "Error! Unknown file status.\n";
        }
    });
}

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
        us.dir = std::move(dir);
        us.address = std::move(address);
        us.port = std::move(port);
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

int main(int argc, char** argv) {

    if (argc <= 1) {
        die("A command between sync and <tbd> is required");
    }
    std::string command = argv[1];

    if (command == "sync") {
        UserSession us;
        if (parse_sync_options(argc, argv, us)) return 0;

        FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
        std::cout << "Your username is " << us.username << std::endl;
        std::cout << "init connection" << std::endl;
        ServerConnectionAsio::init(us.address, us.port);
        std::cout << "connection started" << std::endl;

        Command c;

        std::vector<FileMetadata> server_tree = c.require_tree().get();
        std::cout << "GOT RESULT FOR REQUIRE_TREE" << std::endl;
        for (FileMetadata& fm: server_tree) {
            std::cout << fm.path << " ~ " << fm.hash << std::endl;
        }

    //     std::cout << "attempting login" << std::endl;
    //     FileMetadata fm;
    //     fm.path = us.dir + "/test.txt";
    //     if (!check_dest_dir(fm.path)) {
    //         die("Il file " + fm.path + " non esiste");
    //     }
    //     fm.hash = hash_file(fm.path);

    //    auto login1 = c.login(us.username, us.password);
    //    auto post_file1 = c.post_file(fm);
    //    auto post_file2 = c.post_file(fm);
    //    auto login2 = c.login(us.username, us.password);
    //    auto login3 = c.login(us.username, us.password);

    //    bool login_result_1 = login1.get();
    //    bool post_file_result_1 = post_file1.get();
    //    bool login_result_3 = login3.get();
    //    bool post_file_result_2 = post_file2.get();
    //    bool login_result_2 = login2.get();

    //    std::cout << "Login 1 effettuato con " << (login_result_1 ? "successo" : "fallimento") << std::endl;
    //    std::cout << "Login 2 effettuato con " << (login_result_2 ? "successo" : "fallimento") << std::endl;
    //    std::cout << "Login 3 effettuato con " << (login_result_3 ? "successo" : "fallimento") << std::endl;
    //    std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
    //    std::cout << "Post file effettuato con " << (post_file_result_2 ? "successo" : "fallimento") << std::endl;
        // if (login_result) {
        //     std::cout << "Login effettuato con successo" << std::endl;
        //     FileMetadata fm;
        //     fm.path = "./main.cpp";
        //     fm.hash = hash_file(fm.path);
        //     try {
        //         c.post_file(fm).get();
        //         std::cout << "READ DONE!!" << std::endl;
        //     } catch (...) {
        //         std::cout << "An error occurred " << std::endl;
        //     }

        // } else {
        //     std::cout << "Login fallito " << std::endl;
        // }

        init_file_watcher(fw);
        return 0;
    }
// LOGINSNC USERNAME _jD1 PEPPE PASSWORD 003 ABC STOPFLOW
    if (command == "restore") {
        // ...
        return 0;
    }
}


// char* data;
// std::string command;

// unsigned short ch1 = (unsigned short)data[i];
// unsigned short ch2 = (unsigned short)data[i+1];
// unsigned short ch3 = (unsigned short)data[i+2];
// unsigned short ch4 = (unsigned short)data[i+3];

// int size = ch1 * 256^3 + ch2*256^2 + ch3*256 + ch4;

// LOGINSNC USERNAME _jD1 PEPPE PASSWORD 003 ABC STOPFLOW
// LOGINSNC __RESULT 0002 OK STOPFLOW

// REQRTREE
// REQRTREE FILEHASH ABC9 <HASH> FILEPATH 0123 <FULL PATH> FILEHASH A123 <HASH> FILEPATH FILEHASH

// POSTFILE FILEPATH 0123 <FULL PATH> FILEDATA <FULL DATA> FILEHASH A123 <HASH> 
// POSTFILE __RESULT 0002 OK

// REMVFILE FILEPATH 0123 <FULL PATH>
// REMVFILE __RESULT 0002 OK

// - generare l''hash


// Cri:
// - creare un parser
// - realizzare sign-up


// Peppe:


// CLIENT                          SERVER
// ./a.txt  ABC123                 ./a.txt  ABC123
// ./b.txt  DEF234                 ./b.txt  0DE1FF
// ./c.txt  FGH129                 ./d.txt  0D032F


// toRemove = [d.txt];
// toPost = [b.txt, c.txt];


// - analizza le cartelle sul file system locale del client, lo confronta con l'albero ottenuto del server e crea due array:
// 1. un array con i file che vanno eliminati sul server
// 2. un array con i file che vanno aggiunti/modificati sul server

// - dato un percorso crei l'albero delle cartelle necessario



// Andrea:

// - boost asio
// - impl. comandi
