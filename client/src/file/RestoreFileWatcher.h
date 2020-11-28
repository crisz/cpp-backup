//
// Created by Andrea Vara on 27/11/20.
//

#ifndef CPP_BACKUP_RESTOREFILEWATCHER_H
#define CPP_BACKUP_RESTOREFILEWATCHER_H
#include "../command/ClientCommand.h"
#include "FileWatcher.h"

class RestoreFileWatcher {
private:
    FileWatcher & fw;
    ClientCommand & c;
public:
    RestoreFileWatcher(FileWatcher & fw , ClientCommand & c);
    void run();

};


#endif //CPP_BACKUP_RESTOREFILEWATCHER_H
