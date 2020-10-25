#include <boost/any.hpp>
#include <map>
#include <condition_variable>
#include <mutex>

#define DEFAULT_BUFFER_MAX_SIZE 4096

class TLV {
private:
    char* buffer;
    size_t buffer_size;

    size_t buffer_max_size;
    std::condition_variable cv;

    std::function<void(char*)> flush;
public:
    TLV() {
        TLV(DEFAULT_BUFFER_MAX_SIZE);
    }
    TLV(size_t buffer_max_size): buffer_max_size{buffer_max_size} {
        this->buffer = new char[this->buffer_max_size];
        this->buffer_size = 0;
    }

    void register_flush(std::function<void(char*)> flush) {
        this->flush = flush;
    }

    void encode_map(std::map<std::string, std::string>) {
        int len = 0;
        if (buffer_size + len > buffer_max_size) {
            this->flush(this->buffer);
        }
    }

    void encode_file(std::string key, std::string file_path) {

    }
};
