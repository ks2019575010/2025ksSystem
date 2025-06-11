/*
 * mv 명령어 C 구현
 * 리눅스 mv 명령어의 주요 기능들을 C로 구현한 프로그램
 * 파일 이동, 이름 변경, 디렉토리 이동 등의 기능을 제공
 */

#include <stdio.h>      // printf, fprintf, fopen, fclose 등 표준 입출력 함수
#include <stdlib.h>     // malloc, free, exit 등 메모리 관리 및 시스템 함수
#include <string.h>     // strcmp, strlen, strdup 등 문자열 처리 함수
#include <unistd.h>     // access, unlink 등 POSIX 시스템 호출
#include <sys/stat.h>   // stat 구조체 및 파일 정보 관련 함수
#include <errno.h>      // 에러 코드 및 strerror 함수
#include <libgen.h>     // basename, dirname 등 경로 처리 함수
#include <time.h>       // 시간 관련 구조체 및 함수

/*
 * mv 명령어의 옵션들을 저장하는 구조체
 * 각 필드는 해당 옵션이 활성화되었는지를 나타내는 플래그
 */
typedef struct {
    int interactive;     // -i: 덮어쓰기 전 사용자에게 확인 요청
    int force;          // -f: 강제 실행 (interactive 옵션 무시)
    int verbose;        // -v: 자세한 출력 (이동 과정을 화면에 표시)
    int no_clobber;     // -n: 기존 파일을 덮어쓰지 않음
    int update;         // -u: 소스 파일이 대상 파일보다 새로운 경우에만 이동
} mv_options;

/*
 * 프로그램 사용법을 출력하는 함수
 * 매개변수: program_name - 실행된 프로그램의 이름 (argv[0])
 */
void print_usage(const char *program_name) {
    // 기본 사용법 출력
    printf("사용법: %s [옵션] 소스 대상\n", program_name);
    printf("       %s [옵션] 소스... 디렉토리\n", program_name);
    printf("\n옵션:\n");
    
    // 각 옵션의 설명 출력
    printf("  -i, --interactive    덮어쓰기 전 확인\n");
    printf("  -f, --force         강제 실행 (interactive 무시)\n");
    printf("  -v, --verbose       자세한 출력\n");
    printf("  -n, --no-clobber    기존 파일 덮어쓰지 않음\n");
    printf("  -u, --update        새로운 파일일 때만 이동\n");
    printf("  -h, --help          이 도움말 출력\n");
}

/*
 * 파일이 존재하는지 확인하는 함수
 * 매개변수: path - 확인할 파일 경로
 * 반환값: 파일이 존재하면 1, 존재하지 않으면 0
 */
int file_exists(const char *path) {
    struct stat st;  // 파일 정보를 저장할 구조체
    // stat() 함수로 파일 정보를 가져옴. 성공하면 0, 실패하면 -1 반환
    return stat(path, &st) == 0;
}

/*
 * 주어진 경로가 디렉토리인지 확인하는 함수
 * 매개변수: path - 확인할 경로
 * 반환값: 디렉토리이면 1, 아니면 0
 */
int is_directory(const char *path) {
    struct stat st;  // 파일 정보를 저장할 구조체
    
    // stat() 호출 실패 시 (파일이 존재하지 않음) 0 반환
    if (stat(path, &st) != 0) return 0;
    
    // S_ISDIR 매크로로 디렉토리 여부 확인
    // st_mode 필드에서 파일 타입 비트를 검사
    return S_ISDIR(st.st_mode);
}

/*
 * 소스 파일이 대상 파일보다 새로운지 확인하는 함수 (-u 옵션용)
 * 매개변수: src - 소스 파일 경로, dest - 대상 파일 경로
 * 반환값: 소스가 더 새로우면 1, 아니면 0
 */
