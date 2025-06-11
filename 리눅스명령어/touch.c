#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

//전역변수 플래그, 구조체로 묶을 수 있다
int no_create = 0;
int verbose = 0;

void print_help(const char *prog_name) {
    printf("사용법: %s [옵션] <파일들>\n", prog_name);
    printf("옵션:\n");
    printf("  --no-create    파일이 없으면 생성하지 않음\n");
    printf("  -v             처리한 파일 이름 출력 (verbose)\n");
    printf("  -h             도움말 출력\n");
}

int touch_file(const char *filename) {
    int fd;

    if (access(filename, F_OK) == -1) {
        if (no_create) {
            if (verbose) {
                printf("%s: 파일이 존재하지 않아 생성하지 않음 (--no-create)\n", filename);
            }
            return 0;
        }

        // 파일이 없으면 생성
        fd = open(filename, O_CREAT | O_WRONLY, 0644);
        if (fd == -1) {
            perror("open");
            return -1;
        }
        close(fd);

        if (verbose) {
            printf("%s: 새로 생성됨\n", filename);
        }
    } else {
        // 파일이 존재하면 시간 갱신
        if (utime(filename, NULL) == -1) {
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
    static struct option long_options[] = {
        {"no-create", no_argument, 0, 1},
        //옵션이름,옵션인자 여부, 옵션이름이랑 다른값일때 반환, 옵션이름이랑 같을때 반환
        {0, 0, 0, 0}//long_options 배열의 끝을 나타내는 종료 마커(null terminator)
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "vh", long_options, &option_index)) != -1) {
    //getopt_long()은 긴 옵션 (--no-create)도 처리 가능, #include <getopt.h> 필요
        switch (opt) {
            case 1: // long option
                if (strcmp(long_options[option_index].name, "no-create") == 0) {
                    no_create = 1;
                }
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                print_help(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "파일 이름을 지정하세요.\n");//오류출력
        print_help(argv[0]);
        return 1;
    }

    for (int i = optind; i < argc; i++) {
        if (touch_file(argv[i]) == -1) {
            fprintf(stderr, "파일 %s 처리 중 오류 발생\n", argv[i]);
        }
    }

    return 0;
}