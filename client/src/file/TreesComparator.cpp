//
// Created by andrea on 24/11/20.
//
//
// Classe che gestisce il confronto tra l'albero ricevuto dal server, contenente le informazioni (path ed hash) di ogni file,
// e l'albero locale relativo ai file contenuti all'interno della cartella da monitorare
//

#include <common/file_system_helper.h>
#include "TreesComparator.h"

// Il Costruttore genera l'albero locale
TreesComparator::TreesComparator(std::string current_path) : current_path{current_path}{
    for(auto &file : boost::filesystem::recursive_directory_iterator(current_path)) {
        if (!boost::filesystem::is_regular_file(file.path())) continue;
        FileMetadata fm;
        fm.path = file.path().string();
        std::string path = file.path().string().substr(current_path.size());
        fm.path_to_send = path; //remove_first_folder(path);

        if (file.path().string().find("/.") != std::string::npos) continue;

        try {
            fm.hash = hash_file(fm.path);
        } catch (BufferedFileReaderException& excp) {
            std::cout << "Sto ignorando il file \"" << fm.path << "\" poiché non è possibile aprirlo in lettura" << std::endl;
            std::cout << "Maggiori dettagli in seguito: " << std::endl;
            std::cout << excp.what() << std::endl;
            continue;
        }
        local_tree_vect.push_back(fm);
    }
}

// Funzione che confronta l'albero locale con quello ricevuto dal server e ritorna tre vettori di file:
// -File da rimuovere
// -File da modificare
// -File da aggiungere
std::array<std::shared_ptr<std::vector<FileMetadata>>, 3>
TreesComparator::compare(std::vector<FileMetadata> &server_tree) {

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
