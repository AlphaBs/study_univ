#include "daemonconfig.h"
#include "file.h"
#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// DaemonConfig 구조체 기본값으로 초기화
void initDaemonConfig(DaemonConfig *config) {
    config->dirPath[0] = '\0';
    config->outputPath[0] = '\0';
    config->timeInterval = 10;
    config->maxLogLines = -1;
    config->excludePathCount = 0;
    config->extensionCount = 0;
    config->duplicatedFileMode = DUPLICATED_FILE_MODE_LATEST;
    config->startTime = 0;
}

// 디렉토리 유효성 검사, 디몬에서 처리 가능한 디렉토리인지 확인
bool validateDaemonDirPath(char *dirPath, char *realDirPath) {
    // PATH_MAX 초과 확인
    if (strlen(dirPath) > PATH_MAX) {
        printf("error: too long path, %s\n", dirPath);
        return false;
    }

    // ~ 문자 치환
    char fullPath[PATH_MAX];
    replaceHomeDir(fullPath, dirPath);
    if (fullPath[0] == '\0') {
        printf("error: invalid path, %s\n", dirPath);
        return false;
    }

    // PATH_MAX 초과 확인
    if (strlen(fullPath) > PATH_MAX) {
        printf("error: too long path, %s\n", fullPath);
        return false;
    }

    // 존재하는지 확인
    struct stat dirStat;
    if (stat(fullPath, &dirStat) != 0) {
        printf("%s does not exist\n", dirPath);
        return false;
    }

    // 디렉토리가 맞는지 확인
    if (!S_ISDIR(dirStat.st_mode)) {
        printf("%s is not a directory\n", dirPath);
        return false;
    }

    // 절대 경로로 변환
    // 동시에 잘못된 경로인지 확인, 파일 이름의 최대 크기가 255를 초과하는지 확인
    if (realpath(fullPath, realDirPath) == NULL) {
        printf("error: invalid path, %s\n", dirPath);
        return false;
    }

    // 홈 디렉토리를 벗어나는지 확인
    char *homeDir = getHomeDir();
    if (!isSubDir(homeDir, realDirPath)) {
        printf("%s is outside the home directory\n", dirPath);
        return false;
    }

    // 읽기 권한이 있는지 확인
    if (access(realDirPath, R_OK) == -1) {
        printf("error: no read permission, %s\n", dirPath);
        return false;
    }

    return true;
}

// dirPath, outputPath, excludePathList 의 경로 유효성 검사
bool validateDaemonConfig(DaemonConfig *config) {
    char buffer[PATH_MAX];

    // <DIR_PATH> 유효성 검사
    if (!validateDaemonDirPath(config->dirPath, buffer)) {
        return false;
    }
    strcpy(config->dirPath, buffer);

    // <OUTPUT_PATH> 유효성 검사
    if (!validateDaemonDirPath(config->outputPath, buffer)) {
        return false;
    }
    strcpy(config->outputPath, buffer);

    // <OUTPUT_PATH> 가 <DIR_PATH> 의 하위 디렉토리인 경우
    if (isSubDir(config->dirPath, config->outputPath)) {
        printf("error: output path is inside the directory path\n");
        return false;
    }

    // <EXCLUDE_PATH> 유효성 검사 후 절대 경로로 변환
    for (int i = 0; i < config->excludePathCount; i++) {
        if (!validateDaemonDirPath(config->excludePathList[i], buffer)) {
            return false;
        }

        if (!isSubDir(config->dirPath, buffer)) {
            printf("error: exclude path is outside the directory path\n");
            return false;
        }

        free(config->excludePathList[i]);
        config->excludePathList[i] = strdup(buffer);
    }

    // <EXCLUDE_PATH> 에서 포함관계 있는 디렉토리 확인
    for (int i = 0; i < config->excludePathCount; i++) {
        for (int j = 0; j < config->excludePathCount; j++) {
            if (i == j)
                continue;

            if (isSubDir(config->excludePathList[i], config->excludePathList[j]) ||
                isSubDir(config->excludePathList[j], config->excludePathList[i])) {
                printf("error: exclude path is a superset of another exclude path, %s, %s\n",
                       config->excludePathList[i],
                       config->excludePathList[j]);
                return false;
            }
        }
    }

    return true;
}

