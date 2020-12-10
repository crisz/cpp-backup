//
// Classe che si occupa dell'interpretazione di un comando ricevuto, attua le azione necessarie e indine invoca la distach
// in modo da mandare una risposta al client.
//

#pragma once

#include "ServerCommand.h"
#include "TreeManager.h"
#include "common/BufferedFileWriter.h"
#include "common/BufferedFileReader.h"
#include "common/encode_length_utils.h"
#include <map>
#include <boost/asio.hpp>
#include <memory>
#include "RemovalManager.h"
#include "SignupManager.h"
#include <boost/asio/ip/tcp.hpp>
#include <common/BufferedFileWriter.h>
#include <server/src/command/ServerCommand.h>

#define BUFFER_SIZE 1024

using boost::asio::ip::tcp;

class CommandParser {
private:
    // BufferedFileWriter è un puntatore nativo perché lo costruiamo nel start_send_file e lo distruggiamo nel end_send_file
    BufferedFileWriter* bfw;

    // Serve per ottenere il path locale di un file relativo alla directory associata ad un utente lato server
    static std::string get_file_path(tcp::socket& socket, ServerCommand& command);

    // Stampa un errore e disconnette il client
    static void error(tcp::socket& socket);
public:

    // Si occupa dell'interpretazione di un comando ricevuto, attua le azioni necessarie e infine invoca la dispatch
    // in modo da mandare una risposta al client.
    void digest(tcp::socket& socket, ServerCommand& command);

    // Predispone l'invio di un file
    void start_send_file(tcp::socket& socket, long file_size, ServerCommand& command);

    // Svuota il buffer su disco
    std::future<void> send_file_chunk(char* buffer, int buffer_size);

    // Termina la scrittura del file
    void end_send_file(tcp::socket& socket, ServerCommand& command);

    // Viene invocato in caso di errore. Annulla l'invocazione di un comando pendente in caso di errore
    void rollback_command(tcp::socket& socket, ServerCommand& command);
};