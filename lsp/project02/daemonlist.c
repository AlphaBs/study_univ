#include "daemonlist.h"
#include "daemon.h"
#include "shell.h"

#include <string.h>
#include <stdlib.h>

// ~/.ssu_cleanupd/current_daemon_list 경로 반환
void getCurrentDaemonListFilePath(char *path) {
    getDaemonDirPath(path);
    strcat(path, "/");
    strcat(path, CURRENT_DAEMON_LIST_FILE_NAME);
}

// 디몬 리스트 파일 열기
FILE *openCurrentDaemonListFile(const char* mode) {
    char path[PATH_MAX];
    getCurrentDaemonListFilePath(path); // 경로

    FILE *file = fopen(path, mode);
    return file;
}

// 디몬 리스트 읽기
DaemonList *readCurrentDaemonList(FILE *file) {
    rewind(file);

    DaemonList *list = malloc(sizeof(DaemonList)); // 디몬 리스트 생성
    list->dirPath = NULL;
    list->next = NULL;

    char buf[PATH_MAX];
    char trimmed[PATH_MAX];
    
    DaemonList* last = list; // 마지막 노드. 여기에 새로운 노드가 추가될 예정
    while (fgets(buf, PATH_MAX, file) != NULL) { // 한줄씩 읽기
        trim(trimmed, buf);
        if (trimmed[0] != '\0') { // 빈 줄은 무시
            DaemonList *newList = malloc(sizeof(DaemonList)); // 새 노드 생성
            newList->dirPath = strdup(trimmed);
            newList->next = NULL;
            last->next = newList; // 마지막 노드에 추가
            last = newList;
        }
    }

    return list;
}

// 디몬 리스트 쓰기
void writeCurrentDaemonList(FILE *file, DaemonList *list) {
    rewind(file);
    list = list->next;
    while (list != NULL) {
        fprintf(file, "%s\n", list->dirPath);
        list = list->next;
    }
}

// 디몬 리스트 메모리 해제
void freeCurrentDaemonList(DaemonList *list) {
    while (list != NULL) {
        if (list->dirPath != NULL) {
            free(list->dirPath);
        }

        DaemonList *next = list->next;
        free(list);
        list = next;
    }
}

// 디몬 리스트 파일 닫기
void closeCurrentDaemonListFile(FILE *file) {
    fclose(file);
}

// 인덱스 번호로 디몬 리스트의 노드 찾기
DaemonList *findDaemonListItemByIndex(DaemonList *list, int find) {
    DaemonList *current = list->next;
    int index = 0;
    while (current != NULL) { // 처음부터 끝까지 순회
        if (index == find) { // 발견
            break;
        }

        current = current->next;
        index++;
    }

    return current;
}

// 경로 이름으로 디몬 리스트의 노드 찾기
DaemonList *findDaemonListItemByDirPath(DaemonList *list, char *dirPath) {
    list = list->next;
    while (list != NULL) { // 처음부터 끝까지 순회
        if (strcmp(list->dirPath, dirPath) == 0) { // 발견
            return list;
        }
        list = list->next;
    }

    return NULL;
}

// 디몬 리스트에 노드 추가
void addDaemonListItem(DaemonList *list, char *dirPath) {
    DaemonList* newItem = malloc(sizeof(DaemonList)); // 새 노드 할당
    newItem->dirPath = strdup(dirPath);
    newItem->next = NULL;

    DaemonList* last = list; // 마지막 노드 찾기
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = newItem; // 마지막 노드에 추가
}

// 경로 이름으로 디몬 리스트의 노드 삭제
bool deleteDaemonListItemByDirPath(DaemonList *list, char *dirPath) {
    DaemonList *current = list->next;
    DaemonList *prev = list;
    bool result = false;
    while (current != NULL) { // 처음부터 끝까지 순회
        if (strcmp(current->dirPath, dirPath) == 0) { // 발견
            prev->next = current->next; // 링크 이어붙이기
            result = true;

            free(current->dirPath); // 메모리 해제 및 삭제
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }

    return result;
}