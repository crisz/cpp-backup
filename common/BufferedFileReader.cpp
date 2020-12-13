#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
//
// Created by Andrea Vara on 06/12/20.
//
// Classe che gestisce la lettura del file bufferizzato
//
#include "BufferedFileReader.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-count-overflow"
BufferedFileReaderException::BufferedFileReaderException(std::string &&message, int code) : message{message}, code{code} {}

const char *BufferedFileReaderException::what() const throw() {
    return ("An exception occurred in BufferedFileReader: " + message).c_str();
}

// Costruttore per il settaggio di tutti i parametri necessari per la lettura di un file
BufferedFileReader::BufferedFileReader(int buffer_size, std::string &file_path) :
        buffer_size{buffer_size}, file_path{file_path}, stream{file_path, std::ios::in | std::ios::binary} {
    buffer = new char[buffer_size + 1];
    if (stream.fail()) {
        stream.close();
        throw BufferedFileReaderException("File " + file_path + " does not exist", -1);
    }
    stream.clear();   //  Since ignore will have set eof.
    stream.seekg( 0, std::ios_base::beg);
    stream.ignore( std::numeric_limits<std::streamsize>::max() );
    this->file_size =  stream.gcount();

    // Supportiamo solo file minori di 4GB = 2^32 byte
    long long _4gb = 0xFFFFFFFF;
    if (this->file_size > _4gb) {
        stream.close();
        throw BufferedFileReaderException("File " + file_path + " is bigger than 4GB", -1);
    }

    stream.clear();   //  Since ignore will have set eof.
    stream.seekg( 0, std::ios_base::beg);
}

BufferedFileReader::~BufferedFileReader() {
    stream.close();
    delete[] buffer;
}

// Funzione che effettua il flush del buffer (ovvero esegue la funzione che è stata registrata nella callback)
void BufferedFileReader::flush_buffer(bool done, int chars_read) {
    this->buffer[chars_read] = 0;
    this->busy = true;
    this->callback(done, this->buffer, chars_read);
}

// Ritorna la dimensione del file da leggere
long BufferedFileReader::get_file_size() {
    return this->file_size;
}

// Segnala al BufferFileReader che può essere effettuato un'altro flush
void BufferedFileReader::signal() {
    this->busy = false;
    cv.notify_all();
}

// Funzione che apre uno stream legato al file da leggere ed esegue
// la funzione registrata nella callback tramite il flush del buffer, fino a quando non abbiamo
// raggiunto la fine del file
void BufferedFileReader::run() {
    std::unique_lock ul(m);
    stream.seekg(0, stream.beg);

    if (stream.eof())
        flush_buffer(true, 0);

    bool done = false;
    this->busy = false;
    int bytes_to_read = this->file_size;//-1;
    do {
        // leggiamo il minimo tra buffer size e la dimensione dei dati rimanenti
        stream.read(buffer, buffer_size);
        std::streamsize bytes_read = stream.gcount();


        // se la dimensione dei dati letti è minore della dimensione del buffer allora
        // abbiamo finito di leggere, quindi settiamo done=true.
        // done = (buffer_size - bytes_read) != 0;
        bytes_to_read -= bytes_read;


        // se il flush del buffer è in esecuzione allora ci mettiamo in attesa
        cv.wait(ul, [this](){ return !this->busy; });
        this->busy = true;
        flush_buffer(bytes_to_read <= 0, bytes_read);

    } while (bytes_to_read > 0);
    this->read_done.set_value(true);
}

// Funzione che permette la registrazione della callback da assocciare al flush del buffer.
std::promise<bool> &BufferedFileReader::register_callback(std::function<void(bool, char *, int)> fn) {
    this->callback = fn;
    return this->read_done;
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop