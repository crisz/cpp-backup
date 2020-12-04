//
// Created by giuseppe on 08/11/20.
//

#ifndef CPP_BACKUP_LOGINMANAGER_H
#define CPP_BACKUP_LOGINMANAGER_H


#include <future>
#include <string>
#include <fstream>
#include "common/md5.h"
#include "common/file_system_helper.h"
#include "server/src/pool/ServerConf.h"


class LoginManager {
public:
    std::future<bool> check_login(std::string username, std::string password);
};



#endif //CPP_BACKUP_LOGINMANAGER_H
