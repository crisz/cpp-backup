#include "restore.h"

void restore(UserSession &us) {
    ClientCommand c;

    bool at_least_one_removed = false;
    bool at_least_one_added = false;
    bool at_least_one_failed = false;

    try {

        auto login = c.login(us.username, us.password);
        bool login_result = login.get();

        if (!login_result) die("Invalid credentials");

        auto server_tree = c.require_tree().get();

        TreesComparator tc{us.dir};
        auto arr = tc.compare(server_tree);

        auto new_files = arr[0];
        auto changed_files = arr[1];
        auto removed_files = arr[2];

        std::vector<FileMetadata> files_to_require = *removed_files;
        std::vector<FileMetadata> files_to_remove = *new_files;

        files_to_require.insert(
                (files_to_require).end(),
                std::make_move_iterator((*changed_files).begin()),
                std::make_move_iterator((*changed_files).end())
        );

        for (auto file: files_to_remove) {
            if (boost::filesystem::exists(file.path)) {
                boost::filesystem::remove_all(file.path);
                at_least_one_removed = true;
            }
        }

        std::map<std::string, bool> require_results;
        std::vector<std::future<bool>> futures_to_wait;

        for (auto file: files_to_require) {
            file.path = us.dir + file.path_to_send;
            futures_to_wait.push_back(c.require_file(file));
            require_results[file.path] = false;
        }

        auto it = require_results.begin();
        for (auto &future: futures_to_wait) {
            it->second = future.get();
            if (it->second) at_least_one_added = true;
            else at_least_one_failed = true;
            it++;
        }

        if (at_least_one_removed) {
            std::cout << "I seguenti file sono stati eliminati: " << std::endl;

            for (auto file: files_to_remove) {
                std::cout << " - " << file.path << std::endl;
            }
        }


        if (at_least_one_added) {
            std::cout << "I seguenti file sono stati aggiunti: " << std::endl;

            for (auto file: require_results) {
                if (file.second) {
                    std::cout << " + " << file.first << std::endl;
                }
            }
        }

        if (at_least_one_failed) {
            std::cout << "I seguenti file NON sono stati aggiunti: " << std::endl;

            for (auto file: require_results) {
                if (!file.second) {
                    std::cout << " ? " << file.first << std::endl;
                }
            }
        }
    } catch (std::exception& e) {
        std::cout << "Si è verficato un errore." << std::endl;
        return;
    };


    std::cout << "\n\n=================================" << std::endl;
    if (!at_least_one_added && !at_least_one_removed) {
        std::cout << "Non sono state apportate modifiche " << std::endl;
    }

    if (at_least_one_failed) {
        std::cout << "Restore completato, ma non tutti i file sono stati scaricati. Si prega di riprovare" << std::endl;
    } else {
        std::cout << "Restore completato con successo" << std::endl;
    }

    std::cout << "=================================\n\n" << std::endl;

}
