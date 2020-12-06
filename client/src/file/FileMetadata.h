//
// Struttura tutte le informazioni utili alla gestione dei file
//

#pragma once

#include <string>
struct FileMetadata {
    std::string path;
    int size;
    std::string hash;
    std::string name;
    std::string path_to_send;
};