#include "ServerThreadPool.h"

void ServerThreadPool::init() {
    size_t num_threads = std::thread::hardware_concurrency();
    boost::asio::thread_pool pool(num_threads);

    ConnectionPool server{pool};
    pool.join();
}
