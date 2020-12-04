//
// Created by giuseppe on 04/12/20.
//

#include "SignupManager.h"

std::future<bool> SignupManager::signup(std::string username, std::string password) {
    return std::async([&username, &password]() {
        MD5 md5(password);
        std::string hash_pwd= md5.hexdigest();
        std::string dest = ServerConf::get_instance().dest;
        std::string path= dest+username;
        if (check_dest_dir(path)){
            std::cout<<"Esiste già un utente con username "<< username <<std::endl;
            return false;
        }else{
            if (boost::filesystem::create_directories(path)) {
                std::cout << "....Successfully Created !" << std::endl;
                std::ofstream stream;
                stream = std::ofstream {path+"/.credentials", std::ios::out};
                if (stream.fail()) {
                    stream.close();
                    return false;
                }
                stream.write(hash_pwd.c_str(), hash_pwd.size());
                return true;
            } else {
                return false;
            }
        }
    });
}


