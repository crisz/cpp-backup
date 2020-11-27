//
// Created by giuseppe on 04/10/20.
//

#ifndef CPP_BACKUP_TREESCOMPARATOR_H
#define CPP_BACKUP_TREESCOMPARATOR_H
#include <boost/filesystem.hpp>
#include "common/hash_file.h"
#include "FileMetadata.h"
#include "client/src/command/ClientCommand.h"

// CLIENT                          SERVER
// ./a.txt  ABC123                 ./a.txt  ABC123
// ./b.txt  DEF234                 ./b.txt  0DE1FF
// ./c.txt  FGH129                 ./d.txt  0D032F

class TreesComparator {
    boost::filesystem::path current_path;
    std::map<std::string, FileMetadata> local_tree_map;
    std::vector<FileMetadata> file_to_remove;
    std::vector<FileMetadata> file_to_post;

public:
    TreesComparator(std::string current_path) : current_path{current_path}{
        for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {

            FileMetadata fm;
            fm.path=file.path().string();

            std::size_t found = current_path.find_last_of("/\\");
            std::string dir = current_path.substr(found);

            std::size_t found2 = fm.path.find_last_of("/\\");
            std::string filename = fm.path.substr(found2);

            std::string path_to_send = dir+filename;

            fm.path_to_send=path_to_send;
            fm.hash= hash_file(fm.path);
            local_tree_map[path_to_send] = fm;
        }
    }

    std::future<std::pair<std::vector<FileMetadata>,std::vector<FileMetadata>>> compare(std::vector<FileMetadata>& server_tree){
       return std::async([&](){
            std::vector<std::string> server_trees_vect;
            std::vector<std::string> local_trees_vect;
            std::vector<std::string> intersection;
            std::map<std::string, FileMetadata> server_tree_map;

            std::cout<< "MAPPA RICEVUTA DAL SERVER: "<< std::endl;
            for(auto se : server_tree){
                std::cout<< se.path <<std::endl;
                server_tree_map[se.path]= se;
            }

            for(auto se : server_tree_map){
                server_trees_vect.push_back(se.first);
            }

           std::cout<< "MAPPA LOCALE: "<< std::endl;
            for(auto ce : local_tree_map){
                std::cout<< ce.second.path_to_send<<std::endl;
                local_trees_vect.push_back(ce.first);
            }

            std::set_intersection(local_trees_vect.begin(),local_trees_vect.end(),
                                  server_trees_vect.begin(),server_trees_vect.end(),
                                  back_inserter(intersection));

            for(auto path: intersection){
                if(server_tree_map.find(path)->first==local_tree_map.find(path)->first){
                    if(server_tree_map.find(path)->second.hash!=local_tree_map.find(path)->second.hash){
                        file_to_post.push_back(local_tree_map.find(path)->second);
                    }
                    local_trees_vect.erase(std::find(local_trees_vect.begin(),local_trees_vect.end(),server_tree_map.find(path)->first));
                    server_trees_vect.erase(std::find(server_trees_vect.begin(),server_trees_vect.end(),server_tree_map.find(path)->first));
                }
            }

            //the remaining elements in the server_tree are the ones to remove
            for(auto se: server_trees_vect){
                file_to_remove.push_back(server_tree_map[se]);
            }

            //the remaining elements in the local_tree are the ones to add
            for(auto se: local_trees_vect){
                file_to_post.push_back(local_tree_map[se]);
            }
            return std::pair<std::vector<FileMetadata>,std::vector<FileMetadata>>(file_to_post,file_to_remove);
        });

    }
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H