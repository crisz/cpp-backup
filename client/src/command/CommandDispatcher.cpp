//
// Classe che gestisce la comunicazione con il server
//

#include "CommandDispatcher.h"

// Il costruttre ottiene l'istanza della connessione con il server
CommandDispatcher::CommandDispatcher() {
    sc = ServerConnectionAsio::get_instance();
}

// Manda il comando con tutti i suoi parametri senza acquisire il lock, che dovrà quindi essere gestito più a basso livello,
// e senza richiamare automaticamente la wait_for_response. Viene richiama tutte le volte che dobbiamo gestire dei file bufferizzati
void CommandDispatcher::dispatch_partial(std::string command, const CommandDTO parameters) {

    sc->send(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }
}

// Acuisisce il lock, manda il comando con tutti i suoi parametri, rilascia il lock
// e si mette in attesa della risposta da parte del server
std::future<CommandDTO> CommandDispatcher::dispatch(std::string command, const CommandDTO &parameters) {
    std::unique_lock ul(dispatch_mutex);

    // lock acquire
    sc->send(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }

    send_parameter(STOPFLOW, "");

    // lock release
    ul.unlock();
    return wait_for_response(command);
}

// Viene chiamata quando il client si deve mettere in attesa di una risposta. Quando ess viene ricevuta viene gestita in modo opportuno
std::future<CommandDTO>
CommandDispatcher::wait_for_response(std::string command, bool buffered, std::function<void(char *, int)> fn) {
    return std::async([this, command, &fn, buffered]() {
        std::unique_lock ul(dispatch_mutex);

        CommandDTO result;
        std::string received_command = sc->read_as_str(8);
        while(true) {
            std::string parameter_name = sc->read_as_str(8);

            // Se il parametro letto è STOPFLOW vuol dire che siamo arrivati alla fine del comando
            if (parameter_name == STOPFLOW) {
                sc->read(4); // viene letta la dimensione che sarà sempre \0\0\0\0
                IncomingCommand cc;
                cc.command_name = received_command;
                cc.parameters = result;
                this->pending_commands.push_back(cc);
                // Si mette in attesa se il comando passato come parametro non è stato inserito ancora fra i comandi pendenti
                // quindi è stato letto (ricevuto) un comando diverso da quello che si stava aspettando
                cv.wait(ul, [&command, this]() {
                    for (auto pending_command: this->pending_commands) {
                        if (pending_command.command_name == command) return true;
                    }
                    return false;
                });

                // Ritorna tutti i parametri ricevuti associati al comando che si stava aspettando
                IncomingCommand cc_result;
                for (auto pending_command: this->pending_commands) {
                    if (pending_command.command_name == command) {
                        cc_result = pending_command;
                        break;
                    }
                }
                cv.notify_all();
                ul.unlock();
                return cc_result.parameters;
            }
            // Se il parametro letto non è STOPFLOW
            long length = decode_length(sc->read(4));

            // Se buffered è true vuol dire che stiamo ricevendo un file bufferizzato dal server
            if (buffered) {
                while (length != 0) { // Questo while serve per segnalare al controllo di flusso del tcp
                    int size_to_read = length > BUFFER_SIZE ? BUFFER_SIZE : length;
                    length -= size_to_read;
                    char* parameter_value = sc->read(size_to_read);
                    fn(parameter_value, size_to_read);
                }
            } else { // Altrimenti vuol dire che stiamo ricevendo un normale parametro
                std::string parameter_value = sc->read_as_str(length);
                result.insert(std::pair<std::string, std::string>(parameter_name, parameter_value));
            }
        }
    });
}

// Serve per acquisire a basso livello il lock
void CommandDispatcher::lock_raw() {
    this->dispatch_mutex.lock();
}

// Serve per rilasciare a basso livello il lock
void CommandDispatcher::unlock_raw() {
    this->dispatch_mutex.unlock();
}

// Serve per mandare dati quando non sono delle stringhe (in quanto esse ignorano i caratteri non stampabili)
void CommandDispatcher::send_raw(const char *raw_data, int size) {
    sc->send(raw_data, size);
}

// Serve per mandare dei parametri al server
void CommandDispatcher::send_parameter(std::string key, std::string value) {
    sc->send(key);
    sc->send(encode_length(value.size()), 4);
    sc->send(value);
}
