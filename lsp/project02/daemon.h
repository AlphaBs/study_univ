#ifndef DAEMON_H
#define DAEMON_H

#include "daemonconfig.h"
#include <stdbool.h>
#include <stdio.h>
#include <linux/limits.h>

#define DAEMON_DIR_NAME ".ssu_cleanupd"

void initDaemonDir();
void getDaemonDirPath(char* path);
void runDaemon(DaemonConfig *config);
void forkDaemon(DaemonConfig* config);

#endif
