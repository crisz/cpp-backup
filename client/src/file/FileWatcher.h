//
// Classe che si occupa del monitoraggio continuo della cartella specificata dall'utente
// e agisce in modo opportuno in caso di modifiche
//


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

     // Controlla se la mappa contiene una certa chiave
     bool contains(const std::string &key);

     boost::filesystem::path path_to_watch;

 public:

     // Time interval at which we check the base folder for changes
     std::chrono::duration<int, std::milli> delay;

     // Costruttore che  setta il path della cartella da monitorare e dopo quanto tempo di deve ricontrollare per eventuali modifiche,
     // inoltre crea una mappa (path, ultima modifica)
     FileWatcher(std::string path_to_watch, std::chrono::duration<int, std::milli> delay);

     // Monitora la cartella specificata e in caso di cambiamenti esegue l'azione necessaria
     void on_file_changed(const std::function<void (std::string, FileStatus)> &callback);

     boost::filesystem::path get_path_to_watch();


 };
