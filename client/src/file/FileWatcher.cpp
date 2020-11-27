#include "FileWatcher.h"


FileWatcher::FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay) : path_to_watch{path_to_watch}, delay{delay} {
    for(auto &file : boost::filesystem::recursive_directory_iterator(path_to_watch)) {
        paths_[file.path().string()] = boost::filesystem::last_write_time(file);
    }
}

bool FileWatcher::contains(const std::string &key) {
    auto el = paths_.find(key);
    return el != paths_.end();
}

void FileWatcher::on_file_changed(const std::function<void(std::string, FileStatus)> &callback) {
    while(running_) {
        std::this_thread::sleep_for(delay);

        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!boost::filesystem::exists(it->first)) {
                callback(it->first, FileStatus::erased);
                it = paths_.erase(it);
            } else it++;
        }
        // Check if a file was created or modified
        for(auto &file : boost::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = boost::filesystem::last_write_time(file);
            // File creation
            if (!contains(file.path().string())) {
                paths_[file.path().string()] = current_file_last_write_time;
                callback(file.path().string(), FileStatus::created);
                // File modification
            } else if (paths_[file.path().string()] != current_file_last_write_time) {
                paths_[file.path().string()] = current_file_last_write_time;
                callback(file.path().string(), FileStatus::modified);
            }
        }
    }
}
