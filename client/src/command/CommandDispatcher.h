//
// Classe che gestisce la comunicazione con il server
//


#pragma once
#include <string>
#include <map>
#include "client/src/server/ServerConnectionAsio.h"
#include <future>
#include <vector>
#include <mutex>
#include "CommandDTO.h"
#include "common/Constants.h"
#include "common/Constants.h"
#include "common/BufferedFileWriter.h"
#include "common/encode_length_utils.h"
#include <condition_variable>
#define BUFFER_SIZE 1024

struct IncomingCommand {
    std::string command_name;
    CommandDTO parameters;
};
class CommandDispatcher {
private:
    std::shared_ptr<ServerConnectionAsio> sc;
    std::vector<IncomingCommand> pending_commands;
    std::mutex dispatch_mutex;
    std::condition_variable cv;

public:
    // Il costruttre ottiene l'istanza della connessione con il server
    CommandDispatcher();

    // Manda il comando con tutti i suoi parametri senza acquisire il lock, che dovrà quindi essere gestito più a basso livello,
    // e senza richiamare automaticamente la wait_for_response. Viene richiama tutte le volte che dobbiamo gestire dei file bufferizzati
    void dispatch_partial(std::string command, const CommandDTO parameters);

    // Acuisisce il lock, manda il comando con tutti i suoi parametri, rilascia il lock
    // e si mette in attesa della risposta da parte del server
    std::future<CommandDTO> dispatch(std::string command, const CommandDTO& parameters);

    // Viene chiamata quando il client si deve mettere in attesa di una risposta. Quando ess viene ricevuta viene gestita in modo opportuno
    std::future<CommandDTO> wait_for_response(std::string command, bool buffered = false, std::function<void(char* buffer, int)> fn = nullptr);

    // Serve per acquisire a basso livello il lock
    void lock_raw();

    // Serve per rilasciare a basso livello il lock
    void unlock_raw();

    // Serve per mandare dati quando non sono delle stringhe (in quanto esse ignorano i caratteri non stampabili)
    void send_raw(const char* raw_data, int size);

    // Serve per mandare dei parametri al server
    void send_parameter(std::string key, std::string value);
};