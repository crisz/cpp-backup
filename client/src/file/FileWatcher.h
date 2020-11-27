#pragma once
#include <experimental/filesystem>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>
#include <boost/filesystem.hpp>


 enum class FileStatus {created, modified, erased};

 class FileWatcher {

     std::unordered_map<std::string, std::time_t> paths_;
     bool running_ = true;

     // Check if "paths_" contains a given key
     bool contains(const std::string &key) {
         auto el = paths_.find(key);
         return el != paths_.end();
     }

 public:
     boost::filesystem::path path_to_watch;

     // Time interval at which we check the base folder for changes
     std::chrono::duration<int, std::milli> delay;

     // Keep a record of files from the base directory and their last modification time on a unordered map
     FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay) : path_to_watch{path_to_watch}, delay{delay} {
             for(auto &file : boost::filesystem::recursive_directory_iterator(path_to_watch)) {
                  paths_[file.path().string()] = boost::filesystem::last_write_time(file);
              }
     }

     // Monitor "path_to_watch" for changes and in case of a change execute the user supplied "action" function
     void on_file_changed(const std::function<void (std::string, FileStatus)> &callback) {
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


};
