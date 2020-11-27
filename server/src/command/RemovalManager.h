//
// Created by Andrea Vara on 21/11/20.
//

#ifndef CPP_BACKUP_REMOVALMANAGER_H
#define CPP_BACKUP_REMOVALMANAGER_H

#include <future>
#include <string>
#include <fstream>
#include "server/src/pool/ServerConf.h"
#include <boost/filesystem.hpp>
#include <iostream>
class RemovalManager {
public:
    std::future<bool> remove_file(std::string path) {
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
};


#endif //CPP_BACKUP_REMOVALMANAGER_H
