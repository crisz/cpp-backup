#include "hash_file.h"
#include "md5.h"
#include "BufferedFileReader.h"
#include <stdio.h>

#define BUFFER_SIZE 128

// Funzione che ritorna l'hash del contenuto di un file
std::string hash_file(std::string path) {
    MD5 md5;

    BufferedFileReader bfr{BUFFER_SIZE, path};

    auto cb = [&bfr, &md5] (bool done, char* data, int bytes_read)  {
        std::cout << "adding " << data << " to hash " << std::endl;
        md5.update(data, bytes_read);
        bfr.signal();
    };
    std::promise<bool>& read_done = bfr.register_callback(cb);
    bfr.run();

    read_done.get_future().get();

    md5.finalize();
    return md5.hexdigest();
}
