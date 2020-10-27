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

    void handleCommand() {
            if(command_name.compare("LOGINSNC")==0) {
                (parameters.find("USERNAME") != parameters.end() && parameters.find("PASSWORD") != parameters.end()) ? sendCredentials(parameters["USERNAME"],parameters["PASSWORD"]): error();
            }else if(command_name.compare("REQRTREE")==0){
                (parameters.find("FILEHASH")!=parameters.end() && parameters.find("FILEPATH")!=parameters.end()) ? sendTree(parameters["FILEHASH"],parameters["FILEPATH"]) : error();
            }
            else if(command_name.compare("POSTFILE")==0) {
                    (parameters.find("FILEPATH") != parameters.end() && parameters.find("FILEDATA") != parameters.end() && parameters.find("FILEHASH") != parameters.end())
                    ? saveFile(parameters["FILEPATH"], parameters["FILEDATA"], parameters["FILEHASH"]) : error();
                }
            else if(command_name.compare("REMVFILE")==0){
                parameters.find("REMVFILE")!=parameters.end() ? removeFile(parameters["REMVFILE"]) : error();
            }
            else{
                error();
            }

    }

    void sendCredentials(std::string & username ,std::string & password ){
        std::cout<<"Credenziali ricevute us: "<<username << " pass: "<< password<<std::endl;
    }
    void error(){
        std::cout<<"Errore nel comando"<<std::endl;
    }
    void sendResult(std::string & r){}
    void sendTree(std::string & hash ,std::string & path ){}
    void saveFile(std::string & path ,std::string & data,std::string & hash){}
    void removeFile(std::string & path){}
};