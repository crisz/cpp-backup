#include "sync.h"

void sync(UserSession &us) {
    FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
    ClientCommand c;

    bool at_least_one_removed = false;
    bool at_least_one_added = false;
    bool at_least_one_remove_failed = false;
    bool at_least_one_add_failed = false;

    auto login = c.login(us.username, us.password);
    bool login_result = login.get();

    if (!login_result) die("Invalid credentials");

    auto server_tree = c.require_tree().get();


    TreesComparator tc{us.dir};
    auto arr = tc.compare(server_tree);

    auto new_files = arr[0];
    auto changed_files = arr[1];
    auto removed_files = arr[2];

    std::vector<FileMetadata> files_to_remove = *removed_files;
    std::vector<FileMetadata> files_to_post = *new_files;
    files_to_post.insert(
            (files_to_post).end(),
            std::make_move_iterator((*changed_files).begin()),
            std::make_move_iterator((*changed_files).end())
    );

    std::vector<std::future<bool>> futures_to_wait;
    std::map<std::string, bool> remove_results;


    // Esecuzione delle remove

    for (auto fm_rm: files_to_remove) {
        fm_rm.path_to_send=fm_rm.path;
        auto remove_file = c.remove_file(fm_rm);
        remove_results[fm_rm.path] = false;
        futures_to_wait.push_back(std::move(remove_file));
    }

    auto it = remove_results.begin();

    for (int i=0; i<futures_to_wait.size(); i++) {
        bool remove_file_result = futures_to_wait[i].get();
        it->second = remove_file_result;
        if (it->second) at_least_one_removed = true;
        else at_least_one_remove_failed = true;
        it++;
    }

    // Esecuzione delle post
    futures_to_wait.clear();
    std::map<std::string, bool> post_results;

    for(auto fm_po: files_to_post){
        auto post_file1 = c.post_file(fm_po);
        futures_to_wait.push_back(std::move(post_file1));
        post_results[fm_po.path] = false;
    }

    auto it_pr = post_results.begin();

    for (int i=0; i<futures_to_wait.size(); i++) {
        bool post_file_result = futures_to_wait[i].get();
        it_pr->second = post_file_result;
        if (it_pr->second) at_least_one_added = true;
        else at_least_one_add_failed = true;
        it_pr++;
    }

    if (at_least_one_added) {
        std::cout << "I seguenti file sono stati aggiunti sul server: " << std::endl;

        for (auto file: post_results) {
            if (file.second) {
                std::cout << " + " << file.first << std::endl;
            }
        }
    }


    if (at_least_one_removed) {
        std::cout << "I seguenti file sono stati eliminati dal server: " << std::endl;

        for (auto file: remove_results) {
            if (file.second) {
                std::cout << " - " << file.first << std::endl;
            }
        }
    }

    if (at_least_one_add_failed) {
        std::cout << "I seguenti file NON sono stati aggiunti sul server: " << std::endl;

        for (auto file: post_results) {
            if (!file.second) {
                std::cout << " ? " << file.first << std::endl;
            }
        }
    }

    if (at_least_one_remove_failed) {
        std::cout << "I seguenti file NON sono stati eliminati dal server: " << std::endl;

        for (auto file: remove_results) {
            if (!file.second) {
                std::cout << " ? " << file.first << std::endl;
            }
        }
    }

    SyncFileWatcher sfw{fw, c};
    sfw.run();
}
