//
// Created by giuseppe on 04/12/20.
//
// Classe che serve per la registrazione di nuovi utenti.
//

#ifndef CPP_BACKUP_SIGNUPMANAGER_H
#define CPP_BACKUP_SIGNUPMANAGER_H

#include <future>
#include <string>
#include <fstream>
#include "common/md5.h"
#include "common/file_system_helper.h"
#include "server/src/pool/ServerConf.h"
#include <boost/filesystem.hpp>


class SignupManager {
public:
    std::future<bool> signup (std::string username, std::string password);

};


#endif //CPP_BACKUP_SIGNUPMANAGER_H
