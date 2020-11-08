#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "common/hash_file.h"
#include "LoginManager.h"
#include "SingleUserConnection.h"
#include "MessageDispatcher.h"
#include "../common/Constants.h"

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
// REMVFILE __RESULT 000

    std::string handleCommand(SingleUserConnection* suc) {  // TODO: digest
        // TODO: risolvere il problema del this (ad es. spostando la logica in command parser)
        if (this->command_name == LOGINSNC) {
            LoginManager lm;
            std::string username = parameters[USERNAME];
            std::string password = parameters[PASSWORD];
            bool result = lm.check_login(username, password).get();
            std::map<std::string, std::string> result_map;
            result_map[__RESULT] = result ? "OK" : "KO";
            MessageDispatcher md{suc};
            md.dispatch(this->command_name,result_map);
            this->clear();
        }
    }

        /*
            if(command_name.compare("LOGINSNC")==0) {
                if(server){
                    (parameters.find("USERNAME") != parameters.end() && parameters.find("PASSWORD") != parameters.end())
                                ? sendCredentials(parameters["USERNAME"],parameters["PASSWORD"]): error();
                }else{
                    //implementare client
                }
            }else if(command_name.compare("REQRTREE")==0){
              if(server) return sendTree( "/Users/AndreaVara/Desktop/affucati");
              else {
                  (parameters.find("FILEHASH") != parameters.end() && parameters.find("FILEPATH") != parameters.end())
                            ? saveTree() : error();
              }
            }

            else if(command_name.compare("POSTFILE")==0) {
                if(server) {
                    (parameters.find("FILEPATH") != parameters.end() &&
                        parameters.find("FILEDATA") != parameters.end() && parameters.find("FILEHASH") != parameters.end())
                                    ? saveFile(parameters["FILEPATH"], parameters["FILEDATA"], parameters["FILEHASH"]) : error();
                }else {
                    //implementare client
                }
                }
            else if(command_name.compare("REMVFILE")==0){
                if(server) {
                    parameters.find("REMVFILE")!=parameters.end() ? removeFile(parameters["REMVFILE"]) : error();
                }else{
                    //implementare server
                }
            }

            else{
                error();
            }
            std::string prova="afadsfd";
            return prova;


    }
         */

    void sendCredentials(std::string & username ,std::string & password ){
        std::cout<<"Credenziali ricevute us: "<<username << " pass: "<< password<<std::endl;
    }
    void error(){
        std::cout<<"Errore nel comando"<<std::endl;
    }
    void sendResult(std::string & r){}

    std::string sendTree(std::string const& path) {
        std::map<std::string, std::string> local_tree;
        for(auto &file : boost::filesystem::recursive_directory_iterator(path)) {
            local_tree[file.path().string()] = hash_file(file.path().string());
        }
        std::cout<<"sto mandando sto"<<std::endl;

        std::string buffer;
        // boost::iostreams::back_insert_device<std::string> inserter(buffer);
        // boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> ostr(inserter);
        // boost::archive::binary_oarchive oa(ostr);
        // oa << local_tree;
        // ostr.flush();
        // std::stringstream stream;
        // stream <<"REQRTREERESPTREE"<<std::hex <<std::setfill('0')<<std::setw(4)<<buffer.size()<<buffer<<"STOPFLOW0000";
        // std::string result( stream.str());
        // std::cout<<buffer.size()<<std::endl;
        // std::cout<<result<<std::endl;
        return buffer;
    }

    void saveFile(std::string & path ,std::string & data,std::string & hash){}
    void removeFile(std::string & path){}
    void saveTree(){}
};