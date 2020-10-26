#include <string>
#include <map>

class Command {
private:
    std::string command_name;
    std::map<std::string, std::string> parameters;
public:
    setName(std::string name);
    addParameter(std::string par_name, std::string par_value);
}