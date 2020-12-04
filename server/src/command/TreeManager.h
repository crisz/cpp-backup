//
// Created by Andrea Vara on 10/11/2020.
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
    std::future<std::map<std::string,std::string>> obtain_tree(std::string const& path);

};


#endif //CPP_BACKUP_TREEMANAGER_H
