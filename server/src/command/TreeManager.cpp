//
// Created by Andrea Vara on 10/11/2020.
//
// Classe che si occupa della generazione del local tree della cartella associata all'utente corrente.
//

#include "TreeManager.h"

// Funzione asincrona che ritorna un future<std::map<std::string, std::string>> contenente
// il local tree (coppie: filname, hash)  della cartella associata all'utente corrente.
std::future<std::map<std::string, std::string>> TreeManager::obtain_tree(const std::string &path) {
    return std::async([path]() {
        std::map<std::string, std::string> local_tree;
        for (auto &file : boost::filesystem::recursive_directory_iterator(path)) {
            std::string file_path = file.path().string();
            std::string file_name = file_path.erase(0, path.size());
            if (file_name.find(".") == 1) continue;
            if (file_name.find("/.") != std::string::npos) continue;
            if (boost::filesystem::is_directory(file.path().string())) continue;
            local_tree[file_name] = hash_file(file.path().string());
        }
        return local_tree;
    });
}
