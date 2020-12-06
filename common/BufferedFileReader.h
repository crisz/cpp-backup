//
// Classe che gestisce la lettura del file bufferizzato
//
#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <future>

// Classe User defined che estende std::exception per la gestione degli errori in fase di lettura
class BufferedFileReaderException : public std::exception {
private:
    std::string message;
public:
    int code;
    BufferedFileReaderException(std::string&& message, int code=0);

	const char* what() const throw();
};

class BufferedFileReader {
private:
    int buffer_size;
    char* buffer;
    std::string file_path;
    std::ifstream stream;
    std::function<void(bool, char*, int)> callback;
    std::mutex m;
    std::condition_variable cv;
    long file_size = -1;
    bool busy = false;
    std::promise<bool> read_done;
public:
    BufferedFileReader(int buffer_size, std::string& file_path);

    ~BufferedFileReader();

    BufferedFileReader(BufferedFileReader& bfm) = delete; 
    BufferedFileReader(BufferedFileReader&& bfm) = delete;

    void flush_buffer(bool done, int chars_read);

    long get_file_size();

    void signal();

    void run();
    std::promise<bool>& register_callback(std::function<void(bool, char*, int)> fn);
};


