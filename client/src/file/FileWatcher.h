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
     bool contains(const std::string &key);

 public:
     boost::filesystem::path path_to_watch;

     // Time interval at which we check the base folder for changes
     std::chrono::duration<int, std::milli> delay;

     // Keep a record of files from the base directory and their last modification time on a unordered map
     FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay);

     // Monitor "path_to_watch" for changes and in case of a change execute the user supplied "action" function
     void on_file_changed(const std::function<void (std::string, FileStatus)> &callback);


};
