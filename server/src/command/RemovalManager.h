//
// Created by Andrea Vara on 21/11/20.
//
// Classe che si occupa della rimozione dei FILE
//

#ifndef CPP_BACKUP_REMOVALMANAGER_H
#define CPP_BACKUP_REMOVALMANAGER_H

#include <future>
#include <string>
#include <fstream>
#include "server/src/pool/ServerConf.h"
#include <boost/filesystem.hpp>
#include <iostream>
class RemovalManager {
public:
    // Funzione asincrona che ritorna un future<bool> in base all'esito della rimozione del file.
    std::future<bool> remove_file(const std::string& path);
};


#endif //CPP_BACKUP_REMOVALMANAGER_H
