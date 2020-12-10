#include <iostream>
#include <exception>
#include <stdexcept>
#include "client/src/file/FileWatcher.h"
#include <boost/program_options.hpp>
#include "client/src/server/UserSession.h"
#include "client/src/server/ServerConnectionAsio.h"
#include "client/src/command/ClientCommand.h"
#include <iostream>
#include "client/src/file/TreesComparator.h"
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

void init_connection(UserSession& us) {
    try {
        ServerConnectionAsio::init(us.address, us.port);
    } catch (boost::system::system_error& error) {
        std::cout << "Impossibile effettuare la connessione al server " << std::endl;
        die(error.what());
    } catch (...) {
        die("Impossibile effettuare la connessione al server ");
    }
}


int main(int argc, char** argv) {

    if (argc <= 1) {
        die("Please, choose a command among sync, restore and signup."
            "Example: ./client sync"
        );
    }

    std::ostringstream oss;
    oss << "   ___ ___ ___     ___   _   ___ _  ___   _ ___ \n"
           "  / __| _ \\ _ \\___| _ ) /_\\ / __| |/ / | | | _ \\\n"
           " | (__|  _/  _/___| _ \\/ _ \\ (__| ' <| |_| |  _/\n"
           "  \\___|_| |_|     |___/_/ \\_\\___|_|\\_\\\\___/|_|  \n"
           "                                                ";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << oss.str() << std::endl;

    std::string command = argv[1];
    if (command == "sync") {
        UserSession us;
        if (parse_sync_options(argc, argv, us)) return 0;
        init_connection(us);

        FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
        ClientCommand c;
        auto login = c.login(us.username, us.password);

        bool login_result = login.get();

        if (!login_result) die("Invalid credentials");

        auto server_tree = c.require_tree().get();

        for (auto st: server_tree) {
            std::cout<<st.path<<std::endl;
        }

        TreesComparator tc{us.dir};
        auto arr = tc.compare(server_tree);

        auto new_files = arr[0];
        auto changed_files = arr[1];
        auto removed_files = arr[2];

        std::vector<FileMetadata> files_to_remove = *removed_files;
        std::vector<FileMetadata> files_to_post = *new_files;
        files_to_post.insert(
           (files_to_post).end(),
            std::make_move_iterator((*changed_files).begin()),
            std::make_move_iterator((*changed_files).end())
         );

        std::cout<<"FILES TO POST"<<std::endl;
        for (auto file: files_to_post) {
            std::cout << file.path << std::endl;
        }

        std::cout<<"FILES TO REMOVE"<<std::endl;
        for (auto file: files_to_remove) {
            std::cout << file.path << std::endl;
        }



        std::vector<std::future<bool>> futures_to_wait;

        for (auto fm_rm: files_to_remove) {
            std::cout<< fm_rm.path<<std::endl;
            fm_rm.path_to_send=fm_rm.path;
            auto remove_file= c.remove_file(fm_rm);
            futures_to_wait.push_back(std::move(remove_file));
        }

        for (int i=0; i<futures_to_wait.size(); i++) {
            bool remove_file_result = futures_to_wait[i].get();
            std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
        }

        futures_to_wait.clear();
        for(auto fm_po: files_to_post){
            std::cout<< fm_po.path_to_send <<std::endl;
            auto post_file1 = c.post_file(fm_po);
            futures_to_wait.push_back(std::move(post_file1));
        }

        for (int i=0; i<futures_to_wait.size(); i++) {
            bool post_file_result_1 = futures_to_wait[i].get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        }

        SyncFileWatcher sfw{fw, c};
        sfw.run();
        return 0;
    }
    if (command == "restore") {
        UserSession us;
        if (parse_restore_options(argc, argv, us)) return 0;
        init_connection(us);
        ClientCommand c;

        auto login = c.login(us.username, us.password);

        bool login_result = login.get();

        if (!login_result) die("Invalid credentials");

        auto server_tree = c.require_tree().get();

        for (auto st: server_tree) {
            std::cout << st.path << std::endl;
        }

        TreesComparator tc{us.dir};
        auto arr = tc.compare(server_tree);

        auto new_files = arr[0];
        auto changed_files = arr[1];
        auto removed_files = arr[2];

        std::vector<FileMetadata> files_to_require = *removed_files;
        std::vector<FileMetadata> files_to_remove = *new_files;

        files_to_require.insert(
                (files_to_require).end(),
                std::make_move_iterator((*changed_files).begin()),
                std::make_move_iterator((*changed_files).end())
        );


        std::cout << "REQRFILE FOR: " << std::endl;
        for (auto file: files_to_require) {
            std::cout << file.path_to_send << std::endl;
        }

        std::cout << "delete FOR: " << std::endl;
        for (auto file: files_to_remove) {
            std::cout << file.path_to_send << std::endl;
        }

        for (auto file: files_to_remove) {
            std::cout << "removing " << file.path << std::endl;
            if (boost::filesystem::exists(file.path)) {
                boost::filesystem::remove_all(file.path);
            }
        }

        std::vector<std::future<void>> futures_to_wait;
        for (auto file: files_to_require) {

//            boost::filesystem::path final_path = us.dir;
//            final_path = final_path / "..";
//            final_path = final_path / file.path_to_send;
//
//            file.path = final_path.c_str();

            int count = 0;
            size_t index = 0;
            for (;; index++) {
                if (file.path_to_send[index] == '/') count++;
                if (count == 2) break;
            }
            file.path = us.dir + file.path_to_send.substr(index);

            std::cout << "file.path = " << file.path << std::endl;
            std::cout << "file.path_to_send = " << file.path_to_send << std::endl;

            futures_to_wait.push_back(c.require_file(file));
            //c.require_file(file).get();
        }

        for (auto& future: futures_to_wait) {
            future.get();
        }

        std::cout << "Restore completato con successo " << std::endl;

        return 0;
    }

    if (command == "signup") {
        UserSession us;
        if (parse_signup_options(argc, argv, us)) return 0;
        init_connection(us);

        ClientCommand c;
        auto signup = c.signup(us.username, us.password);
        bool signup_result = signup.get();

        if (!signup_result) die("Signup failed!");

        std::cout<<"La registrazione è andata a buon fine! Benvenuto!\n"
                   "Effettua l'accesso per accedere al servizio."<< std::endl;

        return 0;
    }

    std::cout << "The command " << command << " is not valid " << std::endl;
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

// SIGNUPNU USERNAME _jD1 PEPPE PASSWORD 003 ABC STOPFLOW
// SIGNUPNU __RESULT 0002 OK STOPFLOW

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

// in caso di sync
// toRemove = [d.txt];
// toPost = [b.txt, c.txt];

// in caso di restore
// toRemove = [c.txt];
// toRequire = [b.txt, d.txt]



// - analizza le cartelle sul file system locale del client, lo confronta con l'albero ottenuto del server e crea due array:
// 1. un array con i file che vanno eliminati sul server
// 2. un array con i file che vanno aggiunti/modificati sul server

// - dato un percorso crei l'albero delle cartelle necessario



// Andrea:

// - boost asio
// - impl. comandi

// ../client/cris_dir2/cris_dir/..
//
// /cris_dir/banana.txt

// ../client/cris_dir2/../cris_dir/banana.txt


// us.dir: ../client/cris_dir
// file.path_to_send: /cris_dir/

// file.path:
#pragma clang diagnostic pop


