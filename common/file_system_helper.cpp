#include <unistd.h>
#include <string>
#include <iostream>
#include <boost/range/algorithm/count.hpp>

// Funzione che verifica l'esistenza di una directory
bool check_dest_dir(const std::string& dir) {
    return access(dir.c_str(), W_OK) == 0;
}

std::string remove_first_folder(const std::string& path) {
    if (boost::count(path, '/') <= 1)
        return path;

    int count = 0;
    size_t index = 0;
    for (;; index++) {
        if (path[index] == '/') count++;
        if (count == 2) break;
    }

    return path.substr(index);
}