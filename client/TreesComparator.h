//
// Created by giuseppe on 04/10/20.
//

#ifndef CPP_BACKUP_TREESCOMPARATOR_H
#define CPP_BACKUP_TREESCOMPARATOR_H
#include <boost/filesystem.hpp>
#include "../common/hash_file.h"

// CLIENT                          SERVER
// ./a.txt  ABC123                 ./a.txt  ABC123
// ./b.txt  DEF234                 ./b.txt  0DE1FF
// ./c.txt  FGH129                 ./d.txt  0D032F

class TreesComparator {
    boost::filesystem::path current_path;
    std::map<std::string, std::string> local_tree;
    std::vector<std::string> file_to_remove;
    std::vector<std::string> file_to_post;

/*
    bool contains(const std::string &key, std::unordered_map<std::string, std::string> tree) {
        auto el = tree.find(key);
        return el != tree.end();
    }
*/
public:
    TreesComparator(std::string current_path) : current_path{current_path}{
        for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {
            local_tree[file.path().string()] = hash_file(file.path().string());
        }
    }

    void compare(std::map<std::string, std::string>& server_tree){
        std::vector<std::string> server_trees_vect;
        std::vector<std::string> local_trees_vect;
        std::vector<std::string> intersection;

        for(auto se : server_tree){
            std::cout<< se.first <<std::endl;
            server_trees_vect.push_back(se.first);
        }
        for(auto ce : local_tree){
            std::cout<< ce.first<<std::endl;
            local_trees_vect.push_back(ce.first);
        }

        std::set_intersection(local_trees_vect.begin(),local_trees_vect.end(),
                              server_trees_vect.begin(),server_trees_vect.end(),
                              back_inserter(intersection));

        for(auto path: intersection){
            if(server_tree.find(path)->first==local_tree.find(path)->first){
                if(server_tree.find(path)->second!=local_tree.find(path)->second){
                    file_to_post.push_back(server_tree.find(path)->first);
                }
                local_trees_vect.erase(std::find(local_trees_vect.begin(),local_trees_vect.end(),server_tree.find(path)->first));
                server_trees_vect.erase(std::find(server_trees_vect.begin(),server_trees_vect.end(),server_tree.find(path)->first));
            }
        }

        //the remaining elements in the server_tree are the ones to remove
        for(auto se: server_trees_vect){
            file_to_remove.push_back(se);
        }
        //the remaining elements in the local_tree are the ones to add
        for(auto ce: local_trees_vect){
            file_to_post.push_back(ce);
        }


        //print the vectors
        std::cout<<"FILES TO REMOVE"<<std::endl;
        for(std::string path: file_to_remove){
            std::cout<< path<<std::endl;
        }
        std::cout<<"FILES TO POST"<<std::endl;
        for(std::string path: file_to_post){
            std::cout<< path <<std::endl;
        }

    }
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H
