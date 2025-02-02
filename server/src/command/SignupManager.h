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
    // Funzione asincrona che ritorna un future<bool> in base all'esito della registrazione dell'utente.
    std::future<bool> signup (const std::string& username, const std::string& password);
};


#endif //CPP_BACKUP_SIGNUPMANAGER_H
