
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <thread>
#include <string>
#include <memory>

void die(std::string message) {
    std::cout << message << std::endl;
    exit(-1);
}

struct UserConnected {
    int fd;
};

class ConnectionPool {
private:
    int sockfd;
    std::vector<UserConnected> usersConnected;
    std::vector<std::function<void(UserConnected& uc)>> new_user_callbacks;
    std::shared_ptr<boost::asio::thread_pool> pool_ptr;
    void invoke_new_user_callbacks(UserConnected& uc) {
        for (auto callback: this->new_user_callbacks) {
            callback(uc);
        }
    }
public:
    void add_new_user_callback(std::function<void(UserConnected&)> uc) {
        new_user_callbacks.push_back(uc);
    }
    ConnectionPool(int port) {
        
        this->pool_ptr = std::make_shared<boost::asio::thread_pool>(std::thread::hardware_concurrency());

        struct sockaddr_in serv_addr;

        // socket
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (!this->sockfd) die("Cannot create socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;  
        serv_addr.sin_addr.s_addr = INADDR_ANY;  
        serv_addr.sin_port = htons(port);

        // bind
        int bindResult = ::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));       
        if (bindResult < 0) die("Cannot bind");

        std::cout << "bound" << std::endl;
        // listen
        ::listen(this->sockfd, 5);
        std::cout << "listening" << std::endl;
        auto pool = this->pool_ptr;
        std::thread t([this, pool]() {
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);
            while (true) {
                std::cout << "waiting connection"<<std::endl;

                int newsockfd = ::accept(this->sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0) die("Cannot bind");
                UserConnected uc;
                std::cout << "uccc\n";

                uc.fd = newsockfd;
                this->usersConnected.push_back(uc);
                this->invoke_new_user_callbacks(uc);
                boost::asio::post(*pool, [this, &uc]() {
                    std::cout << "posting message" << std::endl;
                    while (true) {
                        char buffer[50];
                        buffer[0] = 0;
                        std::cout << "trying to read\n";
                        int bytes_read = recv(uc.fd, buffer, 50, 0);
                        if (bytes_read <= 0) {
                            std::cout << "Cannot read, closing client" << std::endl;
                            close(uc.fd);
                            break;
                        }
                        std::string message = buffer;
                        try {
                            std::cout << "Message received " << message << std::endl;
                        } catch(const std::bad_function_call& e) {
                            std::cout << e.what() << '\n';
                        }
                    }
                });
            }
        });

        t.detach();
    }
};