int is_newer(const char *src, const char *dest) {
    struct stat src_stat, dest_stat;  // 각 파일의 정보를 저장할 구조체
    
    // 소스 파일 정보 가져오기 실패 시 0 반환
    if (stat(src, &src_stat) != 0) return 0;
    
    // 대상 파일이 존재하지 않으면 이동해야 함 (1 반환)
    if (stat(dest, &dest_stat) != 0) return 1;
    
    // 수정 시간(mtime) 비교
    // st_mtime은 마지막 수정 시간을 나타내는 time_t 타입
    return src_stat.st_mtime > dest_stat.st_mtime;
}

/*
 * 사용자에게 확인을 요청하는 함수 (-i 옵션용)
 * 매개변수: message - 사용자에게 표시할 메시지
 * 반환값: 사용자가 'y' 또는 'Y'를 입력하면 1, 아니면 0
 */
int ask_confirmation(const char *message) {
    // 메시지 출력 후 즉시 화면에 표시 (버퍼 플러시)
    printf("%s (y/n): ", message);
    fflush(stdout);
    
    char response[10];  // 사용자 입력을 저장할 버퍼
    
    // 표준 입력에서 한 줄 읽기
    if (fgets(response, sizeof(response), stdin) == NULL) {
        // 입력 읽기 실패 시 0 반환 (취소로 처리)
        return 0;
    }
    
    // 첫 번째 문자가 'y' 또는 'Y'인지 확인
    return (response[0] == 'y' || response[0] == 'Y');
}

/*
 * 소스 파일명을 대상 디렉토리와 결합하여 최종 경로를 생성하는 함수
 * 예: src="/home/user/file.txt", dest_dir="/tmp" -> "/tmp/file.txt"
 * 매개변수: src - 소스 파일 경로, dest_dir - 대상 디렉토리 경로
 * 반환값: 동적 할당된 최종 경로 문자열 (호출자가 free 해야 함)
 */
char* build_dest_path(const char *src, const char *dest_dir) {
    // strdup으로 소스 경로 복사 (basename이 원본을 수정할 수 있으므로)
    char *src_copy = strdup(src);
    
    // basename: 경로에서 파일명만 추출 (/path/to/file.txt -> file.txt)
    char *filename = basename(src_copy);
    
    // 최종 경로 길이 계산: 디렉토리 + '/' + 파일명 + '\0'
    size_t dest_len = strlen(dest_dir) + strlen(filename) + 2;
    char *dest_path = malloc(dest_len);  // 동적 메모리 할당
    
    // 경로 조합: "디렉토리/파일명" 형태로 생성
    snprintf(dest_path, dest_len, "%s/%s", dest_dir, filename);
    
    free(src_copy);  // 임시로 할당한 메모리 해제
    return dest_path;
}

/*
 * 실제 파일 이동을 수행하는 핵심 함수
 * 매개변수: src - 소스 파일 경로, dest - 대상 경로, opts - 옵션 구조체
 * 반환값: 성공하면 0, 실패하면 1
 */
