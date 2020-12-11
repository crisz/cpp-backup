//
// Created by Andrea Vara on 06/12/20.
//
// Classe che gestisce la lettura del file bufferizzato
//
#include "BufferedFileReader.h"

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
    //this->file_size = stream.gcount();//+1;
    this->file_size = std::filesystem::file_size(file_path);
    std::cout << "Calcolato file size: " << this->file_size << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    stream.clear();   //  Since ignore will have set eof.
    stream.seekg( 0, std::ios_base::beg);
    std::cout<<"Dimensione del file: " << file_size << std::endl;
    //stream.seekg(0, stream.beg);
}

BufferedFileReader::~BufferedFileReader() {
    stream.close();
    delete[] buffer;
}

// Funzione che effettua il flush del buffer (ovvero esegue la funzione che è stata registrata nella callback)
void BufferedFileReader::flush_buffer(bool done, int chars_read) {
    this->buffer[chars_read] = 0;
    this->busy = true;
    // std::thread([this, done, chars_read] () { // TODO: cancellare o decommentare
        this->callback(done, this->buffer, chars_read);
       //  return;
    // }).detach();
}

// Ritorna la dimensione del file da leggere
long BufferedFileReader::get_file_size() {
    return this->file_size;
}

// Segnale al BuffereFileReader che può essere effettuato un'altra flush
void BufferedFileReader::signal() {
    this->busy = false;
    cv.notify_all();
}

// Funzione che apre uno stream legato al file da leggere ed esegue
// la funzione registrata nella callback tramite il flush del buffer fino a quando non abbiamo
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
        std::cout << "Reading " << buffer_size << " bytes from file system" << std::endl;
        // leggiamo il minimo tra buffer size e la dimensione dei dati rimanenti
        stream.read(buffer, buffer_size);
        std::cout << "had read from fs " << std::endl;
        std::streamsize bytes_read = stream.gcount();

        std::cout << "n =  " <<  bytes_read  << std::endl;

        // se la dimensione dei dati letti è minore della dimensione del buffer allora
        // abbiamo finito di leggere, quindi settiamo done=true.
        // done = (buffer_size - bytes_read) != 0;
        bytes_to_read -= bytes_read;


        // se il flush del buffer è in esecuzione allora ci mettiamo in attesa
        cv.wait(ul, [this](){ return !this->busy; });
        this->busy = true;
        std::cout << "DONE IS " << (bytes_to_read <= 0) << std::endl;
        flush_buffer(bytes_to_read <= 0, bytes_read);

        std::cout << "after wake up " << std::endl;
    } while (bytes_to_read > 0);
    std::cout << "client read done" << std::endl;
    this->read_done.set_value(true);
}

// Funzione che permette la registrazione della callback da assocciare al flush del buffer.
std::promise<bool> &BufferedFileReader::register_callback(std::function<void(bool, char *, int)> fn) {
    this->callback = fn;
    return this->read_done;
}
