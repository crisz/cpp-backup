#include <string>
#include <fstream>
#include <iostream>
#include <future>

class BufferedFileWriterException : public std::exception {
private:
    std::string message;
public:
    int code;
    BufferedFileWriterException(std::string&& message, int code=0): message{message}, code{code} {};
	const char* what() const throw(){
    	return ("An exception occurred in BufferedFileWriter: " + message).c_str();
    }
};

class BufferedFileWriter {
private:
    std::string file_path;
    std::string file_hash;
    std::ofstream stream;
public:
    BufferedFileWriter(std::string& file_path, std::string& file_hash, long file_size): file_path{file_path}, file_hash{file_hash},
                stream{file_path, std::ios::out | std::ios::binary | std::ios::app} {

        if (stream.fail()) { // TODO: dovrebbe eliminare il file se esiste e crearlo (vuoto) se non esiste
            stream.close();
            throw BufferedFileWriterException("File " + file_path + " does not exist", -1);
        }
    }

    ~BufferedFileWriter() {
        if (stream.fail()) { // TODO: è necessario questo if?
            stream.close();
        }
    }

    BufferedFileWriter(BufferedFileWriter& bfm) = delete; // TODO: copy & swaps
    BufferedFileWriter(BufferedFileWriter&& bfm) = delete;

    std::future<void> append(char* buffer, int size) {
        return std::async([this, buffer, size] () {
            stream.write(buffer, size);
        });
    }

};


