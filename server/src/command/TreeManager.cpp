//
// Created by Andrea Vara on 10/11/2020.
//

#include "TreeManager.h"

std::future<std::map<std::string, std::string>> TreeManager::obtain_tree(const std::string &path) {
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
