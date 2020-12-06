//
// Struttura contenente tutte le informazioni della sessione relativa all'utente corrente
//

#pragma once
#include <string>
struct UserSession {
    std::string username;
    std::string password;
    int port;
    std::string address;
    std::string dir;
};