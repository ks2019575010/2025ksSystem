/*
 * 옵션:
 *   -f    존재하지 않는 파일을 무시하고 확인 메시지를 표시하지 않음
 *   -i    모든 삭제 전에 확인 메시지 표시
 *   -r    디렉토리와 그 내용을 재귀적으로 삭제
 *   -v    수행 중인 작업 설명
 *   -d    빈 디렉토리 삭제
 *   -h    도움말 메시지 표시
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>

/* 옵션 플래그 구조체 */
typedef struct {
    int force;          /* -f 옵션: 강제 삭제 */
    int interactive;    /* -i 옵션: 상호작용 모드 */
    int recursive;      /* -r 옵션: 재귀 삭제 */
    int verbose;        /* -v 옵션: 자세한 출력 */
    int dir_only;       /* -d 옵션: 빈 디렉토리 삭제 */
} options_t;

/* 함수 선언 */
void print_usage(const char *program_name);
int prompt_user(const char *message);
int remove_file(const char *path, const options_t *opts);
int remove_directory(const char *path, const options_t *opts);
int remove_directory_recursive(const char *path, const options_t *opts);
int is_directory(const char *path);

/**
 * 사용법을 출력합니다.
 * @param program_name 프로그램 이름
 */
void print_usage(const char *program_name) {
    printf("사용법: %s [옵션] 파일...\n", program_name);
    printf("파일 또는 디렉토리를 삭제합니다.\n\n");
    printf("옵션:\n");
    printf("  -f    존재하지 않는 파일을 무시하고 확인 메시지를 표시하지 않음\n");
    printf("  -i    모든 삭제 전에 확인 메시지 표시\n");
    printf("  -r    디렉토리와 그 내용을 재귀적으로 삭제\n");
    printf("  -v    수행 중인 작업 설명\n");
    printf("  -d    빈 디렉토리 삭제\n");
    printf("  -h    이 도움말 메시지 표시\n");
}

/**
 * 사용자에게 확인 메시지를 표시하고 응답을 받습니다.
 * @param message 표시할 메시지
 * @return 사용자가 'y' 또는 'Y'를 입력했으면 1, 아니면 0
 */
int prompt_user(const char *message) {
    char response[10];
    printf("%s (y/n): ", message);
    fflush(stdout);
    
    if (fgets(response, sizeof(response), stdin) != NULL) {
        return (response[0] == 'y' || response[0] == 'Y');
    }
    return 0;
}

/**
 * 경로가 디렉토리인지 확인합니다.
 * @param path 확인할 경로
 * @return 디렉토리이면 1, 아니면 0
 */
int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return 0;
}

/**
 * 단일 파일을 삭제합니다.
 * @param path 삭제할 파일 경로
 * @param opts 옵션 구조체
 * @return 성공하면 0, 실패하면 1
 */
int remove_file(const char *path, const options_t *opts) {
    struct stat st;
    
    /* 파일이 존재하는지 확인 */
    if (stat(path, &st) != 0) {
        if (!opts->force) {
            fprintf(stderr, "rm: %s: 그런 파일이나 디렉토리가 없습니다\n", path);
        }
        return 1;
    }
    
    /* 상호작용 모드에서 사용자에게 확인 */
    if (opts->interactive) {
        char message[256];
        snprintf(message, sizeof(message), "'%s' 삭제하시겠습니까?", path);
        if (!prompt_user(message)) {
            return 0;
        }
    }
    
    /* 파일 삭제 */
    if (unlink(path) == 0) {
        if (opts->verbose) {
            printf("'%s' 삭제됨\n", path);
        }
        return 0;
    } else {
        if (!opts->force) {
            fprintf(stderr, "rm: %s 삭제 불가: %s\n", path, strerror(errno));
        }
        return 1;
    }
}

/**
 * 디렉토리 내용을 재귀적으로 삭제합니다.
 * @param path 삭제할 디렉토리 경로
 * @param opts 옵션 구조체
 * @return 성공하면 0, 실패하면 1
 */
