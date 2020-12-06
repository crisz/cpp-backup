//
// Created by Andrea Vara on 21/11/20.
//

#include "server/src/command/RemovalManager.h"

std::future<bool> RemovalManager::remove_file(std::string path) {
    return std::async([&path]() {
        if(boost::filesystem::exists(path)){
            if(boost::filesystem::remove(path)){
                std::cout<<"Rimozione file path: " <<path<<std::endl;
                //Il while successivo serve a vedere ricorsivamente se le cartelle che contenevano il file si sono svuotate.
                // Se si, le elimina.
                std::string dir_path = path;
                while(true){
                    std::size_t found = dir_path.find_last_of("/\\");
                    dir_path = dir_path.substr(0,found);
                    if(boost::filesystem::is_empty(dir_path)){
                        boost::filesystem::remove(dir_path);
                    }else break;
                }
                return true;
            }else{
                std::cout<<"Error deleting file!"<<std::endl;
                return false;
            }
        }
        return false;
    });
}
