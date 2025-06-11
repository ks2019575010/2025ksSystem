/*
 * 파일이나 디렉토리의 접근 권한을 변경하는 기능을 제공
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>

/* 프로그램 버전 정보 */
#define VERSION "1.0.0"

/* 옵션 플래그를 저장하는 구조체 */
typedef struct {
    int recursive;  /* -R 옵션: 재귀적 처리 */
    int verbose;    /* -v 옵션: 상세 출력 */
    int silent;     /* -f 옵션: 조용한 모드 */
    int changes;    /* -c 옵션: 변경사항만 출력 */
} options_t;

/* 전역 옵션 변수 */
options_t g_options = {0, 0, 0, 0};

/*
 * 함수: show_help
 * 목적: 사용법과 옵션에 대한 도움말을 출력
 */
void show_help(void) {
    printf("사용법: chmod [옵션] 모드 파일...\n");
    printf("파일에 대한 모드(권한)를 변경합니다.\n\n");
    printf("모드는 8진수 숫자로 지정합니다 (예: 755 = rwxr-xr-x):\n");
    printf("  4 = 읽기 권한 (r)\n");
    printf("  2 = 쓰기 권한 (w)\n");
    printf("  1 = 실행 권한 (x)\n");
    printf("  각 자리수는 소유자, 그룹, 기타 사용자 순서입니다.\n\n");
    printf("옵션:\n");
    printf("  -R, --recursive   디렉토리 내 모든 파일과 하위 디렉토리에 재귀적으로 적용\n");
    printf("  -v, --verbose     처리되는 각 파일에 대한 진단 정보 출력\n");
    printf("  -f, --silent      오류 메시지 대부분을 표시하지 않음\n");
    printf("  -c, --changes     변경된 경우에만 상세 정보 제공\n");
    printf("  -h, --help        이 도움말 표시 후 종료\n");
    printf("      --version     버전 정보 표시 후 종료\n\n");
    printf("예시:\n");
    printf("  chmod 755 myfile       # 소유자에게 모든 권한, 그룹과 기타에게 읽기+실행\n");
    printf("  chmod -R 644 mydir     # 디렉토리 내 모든 파일을 644로 변경\n");
    printf("  chmod -v 600 secret    # 권한 변경 과정을 자세히 출력\n");
}

/*
 * 함수: show_version
 * 목적: 프로그램 버전 정보를 출력
 */
void show_version(void) {
    printf("chmod %s - C 구현\n", VERSION);
    printf("Copyright (C) 2025 - chmod C implementation\n");
}

/*
 * 함수: mode_to_string
 * 목적: 숫자 모드를 문자열 형태로 변환 (예: 0755 -> "rwxr-xr-x")
 * 매개변수: mode - 변환할 권한 모드
 * 반환값: 변환된 권한 문자열 (정적 버퍼 사용)
 */
char* mode_to_string(mode_t mode) {
    static char str[10];
    char perms[8][4] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
    
    /* 각 권한 비트를 문자열로 변환 */
    sprintf(str, "%s%s%s",
            perms[(mode >> 6) & 7],  /* 소유자 권한 */
            perms[(mode >> 3) & 7],  /* 그룹 권한 */
            perms[mode & 7]);        /* 기타 권한 */
    
    return str;
}

/*
 * 함수: parse_mode
 * 목적: 문자열 모드를 숫자로 변환 (예: "755" -> 0755)
 * 매개변수: mode_str - 변환할 모드 문자열
 * 반환값: 변환된 모드 값, 실패시 -1
 */
mode_t parse_mode(const char* mode_str) {
    char* endptr;
    long mode;
    
    /* 문자열이 NULL이거나 비어있는지 확인 */
    if (!mode_str || *mode_str == '\0') {
        return -1;
    }
    
    /* 8진수로 변환 시도 */
    mode = strtol(mode_str, &endptr, 8);
    
    /* 변환 실패 또는 유효하지 않은 문자가 있는 경우 */
    if (*endptr != '\0' || mode < 0 || mode > 07777) {
        return -1;
    }
    
    return (mode_t)mode;
}

/*
 * 함수: change_file_mode
 * 목적: 단일 파일의 권한을 변경하고 결과를 출력
 * 매개변수: filepath - 권한을 변경할 파일 경로
 *          new_mode - 새로운 권한 모드
 * 반환값: 성공시 0, 실패시 -1
 */
int change_file_mode(const char* filepath, mode_t new_mode) {
    struct stat st;
    mode_t old_mode;
    
    /* 파일 정보 가져오기 */
    if (stat(filepath, &st) == -1) {
        if (!g_options.silent) {
            fprintf(stderr, "chmod: '%s' 파일 정보를 가져올 수 없습니다: %s\n", 
                    filepath, strerror(errno));
        }
        return -1;
    }
    
    old_mode = st.st_mode & 07777;  /* 권한 비트만 추출 */
    
    /* 파일 권한 변경 */
    if (chmod(filepath, new_mode) == -1) {
        if (!g_options.silent) {
            fprintf(stderr, "chmod: '%s' 권한을 변경할 수 없습니다: %s\n", 
                    filepath, strerror(errno));
        }
        return -1;
    }
    
    /* 출력 옵션에 따른 메시지 표시 */
    if (g_options.verbose || (g_options.changes && old_mode != new_mode)) {
        printf("권한을 '%s'에서 '%s'로 변경: %s\n", 
               mode_to_string(old_mode), 
               mode_to_string(new_mode), 
               filepath);
    }
    
    return 0;
}

