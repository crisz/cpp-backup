// #include <iostream>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <sys/types.h> 
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <vector>
// #include <thread>
// #include <map>
// #include <sstream>
// #include <regex>
// #include <mutex>


// #define HISTORY_LIMIT 5

// struct ClientConnection {
//     std::string nickname;
//     std::time_t touch;
// };

// #define ClientConnection struct ClientConnection

// void die(std::string message) {
//     std::cout << message << std::endl;
//     exit(-1);
// }

// class Server {
// public:
//     Server(int port): port{port} {
//         struct sockaddr_in serv_addr;

//         // socket
//         this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
//         if (!this->sockfd) die("Cannot create socket");
//         bzero((char *) &serv_addr, sizeof(serv_addr));
//         serv_addr.sin_family = AF_INET;  
//         serv_addr.sin_addr.s_addr = INADDR_ANY;  
//         serv_addr.sin_port = htons(port);

//         // bind
//         int bindResult = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));       
//         if (bindResult < 0) die("Cannot bind");

//         // listen
//         listen(this->sockfd, 5);
//         this->checkConnections();
//     }

//     ~Server() {
//         std::cout << "freeing server resources " << std::endl;
//         // int result = close(this->sockfd);
//         shutdown(this->sockfd, SHUT_RDWR);
//         // if (result != 0) die("Cannot close");
//     }

//     std::vector<std::string> getUserList() {
//         std::unique_lock ul(this->mutex);
//         std::vector<std::string> v;
//         for(auto[key, value]: this->clients) {
//             std::cout << "read nickname : " << value->nickname << std::endl;
//             v.push_back(value->nickname);
//         }
//         return v;
//     }

//     int accept() {
//         struct sockaddr_in cli_addr;
//         socklen_t clilen = sizeof(cli_addr);
//         int newsockfd = ::accept(this->sockfd, (struct sockaddr *) &cli_addr, &clilen);
//         if (newsockfd < 0) die("Cannot bind");
//         return newsockfd;
//     }

//     void send(const std::string& nickname, const std::string& message) {
//         std::unique_lock ul(this->mutex);
//         for (auto[key, value]: this->clients) {
//             if (value->nickname == nickname) {
//                 this->send(key, message);
//                 break;
//             }
//         }
//     }
//     void send(int fd, const std::string& message) {
//         int send_result = ::send(fd, message.c_str(), message.size()+1, 0);
//         if (send_result < 0) die("Cannot send"); 
//     }

//     void sendAll(int fd, const std::string& message) {
//         std::string myname = this->clients[fd]->nickname;
//         if (this->history.size() >= HISTORY_LIMIT) {
//             this->history.erase(this->history.begin());
//         }
//         this->history.push_back(myname+ ": " +message);

//         for (auto[k, value]: this->clients) {
//             if (k == fd) continue;

//             this->send(k, myname+ ": " +message);
//         }
//     }

//     void checkConnections() {
//         std::thread idle_thread([this]() {
//             while (true) {
//                 sleep(1);
//                 std::vector<int> fd_to_kickout;
//                 {
//                     std::unique_lock ul(this->mutex);
//                     std::time_t now = std::time(nullptr);
//                     for (auto[fd, conn]: this->clients) {
//                         if (now - conn->touch >= 60) {
//                             std::cout << "Kicking out user " << conn->nickname << std::endl;
//                             fd_to_kickout.push_back(fd);
//                         }
//                     }
//                 }

//                 for (int fd: fd_to_kickout) {
//                     this->quit(fd);
//                 }
//             }
//         });

//         idle_thread.detach();
//     }

//     void registerClient(int fd, const std::string nickname) {
//         std::unique_lock ul(this->mutex);
//         ClientConnection* cc = new ClientConnection();
//         cc->nickname = nickname;
//         cc->touch = std::time(nullptr);
//         this->clients[fd] = cc;
//     }

//     void subscribeClient(int fd, std::function<void(std::string, int fd)> fn) {
//         std::thread t([fd, &fn]() {
//             while (true) {
//                 char buffer[50];
//                 buffer[0] = 0;
//                 int bytes_read = recv(fd, buffer, 50, 0);
//                 if (bytes_read <= 0) {
//                     std::cout << "Cannot read, closing client" << std::endl;
//                     close(fd);
//                     break;
//                 }
//                 std::string message = buffer;
//                 try {
//                     fn(message, fd);
//                 } catch(const std::bad_function_call& e) {
//                     std::cout << e.what() << '\n';
//                 }
                
