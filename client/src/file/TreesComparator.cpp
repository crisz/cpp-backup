//
// Created by andrea on 24/11/20.
//

#include "TreesComparator.h"

// Il Costruttore genera l'albero locale
TreesComparator::TreesComparator(std::string current_path) : current_path{current_path}{
    std::cout << "current path is " << current_path << std::endl;
    for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {
        if(!boost::filesystem::is_regular_file(file.path())){
            continue;
        }
        FileMetadata fm;
        fm.path = file.path().string();

        std::size_t found = current_path.find_last_of("/\\");
        std::string filename = fm.path.substr(found);

        std::string path_to_send = filename;

        fm.path_to_send = path_to_send;
        fm.hash = hash_file(fm.path);
        std::cout << fm.hash << std::endl;
        local_tree_vect.push_back(fm);
    }
}

// Funzione che confronta l'lbero locale con quello ricevuto dal server e ritorna tre vettori di file:
// -File da rimuovere
// -File da modificare
// -File da aggiungere
std::array<std::shared_ptr<std::vector<FileMetadata>>, 3>
TreesComparator::compare(std::vector<FileMetadata> &server_tree) {

    std::cout << "printing local tree vect" << std::endl;
    for (auto file: local_tree_vect) {
        std::cout << "file in local tree vect " << file.path << std::endl;
    }

    // Ricaviamo l'intersezione tra i due alberi facendo riferimento ai path dei file
    auto set_intersection = [ this ](FileMetadata& fm) {
        for (auto el: this->local_tree_vect) {
            if (el.path_to_send == fm.path) return true;
        }
        return false;
    };
    std::vector<FileMetadata> intersection;
    std::copy_if(server_tree.begin(), server_tree.end(), back_inserter(intersection), set_intersection);

    // Ricaviamo la lista di file da aggiungere nel server
    auto is_file_new = [&intersection](FileMetadata& fm) {
        for (auto el: intersection) {
            if (el.path == fm.path_to_send) return false;
        }
        return true;
    };

    // Ricaviamo la lista di file che hanno un hash diverso e sono stati quindi modificati
    auto is_file_changed = [&intersection](FileMetadata& fm) {
        for (auto el: intersection) {
            if (el.path == fm.path_to_send) return el.hash != fm.hash;
        }
        return false;
    };

    // Ricaviamo la lista di file che devono essere rimossi dal server
    auto is_file_removed = [&intersection](FileMetadata& fm) {
        for (auto el: intersection) {
            if (el.path == fm.path) return false;
        }
        return true;
    };

    //Di seguito vengono richiamate le callback precedentemente definite in modo da tornare i tre vettori distinti
    std::array<std::shared_ptr<std::vector<FileMetadata>>, 3> ret_arr;

    std::shared_ptr<std::vector<FileMetadata>> new_files = std::make_shared<std::vector<FileMetadata>>();
    std::copy_if(local_tree_vect.begin(), local_tree_vect.end(), back_inserter(*new_files), is_file_new);
    ret_arr[0] = std::move(new_files);

    std::shared_ptr<std::vector<FileMetadata>> changed_files = std::make_shared<std::vector<FileMetadata>>();
    std::copy_if(local_tree_vect.begin(), local_tree_vect.end(), back_inserter(*changed_files), is_file_changed);
    ret_arr[1] = std::move(changed_files);

    std::shared_ptr<std::vector<FileMetadata>> removed_files = std::make_shared<std::vector<FileMetadata>>();
    std::copy_if(server_tree.begin(), server_tree.end(), back_inserter(*removed_files), is_file_removed);
    ret_arr[2] = std::move(removed_files);

    return ret_arr;
}
