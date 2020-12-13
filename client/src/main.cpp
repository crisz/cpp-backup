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
#include "mode/sync.h"
#include "mode/restore.h"
#include "mode/signup.h"

void die(std::string message) {
    std::cerr << message << std::endl;
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
        die("Please, choose a command among sync, restore and signup.\n"
            "Example: \n " + std::string(argv[0]) + " sync --help"
        );
    }

    std::ostringstream oss;
    oss << "   ___ ___ ___     ___   _   ___ _  ___   _ ___ \n"
           "  / __| _ \\ _ \\___| _ ) /_\\ / __| |/ / | | | _ \\\n"
           " | (__|  _/  _/___| _ \\/ _ \\ (__| ' <| |_| |  _/\n"
           "  \\___|_| |_|     |___/_/ \\_\\___|_|\\_\\\\___/|_|  \n"
           "  ®                                     v 1.0.0 stable \n\n\n";

    std::cout << oss.str();
    // Il timeout serve per apprezzare l'ascii art
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::string mode = argv[1];

    try {
        if (mode == "sync") {
            UserSession us;
            if (parse_sync_options(argc, argv, us)) return 0;

            init_connection(us);
            sync(us);

            return 0;
        }
        if (mode == "restore") {
            UserSession us;
            if (parse_restore_options(argc, argv, us)) return 0;

            init_connection(us);
            restore(us);

            return 0;
        }
        if (mode == "signup") {
            UserSession us;
            if (parse_signup_options(argc, argv, us)) return 0;

            init_connection(us);
            signup(us);

            return 0;
        }

    } catch (ServerConnectionAsioException& exc) {
        std::cout << "La connessione con il server è stata interrotta." << std::endl;
        std::cout << "Motivo: " << exc.what() << std::endl;
        return -1;
    }

    std::cout << "The command " << mode << " is not valid " << std::endl;
}