int move_file(const char *src, const char *dest, mv_options *opts) {
    // 1단계: 소스 파일 존재 여부 확인
    if (!file_exists(src)) {
        fprintf(stderr, "mv: '%s': 파일이나 디렉토리가 없습니다\n", src);
        return 1;
    }
    
    // 2단계: 대상이 디렉토리인 경우 파일명을 추가하여 최종 경로 생성
    char *final_dest = NULL;  // 동적 할당될 경로를 저장할 포인터
    if (is_directory(dest)) {
        // 디렉토리 + 파일명 조합
        final_dest = build_dest_path(src, dest);
        dest = final_dest;  // dest 포인터를 새로운 경로로 변경
    }
    
    // 3단계: --no-clobber 옵션 확인
    // 이 옵션이 활성화되고 대상 파일이 존재하면 덮어쓰지 않음
    if (opts->no_clobber && file_exists(dest)) {
        if (opts->verbose) {
            printf("'%s': 기존 파일을 덮어쓰지 않습니다\n", dest);
        }
        free(final_dest);  // 할당된 메모리 해제
        return 0;
    }
    
    // 4단계: --update 옵션 확인
    // 대상 파일이 존재하고 소스 파일이 더 새롭지 않으면 건너뜀
    if (opts->update && file_exists(dest) && !is_newer(src, dest)) {
        if (opts->verbose) {
            printf("'%s': 더 새로운 파일이 아니므로 건너뜁니다\n", src);
        }
        free(final_dest);
        return 0;
    }
    
    // 5단계: --interactive 옵션 확인 (--force가 아닌 경우에만)
    // 대상 파일이 존재할 때 사용자에게 덮어쓸지 확인
    if (opts->interactive && !opts->force && file_exists(dest)) {
        char message[512];  // 확인 메시지를 저장할 버퍼
        snprintf(message, sizeof(message), "'%s'를 덮어쓰시겠습니까?", dest);
        if (!ask_confirmation(message)) {
            // 사용자가 거부한 경우
            free(final_dest);
            return 0;
        }
    }
    
    // 6단계: 파일 이동 시도 - 먼저 rename() 사용
    // rename()은 같은 파일시스템 내에서 효율적으로 이동/이름변경
    if (rename(src, dest) == 0) {
        // 이동 성공
        if (opts->verbose) {
            printf("'%s' -> '%s'\n", src, dest);
        }
        free(final_dest);
        return 0;
    }
    
    // 7단계: rename() 실패 시 복사 후 삭제 방식 사용
    // 다른 파일시스템 간 이동 시 필요한 방법
    FILE *src_file = fopen(src, "rb");  // 소스 파일을 읽기 모드로 열기
    if (!src_file) {
        fprintf(stderr, "mv: '%s'를 열 수 없습니다: %s\n", src, strerror(errno));
        free(final_dest);
        return 1;
    }
    
    FILE *dest_file = fopen(dest, "wb");  // 대상 파일을 쓰기 모드로 열기
    if (!dest_file) {
        fprintf(stderr, "mv: '%s'를 생성할 수 없습니다: %s\n", dest, strerror(errno));
        fclose(src_file);  // 열린 파일 닫기
        free(final_dest);
        return 1;
    }
    
    // 8단계: 파일 내용 복사
    char buffer[8192];      // 8KB 버퍼 (효율적인 I/O를 위한 크기)
    size_t bytes;           // 읽은/쓴 바이트 수
    int copy_success = 1;   // 복사 성공 여부 플래그
    
    // 소스 파일에서 버퍼 크기만큼 반복해서 읽고 대상 파일에 쓰기
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        // 읽은 바이트 수만큼 대상 파일에 쓰기
        if (fwrite(buffer, 1, bytes, dest_file) != bytes) {
            // 쓰기 실패 시 루프 종료
            copy_success = 0;
            break;
        }
    }
    
    // 파일 닫기
    fclose(src_file);
    fclose(dest_file);
    
    // 9단계: 복사 결과 확인
    if (!copy_success) {
        fprintf(stderr, "mv: '%s'에서 '%s'로 복사 중 오류 발생\n", src, dest);
        remove(dest);  // 불완전한 대상 파일 삭제
        free(final_dest);
        return 1;
    }
    
    // 10단계: 복사 성공 시 원본 파일 삭제
    if (remove(src) != 0) {
        fprintf(stderr, "mv: '%s'를 삭제할 수 없습니다: %s\n", src, strerror(errno));
        free(final_dest);
        return 1;
    }
    
    // 성공 메시지 출력 (verbose 모드)
    if (opts->verbose) {
        printf("'%s' -> '%s'\n", src, dest);
    }
    
    free(final_dest);  // 동적 할당된 메모리 해제
    return 0;
}

/*
 * 명령줄 인수를 파싱하여 옵션을 추출하는 함수
 * 매개변수: 
 *   argc - 명령줄 인수 개수
 *   argv - 명령줄 인수 배열
 *   opts - 파싱된 옵션을 저장할 구조체
 *   first_file_idx - 첫 번째 파일 인수의 인덱스를 저장할 포인터
 * 반환값: 성공 시 0, 도움말 출력 시 -1, 오류 시 1
 */
