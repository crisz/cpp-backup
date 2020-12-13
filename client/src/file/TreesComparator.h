//
// Created by andrea on 24/11/20.
//
// Classe che gestice il confronto tra l'albero ricevuto dal server, contenente le informazioni (path ed hash) di ogni file,
// e l'albero locale relativo ai file contenuti all'interno della cartella da monitorare
//

#ifndef CPP_BACKUP_TREESCOMPARATOR_H
#define CPP_BACKUP_TREESCOMPARATOR_H

#include <boost/filesystem.hpp>
#include "common/hash_file.h"
#include "FileMetadata.h"
#include "client/src/command/ClientCommand.h"

class TreesComparator {
    boost::filesystem::path current_path;
    std::vector<FileMetadata> local_tree_vect;

public:
    // Il Costruttore genera l'albero locale
    TreesComparator(std::string current_path);

    // Funzione che confronta l'albero locale con quello ricevuto dal server e ritorna tre vettori di file:
    // -File da rimuovere
    // -File da modificare
    // -File da aggiungere
    std::array<std::shared_ptr<std::vector<FileMetadata>>, 3> compare(std::vector<FileMetadata>& server_tree);
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H
