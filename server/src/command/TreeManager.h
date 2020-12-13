//
// Created by Andrea Vara on 10/11/2020.
//
// Classe che si occupa della generazione del local tree della cartella associata all'utente corrente.
//

#ifndef CPP_BACKUP_TREEMANAGER_H
#define CPP_BACKUP_TREEMANAGER_H
#include <future>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "common/hash_file.h"

class TreeManager {
public:
    // Funzione asincrona che ritorna un future<std::map<std::string, std::string>> contenente
    // il local tree (coppie: filname, hash)  della cartella associata all'utente corrente.
    std::future<std::map<std::string,std::string>> obtain_tree(const std::string& path);

};


#endif //CPP_BACKUP_TREEMANAGER_H
