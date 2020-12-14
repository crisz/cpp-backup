//
// Classe che si occupa del monitoraggio continuo della cartella specificata dall'utente
// e agisce in modo opportuno in caso di modifiche
//

#include "FileWatcher.h"

// Costruttore che  setta il path della cartella da monitorare e dopo quanto tempo di deve ricontrollare per eventuali modifiche,
// inoltre crea una mappa (path, ultima modifica)
FileWatcher::FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay) : path_to_watch{path_to_watch}, delay{delay} {
    for(auto &file : boost::filesystem::recursive_directory_iterator(path_to_watch)) {
        paths_[file.path().string()] = boost::filesystem::last_write_time(file);
    }
}

boost::filesystem::path FileWatcher::get_path_to_watch() {
    return this->path_to_watch;
}

// Controlla se la mappa contiene una certa chiave
bool FileWatcher::contains(const std::string &key) {
    auto el = paths_.find(key);
    return el != paths_.end();
}


// Monitora la cartella specificata e in caso di cambiamenti esegue l'azione necessaria
void FileWatcher::on_file_changed(const std::function<void(std::string, FileStatus)> &callback) {
    while(running_) {
        std::this_thread::sleep_for(delay);

        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!boost::filesystem::exists(it->first)) {
                std::thread(callback, it->first, FileStatus::erased).detach();
                it = paths_.erase(it);
            } else it++;
        }
        // Check if a file was created or modified
        for(auto &file : boost::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = boost::filesystem::last_write_time(file);
            // File creation
            if (!contains(file.path().string())) {
                paths_[file.path().string()] = current_file_last_write_time;
                std::thread(callback, file.path().string(), FileStatus::created).detach();
                // File modification
            } else if (paths_[file.path().string()] != current_file_last_write_time) {
                paths_[file.path().string()] = current_file_last_write_time;
                std::thread(callback, file.path().string(), FileStatus::modified).detach();
            }
        }
    }
}
