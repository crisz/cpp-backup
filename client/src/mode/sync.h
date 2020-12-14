#pragma once

#include <iostream>
#include <exception>
#include <stdexcept>
#include "client/src/file/FileWatcher.h"
#include <boost/program_options.hpp>
#include "client/src/server/UserSession.h"
#include "client/src/server/ServerConnectionAsio.h"
#include "client/src/command/ClientCommand.h"
#include <iostream>
#include "client/src/file/TreesComparator.h"
#include "client/src/file/FileMetadata.h"
#include "common/hash_file.h"
#include "common/file_system_helper.h"
#include "../options/options_utils.h"
#include "../file/SyncFileWatcher.h"

#define SYNCH_INTERVAL 1000
#define RETRY_TIMEOUT 3000

void die(std::string message);

void sync(UserSession& us);;

void retry_sync(UserSession& us);