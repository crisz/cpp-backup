#pragma once
#include <iostream>
#include <string>
#include "common/file_system_helper.h"
#include "server/src/pool/ConnectionPool.h"
#include <boost/asio/thread_pool.hpp>
#include <thread>
#include <server/src/options/parse_server_options.h>

class ServerThreadPool {
public:
    ServerThreadPool() { }

    // crea il pool di thread e lo utilizza per inizzializzare il ConnectionPool
    void init();
};

