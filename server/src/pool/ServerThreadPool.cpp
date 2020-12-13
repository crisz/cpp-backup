#include "ServerThreadPool.h"

// crea il pool di thread e lo utilizza per inizzializzare il ConnectionPool
void ServerThreadPool::init() {
    size_t num_threads = std::thread::hardware_concurrency();
    boost::asio::thread_pool pool(num_threads);

    ConnectionPool server{pool};
    pool.join();
}
