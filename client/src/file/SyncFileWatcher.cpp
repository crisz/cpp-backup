//
// Created by Andrea Vara on 27/11/20.
//
// Classa che wrappa la classe FileWatcher ed esegue le operazioni necessarie per mantenere sincronizata
// la cartella locale con quella del serve
//

#include <common/file_system_helper.h>
#include <shared_mutex>
#include "SyncFileWatcher.h"
#include "common/hash_file.h"


SyncFileWatcher::SyncFileWatcher(FileWatcher &fw, ClientCommand &c): fw{fw}, c{c} {
    for(auto &file : boost::filesystem::recursive_directory_iterator(fw.get_path_to_watch())) {
        if (boost::filesystem::is_directory(file)) {
            directories.insert(file.path().string());
        }
    }


    this->check_results_thread = std::thread([this]() {
        try {
            this->check_results();
        } catch (ServerConnectionAsioException& exception) {
            std::cerr << exception.what() << std::endl;
            this->excep = std::current_exception();
        } catch (...) {
            this->excep = std::current_exception();
        }
    });
}

SyncFileWatcher::~SyncFileWatcher() {
    close_flag = true;
    if (this->check_results_thread.joinable()) {
        this->check_results_thread.join();
    }
}

void SyncFileWatcher::check_results() {
    while (!close_flag) {
        std::unique_lock ul(m);
        std::vector<SyncFileWatcherResult> _results = std::move(results);
        ul.unlock();
        for (auto& item: _results) {
            bool result = item.result_future.get();
            item.result = result;
        }

        print_file_changes(_results, FileStatus::created, "creati", "+");
        print_file_changes(_results, FileStatus::modified, "aggiornati","+");
        print_file_changes(_results, FileStatus::erased, "rimossi","-");

        std::this_thread::sleep_for(std::chrono::milliseconds(PRINT_INTERVAL));
    }
}

void SyncFileWatcher::print_file_changes(std::vector<SyncFileWatcherResult>& _results , FileStatus fs, const std::string& action, const std::string& symbol) {

    bool at_least_one_success = false;
    bool at_least_one_failure = false;

    for (auto& item: _results) {
        if (item.result && fs == item.fs) at_least_one_success = true;
        else if (!item.result && fs == item.fs) at_least_one_failure = true;
        if (at_least_one_success && at_least_one_failure) break;
    }

    if (at_least_one_success) {
        std::cout << std::endl;
        std::cout << "I seguenti file sono stati " << action << " sul server: " << std::endl;

        for (auto& item: _results) {
            if (item.result && item.fs == fs) {
                std::cout << " " << symbol << " " << item.path << std::endl;
            }

        }
    }

    if (at_least_one_failure) {
        std::cout << std::endl;
        std::cout << "I seguenti file NON sono stati " << action << " sul server: " << std::endl;

        for (auto& item: _results) {
            if (!item.result && item.fs == fs) {
                std::cout << " ? " << item.path << std::endl;
            }
        }

    }

}


// Questa funzione setta la callback che gestisce le azioni necessarie in base al tipo di evento
// (modifica, creazione o cancellazione) avvenuto.
void SyncFileWatcher::run() {
    this->fw.on_file_changed([this](std::string path_matched, FileStatus status) -> void {
        std::unique_lock ul(m);

        // Se non è avvenuta nessuna modifica, ci limitiamo a controllare se ci sono eccezioni pendenti
        if (status == FileStatus::nop) {
            if (excep) {
                std::rethrow_exception(excep);
            }
            return;
        }

        FileMetadata fm;
        fm.path = path_matched;
        if (fm.path.find("/.") != std::string::npos) return;

        std::string path = path_matched.substr(fw.get_path_to_watch().string().size());
        fm.path_to_send = path; //remove_first_folder(path);

        if (status == FileStatus::created || status == FileStatus::modified) {
            if (boost::filesystem::is_directory(boost::filesystem::path(path_matched))) {
                directories.insert(path_matched);
                return;
            }
            fm.hash = hash_file(fm.path);
            ul.unlock();
            auto post_file = c.post_file(fm);
            ul.lock();
            results.emplace_back(status, fm.path, std::move(post_file));

        } else if (status == FileStatus::erased) {
            if (directories.count(path_matched)) {
                directories.erase(path_matched);
                return;
            }

            ul.unlock();
            auto remove_file = c.remove_file(fm);
            ul.lock();

            results.emplace_back(status, fm.path, std::move(remove_file));

        }
    });

}