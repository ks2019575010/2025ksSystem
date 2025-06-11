

#include <stdio.h>      // 표준 입출력 함수들 (printf, fprintf 등)
#include <stdlib.h>     // 메모리 할당, 프로그램 종료 함수들 (malloc, exit 등)
#include <string.h>     // 문자열 처리 함수들 (strcmp, strcpy, strlen 등)
#include <sys/stat.h>   // 파일 상태 및 권한 관련 함수들 (mkdir, stat 등)
#include <sys/types.h>  // 시스템 데이터 타입 정의
#include <unistd.h>     // POSIX 운영체제 API (access, getopt 등)
#include <errno.h>      // 에러 코드 및 에러 처리 관련
#include <getopt.h>     // 명령줄 옵션 파싱을 위한 함수들

/* 프로그램 옵션을 저장하는 구조체 */
typedef struct {
    int parents;        // -p 옵션: 부모 디렉토리 자동 생성 여부
    int verbose;        // -v 옵션: 상세 출력 모드 여부
    char *mode;         // -m 옵션: 권한 설정 문자열 (예: "755")
    int help_mode;      // --help-mode 옵션: 권한 도움말 표시 여부
} options_t;

/**
 * 프로그램 사용법을 출력하는 함수
 */
void print_usage(const char *program_name) {
    printf("사용법: %s [옵션] 디렉토리...\n", program_name);
    printf("하나 이상의 디렉토리를 생성합니다.\n\n");
    printf("옵션:\n");
    printf("  -p, --parents     필요한 경우 부모 디렉토리를 생성하고,\n");
    printf("                    이미 존재하는 디렉토리에 대해 오류를 보고하지 않음\n");
    printf("  -m, --mode=MODE   디렉토리 권한을 MODE로 설정 (예: 755)\n");
    printf("  -v, --verbose     생성된 각 디렉토리에 대해 메시지 출력\n");
    printf("  -h, --help        이 도움말을 표시하고 종료\n");
    printf("      --help-mode   권한 모드 설정에 대한 상세 도움말 표시\n");
    
}

/**
 * 권한 모드 설정에 대한 상세 도움말을 출력하는 함수
 */
void print_mode_help(void) {
    printf("권한 모드 설정 도움말:\n\n");
    printf("  -m 옵션은 생성할 디렉토리의 권한을 설정합니다.\n");
    printf("  권한은 세 자리 또는 네 자리 8진수 숫자로 지정할 수 있습니다.\n\n");
    
    printf("  일반적인 권한 설정 예:\n");
    printf("    755  소유자: 읽기+쓰기+실행, 그룹: 읽기+실행, 기타: 읽기+실행\n");
    printf("    750  소유자: 읽기+쓰기+실행, 그룹: 읽기+실행, 기타: 권한없음\n");
    printf("    700  소유자: 읽기+쓰기+실행, 그룹: 권한없음, 기타: 권한없음\n");
    printf("    644  소유자: 읽기+쓰기, 그룹: 읽기, 기타: 읽기\n\n");
    
    printf("  각 자리 숫자는 다음과 같은 권한을 의미합니다:\n");
    printf("    4 = 읽기 권한 (r)\n");
    printf("    2 = 쓰기 권한 (w)\n");
    printf("    1 = 실행 권한 (x)\n");
    printf("  숫자를 더하여 원하는 권한 조합을 만듭니다.\n\n");
    
    printf("  특수 권한 (네 자리 사용시):\n");
    printf("    첫 번째 자리는 특수 권한을 나타냅니다:\n");
    printf("    4 = setuid 비트\n");
    printf("    2 = setgid 비트\n");
    printf("    1 = sticky 비트\n");
}

/**
 * 8진수 문자열을 정수로 변환하는 함수
 * 
 * @param mode_str 8진수 권한 문자열 (예: "755")
 * @return 변환된 권한 값, 실패시 -1 반환
 */
mode_t parse_mode(const char *mode_str) {
    if (!mode_str || strlen(mode_str) == 0) {
        return -1;  // 잘못된 입력
    }
    
    char *endptr;
    long mode_val = strtol(mode_str, &endptr, 8);  // 8진수로 변환
    
    // 변환 실패 또는 잘못된 문자가 포함된 경우
    if (*endptr != '\0' || mode_val < 0 || mode_val > 07777) {
        return -1;
    }
    
    return (mode_t)mode_val;
}

/**
 * 부모 디렉토리들을 재귀적으로 생성하는 함수
 * 
 * @param path 생성할 디렉토리의 전체 경로
 * @param mode 디렉토리에 설정할 권한
 * @param verbose 상세 출력 모드 여부
 * @return 성공시 0, 실패시 -1 반환
 */
