#include <stdio.h>      // 표준 입출력 함수
#include <stdlib.h>     // malloc, free, exit 등
#include <string.h>     // 문자열 처리 함수
#include <unistd.h>     // POSIX API (open, read, write 등)
#include <fcntl.h>      // 파일 열기 옵션 정의
#include <sys/stat.h>   // stat 구조체 및 파일 상태 확인 함수
#include <sys/types.h>  // 시스템 자료형
#include <dirent.h>     // 디렉토리 처리
#include <errno.h>      // 에러 코드
#include <time.h>       // 시간 관련 구조체
#include <utime.h>      // 파일 시간 설정
#include <getopt.h>     // 옵션 파싱
#include <libgen.h>     // basename 사용

#define BUFFER_SIZE 8192 // 버퍼 크기 정의
#define MAX_PATH 4096    // 경로 최대 길이

// 옵션 플래그 구조체 정의
typedef struct {
    int recursive;    // -r: 재귀적으로 복사
    int interactive;  // -i: 덮어쓰기 전 사용자에게 확인
    int force;        // -f: 강제로 덮어쓰기
    int verbose;      // -v: 상세 출력
    int preserve;     // -p: 권한/타임스탬프 유지
    int archive;      // -a: -r과 -p 포함
    int no_clobber;   // -n: 기존 파일 덮어쓰지 않음
    int update;       // -u: 더 오래된 파일만 덮어씀
} cp_options;

// 함수 선언
int copy_file(const char *src, const char *dst, cp_options *opts);
int copy_directory(const char *src, const char *dst, cp_options *opts);
int should_copy_file(const char *src, const char *dst, cp_options *opts);
void preserve_attributes(const char *src, const char *dst);
char *build_path(const char *dir, const char *file);
void print_usage(const char *prog_name);

int main(int argc, char *argv[]) {
    cp_options opts = {0}; // 옵션 기본값 초기화
    int opt;

    // 긴 옵션 정의
    static struct option long_options[] = {
        {"recursive", no_argument, 0, 'r'},
        {"interactive", no_argument, 0, 'i'},
        {"force", no_argument, 0, 'f'},
        {"verbose", no_argument, 0, 'v'},
        {"preserve", no_argument, 0, 'p'},
        {"archive", no_argument, 0, 'a'},
        {"no-clobber", no_argument, 0, 'n'},
        {"update", no_argument, 0, 'u'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // 옵션 파싱
    while ((opt = getopt_long(argc, argv, "rifvpanuh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'r': opts.recursive = 1; break;
            case 'i': opts.interactive = 1; break;
            case 'f': opts.force = 1; break;
            case 'v': opts.verbose = 1; break;
            case 'p': opts.preserve = 1; break;
            case 'a': opts.archive = 1; opts.recursive = 1; opts.preserve = 1; break;
            case 'n': opts.no_clobber = 1; break;
            case 'u': opts.update = 1; break;
            case 'h': print_usage(argv[0]); exit(0);
            default: print_usage(argv[0]); exit(1);
        }
    }

    // 복사할 소스와 대상 경로가 주어졌는지 확인
    if (optind >= argc - 1) {
        fprintf(stderr, "cp: 소스와 대상을 지정해야 합니다\n");
        print_usage(argv[0]);
        exit(1);
    }

    char *destination = argv[argc - 1];
    int num_sources = argc - optind - 1;

    // 여러 개의 소스를 복사할 경우 대상은 디렉토리여야 함
    if (num_sources > 1) {
        struct stat st;
        if (stat(destination, &st) != 0 || !S_ISDIR(st.st_mode)) {
            fprintf(stderr, "cp: 여러 소스를 복사할 때는 대상이 디렉토리여야 합니다\n");
            exit(1);
        }
    }

    // 각 소스에 대해 복사 수행
    for (int i = optind; i < argc - 1; i++) {
        char *source = argv[i];
        struct stat st;

        if (stat(source, &st) != 0) {
            fprintf(stderr, "cp: '%s': %s\n", source, strerror(errno));
            continue;
        }

        // 디렉토리인지 파일인지 확인
        if (S_ISDIR(st.st_mode)) {
            if (!opts.recursive && !opts.archive) {
                fprintf(stderr, "cp: '%s'는 디렉토리입니다 (-r 옵션을 사용하세요)\n", source);
                continue;
            }
            copy_directory(source, destination, &opts);
        } else {
            copy_file(source, destination, &opts);
        }
    }

    return 0;
}

// 일반 파일 복사 함수
int copy_file(const char *src, const char *dst, cp_options *opts) {
    struct stat src_stat, dst_stat;
    char *target_path;
    int src_fd, dst_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    // 소스 파일 정보 가져오기
    if (stat(src, &src_stat) != 0) {
        fprintf(stderr, "cp: '%s': %s\n", src, strerror(errno));
        return -1;
    }

    // 대상 경로 결정 (디렉토리인 경우 파일명 붙이기)
    if (stat(dst, &dst_stat) == 0 && S_ISDIR(dst_stat.st_mode)) {
        target_path = build_path(dst, basename((char*)src));
    } else {
        target_path = strdup(dst);
    }

    // 복사 조건 확인
    if (!should_copy_file(src, target_path, opts)) {
        free(target_path);
        return 0;
    }

    // 파일 열기
    src_fd = open(src, O_RDONLY);
    if (src_fd == -1) {
        fprintf(stderr, "cp: '%s': %s\n", src, strerror(errno));
        free(target_path);
        return -1;
    }

    dst_fd = open(target_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dst_fd == -1) {
        fprintf(stderr, "cp: '%s': %s\n", target_path, strerror(errno));
        close(src_fd);
        free(target_path);
        return -1;
    }

    // 파일 내용 복사
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "cp: 쓰기 오류 '%s': %s\n", target_path, strerror(errno));
            close(src_fd);
            close(dst_fd);
            free(target_path);
            return -1;
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr, "cp: 읽기 오류 '%s': %s\n", src, strerror(errno));
    }

    close(src_fd);
    close(dst_fd);

    // 속성 보존
    if (opts->preserve || opts->archive) {
        preserve_attributes(src, target_path);
    }

    // 상세 출력
    if (opts->verbose) {
        printf("'%s' -> '%s'\n", src, target_path);
    }

    free(target_path);
    return 0;
}

