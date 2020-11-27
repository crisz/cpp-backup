#pragma once

#include <string>
struct FileMetadata {
    std::string path;
    int size;
    std::string hash;
    std::string name; // considerare se includerlo nel path
    std::string path_to_send; //foto/albero.jpeg
};