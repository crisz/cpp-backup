//
// Created by andrea on 24/11/20.
//
// Classe che gestice il confronto tra l'albero ricevuto dal server, contenente le informazioni (path ed hash) di ogni file,
// e l'albero locale relativo ai file contenuti all'interno della cartella da montorare
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
    TreesComparator(std::string current_path);

    std::array<std::shared_ptr<std::vector<FileMetadata>>, 3> compare(std::vector<FileMetadata>& server_tree);
};


#endif //CPP_BACKUP_TREESCOMPARATOR_H
