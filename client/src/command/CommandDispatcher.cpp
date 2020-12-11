//
// Classe che gestisce la comunicazione con il server
//

#include "CommandDispatcher.h"

// Il costruttre ottiene l'istanza della connessione con il server
CommandDispatcher::CommandDispatcher() {
    sc = ServerConnectionAsio::get_instance();
}

// Manda il comando con tutti i suoi parametri
void CommandDispatcher::dispatch_partial(std::string command, const CommandDTO parameters) {

    sc->send(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }
}

// Acuisisce il lock, manda il comando con tutti i suoi parametri, rilascia il lock
// e si mette in attela della risposta da parte del server
std::future<CommandDTO> CommandDispatcher::dispatch(std::string command, const CommandDTO &parameters) {
    std::unique_lock ul(dispatch_mutex);

    // copia e incolla dispatch_partial. Soluzione: recursive_mutex

    // lock acquire
    sc->send(command);

    for (auto it = parameters.begin(); it != parameters.end(); it++ ) {
        send_parameter(it->first, it->second);
    }

    // fine copia e incolla
    send_parameter(STOPFLOW, "");
    ul.unlock();
    return wait_for_response(command);
}

std::future<CommandDTO>
CommandDispatcher::wait_for_response(std::string command, bool buffered, std::function<void(char *, int)> fn) {
    return std::async([this, command, &fn, buffered]() {
        std::unique_lock ul(dispatch_mutex);

        CommandDTO result;
        std::string received_command = sc->read_as_str(8);
        while(true) {
            std::string parameter_name = sc->read_as_str(8);

            if (parameter_name == STOPFLOW) {
                sc->read(4); // viene letta la dimensione che sarà sempre \0\0\0\0
                IncomingCommand cc;
                cc.command_name = received_command;
                cc.parameters = result;
                this->pending_commands.push_back(cc);
                cv.wait(ul, [&command, this]() {
                    for (auto pending_command: this->pending_commands) {
                        if (pending_command.command_name == command) return true;
                    }
                    return false;
                });
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
            long length = decode_length(sc->read(4));
            if (buffered) {
                while (length != 0) { // Questo while serve per segnalare al controllo di flusso del tcp
                    int size_to_read = length > BUFFER_SIZE ? BUFFER_SIZE : length;
                    length -= size_to_read;
                    char* parameter_value = sc->read(size_to_read);
                    fn(parameter_value, size_to_read);
                }
            } else {
                std::string parameter_value = sc->read_as_str(length);
                result.insert(std::pair<std::string, std::string>(parameter_name, parameter_value));
            }
        }
    });
}

void CommandDispatcher::lock_raw() {
    this->dispatch_mutex.lock();
}

void CommandDispatcher::unlock_raw() {
    this->dispatch_mutex.unlock();
}

void CommandDispatcher::send_raw(const char *raw_data, int size) {
    sc->send(raw_data, size);
}

void CommandDispatcher::send_parameter(std::string key, std::string value) {

    sc->send(key);
    sc->send(encode_length(value.size()), 4);
    sc->send(value);
}
