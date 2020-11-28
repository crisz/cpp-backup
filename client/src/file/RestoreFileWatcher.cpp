//
// Created by Andrea Vara on 27/11/20.
//

#include "RestoreFileWatcher.h"

RestoreFileWatcher::RestoreFileWatcher(FileWatcher &fw, ClientCommand &c):fw{fw},c{c} {}

void RestoreFileWatcher::run() {//TODO: implementare la callback in caso di restore e poi agganciarla al fileWatcher

}