/*
 * 함수: process_directory
 * 목적: 디렉토리를 재귀적으로 처리
 * 매개변수: dir_path - 처리할 디렉토리 경로
 *          new_mode - 적용할 권한 모드
 * 반환값: 성공한 파일 수
 */
int process_directory(const char* dir_path, mode_t new_mode) {
    DIR* dir;
    struct dirent* entry;
    char full_path[1024];
    int success_count = 0;
    struct stat st;
    
    /* 디렉토리 열기 */
    dir = opendir(dir_path);
    if (!dir) {
        if (!g_options.silent) {
            fprintf(stderr, "chmod: '%s' 디렉토리를 열 수 없습니다: %s\n", 
                    dir_path, strerror(errno));
        }
        return 0;
    }
    
    /* 디렉토리 내 모든 항목을 순회 */
    while ((entry = readdir(dir)) != NULL) {
        /* 현재 디렉토리(.)와 상위 디렉토리(..) 건너뛰기 */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* 전체 경로 구성 */
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        /* 파일 권한 변경 시도 */
        if (change_file_mode(full_path, new_mode) == 0) {
            success_count++;
        }
        
        /* 하위 디렉토리인 경우 재귀적으로 처리 */
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            success_count += process_directory(full_path, new_mode);
        }
    }
    
    closedir(dir);
    return success_count;
}

/*
 * 함수: process_file
 * 목적: 파일 또는 디렉토리를 처리
 * 매개변수: filepath - 처리할 파일/디렉토리 경로
 *          new_mode - 적용할 권한 모드
 * 반환값: 성공시 0, 실패시 -1
 */
int process_file(const char* filepath, mode_t new_mode) {
    struct stat st;
    
    /* 파일 권한 변경 */
    if (change_file_mode(filepath, new_mode) == -1) {
        return -1;
    }
    
    /* 재귀 옵션이 설정되고 대상이 디렉토리인 경우 */
    if (g_options.recursive && stat(filepath, &st) == 0 && S_ISDIR(st.st_mode)) {
        process_directory(filepath, new_mode);
    }
    
    return 0;
}

/*
 * 함수: main
 * 목적: 프로그램의 진입점, 명령행 인수 처리 및 주요 로직 실행
 * 매개변수: argc - 인수 개수
 *          argv - 인수 배열
 * 반환값: 프로그램 종료 코드 (0: 성공, 1: 실패)
 */
int main(int argc, char* argv[]) {
    int opt;
    mode_t new_mode;
    int success_count = 0;
    int total_files = 0;
    
    /* getopt_long을 위한 긴 옵션 정의 */
    static struct option long_options[] = {
        {"recursive", no_argument, 0, 'R'},
        {"verbose",   no_argument, 0, 'v'},
        {"silent",    no_argument, 0, 'f'},
        {"changes",   no_argument, 0, 'c'},
        {"help",      no_argument, 0, 'h'},
        {"version",   no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    /* 명령행 옵션 파싱 */
    while ((opt = getopt_long(argc, argv, "Rvfch", long_options, NULL)) != -1) {
        switch (opt) {
            case 'R':
                g_options.recursive = 1;
                break;
            case 'v':
                g_options.verbose = 1;
                break;
            case 'f':
                g_options.silent = 1;
                break;
            case 'c':
                g_options.changes = 1;
                break;
            case 'h':
                show_help();
                return 0;
            case 'V':
                show_version();
                return 0;
            case '?':
                /* getopt_long이 자동으로 오류 메시지를 출력함 */
                fprintf(stderr, "자세한 정보는 'chmod --help'를 입력하세요.\n");
                return 1;
            default:
                fprintf(stderr, "chmod: 내부 오류\n");
                return 1;
        }
    }
    
    /* 최소 두 개의 인수(모드, 파일)가 필요함 */
    if (optind >= argc - 1) {
        fprintf(stderr, "chmod: 누락된 피연산자\n");
        fprintf(stderr, "자세한 정보는 'chmod --help'를 입력하세요.\n");
        return 1;
    }
    
    /* 모드 파싱 */
    new_mode = parse_mode(argv[optind]);
    if (new_mode == (mode_t)-1) {
        fprintf(stderr, "chmod: 잘못된 모드: '%s'\n", argv[optind]);
        return 1;
    }
    
    /* 모드 다음 인수부터 파일 목록 시작 */
    optind++;
    
    /* 각 파일에 대해 권한 변경 실행 */
    for (int i = optind; i < argc; i++) {
        total_files++;
        if (process_file(argv[i], new_mode) == 0) {
            success_count++;
        }
    }
    
    /* 결과 출력 (verbose 모드에서만) */
    if (g_options.verbose && total_files > 1) {
        printf("총 %d개 파일 중 %d개 파일 처리 완료\n", total_files, success_count);
    }
    
    /* 모든 파일이 성공적으로 처리되었으면 0, 아니면 1 반환 */
    return (success_count == total_files) ? 0 : 1;
}