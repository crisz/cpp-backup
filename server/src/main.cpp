
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include "common/file_system_helper.h"
#include "server/src/pool/ConnectionPool.h"
#include <boost/asio/thread_pool.hpp>
#include <thread>
#include <server/src/options/parse_server_options.h>
#include <server/src/pool/ServerThreadPool.h>


int main(int argc, char** argv) {
    if (parse_options(argc, argv)) return 0;

    try {
        ServerThreadPool stp;
        stp.init();
    } catch (std::exception& e) {
        std::cerr << "Si è verificato un errore nel server. " << std::endl;
        std::cerr << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Si è verificato un errore nel server. " << std::endl;
    }

    return 0;
}
