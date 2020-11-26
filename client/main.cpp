#include <iostream>
#include <exception>
#include <stdexcept>
#include "FileWatcher.h"
#include <boost/program_options.hpp>
#include "UserSession.h"
#include "ServerConnectionAsio.h"
#include "ClientCommand.h"
#include <iostream>
#include "TreesComparator1.h"
#include "FileMetadata.h"
#include "../common/hash_file.h"
#include "../common/file_system_helper.h" 


#define SYNCH_INTERVAL 1000

namespace po = boost::program_options;

void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

void init_file_watcher(FileWatcher &fw, ClientCommand& c) {

    // std::shared_ptr<ServerConnectionAsio> sc = ServerConnectionAsio::get_instance();
    fw.on_file_changed([&fw, &c](std::string path_matched, FileStatus status) -> void {
        if (!boost::filesystem::is_regular_file(boost::filesystem::path(path_matched)) && status != FileStatus::erased) {
            return;
        }
        FileMetadata fm;
        fm.path = path_matched;
        std::size_t found = fw.path_to_watch.string().find_last_of("/\\");
        fm.path_to_send=fm.path.substr(found);


        if(status==FileStatus::created) {
            std::cout << "File created: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        }else if (status==FileStatus::modified){
            std::cout << "File modified: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        }else if (status==FileStatus::erased) {
            std::cout << "File erased: " << fm.path_to_send << '\n';
            auto remove_file= c.remove_file(fm);
            bool remove_file_result=remove_file.get();
            std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
        }else {
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

        ClientCommand c;
        std::cout << "attempting login" << std::endl;

        auto login1 = c.login(us.username, us.password);

        bool login_result_1 = login1.get();

        if( login_result_1 ){
            auto server_tree = c.require_tree().get();
            for(auto st: server_tree){
                std::cout<<st.path<<std::endl;
            }

            TreesComparator1 tc{us.dir};
            /*
            std::pair<std::shared_ptr<std::vector<FileMetadata>>,std::shared_ptr<std::vector<FileMetadata>>> result_trees_comparator =  tc.compare(server_tree).get();
            auto file_to_remove =  result_trees_comparator.second;
            auto file_to_post = result_trees_comparator.first;
            */

            std::promise<std::shared_ptr<std::vector<FileMetadata>>> p_to_post;
            std::promise<std::shared_ptr<std::vector<FileMetadata>>> p_to_remove;
            auto arr = tc.compare(server_tree);

            std::cout<<"FILES TO REMOVE"<<std::endl;
            auto file_to_post = arr[0];
            auto file_to_remove = arr[1];

            std::vector<std::future<bool>> futures_to_wait;
            
            for (auto fm_rm: *file_to_remove) {
                std::cout<< fm_rm.path<<std::endl;
                fm_rm.path_to_send=fm_rm.path;
                auto remove_file= c.remove_file(fm_rm);
                futures_to_wait.push_back(std::move(remove_file));
            }

            for (int i=0; i<futures_to_wait.size(); i++) {
                bool remove_file_result = futures_to_wait[i].get();
                std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
            }

            std::cout<<"FILES TO POST"<<std::endl;
            futures_to_wait.clear();
            for(auto fm_po: *file_to_post){
                std::cout<< fm_po.path_to_send <<std::endl;
                auto post_file1 = c.post_file(fm_po);
                futures_to_wait.push_back(std::move(post_file1));
            }

            for (int i=0; i<futures_to_wait.size(); i++) {
                bool post_file_result_1 = futures_to_wait[i].get();
                std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
            }

        }
        init_file_watcher(fw, c);
        return 0;
    }
    if (command == "restore") {

        return 0;
    }
    if (command == "signup") { // TODO: implementare
        // crea la cartella dell'utente con solo il file .credentials
        // oppure aggiunge la riga nel file degli utenti

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
