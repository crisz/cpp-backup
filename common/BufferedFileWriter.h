//
// Classe che gestisce la scrittura del file bufferizzato
//
#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <future>
#include <boost/filesystem.hpp>

// Classe User defined che estende std::exception per la gestione degli errori in fase di scrittura
class BufferedFileWriterException : public std::exception {
private:
    std::string message;
public:
    int code;
    BufferedFileWriterException(std::string&& message, int code=0);

	const char* what() const throw();
};

class BufferedFileWriter {
private:
    std::string file_path;
    std::ofstream stream;
public:
    BufferedFileWriter(std::string file_path, long file_size);

    BufferedFileWriter(BufferedFileWriter& bfm) = delete;

    BufferedFileWriter(BufferedFileWriter&& bfm) = delete;

    std::future<void> append(char* buffer, int size);

    void close();

};


