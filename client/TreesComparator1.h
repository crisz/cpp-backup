//
// Created by andrea on 24/11/20.
//

#ifndef CPP_BACKUP_TREESCOMPARATOR1_H
#define CPP_BACKUP_TREESCOMPARATOR1_H

#include <boost/filesystem.hpp>
#include "../common/hash_file.h"
#include "FileMetadata.h"
#include "ClientCommand.h"

// CLIENT                          SERVER
// ./a.txt  ABC123                 ./a.txt  ABC123
// ./b.txt  DEF234                 ./b.txt  0DE1FF
// ./c.txt  FGH129                 ./d.txt  0D032F

class TreesComparator1 {
    boost::filesystem::path current_path;
    std::map<std::string, FileMetadata> local_tree_map;

public:
    TreesComparator1(std::string current_path) : current_path{current_path}{
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
            std::cout<<fm.hash<<std::endl;
            local_tree_map[path_to_send] = fm;
        }
    }

    void compare(std::vector<FileMetadata>& server_tree,
                 std::promise<std::shared_ptr<std::vector<FileMetadata>>> p_to_post,
                 std::promise<std::shared_ptr<std::vector<FileMetadata>>> p_to_remove){
            std::vector<FileMetadata> local_trees_vect;

            std::cout<< "MAPPA RICEVUTA DAL SERVER: "<< std::endl;
            for(auto se : server_tree){
                std::cout<< se.path <<std::endl;
            }

            std::cout<< "MAPPA LOCALE: "<< std::endl;
            for(auto ce : local_tree_map){
                std::cout<< ce.second.path_to_send<<std::endl;
                local_trees_vect.push_back(ce.second);
            }

            auto set_intersection= [ &local_trees_vect](FileMetadata& fm) {
                for(auto el: local_trees_vect){
                    if(el.path_to_send==fm.path)return true;
                }
                return false;
            };
            //calcolo intersezione
            std::vector<FileMetadata> intersection;
            std::copy_if(server_tree.begin(), server_tree.end(),  back_inserter(intersection), set_intersection);

            auto set_post_file = [ &intersection](FileMetadata& fm) {
                for(auto el: intersection) {
                    if(el.path==fm.path_to_send) return el.hash!=fm.hash;
                }
                return true;
            };



            auto set_remove_file =[&intersection](FileMetadata& fm) {
                for(auto el: intersection) {
                    if(el.path==fm.path) return false;
                }
                return true;
            };


            auto post =std::async([&](){
                std::shared_ptr<std::vector<FileMetadata>>to_post= std::make_shared<std::vector<FileMetadata>>();
                std::copy_if(local_trees_vect.begin(), local_trees_vect.end(),  back_inserter(*to_post), set_post_file);
                p_to_post.set_value(std::move(to_post));
            });
           auto remove =std::async([&](){
                std::shared_ptr<std::vector<FileMetadata>>to_remove=std::make_shared<std::vector<FileMetadata>>();
                std::copy_if(server_tree.begin(), server_tree.end(), back_inserter(*to_remove), set_remove_file);
               p_to_remove.set_value(std::move(to_remove));
            });
            post.get();
            remove.get();
    }
};


#endif //CPP_BACKUP_TREESCOMPARATOR1_H
