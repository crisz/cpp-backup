//
// Created by Andrea Vara on 27/11/20.
//
// Classa che wrappa la classe FileWatcher ed esegue le operazioni necessarie per mantenere sincronizata
// la cartella locale con quella del server
//

#ifndef CPP_BACKUP_SYNCFILEWATCHER_H
#define CPP_BACKUP_SYNCFILEWATCHER_H


#include "../command/ClientCommand.h"
#include "FileWatcher.h"
#include <set>
#include <shared_mutex>

class SyncFileWatcherResult {
public:
    SyncFileWatcherResult(
        FileStatus fs,
        std::string path,
        std::future<bool> result_future
        ): fs{fs}, path{path} {
        this->result_future = std::move(result_future);
    }

    FileStatus fs;
    std::string path;
    std::future<bool> result_future;
    bool result;
};

class SyncFileWatcher {
private:
    FileWatcher& fw;
    ClientCommand& c;
    std::set<std::string> directories;
    std::vector<SyncFileWatcherResult> results;
    std::thread check_results_thread;
    bool close_flag = false;
    std::shared_mutex m;

    void check_results();
    void print_file_changes(std::vector<SyncFileWatcherResult>& _results, FileStatus fs, const std::string& action, const std::string& symbol);
public:
    SyncFileWatcher(FileWatcher& fw, ClientCommand& c);
    ~SyncFileWatcher();

    // Questa funzione setta la callback che gestisce le azioni necessarie in base al tipo di evento
    // (modifica, creazione o cancellazione) avvenuto.
    void run();
};


#endif //CPP_BACKUP_SYNCFILEWATCHER_H
