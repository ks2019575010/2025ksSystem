#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h> // 파일 제어 (open, O_CREAT 등)를 위한 헤더
#include <unistd.h>
#include <utime.h> // 파일의 접근/수정 시간 설정 함수 utime()
#include <string.h>
#include <errno.h>
#include <getopt.h> //getopt_long() 사용

/// 전역 변수: 옵션 상태를 저장
int no_create = 0;      // --no-create 옵션이 설정되었는지 여부
int verbose = 0;        // -v (verbose) 옵션이 설정되었는지 여부

// 도움말 출력 함수
void print_help(const char *prog_name) {
    printf("사용법: %s [옵션] <파일들>\n", prog_name);
    printf("옵션:\n");
    printf("  --no-create    파일이 없으면 생성하지 않음\n");
    printf("  -v             처리한 파일 이름 출력 (verbose)\n");
    printf("  -h             도움말 출력\n");
}

// 파일을 생성하거나 시간 정보를 갱신하는 함수
int touch_file(const char *filename) {
    int fd;

    // 파일 존재 여부 확인
    if (access(filename, F_OK) == -1) {
        // 파일이 존재하지 않음
        if (no_create) {
            // --no-create 옵션이 설정된 경우, 생성하지 않고 종료
            if (verbose) {
                printf("%s: 파일이 존재하지 않아 생성하지 않음 (--no-create)\n", filename);
            }
            return 0;
        }

        // 파일 생성 시도
        fd = open(filename, O_CREAT | O_WRONLY, 0644);
        if (fd == -1) {
            // 생성 실패 시 에러 출력
            perror("open");
            return -1;
        }
        close(fd);  // 파일 디스크립터 닫기

        if (verbose) {
            printf("%s: 새로 생성됨\n", filename);
        }
    } else {
        // 파일이 이미 존재하면 시간 정보 갱신
        if (utime(filename, NULL) == -1) {
            // 갱신 실패 시 에러 출력
            perror("utime");
            return -1;
        }

        if (verbose) {
            printf("%s: 시간 갱신됨\n", filename);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    // getopt_long에 사용할 long 옵션 정의
    static struct option long_options[] = {
        {"no-create", no_argument, 0, 1}, // --no-create 옵션, 인자 없음, 값 1 반환
        {0, 0, 0, 0} // 배열의 끝을 나타내는 종료 마커 (null terminator)
    };

    int opt;               // getopt_long에서 반환하는 옵션 문자
    int option_index = 0;  // 현재 처리 중인 long 옵션의 인덱스

    // 옵션 파싱 루프
    while ((opt = getopt_long(argc, argv, "vh", long_options, &option_index)) != -1) {
        // getopt_long은 short(-v)와 long(--no-create) 옵션 모두 처리
        switch (opt) {
            case 1: // long option이 감지된 경우
                if (strcmp(long_options[option_index].name, "no-create") == 0) {
                    no_create = 1;
                }
                break;
            case 'v': // -v 옵션
                verbose = 1;
                break;
            case 'h': // -h 옵션
                print_help(argv[0]);
                return 0;
            default:  // 알 수 없는 옵션 또는 잘못된 옵션
                print_help(argv[0]);
                return 1;
        }
    }

    // 인자가 부족한 경우 (파일 이름이 없음)
    if (optind >= argc) {
        fprintf(stderr, "파일 이름을 지정하세요.\n");
        print_help(argv[0]);
        return 1;
    }

    // 나머지 인자들은 파일 이름이므로, 각 파일에 대해 touch_file 수행
    for (int i = optind; i < argc; i++) {
        if (touch_file(argv[i]) == -1) {
            // 파일 처리 중 에러가 발생하면 에러 메시지 출력
            fprintf(stderr, "파일 %s 처리 중 오류 발생\n", argv[i]);
        }
    }

    return 0; 
}
