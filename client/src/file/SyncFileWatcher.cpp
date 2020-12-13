//
// Created by Andrea Vara on 27/11/20.
//
// Classa che wrappa la classe FileWatcher ed esegue le operazioni necessarie per mantenere sincronizata
// la cartella locale con quella del serve
//

#include <common/file_system_helper.h>
#include "SyncFileWatcher.h"
#include "common/hash_file.h"


SyncFileWatcher::SyncFileWatcher(FileWatcher &fw, ClientCommand &c): fw{fw}, c{c} {
    for(auto &file : boost::filesystem::recursive_directory_iterator(fw.get_path_to_watch())) {
        if (boost::filesystem::is_directory(file)) {
            directories.insert(file.path().string());
        }
    }
}

// Questa funzione setta la callback che gestisce le azioni necessarie in base al tipo di evento
// (modifica, creazione o cancellazione) avvenuto.
void SyncFileWatcher::run() {
    this->fw.on_file_changed([this](std::string path_matched, FileStatus status) -> void {
        FileMetadata fm;
        fm.path = path_matched;
        if (fm.path.find("/.") != std::string::npos) return;
        fm.path_to_send = remove_first_folder(fm.path);

        if (status == FileStatus::created || status == FileStatus::modified) {
            if (boost::filesystem::is_directory(boost::filesystem::path(path_matched))) {
                directories.insert(path_matched);
                return;
            }
            fm.hash = hash_file(fm.path);
            auto post_file = c.post_file(fm);
            bool post_file_result = post_file.get();
            std::string action = status == FileStatus::created ? "aggiunti" : "aggiornati";
            if (post_file_result) {
                std::cout << "I seguenti file sono stati " << action << " sul server: " << std::endl;
                std::cout << " + " << fm.path << std::endl;
            } else {
                std::cout << "I seguenti file NON sono stati " << action << " sul server: " << std::endl;
                std::cout << " ? " << fm.path << std::endl;
            }
        } else if (status == FileStatus::erased) {
            if (directories.count(path_matched)) {
                directories.erase(path_matched);
                return;
            }

            auto remove_file = c.remove_file(fm);
            bool remove_file_result = remove_file.get();

            if (remove_file_result) {
                std::cout << "I seguenti file sono stati eliminati dal server: " << std::endl;
                std::cout << " - " << fm.path << std::endl;
            } else {
                std::cout << "I seguenti file NON sono stati eliminati dal server: " << std::endl;
                std::cout << " ? " << fm.path << std::endl;
            }
        } else {
            std::cout << "Error! Unknown file status.\n";
        }

    });
}