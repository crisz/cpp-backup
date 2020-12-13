//
// Classe Singleton che tiene traccia della porta in cui il server è in listening e la cartella "users"
// conenente tutte le cartelle rispettive ad ogni singolo utente
//

#pragma once
#include <string>

class ServerConf {
private:
    ServerConf(ServerConf& source) = delete;
    ServerConf(ServerConf&& source) = delete;
    ServerConf(){}
public:
    int port;
    std::string dest;
    //Questo metodo ritorna l'istanza della classe.
    static ServerConf& get_instance() {
        static ServerConf instance;
        return instance;
    }
};