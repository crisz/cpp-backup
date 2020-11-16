#include <unistd.h>
#include <string>
#include <iostream>

bool check_dest_dir(const std::string& dir) {
    return access(dir.c_str(), W_OK) == 0;
}