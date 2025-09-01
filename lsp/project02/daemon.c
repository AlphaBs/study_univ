#include "daemon.h"
#include "daemonlog.h"
#include "daemonconfig.h"
#include "daemonlist.h"
#include "file.h"
#include "arrange.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

// 디몬 디렉토리 (~/.ssu_cleanupd) 초기화
void initDaemonDir() {
    // 현재 디몬 목록을 저장하는 파일의 경로 생성
    char path[PATH_MAX];
    getDaemonDirPath(path);
    mkdir(path, 0755);
    getCurrentDaemonListFilePath(path);

    // 파일이 없으면 생성
    const int fd = open(path, O_CREAT | O_EXCL, 0644);
    if (fd != -1) {
        close(fd);
    }
}

// 디몬 디렉토리 경로 가져오기, ~/.ssu_cleanupd 의 절대 경로
void getDaemonDirPath(char *path) {
    char *homeDir = getHomeDir();
    strcpy(path, homeDir);
    strcat(path, "/");
    strcat(path, DAEMON_DIR_NAME);
}

// 디몬 실행
void runDaemon(DaemonConfig *config) {
    // 디몬 설정
    config->startTime = time(NULL);
    config->pid = getpid();
    
    writeDaemonConfigFile(config); // config 파일 생성
    ensureDaemonLogFile(config); // 로그 파일 생성

    while (true) {
        // 주기마다 작업 실행
        sleep(config->timeInterval);

        // config 다시 읽기
        freeDaemonConfigItems(config);
        readDaemonConfigFile(config);

        // arrange 실행
        arrangeDaemon(config);
    }
}

// 디몬 프로세스 생성
void forkDaemon(DaemonConfig *config) {
    pid_t pid;
    int fd, maxfd;

    if ((pid = fork()) < 0) {
        printf("error: failed to fork daemon\n");
        return;
    }
    if (pid != 0) {
        // 부모 프로세스는 프롬프트 재출력을 위해 탈출
        return;
    }

    setsid();
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    maxfd = getdtablesize();
    for (fd = 0; fd < maxfd; fd++)
        close(fd);

    umask(0);
    chdir("/");
    fd = open("/dev/null", O_RDWR); // stdin
    dup(0); // stdout
    dup(0); // stderr

    runDaemon(config); // daemon 작업 후에는 프로그램 종료
    exit(0);
}
