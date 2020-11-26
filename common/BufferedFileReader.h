#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <future>

class BufferedFileReaderException : public std::exception {
private:
    std::string message;
public:
    int code;
    BufferedFileReaderException(std::string&& message, int code=0): message{message}, code{code} {};
	const char* what() const throw(){
    	return ("An exception occurred in BufferedFileReader: " + message).c_str();
    }
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
    BufferedFileReader(int buffer_size, std::string& file_path):
                buffer_size{buffer_size}, file_path{file_path}, stream{file_path, std::ios::in | std::ios::binary} {
        buffer = new char[buffer_size + 1];
        if (stream.fail()) {
            stream.close();
            throw BufferedFileReaderException("File does not exist", -1);
        }
        stream.seekg(0, stream.end);
        file_size = stream.tellg();
        std::cout<<"Dimensione del file: " << file_size << std::endl;
        stream.seekg(0, stream.beg);
    }

    ~BufferedFileReader() { 
        stream.close();
        delete[] buffer;
    }

    BufferedFileReader(BufferedFileReader& bfm) = delete; 
    BufferedFileReader(BufferedFileReader&& bfm) = delete;

    void flush_buffer(bool done, int chars_read) {
        this->buffer[chars_read] = 0;
        this->busy = true;
        std::async([this, done, chars_read] () {
            this->callback(done, this->buffer, chars_read);
        });
    }

    long get_file_size() {
        return this->file_size;
    }

    void signal() {
        this->busy = false;
        cv.notify_all();
    }

    void run() {
        std::unique_lock ul(m);
        stream.seekg(0, stream.beg);

        if (stream.eof())
            flush_buffer(true, 0);

        //if (length > 0) { // altrimenti? @Andrea
        bool done = false;
        do {
            std::cout << "Reading " << buffer_size << " bytes from file system" << std::endl;
            stream.read(buffer, buffer_size);
            std::streamsize bytes_read = stream.gcount();
            done = buffer_size - bytes_read != 0;
            flush_buffer(done, bytes_read);
            cv.wait(ul, [this](){ return !this->busy; });
        } while (!done);
        this->read_done.set_value(true);
    }
    std::promise<bool>& register_callback(std::function<void(bool, char*, int)> fn) {
        this->callback = fn;
        return this->read_done;
    }
};