//             }
//         });

//         t.detach();
//     }

//     std::vector<std::string> getHistory() {
//         return this->history;
//     }

//     bool containsFd(int fd) {
//         std::unique_lock ul(this->mutex);
//         return this->clients.count(fd) == 1;
//     }

//     void quit(int fd) {
//         std::unique_lock ul(this->mutex);
//         this->clients.erase(this->clients.find(fd));
//         this->send(fd, "Exiting...");
//         shutdown(fd, SHUT_RDWR);
//     }

//     void touch(int fd) {
//         this->clients[fd]->touch = std::time(nullptr);
//     }

// private:
//     int sockfd;
//     int port;
//     std::map<int, ClientConnection*> clients;
//     std::mutex mutex;
//     std::vector<std::string> history;

// };

// int main(int argc, char** argv) {


//     std::mutex history_mutex;
//     std::unique_lock history_lock(history_mutex);
//     history_lock.unlock();

//     if (argc < 2) {
//         std::cout << "Use: /server <port>" << std::endl;
//     }
    
//     int port = atoi(argv[1]);

//     Server s(port);
//     std::cout << "Server created " << std::endl;

//     while (true) {
//         int fd = s.accept();

//         s.subscribeClient(fd, [&s, &history_lock](std::string message, int fd) {
//             if (message == "") return;
//             if (!s.containsFd(fd)) {
//                 s.registerClient(fd, message);
//                 std::ostringstream oss;
//                 oss << "Welcome " << message << std::endl;
//                 oss << "The users connected are: " << std::endl;
//                 for (std::string user: s.getUserList()) {
//                     oss << user << std::endl;
//                 }
//                 oss << "The last " << std::to_string(HISTORY_LIMIT) << " messages are:" << std::endl;

//                 history_lock.lock();
//                 for (std::string message: s.getHistory()) {
//                     oss << message << std::endl;
//                 }
//                 history_lock.unlock();
//                 s.send(fd, oss.str());
                
//             } else {
//                 s.touch(fd);

//                 std::regex rgx("\\/([a-zA-Z]+)\\s?([a-zA-Z]*)");
//                 std::smatch matches;

//                 if (std::regex_search(message, matches, rgx)) {
//                     if (matches[1] == "private") {
//                         s.send(matches[2], message);
//                     } else if (matches[1] == "quit") {
//                         history_lock.lock();
//                         s.sendAll(fd, "*exit*");
//                         history_lock.unlock();
//                         s.quit(fd);
//                     } else {
//                         s.send(fd, "Command not found");
//                     }
//                     return;
//                 }
//                 history_lock.lock();
//                 s.sendAll(fd, message);
//                 history_lock.unlock();
//             }
//         });

//     }
    
//     return 0;
// }





//         // std::thread t([fd](){
//         //     char buffer[50];
//         //     buffer[0] = 0;
//         //     int bytes_read, bytes_sent;
//         //     while (strcmp(buffer, "exit\r\n") != 0) {
//         //         bytes_read = recv(fd, buffer, 50, 0);
//         //         if (bytes_read < 0) {
//         //             close(fd);
//         //             die("Cannot read");
//         //         }
//         //         buffer[bytes_read] = 0;
//         //         std::cout << "Received: " << buffer;

//         //         bytes_sent = send(fd, buffer, bytes_read, 0);
//         //         if (bytes_sent < 0) {
//         //             close(fd);
//         //             die("Cannot send");
//         //         }
//         //     }
//         //     close(fd);
//         // });
//         // t.detach();



// // 1. asio file system per il client per ottenere un software che faccia il guard delle cartelle e notifichi se un file è stato rimosso, modificato o aggiunto
// // 2. inviare il file nel server e creare un server che, ricevendo un file, lo copi nel file system del server
// // 3. gestire l'autenticazione analizzando vari metodi e creare un server che crei l'area utente di un'utente che non esiste e gestisca il login di utenti che esistono

// // definire il protocollo
// // socket
// // socket.send(file.bytecode)


// // new file(quello che ho ricevuto);
// // file.save()

// // /login cris password

// // cris

// // echo-backup sync ./dir --username cris --password pass

// // dsadsads