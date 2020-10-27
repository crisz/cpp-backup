#include <string>
#include <map>

class Command {
private:
    std::string command_name;
    std::map<std::string, std::string> parameters;
public:
    void setName(std::string name){
        command_name=name;
    }
    void addParameter(std::string par_name, std::string par_value){
        parameters[par_name]=par_value;
    }
    std::string getCommand_name(){
        return command_name;
    }
    std::map<std::string, std::string> getParameters(){
        return parameters;
    }
    void clear (){
        command_name="";
        parameters.clear();
    }
// LOGINSNC USERNAME _jD1 PEPPE PASSWORD 003 ABC STOPFLOW
// LOGINSNC __RESULT 0002 OK END

// REQRTREE
// REQRTREE FILEHASH ABC9 <HASH> FILEPATH 0123 <FULL PATH> FILEHASH A123 <HASH> FILEPATH FILEHASH

// POSTFILE FILEPATH 0123 <FULL PATH> FILEDATA <FULL DATA> FILEHASH A123 <HASH>
// POSTFILE __RESULT 0002 OK

// REMVFILE FILEPATH 0123 <FULL PATH>
// REMVFILE __RESULT 0002 OK

    void commandParser(Command cmd, bool server) {
        std::string commandName = cmd.getCommand_name();
        auto a=cmd.getParameters();
            if(commandName.compare("LOGINSNC")==0){
                if(server)
                    (a.find("USERNAME")!=a.end() && a.find("PASSWORD")!=a.end()) ? sendCredentials(a["USERNAME"],a["PASSWORD"]) : error();
                else
                    (a.find("__RESULT")!= a.end()) ? sendResult(a["Result"]): error();

            }

            else if(commandName.compare("REQRTREE")==0){
                (a.find("FILEHASH")!=a.end() && a.find("FILEPATH")!=a.end()) ? sendTree(a["FILEHASH"],a["FILEPATH"]) : error();
            }

            else if(commandName.compare("POSTFILE")==0){
                if(server)
                    (a.find("FILEPATH")!=a.end() && a.find("FILEDATA")!=a.end() && a.find("FILEHASH")!=a.end()) ? saveFile(a["FILEPATH"],a["FILEDATA"],a["FILEHASH"] ) : error();
                else
                    (a.find("__RESULT")!= a.end()) ? sendResult(a["Result"]): error();
            }

            else if(commandName.compare("REMVFILE")==0){
                a.find("REMVFILE")!=a.end() ? removeFile(a["REMVFILE"]) : error();
            }
            else{
                error();
            }

    }

    void sendCredentials(std::string & username ,std::string & password );
    void error();
    void sendResult(std::string & r);
    void sendTree(std::string & hash ,std::string & path );
    void saveFile(std::string & path ,std::string & data,std::string & hash);
    void removeFile(std::string & path);
};