int parse_options(int argc, char *argv[], mv_options *opts, int *first_file_idx) {
    // 옵션 구조체를 0으로 초기화 (모든 옵션을 비활성화 상태로 설정)
    memset(opts, 0, sizeof(mv_options));
    
    int i;
    // 명령줄 인수를 순회하면서 옵션 파싱
    for (i = 1; i < argc; i++) {
        // '-'로 시작하지 않으면 옵션이 아닌 파일 인수
        if (argv[i][0] != '-') {
            break; // 첫 번째 파일 인수 발견 시 루프 종료
        }
        
        // 긴 형태의 옵션들 처리
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            opts->interactive = 1;
        }
        else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
            opts->force = 1;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            opts->verbose = 1;
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-clobber") == 0) {
            opts->no_clobber = 1;
        }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0) {
            opts->update = 1;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // 도움말 출력 후 -1 반환 (정상 종료를 위한 특별한 값)
            print_usage(argv[0]);
            return -1;
        }
        else if (argv[i][1] != '\0') {
            // 단일 문자 옵션들을 조합한 경우 처리 (예: -iv, -fn 등)
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'i': opts->interactive = 1; break;
                    case 'f': opts->force = 1; break;
                    case 'v': opts->verbose = 1; break;
                    case 'n': opts->no_clobber = 1; break;
                    case 'u': opts->update = 1; break;
                    default:
                        // 인식되지 않는 옵션
                        fprintf(stderr, "mv: 잘못된 옵션 -- '%c'\n", argv[i][j]);
                        return 1;
                }
            }
        }
    }
    
    // 첫 번째 파일 인수의 인덱스 저장
    *first_file_idx = i;
    return 0;  // 성공
}

/*
 * 프로그램의 메인 함수
 * 명령줄 인수를 처리하고 파일 이동 작업을 수행
 */
int main(int argc, char *argv[]) {
    mv_options opts;        // 옵션 저장 구조체
    int first_file_idx;     // 첫 번째 파일 인수의 인덱스
    int result = 0;         // 최종 반환값 (0: 성공, 1: 실패)
    
    // 1단계: 명령줄 옵션 파싱
    int parse_result = parse_options(argc, argv, &opts, &first_file_idx);
    if (parse_result == -1) {
        // 도움말 출력 후 정상 종료
        return 0;
    }
    if (parse_result != 0) {
        // 옵션 파싱 오류 시 비정상 종료
        return 1;
    }
    
    // 2단계: 인수 개수 확인
    // 최소한 소스와 대상 2개의 인수가 필요
    int file_count = argc - first_file_idx;
    if (file_count < 2) {
        fprintf(stderr, "mv: 인수가 부족합니다\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // 3단계: 대상 인수 추출
    // 마지막 인수가 대상 (파일 또는 디렉토리)
    const char *dest = argv[argc - 1];
    
    // 4단계: 다중 파일 이동 시 대상 디렉토리 확인
    // 여러 파일을 이동하는 경우 대상은 반드시 디렉토리여야 함
    if (file_count > 2 && !is_directory(dest)) {
        fprintf(stderr, "mv: 대상 '%s'이 디렉토리가 아닙니다\n", dest);
        return 1;
    }
    
    // 5단계: 각 소스 파일에 대해 이동 작업 수행
    // first_file_idx부터 마지막 인수 전까지 순회
    for (int i = first_file_idx; i < argc - 1; i++) {
        // 개별 파일 이동 시도
        if (move_file(argv[i], dest, &opts) != 0) {
            // 하나라도 실패하면 결과를 1로 설정
            // 하지만 다른 파일들은 계속 처리
            result = 1;
        }
    }
    
    // 6단계: 최종 결과 반환
    // 0: 모든 파일 이동 성공
    // 1: 하나 이상의 파일 이동 실패
    return result;
}