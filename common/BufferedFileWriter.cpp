//
// Created by Andrea Vara on 06/12/20.
//
// Classe che gestisce la scrittura del file bufferizzato
//
#include "BufferedFileWriter.h"


BufferedFileWriterException::BufferedFileWriterException(std::string &&message, int code) : message{message}, code{code} {}

const char *BufferedFileWriterException::what() const throw() {
    return ("An exception occurred in BufferedFileWriter: " + message).c_str();
}

// Costruttore per il settaggio di tutti i parametri necessari per la scrittura di un file
BufferedFileWriter::BufferedFileWriter(std::string file_path, long file_size) : file_path{file_path} {

    // Estrapoliamo il path della directory di destinazione
    std::size_t found = file_path.find_last_of("/\\");
    std::string path = file_path.substr(0, found);
    boost::filesystem::path dir(path);

    // Se la cartella di destinazione non esiste la creiamo
    if (!(boost::filesystem::exists(dir))) {
        std::cout << "Doesn't Exists" << std::endl;
        if (boost::filesystem::create_directories(dir)) {
            std::cout << "....Successfully Created !" << std::endl;
        } else {
            throw BufferedFileWriterException("Cannot create file " + file_path, -2);
        }
    }

    // se il file già esiste viene rimosso per essere riscritto
    if (boost::filesystem::exists(file_path)) {
        std::cout<<"File già esistente, lo rimuovo!"<<std::endl;
        if (boost::filesystem::remove(file_path)) {
            std::cout<<"File rimosso con successo!"<<std::endl;
        } else {
            throw BufferedFileWriterException("Cannot delete file " + file_path, -3);
        }
    }

    stream = std::ofstream {file_path, std::ios::out | std::ios::binary | std::ios::app};

    if (stream.fail()) {
        stream.close();
        throw BufferedFileWriterException("File " + file_path + " does not exist", -1);
    }

}

// Funzione asincrona che effettua la scrittura del file bufferizzato
std::future<void> BufferedFileWriter::append(char *buffer, int size) {
    return std::async([this, buffer, size] () {
        stream.write(buffer, size);
    });
}
