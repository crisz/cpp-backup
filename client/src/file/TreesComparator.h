//
// Created by andrea on 24/11/20.
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
    std::vector<FileMetadata> local_tree_vect;

public:
    TreesComparator(std::string current_path) : current_path{current_path}{
        std::cout << "current path is " << current_path << std::endl;
        for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {

            FileMetadata fm;
            fm.path = file.path().string();

            std::size_t found = current_path.find_last_of("/\\");
            std::string dir = current_path.substr(found);

            std::size_t found2 = fm.path.find_last_of("/\\");
            std::string filename = fm.path.substr(found2);

            std::string path_to_send = dir + filename;

            fm.path_to_send = path_to_send;
            fm.hash = hash_file(fm.path);
            std::cout << fm.hash << std::endl;
            local_tree_vect.push_back(fm);
        }
    }

    // modificati
    // aggiunti
    // rimossi

    std::array<std::shared_ptr<std::vector<FileMetadata>>, 3> compare(std::vector<FileMetadata>& server_tree) {

            auto set_intersection = [ this ](FileMetadata& fm) {
                for (auto el: this->local_tree_vect) {
                    if (el.path_to_send == fm.path) return true;
                }
                return false;
            };

            std::vector<FileMetadata> intersection;
            std::copy_if(server_tree.begin(), server_tree.end(), back_inserter(intersection), set_intersection);

            auto is_file_new = [&intersection](FileMetadata& fm) {
                for (auto el: intersection) {
                    if (el.path == fm.path_to_send) return false;
                }
                return true;
            };

            auto is_file_changed = [&intersection](FileMetadata& fm) {
                for (auto el: intersection) {
                    if (el.path == fm.path_to_send) return el.hash != fm.hash;
                }
                return false;
            };

            auto is_file_removed = [&intersection](FileMetadata& fm) {
                for (auto el: intersection) {
                    if (el.path == fm.path) return false;
                }
                return true;
            };


            std::vector<FileMetadata> local_trees_vect;


            std::array<std::shared_ptr<std::vector<FileMetadata>>, 3> ret_arr;

            std::shared_ptr<std::vector<FileMetadata>> new_files = std::make_shared<std::vector<FileMetadata>>();
            std::copy_if(local_trees_vect.begin(), local_trees_vect.end(), back_inserter(*new_files), is_file_new);
            ret_arr[0] = std::move(new_files);

            std::shared_ptr<std::vector<FileMetadata>> changed_files = std::make_shared<std::vector<FileMetadata>>();
            std::copy_if(local_trees_vect.begin(), local_trees_vect.end(), back_inserter(*changed_files), is_file_changed);
            ret_arr[1] = std::move(changed_files);

            std::shared_ptr<std::vector<FileMetadata>> removed_files = std::make_shared<std::vector<FileMetadata>>();
            std::copy_if(server_tree.begin(), server_tree.end(), back_inserter(*removed_files), is_file_removed);
            ret_arr[2] = std::move(removed_files);

            return ret_arr;
    }
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H
