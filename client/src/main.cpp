#include <iostream>
#include <exception>
#include <stdexcept>
#include "client/src/file/FileWatcher.h"
#include <boost/program_options.hpp>
#include "client/src/server/UserSession.h"
#include "client/src/server/ServerConnectionAsio.h"
#include "client/src/command/ClientCommand.h"
#include <iostream>
#include "client/src/file/TreesComparator1.h"
#include "client/src/file/FileMetadata.h"
#include "common/hash_file.h"
#include "common/file_system_helper.h"
#include "options/options_utils.h"
#include "file/SyncFileWatcher.h"


#define SYNCH_INTERVAL 1000



void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

/*void init_file_watcher(FileWatcher &fw, ClientCommand& c) { // Muovere in un file più opportuno
    // std::shared_ptr<ServerConnectionAsio> sc = ServerConnectionAsio::get_instance();
    fw.on_file_changed([&fw, &c](std::string path_matched, FileStatus status) -> void {
        if (!boost::filesystem::is_regular_file(boost::filesystem::path(path_matched)) && status != FileStatus::erased) {
            return;
        }
        FileMetadata fm;
        fm.path = path_matched;
        std::size_t found = fw.path_to_watch.string().find_last_of("/\\");
        fm.path_to_send=fm.path.substr(found);


        if (status==FileStatus::created) {
            std::cout << "File created: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        } else if (status==FileStatus::modified) {
            std::cout << "File modified: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        } else if (status==FileStatus::erased) {
            std::cout << "File erased: " << fm.path_to_send << '\n';
            auto remove_file= c.remove_file(fm);
            bool remove_file_result=remove_file.get();
            std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
        } else {
            std::cout << "Error! Unknown file status.\n";
        }

    });
}*/



int main(int argc, char** argv) {

    if (argc <= 1) {
        die("A command between sync and <tbd> is required");
    }

    std::cout << "Numero di parametri: " << argc << std::endl;
    std::cout << "1)" << argv[0] << std::endl;
    std::cout << "2)" << argv[1] << std::endl;

    std::string command = argv[1];
    std::cout << "command is ->" << command << "<-" << std::endl;


    std::cout << "cc" << std::endl;


    if (command == "sync") {
        UserSession us;
        if (parse_sync_options(argc, argv, us)) return 0;

        FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
        std::cout << "Your username is " << us.username << std::endl;
        std::cout << "init connection" << std::endl;
        ServerConnectionAsio::init(us.address, us.port);
        std::cout << "connection started" << std::endl;

        std::cout << "attempting login" << std::endl;

        ClientCommand c;
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
        SyncFileWatcher sfw{fw, c};
        sfw.run();
        return 0;
    }
    if (command == "restore") {
        std::string address = "0.0.0.0";
        ServerConnectionAsio::init(address, 3333);
        ClientCommand c;

        std::cout << "restore" << std::endl;
        c.login("cris", "password").get();
        std::cout << "login!" << std::endl;

        FileMetadata fm;
        std::cout << "fm" << std::endl;

        fm.name = "ciccio.txt";
        fm.path = "./ciccio.txt";
        fm.path_to_send = "/ciccio.txt";
        c.require_file(fm).get();
        std::cout << "after get rqrfile" << std::endl;
        //SyncFileWatcher sfw(command c);
        //sfw.run()
        return 0;
    }
    if (command == "signup") { // TODO: implementare
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
