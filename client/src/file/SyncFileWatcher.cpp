//
// Created by Andrea Vara on 27/11/20.
//
// Classa che wrappa la classe FileWatcher ed esegue le operazioni necessarie per mantenere sincronizata
// la cartella locale con quella del serve
//

#include "SyncFileWatcher.h"
#include "common/hash_file.h"

SyncFileWatcher::SyncFileWatcher(FileWatcher &fw, ClientCommand &c): fw{fw}, c{c} {}

// Questa funzione setta la callback che gestisce le azioni necessarie in base al tipo di evento
// (modifica, creazione o cancellazione) avvenuto.
void SyncFileWatcher::run() {
    this->fw.on_file_changed([this](std::string path_matched, FileStatus status) -> void {
        if (!boost::filesystem::is_regular_file(boost::filesystem::path(path_matched)) && status != FileStatus::erased) {
            return;
        }
        FileMetadata fm;
        fm.path = path_matched;
        std::size_t found = fw.path_to_watch.string().find_last_of("/\\");
        fm.path_to_send=fm.path.substr(found);


        if (status==FileStatus::created) {
            std::cout << "File created: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        } else if (status==FileStatus::modified) {
            std::cout << "File modified: " << fm.path_to_send << std::endl;
            fm.hash=hash_file(fm.path);
            auto post_file1 = c.post_file(fm);
            bool post_file_result_1 = post_file1.get();
            std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
        } else if (status==FileStatus::erased) {
            std::cout << "File erased: " << fm.path_to_send << '\n';
            auto remove_file= c.remove_file(fm);
            bool remove_file_result=remove_file.get();
            std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
        } else {
            std::cout << "Error! Unknown file status.\n";
        }

    });
}


