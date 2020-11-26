#include <string>
#include <fstream>
#include <iostream>
#include <future>
#include <boost/filesystem.hpp>

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
    BufferedFileWriter(std::string& file_path, std::string& file_hash, long file_size): file_path{file_path}, file_hash{file_hash}{

        std::size_t found = file_path.find_last_of("/\\");
        std::string path= file_path.substr(0,found);
        boost::filesystem::path dir(path);

        if(!(boost::filesystem::exists(dir))){
            std::cout<<"Doesn't Exists"<<std::endl;
            if (boost::filesystem::create_directories(dir))
                std::cout << "....Successfully Created !" << std::endl;
        }

        if(boost::filesystem::exists(file_path)){
           if(boost::filesystem::remove(file_path)){
               std::cout<<"File già esistente, lo rimuovo!"<<std::endl;
           }else{
               std::cout<<"Error deleting file!"<<std::endl;
           }
        }

        stream = std::ofstream {file_path, std::ios::out | std::ios::binary | std::ios::app};

        if (stream.fail()) {
            stream.close();
            throw BufferedFileWriterException("File " + file_path + " does not exist", -1);
        }

    }

    ~BufferedFileWriter() {
        //Non c'è bisogno di chiudere lo stream in qunato lo fa da solo non appena esce dallo scope
    }

    BufferedFileWriter(BufferedFileWriter& bfm) = delete;
    BufferedFileWriter(BufferedFileWriter&& bfm) = delete;

    std::future<void> append(char* buffer, int size) {

        return std::async([this, buffer, size] () {
            stream.write(buffer, size);
        });
    }

};