int remove_directory_recursive(const char *path, const options_t *opts) {
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    int result = 0;
    
    /* 디렉토리 열기 */
    dir = opendir(path);
    if (dir == NULL) {
        if (!opts->force) {
            fprintf(stderr, "rm: %s: 디렉토리를 열 수 없습니다: %s\n", path, strerror(errno));
        }
        return 1;
    }
    
    /* 디렉토리 내용 읽기 */
    while ((entry = readdir(dir)) != NULL) {
        /* 현재 디렉토리(.)와 부모 디렉토리(..) 건너뛰기 */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* 전체 경로 생성 */
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        /* 하위 항목 삭제 */
        if (is_directory(full_path)) {
            if (remove_directory_recursive(full_path, opts) != 0) {
                result = 1;
            }
        } else {
            if (remove_file(full_path, opts) != 0) {
                result = 1;
            }
        }
    }
    
    closedir(dir);
    
    /* 빈 디렉토리 삭제 */
    if (rmdir(path) == 0) {
        if (opts->verbose) {
            printf("'%s' 디렉토리 삭제됨\n", path);
        }
    } else {
        if (!opts->force) {
            fprintf(stderr, "rm: %s: 디렉토리 삭제 불가: %s\n", path, strerror(errno));
        }
        result = 1;
    }
    
    return result;
}

/**
 * 디렉토리를 삭제합니다.
 * @param path 삭제할 디렉토리 경로
 * @param opts 옵션 구조체
 * @return 성공하면 0, 실패하면 1
 */
int remove_directory(const char *path, const options_t *opts) {
    struct stat st;
    
    /* 경로가 존재하는지 확인 */
    if (stat(path, &st) != 0) {
        if (!opts->force) {
            fprintf(stderr, "rm: %s: 그런 파일이나 디렉토리가 없습니다\n", path);
        }
        return 1;
    }
    
    /* 디렉토리가 아니면 파일로 처리 */
    if (!S_ISDIR(st.st_mode)) {
        return remove_file(path, opts);
    }
    
    /* 빈 디렉토리 삭제 옵션 */
    if (opts->dir_only) {
        /* 상호작용 모드에서 사용자에게 확인 */
        if (opts->interactive) {
            char message[256];
            snprintf(message, sizeof(message), "'%s' 디렉토리를 삭제하시겠습니까?", path);
            if (!prompt_user(message)) {
                return 0;
            }
        }
        
        if (rmdir(path) == 0) {
            if (opts->verbose) {
                printf("'%s' 디렉토리 삭제됨\n", path);
            }
            return 0;
        } else {
            if (!opts->force) {
                fprintf(stderr, "rm: %s: 디렉토리 삭제 불가: %s\n", path, strerror(errno));
            }
            return 1;
        }
    }
    
    /* 재귀 삭제 옵션 */
    if (opts->recursive) {
        /* 상호작용 모드에서 사용자에게 확인 */
        if (opts->interactive) {
            char message[256];
            snprintf(message, sizeof(message), "'%s' 디렉토리와 그 내용을 삭제하시겠습니까?", path);
            if (!prompt_user(message)) {
                return 0;
            }
        }
        
        return remove_directory_recursive(path, opts);
    } else {
        if (!opts->force) {
            fprintf(stderr, "rm: %s: 디렉토리입니다 (-r 옵션을 사용하세요)\n", path);
        }
        return 1;
    }
}

/**
 * 메인 함수
 * @param argc 인자 개수
 * @param argv 인자 배열
 * @return 프로그램 종료 코드
 */
int main(int argc, char *argv[]) {
    options_t opts = {0}; /* 옵션 구조체 초기화 */
    int opt;
    int result = 0;
    
    /* 명령줄 옵션 파싱 */
    while ((opt = getopt(argc, argv, "firvdh")) != -1) {
        switch (opt) {
            case 'f':
                opts.force = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'r':
                opts.recursive = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'd':
                opts.dir_only = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    /* 삭제할 파일이 지정되지 않은 경우 */
    if (optind >= argc) {
        fprintf(stderr, "rm: 삭제할 파일이 지정되지 않았습니다\n");
        print_usage(argv[0]);
        return 1;
    }
    
    /* 각 파일/디렉토리 처리 */
    for (int i = optind; i < argc; i++) {
        if (is_directory(argv[i])) {
            if (remove_directory(argv[i], &opts) != 0) {
                result = 1;
            }
        } else {
            if (remove_file(argv[i], &opts) != 0) {
                result = 1;
            }
        }
    }
    
    return result;
}