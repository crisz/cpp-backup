#include <unistd.h>
#include <string>
#include <iostream>

// Funzione che verifica l'esistenza di una directory
bool check_dest_dir(const std::string& dir) {
    return access(dir.c_str(), W_OK) == 0;
}