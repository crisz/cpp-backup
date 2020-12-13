//
// Created by giuseppe on 04/12/20.
//
// Classe che serve per la registrazione di nuovi utenti.
//

#include "SignupManager.h"

// Funzione asincrona che ritorna un future<bool> in base all'esito della registrazione dell'utente.
std::future<bool> SignupManager::signup(const std::string& username, const std::string& password) {
    return std::async([&username, &password]() {
        MD5 md5(password);
        std::string hash_pwd = md5.hexdigest();
        std::string dest = ServerConf::get_instance().dest;
        std::string path = dest + username;
        if (check_dest_dir(path)){
            std::cout << "Esiste già un utente con username " << username << std::endl;
            return false;
        } else {
            if (boost::filesystem::create_directories(path)) {
                std::ofstream stream;
                stream = std::ofstream { path + "/.credentials", std::ios::out};
                if (stream.fail()) {
                    stream.close();
                    return false;
                }
                stream.write( hash_pwd.c_str(), hash_pwd.size() );
                ServerConf& conf = ServerConf::get_instance();
                boost::filesystem::create_directory(boost::filesystem::path(path + "/" + conf.user_folder));
                return true;
            } else {
                return false;
            }
        }
    });
}