// 디몬 설정을 파싱해서 DaemonConfig 에 저장
bool parseDaemonConfigOptions(DaemonConfig *config, int argc, char *args[]) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(args[i], "-d") == 0 && i + 1 < argc) {
            strcpy(config->outputPath, args[i + 1]);
            i++;
        } else if (strcmp(args[i], "-i") == 0 && i + 1 < argc) {
            // <TIME_INTERVAL> 파싱
            int previousTimeInterval = config->timeInterval;
            config->timeInterval = parseExactInt(args[i + 1], -1);
            if (config->timeInterval <= 0) {
                printf("error: nature number -i %s\n", args[i + 1]);
                config->timeInterval = previousTimeInterval;
                return false;
            }
            i++;
        } else if (strcmp(args[i], "-l") == 0 && i + 1 < argc) {
            // <MAX_LOG_LINES> 파싱
            int previousMaxLogLines = config->maxLogLines;
            config->maxLogLines = parseExactInt(args[i + 1], -1);
            if (config->maxLogLines <= 0) {
                printf("error: nature number -l %s\n", args[i + 1]);
                config->maxLogLines = previousMaxLogLines;
                return false;
            }
            i++;
        } else if (strcmp(args[i], "-x") == 0 && i + 1 < argc) {
            // <EXCLUDE_PATH> 파싱
            i++;
            config->excludePathCount = 0;
            while (i < argc && config->excludePathCount < MAX_EXCLUDE_PATH_COUNT && args[i][0] != '-') {
                config->excludePathList[config->excludePathCount++] = strdup(args[i++]);
            }
            i--; // 루프 종료후 1 더해지기에 다시 빼줌
        } else if (strcmp(args[i], "-e") == 0 && i + 1 < argc) {
            // <EXTENSION> 파싱
            i++;
            config->extensionCount = 0;
            while (i < argc && config->extensionCount < MAX_EXTENSION_COUNT && args[i][0] != '-') {
                config->extensionList[config->extensionCount++] = strdup(args[i++]);
            }
            i--; // 루프 종료후 1 더해지기에 다시 빼줌
        } else if (strcmp(args[i], "-m") == 0 && i + 1 < argc) {
            // <MODE> 파싱
            i++;
            if (strcmp(args[i], "1") == 0) {
                config->duplicatedFileMode = DUPLICATED_FILE_MODE_LATEST;
            } else if (strcmp(args[i], "2") == 0) {
                config->duplicatedFileMode = DUPLICATED_FILE_MODE_OLDEST;
            } else if (strcmp(args[i], "3") == 0) {
                config->duplicatedFileMode = DUPLICATED_FILE_MODE_IGNORE;
            } else {
                printf("error: invalid <MODE> %s\n", args[i]);
                return false;
            }
        } else {
            printf("error: invalid option %s\n", args[i]);
            return false;
        }
    }

    return true;
}

// DaemonConfig 에 동적할당된 메모리 해제
void freeDaemonConfigItems(DaemonConfig *config) {
    for (int i = 0; i < config->excludePathCount; i++) {
        free(config->excludePathList[i]);
    }
    for (int i = 0; i < config->extensionCount; i++) {
        free(config->extensionList[i]);
    }
}

// DaemonConfig 파일 경로 생성
void getDaemonConfigFilePath(DaemonConfig *config, char *buf) {
    snprintf(buf, PATH_MAX + 24, "%s/%s", config->dirPath, DAEMON_CONFIG_FILE_NAME);
}

