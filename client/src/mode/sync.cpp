#include "sync.h"

void sync(UserSession &us) {
    FileWatcher fw{us.dir, std::chrono::milliseconds(SYNCH_INTERVAL)};
    ClientCommand c;
    auto login = c.login(us.username, us.password);

    bool login_result = login.get();

    if (!login_result) die("Invalid credentials");

    auto server_tree = c.require_tree().get();

    for (auto st: server_tree) {
        std::cout<<st.path<<std::endl;
    }

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

    std::cout<<"FILES TO POST"<<std::endl;
    for (auto file: files_to_post) {
        std::cout << file.path << std::endl;
    }

    std::cout<<"FILES TO REMOVE"<<std::endl;
    for (auto file: files_to_remove) {
        std::cout << file.path << std::endl;
    }

    std::vector<std::future<bool>> futures_to_wait;

    for (auto fm_rm: files_to_remove) {
        std::cout<< fm_rm.path<<std::endl;
        fm_rm.path_to_send=fm_rm.path;
        auto remove_file= c.remove_file(fm_rm);
        futures_to_wait.push_back(std::move(remove_file));
    }

    for (int i=0; i<futures_to_wait.size(); i++) {
        bool remove_file_result = futures_to_wait[i].get();
        std::cout << "Remove file effettuato con " << (remove_file_result ? "successo" : "fallimento") << std::endl;
    }

    futures_to_wait.clear();
    for(auto fm_po: files_to_post){
        std::cout<< fm_po.path_to_send <<std::endl;
        auto post_file1 = c.post_file(fm_po);
        futures_to_wait.push_back(std::move(post_file1));
    }

    for (int i=0; i<futures_to_wait.size(); i++) {
        bool post_file_result_1 = futures_to_wait[i].get();
        std::cout << "Post file effettuato con " << (post_file_result_1 ? "successo" : "fallimento") << std::endl;
    }

    SyncFileWatcher sfw{fw, c};
    sfw.run();
}
