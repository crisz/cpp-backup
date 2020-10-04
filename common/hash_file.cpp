#include "hash_file.h"
#include "md5.h"
#include <stdio.h>

#define BUFFER_SIZE 128

std::string hash_file(std::string path) {
    MD5 md5;
    FILE *fp = fopen(path.c_str(), "r");
    char buffer[BUFFER_SIZE];
    int length;
    while ((length = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        buffer[length] = 0;
        md5.update(buffer, length);
    }
    md5.finalize();
    return md5.hexdigest();
}
