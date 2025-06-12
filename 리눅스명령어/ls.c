#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>  // getopt

void print_help(const char *progname) {
    printf("사용법: %s [옵션] [디렉토리]\n", progname);
    printf("옵션:\n");
    printf("  -a    숨김 파일도 출력\n");
    printf("  -h    도움말 출력\n");
}

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    const char *path = ".";
    int show_all = 0;

    // 옵션 처리
    int opt;
    while ((opt = getopt(argc, argv, "ah")) != -1) {
        switch (opt) {
            case 'a':
                show_all = 1;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                print_help(argv[0]);
                return 1;
        }
    }

    // 남은 인자가 있다면 디렉토리 경로로 설정
    if (optind < argc) {
        path = argv[optind];
    }

    // 디렉토리 열기
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // 항목 출력
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') {
            continue;  // 숨김 파일 제외
        }
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}
