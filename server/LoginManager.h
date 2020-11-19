//
// Created by giuseppe on 08/11/20.
//

#ifndef CPP_BACKUP_LOGINMANAGER_H
#define CPP_BACKUP_LOGINMANAGER_H


#include <future>
#include <string>
#include <fstream>
#include "../common/md5.h"
#include "../common/file_system_helper.h"
#include "ServerConf.h"


class LoginManager {
public:
    std::future<bool> check_login(std::string username, std::string password) {
        return std::async([&username, &password]() {
            MD5 md5(password);
            std::string hash_pwd= md5.hexdigest();
            std::string dest = ServerConf::get_instance().dest;
            std::string path= dest+username;
            if (!check_dest_dir(path)){
                std::cout<<"La cartella "<< path <<" non esiste o non è possibile aprirla in scrittura."<<std::endl;
                return false;
            }
            std::ifstream ifs(path+"/.credentials");
            std::string user_pwd;
            if (ifs.is_open()) {
                getline (ifs,user_pwd);
                std::cout<<"La password letta è: "<< user_pwd <<std::endl;
                ifs.close();
            }else{
                std::cout<<"Errore nell'apertura del file"<<std::endl;
                return false;
            }
            return hash_pwd==user_pwd;
        });
    }
};



#endif //CPP_BACKUP_LOGINMANAGER_H
