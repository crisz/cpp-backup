//
// Created by Andrea Vara on 27/11/20.
//

#ifndef CPP_BACKUP_OPTIONS_UTILS_H
#define CPP_BACKUP_OPTIONS_UTILS_H

#include <iostream>
#include <exception>
#include <stdexcept>
#include "client/src/file/FileWatcher.h"
#include <boost/program_options.hpp>
#include "client/src/server/UserSession.h"
#include "client/src/server/ServerConnectionAsio.h"
#include "client/src/command/ClientCommand.h"
#include <iostream>
#include "client/src/file/TreesComparator1.h"
#include "client/src/file/FileMetadata.h"
#include "common/hash_file.h"
#include "common/file_system_helper.h"

namespace po = boost::program_options;
int parse_sync_options(int argc, char** argv, UserSession& us);


#endif //CPP_BACKUP_OPTIONS_UTILS_H