// 디렉토리 복사 함수
int copy_directory(const char *src, const char *dst, cp_options *opts) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char *src_path, *dst_path, *target_dir;

    // 대상 디렉토리 경로 생성
    if (stat(dst, &st) == 0 && S_ISDIR(st.st_mode)) {
        target_dir = build_path(dst, basename((char*)src));
    } else {
        target_dir = strdup(dst);
    }

    // 대상 디렉토리 생성
    if (mkdir(target_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "cp: 디렉토리 '%s' 생성 실패: %s\n", target_dir, strerror(errno));
        free(target_dir);
        return -1;
    }

    dir = opendir(src);
    if (!dir) {
        fprintf(stderr, "cp: '%s': %s\n", src, strerror(errno));
        free(target_dir);
        return -1;
    }

    // 하위 항목 재귀 복사
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        src_path = build_path(src, entry->d_name);
        dst_path = build_path(target_dir, entry->d_name);

        if (stat(src_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                copy_directory(src_path, dst_path, opts);
            } else {
                copy_file(src_path, dst_path, opts);
            }
        }

        free(src_path);
        free(dst_path);
    }

    closedir(dir);

    // 디렉토리 속성 보존
    if (opts->preserve || opts->archive) {
        preserve_attributes(src, target_dir);
    }

    if (opts->verbose) {
        printf("디렉토리 '%s' -> '%s'\n", src, target_dir);
    }

    free(target_dir);
    return 0;
}

// 복사 여부 결정 함수
int should_copy_file(const char *src, const char *dst, cp_options *opts) {
    struct stat src_stat, dst_stat;
    char response;

    // 대상 파일이 없으면 복사
    if (stat(dst, &dst_stat) != 0) return 1;

    // -n: 덮어쓰지 않음
    if (opts->no_clobber) {
        if (opts->verbose)
            printf("'%s'가 이미 존재하므로 덮어쓰지 않습니다\n", dst);
        return 0;
    }

    // -u: 대상이 오래된 경우에만
    if (opts->update) {
        if (stat(src, &src_stat) == 0) {
            if (src_stat.st_mtime > dst_stat.st_mtime) return 1;
            if (opts->verbose)
                printf("'%s'가 '%s'보다 최신이므로 덮어쓰지 않습니다\n", dst, src);
            return 0;
        }
    }

    // -i: 사용자에게 물어보기
    if (opts->interactive) {
        printf("'%s'를 덮어쓰시겠습니까? (y/n) ", dst);
        response = getchar();
        while (getchar() != '\n');
        return (response == 'y' || response == 'Y');
    }

    return 1;
}

// 권한, 시간 보존 함수
void preserve_attributes(const char *src, const char *dst) {
    struct stat st;
    struct utimbuf times;

    if (stat(src, &st) != 0) return;

    chmod(dst, st.st_mode); // 권한 복원
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;
    utime(dst, &times);     // 시간 복원
    // chown(dst, st.st_uid, st.st_gid); // 소유자 복원 (권한 필요)
}

// 경로 연결 함수 (dir + '/' + file)
char *build_path(const char *dir, const char *file) {
    char *path = malloc(strlen(dir) + strlen(file) + 2);
    if (!path) {
        fprintf(stderr, "cp: 메모리 할당 실패\n");
        exit(1);
    }
    strcpy(path, dir);
    if (path[strlen(path) - 1] != '/') strcat(path, "/");
    strcat(path, file);
    return path;
}

// 도움말 출력
void print_usage(const char *prog_name) {
    printf("사용법: %s [옵션]... 소스... 대상\n", prog_name);
    printf("파일과 디렉토리를 복사합니다.\n\n");
    printf("옵션:\n");
    printf("  -r, --recursive     디렉토리를 재귀적으로 복사\n");
    printf("  -i, --interactive   덮어쓰기 전에 사용자에게 확인\n");
    printf("  -f, --force         덮어쓰기를 강제로 수행\n");
    printf("  -v, --verbose       작업 진행 상황을 자세히 표시\n");
    printf("  -p, --preserve      원본 파일의 모드, 소유권, 타임스탬프 유지\n");
    printf("  -a, --archive       -r -p 옵션을 함께 사용한 것과 동일\n");
    printf("  -n, --no-clobber    기존 파일을 덮어쓰지 않음\n");
    printf("  -u, --update        대상 파일이 없거나 원본보다 오래된 경우에만 복사\n");
    printf("  -h, --help          이 도움말을 표시하고 종료\n");
}
