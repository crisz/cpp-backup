#include "restore.h"

void restore(UserSession &us) {
    ClientCommand c;

    auto login = c.login(us.username, us.password);

    bool login_result = login.get();

    if (!login_result) die("Invalid credentials");

    auto server_tree = c.require_tree().get();

    for (auto st: server_tree) {
        std::cout << st.path << std::endl;
    }

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


    std::cout << "REQRFILE FOR: " << std::endl;
    for (auto file: files_to_require) {
        std::cout << file.path_to_send << std::endl;
    }

    std::cout << "delete FOR: " << std::endl;
    for (auto file: files_to_remove) {
        std::cout << file.path_to_send << std::endl;
    }

    for (auto file: files_to_remove) {
        std::cout << "removing " << file.path << std::endl;
        if (boost::filesystem::exists(file.path)) {
            boost::filesystem::remove_all(file.path);
        }
    }

    std::vector<std::future<void>> futures_to_wait;
    for (auto file: files_to_require) {
        int count = 0;
        size_t index = 0;
        for (;; index++) {
            if (file.path_to_send[index] == '/') count++;
            if (count == 2) break;
        }
        file.path = us.dir + file.path_to_send.substr(index);

        std::cout << "file.path = " << file.path << std::endl;
        std::cout << "file.path_to_send = " << file.path_to_send << std::endl;

        futures_to_wait.push_back(c.require_file(file));
        //c.require_file(file).get();
    }

    for (auto& future: futures_to_wait) {
        future.get();
    }

    std::cout << "Restore completato con successo " << std::endl;
}
