//
// Created by giuseppe on 08/11/20.
//

#ifndef CPP_BACKUP_LOGINMANAGER_H
#define CPP_BACKUP_LOGINMANAGER_H


#include <future>
#include <string>
class LoginManager {
public:
    std::future<bool> check_login(std::string username, std::string password) {
        return std::async([]() {
            return false;
        });
    }
};



#endif //CPP_BACKUP_LOGINMANAGER_H
