
#pragma once
#include <string>

class ServerConf {
private:
    ServerConf(ServerConf& source) = delete;
    ServerConf(ServerConf&& source) = delete;
    ServerConf(){}
public:
    int port;
    std::string dest;
    static ServerConf& get_instance() {
        static ServerConf instance;
        return instance;
    }
};