int create_parent_dirs(const char *path, mode_t mode, int verbose) {
    char *path_copy = strdup(path);  // 경로 문자열 복사 (원본 보존)
    if (!path_copy) {
        perror("메모리 할당 실패");
        return -1;
    }
    
    // 경로를 역순으로 탐색하여 존재하지 않는 부모 디렉토리들을 찾음
    for (char *p = path_copy + strlen(path_copy) - 1; p > path_copy; p--) {
        if (*p == '/') {  // 디렉토리 구분자 발견
            *p = '\0';    // 경로를 해당 지점에서 자름
            
            // 해당 디렉토리가 존재하는지 확인
            struct stat st;
            if (stat(path_copy, &st) == 0) {
                // 디렉토리가 존재하면 더 이상 부모를 생성할 필요 없음
                break;
            }
            
            // 부모 디렉토리들을 재귀적으로 생성
            if (create_parent_dirs(path_copy, mode, verbose) != 0) {
                free(path_copy);
                return -1;
            }
            break;
        }
    }
    
    free(path_copy);
    
    // 현재 디렉토리 생성 시도
    if (mkdir(path, mode) != 0) {
        if (errno != EEXIST) {  // 이미 존재하는 경우는 오류가 아님
            return -1;
        }
    } else {
        // 생성 성공시 verbose 모드에서 메시지 출력
        if (verbose) {
            printf("mkdir: 디렉토리 '%s' 생성됨\n", path);
        }
    }
    
    return 0;
}

/**
 * 단일 디렉토리를 생성하는 함수
 * 
 * @param path 생성할 디렉토리 경로
 * @param opts 프로그램 옵션 구조체
 * @return 성공시 0, 실패시 -1 반환
 */
int create_directory(const char *path, const options_t *opts) {
    // 기본 권한 설정 (기본값: 0755)
    mode_t mode = 0755;
    
    // -m 옵션이 지정된 경우 권한 파싱
    if (opts->mode) {
        mode_t parsed_mode = parse_mode(opts->mode);
        if (parsed_mode == (mode_t)-1) {
            fprintf(stderr, "mkdir: 잘못된 권한 모드 '%s'\n", opts->mode);
            return -1;
        }
        mode = parsed_mode;
    }
    
    int result;
    
    if (opts->parents) {
        // -p 옵션: 부모 디렉토리까지 함께 생성
        result = create_parent_dirs(path, mode, opts->verbose);
        if (result != 0 && errno != EEXIST) {
            perror(path);
            return -1;
        }
    } else {
        // 일반 모드: 단일 디렉토리만 생성
        result = mkdir(path, mode);
        if (result != 0) {
            perror(path);
            return -1;
        }
        
        // 생성 성공시 verbose 모드에서 메시지 출력
        if (opts->verbose) {
            printf("mkdir: 디렉토리 '%s' 생성됨\n", path);
        }
    }
    
    return 0;
}

/**
 * 명령줄 옵션을 파싱하는 함수
 * 
 * @param argc 명령줄 인수 개수
 * @param argv 명령줄 인수 배열
 * @param opts 파싱된 옵션을 저장할 구조체
 * @return 파싱된 옵션의 다음 인덱스
 */
int parse_options(int argc, char *argv[], options_t *opts) {
    // 옵션 구조체 초기화
    opts->parents = 0;
    opts->verbose = 0;
    opts->mode = NULL;
    opts->help_mode = 0;
    
    // getopt_long을 위한 긴 옵션 정의
    static struct option long_options[] = {
        {"parents",   no_argument,       0, 'p'},
        {"verbose",   no_argument,       0, 'v'},
        {"mode",      required_argument, 0, 'm'},
        {"help",      no_argument,       0, 'h'},
        {"help-mode", no_argument,       0, 1000},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    // 명령줄 옵션 파싱 루프
    while ((opt = getopt_long(argc, argv, "pvm:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':  // -p, --parents 옵션
                opts->parents = 1;
                break;
                
            case 'v':  // -v, --verbose 옵션
                opts->verbose = 1;
                break;
                
            case 'm':  // -m, --mode 옵션
                opts->mode = optarg;
                break;
                
            case 'h':  // -h, --help 옵션
                print_usage(argv[0]);
                exit(0);
                break;
                
            case 1000:  // --help-mode 옵션
                opts->help_mode = 1;
                break;
                
            default:
                fprintf(stderr, "예상치 못한 옵션: %c\n", opt);
                exit(1);
        }
    }
    
    return optind;  // 파싱되지 않은 첫 번째 인수의 인덱스 반환
}

/**
 * 메인 함수
 * 
 * @param argc 명령줄 인수 개수
 * @param argv 명령줄 인수 배열
 * @return 프로그램 종료 코드 (성공: 0, 실패: 1)
 */
int main(int argc, char *argv[]) {
    options_t opts;
    
    // 명령줄 옵션 파싱
    int first_dir_index = parse_options(argc, argv, &opts);
    
    // --help-mode 옵션 처리
    if (opts.help_mode) {
        print_mode_help();
        return 0;
    }
    
    // 디렉토리 인수가 없는 경우 오류 처리
    if (first_dir_index >= argc) {
        fprintf(stderr, "mkdir: 디렉토리 인수가 필요합니다\n");
        fprintf(stderr, "사용법에 대한 정보를 보려면 '%s --help'를 사용하세요.\n", argv[0]);
        return 1;
    }
    
    int exit_code = 0;  // 프로그램 종료 코드
    
    // 지정된 모든 디렉토리에 대해 생성 작업 수행
    for (int i = first_dir_index; i < argc; i++) {
        if (create_directory(argv[i], &opts) != 0) {
            exit_code = 1;  // 하나라도 실패하면 종료 코드를 1로 설정
        }
    }
    
    return exit_code;
}