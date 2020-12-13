//
// Created by giuseppe on 08/11/20.
//
// Classe che si occupa della gestione del login
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
    // Funzione asincrona che ritorna un future<bool> in base all'esito del login.
    std::future<bool> check_login(const std::string& username, const std::string& password);
};



#endif //CPP_BACKUP_LOGINMANAGER_H
