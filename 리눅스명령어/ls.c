#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    const char *path;

    // 경로 인자가 없으면 현재 디렉토리를 사용
    if (argc < 2) {
        path = ".";
    } else {
        path = argv[1];
    }

    // 디렉토리 열기
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // 디렉토리 내 항목 읽기
    while ((entry = readdir(dir)) != NULL) {
        // 숨김 파일(예: .git, .bashrc)은 제외
        if (entry->d_name[0] != '.') {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return 0;
}
