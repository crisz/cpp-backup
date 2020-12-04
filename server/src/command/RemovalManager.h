//
// Created by Andrea Vara on 21/11/20.
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
    std::future<bool> remove_file(std::string path);
};


#endif //CPP_BACKUP_REMOVALMANAGER_H
