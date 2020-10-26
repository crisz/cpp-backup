#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <netdb.h>
#include <iostream>
class ServerConnection {
private:
    int sockfd;
public:
    ServerConnection(std::string& address, int port) {
        struct sockaddr_in serv_addr;
        hostent* sever_host = gethostbyname(address.c_str());
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (!this->sockfd) {
            std::cout << ("Cannot create socket") << std::endl;
            exit(-1);
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;

        bcopy((char *)sever_host->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr,
            sever_host->h_length);
        
        serv_addr.sin_port = htons(port);
        int connect_result = connect(this->sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if (connect_result < 0) {
            std::cout << ("Cannot connect to server") << std::endl;
            exit(-1);
        }
    }

    void send(const std::string& m) {
        const char* message = (m).c_str();
        int bytes_sent = ::send(this->sockfd, message, m.size(), 0);
        
        if (bytes_sent < 0) {
            close(this->sockfd);
            std::cout << ("Cannot send") << std::endl;
            exit(-1);
        }
    }

    void send(char* message, int size) {
        // const char* message = (m).c_str();
        int bytes_sent = ::send(this->sockfd, message, size, 0);
        
        if (bytes_sent < 0) {
            close(this->sockfd);
            std::cout << ("Cannot send") << std::endl;
            exit(-1);
        }
    }
};