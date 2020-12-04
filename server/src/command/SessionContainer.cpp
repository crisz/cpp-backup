#include "SessionContainer.h"

SessionContainer &SessionContainer::get_instance() {
    static SessionContainer instance;
    return instance;
}

UserData &SessionContainer::get_user_data(tcp::socket &socket) {
    return container[&socket];
}

void SessionContainer::set_user_data(tcp::socket &socket, UserData user_data) {
    container[&socket] = user_data;
}

void SessionContainer::add_user(tcp::socket &socket) {
    UserData ud;
    container[&socket] = ud;
}

void SessionContainer::remove_user(tcp::socket &socket) {
    container.erase(&socket);
    std::cout<<"Ci sono " << container.size() << " utenti connessi"<< std::endl;
}

int SessionContainer::get_number_users_connected() {
    return container.size();
}
