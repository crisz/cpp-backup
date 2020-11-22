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
    std::map<std::string, FileMetadata> local_tree;
    std::vector<FileMetadata> file_to_remove;
    std::vector<FileMetadata> file_to_post;

/*
    bool contains(const std::string &key, std::unordered_map<std::string, std::string> tree) {
        auto el = tree.find(key);
        return el != tree.end();
    }
*/
public:
    TreesComparator(std::string current_path) : current_path{current_path}{
        for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {
            std::cout<< "CURRENT PATH: "<< file.path().string()<< std::endl;
            FileMetadata fm;
            fm.path=file.path().string();
            std::size_t found = current_path.find_last_of("/\\");
            std::string dir = current_path.substr(found);
            std::size_t found2 = fm.path.find_last_of("/\\");
            std::string filename = fm.path.substr(found2);
            std::string path_to_send = dir+filename;
            fm.path_to_send=path_to_send;
            fm.hash= hash_file(fm.path);
            local_tree[path_to_send] = fm;
        }
    }

    void compare(std::vector<FileMetadata>& server_tree){
        std::vector<FileMetadata> server_trees_vect;
        std::vector<FileMetadata> local_trees_vect;
        std::vector<std::string> intersection;
        std::map<std::string, FileMetadata> server_tree_map;

        for(auto se : server_tree){
            std::cout<< "MEPPA RICEVUTA DAL SERVER: "<< std::endl;
            std::cout<< se.path <<std::endl;
            server_tree_map[se.path]= se;
        }

        for(auto se : server_tree_map){
            server_trees_vect.push_back(se.second);
        }

        for(auto ce : local_tree){
            std::cout<< "MAPPA LOCALE: "<< std::endl;
            std::cout<< ce.second.path_to_send<<std::endl;
            local_trees_vect.push_back(ce.second);
        }

        std::set_intersection(local_trees_vect.begin(),local_trees_vect.end(),
                              server_trees_vect.begin(),server_trees_vect.end(),
                              back_inserter(intersection));

        for(auto path: intersection){

            if(server_tree_map.find(path)->first==local_tree.find(path)->first){
                if(server_tree_map.find(path)->second.hash!=local_tree.find(path)->second.hash){
                    file_to_post.push_back(local_tree.find(path)->second);
                }

                auto pred = [&path, &server_tree_map](FileMetadata& fm) {return fm.path_to_send!= server_tree_map.find(path)->first;};
                std::copy_if(local_trees_vect.begin(), local_trees_vect.end(), local_trees_vect, pred);
                auto pred1 = [&path, &server_tree_map](FileMetadata& fm) {return fm.path_to_send!= server_tree_map.find(path)->first;};
                std::copy_if(server_trees_vect.begin(), server_trees_vect.end(), server_trees_vect, pred1);
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
        for(auto md : file_to_remove){
            std::cout<< md.path<<std::endl;
            ClientCommand c;
            auto remove_file= c.remove_file(md);
            bool remove_file_result=remove_file.get();
            std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
        }
        std::cout<<"FILES TO POST"<<std::endl;
        for(auto md: file_to_post){
            std::cout<< md.path_to_send <<std::endl;
            ClientCommand c;
            auto post_file1 = c.post_file(md);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        }

    }
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H
