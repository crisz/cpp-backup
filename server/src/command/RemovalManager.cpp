//
// Created by Andrea Vara on 21/11/20.
//

#include "server/src/command/RemovalManager.h"

std::future<bool> RemovalManager::remove_file(std::string path) {
    return std::async([&path]() {
        std::string dest = ServerConf::get_instance().dest;
        if(boost::filesystem::exists(path)){
            if(boost::filesystem::remove(path)){
                std::cout<<"Rimozione file path: " <<path<<std::endl;
                return true;
            }else{
                std::cout<<"Error deleting file!"<<std::endl;
                return false;
            }
        }
        return false;
    });
}
