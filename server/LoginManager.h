#include <future>
#include <string>
class LoginManager {
public:
    std::future<bool> check_login(std::string username, std::string password) {
        return std::async([]() {
            return true;
        });
    }
};