// DaemonConfig 파일 열고 락 획득하기
FILE *openDaemonConfigFile(DaemonConfig *config, const char *mode) {
    char path[PATH_MAX];
    getDaemonConfigFilePath(config, path);

    FILE *file = fopen(path, mode);
    if (file == NULL) {
        return NULL;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fileno(file), F_SETLKW, &lock) == -1) {
        fclose(file);
        return NULL;
    }

    return file;
}

// DaemonConfig 파일 닫고 락 해제하기
void closeDaemonConfigFile(FILE *file) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(fileno(file), F_SETLKW, &lock) == -1) {
        fclose(file);
        return;
    }

    fclose(file);
}

// DaemonConfig 파일에서 토큰 읽기
int readToken(FILE *file, char *token, int size) {
    int length = 0;
    bool started = false;
    while (true) {
        int r = getc(file);
        if (r == EOF || r == ',' || r == '\n') {
            token[length] = '\0';
            return r;
        }

        if (r != ' ') {
            started = true;
        }

        if (started && length < size) {
            token[length] = (char)r;
            length++;
        }
    }
}

// DaemonConfig 파일 읽기
void readDaemonConfigFile(DaemonConfig *config) {
    FILE *file = openDaemonConfigFile(config, "r+");
    if (file == NULL) {
        perror("failed to open daemon config file");
        return;
    }

    char token[PATH_MAX];
    // 필드 이름 읽기
    while (fscanf(file, "%s", token) != EOF) {
        // 콜론 읽기
        char tmp[4];
        if (fscanf(file, "%s", tmp) != 1 || tmp[0] != ':') {
            break;
        }

        // 필드 이름 비교
        if (strcmp(token, "monitoring_path") == 0) {
            if (fscanf(file, "%s", config->dirPath) != 1) {
                break;
            }
        } else if (strcmp(token, "pid") == 0) {
            if (fscanf(file, "%s", token) != 1) {
                break;
            }

            config->pid = parseExactInt(token, 0);
        } else if (strcmp(token, "start_time") == 0) {
            char date[16];
            char time[16];
            if (fscanf(file, "%s %s", date, time) != 2) {
                break;
            }

            // tm 구조체 기본값 설정을 위해 미리 초기화
            struct tm tm;
            tm.tm_year = 100;
            tm.tm_mon = 0;
            tm.tm_mday = 1;
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;

            // yyyy-MM-dd 형식
            char* next;
            if ((next = strtok(date, "-")) != NULL) {
                tm.tm_year = parseExactInt(next, 2000) - 1900;
            }
            if ((next = strtok(NULL, "-")) != NULL) {
                tm.tm_mon = parseExactInt(next, 1) - 1;
            }
            if ((next = strtok(NULL, "-")) != NULL) {
                tm.tm_mday = parseExactInt(next, 1);
            }

            // hh:mm:ss 형식
            if ((next = strtok(time, ":")) != NULL) {
                tm.tm_hour = parseExactInt(next, 0);
            }
            if ((next = strtok(NULL, ":")) != NULL) {
                tm.tm_min = parseExactInt(next, 0);
            }
            if ((next = strtok(NULL, ":")) != NULL) {
                tm.tm_sec = parseExactInt(next, 0);
            }

            // 시작 시간 설정
            config->startTime = mktime(&tm);
        } else if (strcmp(token, "output_path") == 0) {
            if (fscanf(file, "%s", config->outputPath) != 1) {
                break;
            }
        } else if (strcmp(token, "time_interval") == 0) {
            if (fscanf(file, "%s", token) != 1) {
                break;
            }

            // time interval 기본값 10
            config->timeInterval = parseExactInt(token, 10);
        } else if (strcmp(token, "max_log_lines") == 0) {
            if (fscanf(file, "%s", token) != 1) {
                break;
            }
            // max_log_lines 기본값 -1, 무제한, none
            config->maxLogLines = parseExactInt(token, -1);
        } else if (strcmp(token, "exclude_path") == 0) {
            config->excludePathCount = 0;
            while (true) {
                int r = readToken(file, token, PATH_MAX); // 토큰 하나 읽기
                char trimmed[PATH_MAX];
                trim(trimmed, token);
                if (trimmed[0] != '\0') { // 빈 문자열이 아니라면 목록에 추가
                    config->excludePathList[config->excludePathCount] = strdup(trimmed);
                    config->excludePathCount++;
                }
                if (r == EOF || r == '\n') { // 파일 끝이거나 다음 라인이면 종료
                    break;
                }
            }

            // none 은 excludePath 가 하나도 없다는 의미
            if (config->excludePathCount == 1 && strcmp(config->excludePathList[0], "none") == 0) {
                for (int i = 0; i < config->excludePathCount; i++) {
                    free(config->excludePathList[i]);
                }
                config->excludePathCount = 0;
            }
        } else if (strcmp(token, "extension") == 0) {
            config->extensionCount = 0;
            while (true) {
                int r = readToken(file, token, PATH_MAX); // 토큰 하나 읽기
                char trimmed[PATH_MAX];
                trim(trimmed, token);
                if (trimmed[0] != '\0') { // 빈 문자열이 아니라면 목록에 추가
                    config->extensionList[config->extensionCount] = strdup(trimmed);
                    config->extensionCount++;
                }
                if (r == EOF || r == '\n') { // 파일 끝이거나 다음 라인이면 종료
                    break;
                }
            }

            // all 은 extension 가 하나도 없다는 의미
            if (config->extensionCount == 1 && strcmp(config->extensionList[0], "all") == 0) {
                for (int i = 0; i < config->extensionCount; i++) {
                    free(config->extensionList[i]);
                }
                config->extensionCount = 0;
            }
        } else if (strcmp(token, "mode") == 0) {
            if (fscanf(file, "%s", token) != 1) {
                break;
            }

            config->duplicatedFileMode = parseExactInt(token, DUPLICATED_FILE_MODE_LATEST);
        }
    }
    closeDaemonConfigFile(file);
}

