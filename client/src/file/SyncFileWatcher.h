//
// Created by Andrea Vara on 27/11/20.
//
// Classa che wrappa la classe FileWatcher ed esegue le operazioni necessarie per mantenere sincronizata
// la cartella locale con quella del server
//

#ifndef CPP_BACKUP_SYNCFILEWATCHER_H
#define CPP_BACKUP_SYNCFILEWATCHER_H


#include "../command/ClientCommand.h"
#include "FileWatcher.h"

class SyncFileWatcher {
private:
    FileWatcher& fw;
    ClientCommand& c;
public:
    SyncFileWatcher(FileWatcher& fw, ClientCommand& c);
    void run();
};


#endif //CPP_BACKUP_SYNCFILEWATCHER_H
