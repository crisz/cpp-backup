//
// Created by Andrea Vara on 10/11/2020.
//

#ifndef CPP_BACKUP_TREEMANAGER_H
#define CPP_BACKUP_TREEMANAGER_H
#include <future>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "../common/hash_file.h"

class TreeManager {
public:
    std::future<std::map<std::string,std::string>> obtain_tree(std::string const& path) {
        return std::async([path]() {
            std::map<std::string, std::string> local_tree;
            for(auto &file : boost::filesystem::recursive_directory_iterator(path)) {
                std::string file_path = file.path().string();
                std::string file_name = file_path.erase(0, path.size());
                if(file_name.find(".")==1) continue;
                if(boost::filesystem::is_directory(file.path().string())) continue;
                local_tree[file_name] = hash_file(file.path().string());
            }
            return local_tree;
        });
    }

};


#endif //CPP_BACKUP_TREEMANAGER_H