// DaemonConfig 파일 쓰기
void writeDaemonConfigFile(DaemonConfig *config) {
    FILE *file = openDaemonConfigFile(config, "w+");
    if (file == NULL) {
        printf("error: failed to open daemon config file\n");
        return;
    }

    fprintf(file, "monitoring_path : %s\n", config->dirPath);
    fprintf(file, "pid : %d\n", config->pid);

    struct tm *startTime = localtime(&config->startTime);
    fprintf(file, "start_time : %d-%02d-%02d %02d:%02d:%02d\n", 
        startTime->tm_year + 1900, 
        startTime->tm_mon + 1, 
        startTime->tm_mday, 
        startTime->tm_hour, 
        startTime->tm_min, 
        startTime->tm_sec);

    fprintf(file, "output_path : %s\n", config->outputPath);
    fprintf(file, "time_interval : %d\n", config->timeInterval);

    fprintf(file, "max_log_lines : ");
    if (config->maxLogLines == -1) {
        fprintf(file, "none\n");
    } else {
        fprintf(file, "%d\n", config->maxLogLines);
    }

    fprintf(file, "exclude_path : ");
    if (config->excludePathCount == 0) {
        fprintf(file, "none");
    } else {
        for (int i = 0; i < config->excludePathCount; i++) {
            fprintf(file, "%s", config->excludePathList[i]);
            if (i < config->excludePathCount - 1) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    fprintf(file, "extension : ");
    if (config->extensionCount == 0) {
        fprintf(file, "all");
    } else {
        for (int i = 0; i < config->extensionCount; i++) {
            fprintf(file, "%s", config->extensionList[i]);
            if (i < config->extensionCount - 1) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    fprintf(file, "mode : %d\n", config->duplicatedFileMode);

    closeDaemonConfigFile(file);
}
