#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>

// 상수 정의
#define MAX_PATH_LEN 4096
#define MAX_LINE_LEN 4096
#define MAX_HISTORY_ENTRIES 50
#define HISTORY_FILE ".ecd_history"
#define BOOKMARK_FILE ".ecd_bookmarks"

// 전역 변수
static int verbose_mode = 0;      // 자세한 정보 표시 여부
static int create_mode = 0;       // 디렉토리 생성 여부
static int list_history = 0;      // 히스토리 표시 여부
static int go_back = 0;           // 이전 디렉토리로 이동 여부
static int go_home = 0;           // 홈 디렉토리로 이동 여부
static int show_bookmarks = 0;    // 북마크 목록 표시 여부
static int add_bookmark = 0;      // 북마크 추가 여부
static char bookmark_name[256];   // 북마크 이름
static char target_dir[MAX_PATH_LEN]; // 이동할 대상 디렉토리

/**
 * 홈 디렉토리 경로를 가져오는 함수
 */
char* get_home_directory() {
    char* home = getenv("HOME");
    if (home == NULL) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != NULL) {
            home = pw->pw_dir;
        }
    }
    return home;
}

/**
 * 파일 경로를 만드는 함수 (홈 디렉토리 + 파일명)
 */
void make_file_path(char* dest, const char* filename) {
    char* home = get_home_directory();
    snprintf(dest, MAX_PATH_LEN, "%s/%s", home, filename);
}

/**
 * 현재 디렉토리를 히스토리에 추가하는 함수
 */
void add_to_history(const char* dir) {
    char history_path[MAX_PATH_LEN];
    make_file_path(history_path, HISTORY_FILE);
    
    FILE* fp = fopen(history_path, "a");
    if (fp != NULL) {
        fprintf(fp, "%s\n", dir);
        fclose(fp);
    }
}

/**
 * 히스토리 파일의 크기를 제한하는 함수 (최근 50개만 유지)
 */
void trim_history() {
    char history_path[MAX_PATH_LEN];
    char temp_path[MAX_PATH_LEN];
    make_file_path(history_path, HISTORY_FILE);
    make_file_path(temp_path, ".ecd_history_temp");
    
    FILE* fp = fopen(history_path, "r");
    if (fp == NULL) return;
    
    // 모든 라인을 배열에 저장
    char lines[MAX_HISTORY_ENTRIES][MAX_PATH_LEN];
    int count = 0;
    char line[MAX_PATH_LEN];
    
    while (fgets(line, sizeof(line), fp) && count < MAX_HISTORY_ENTRIES) {
        // 개행문자 제거
        line[strcspn(line, "\n")] = 0;
        strcpy(lines[count], line);
        count++;
    }
    fclose(fp);
    
    // 최근 50개만 다시 저장
    FILE* temp_fp = fopen(temp_path, "w");
    if (temp_fp != NULL) {
        int start = (count > MAX_HISTORY_ENTRIES) ? count - MAX_HISTORY_ENTRIES : 0;
        for (int i = start; i < count; i++) {
            fprintf(temp_fp, "%s\n", lines[i]);
        }
        fclose(temp_fp);
        rename(temp_path, history_path);
    }
}

/**
 * 디렉토리 히스토리를 표시하는 함수
 */
void show_history() {
    char history_path[MAX_PATH_LEN];
    make_file_path(history_path, HISTORY_FILE);
    
    FILE* fp = fopen(history_path, "r");
    if (fp == NULL) {
        printf("디렉토리 히스토리가 없습니다.\n");
        return;
    }
    
    printf("디렉토리 히스토리:\n");
    char line[MAX_PATH_LEN];
    int line_num = 1;
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0; // 개행문자 제거
        printf("%3d  %s\n", line_num++, line);
    }
    
    fclose(fp);
}

/**
 * 북마크 목록을 표시하는 함수
 */
void show_bookmark_list() {
    char bookmark_path[MAX_PATH_LEN];
    make_file_path(bookmark_path, BOOKMARK_FILE);
    
    FILE* fp = fopen(bookmark_path, "r");
    if (fp == NULL) {
        printf("북마크:\n  아직 저장된 북마크가 없습니다.\n");
        return;
    }
    
    printf("북마크:\n");
    char line[MAX_PATH_LEN];
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0; // 개행문자 제거
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\0'; // 콜론을 null로 바꿔서 문자열 분리
            printf("  %s -> %s\n", line, colon + 1);
        }
    }
    
    fclose(fp);
}

/**
 * 현재 디렉토리를 북마크에 추가하는 함수
 */
void add_to_bookmarks(const char* name) {
    char bookmark_path[MAX_PATH_LEN];
    char current_dir[MAX_PATH_LEN];
    
    make_file_path(bookmark_path, BOOKMARK_FILE);
    
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd 실패");
        return;
    }
    
    // 기존 북마크가 있는지 확인
    FILE* fp = fopen(bookmark_path, "r");
    FILE* temp_fp = NULL;
    char temp_path[MAX_PATH_LEN];
    make_file_path(temp_path, ".ecd_bookmarks_temp");
    
    int found = 0;
    
    if (fp != NULL) {
        temp_fp = fopen(temp_path, "w");
        if (temp_fp != NULL) {
            char line[MAX_PATH_LEN];
            while (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\n")] = 0;
                char* colon = strchr(line, ':');
                if (colon != NULL) {
                    *colon = '\0';
                    if (strcmp(line, name) == 0) {
                        // 기존 북마크 업데이트
                        fprintf(temp_fp, "%s:%s\n", name, current_dir);
                        found = 1;
                    } else {
                        // 다른 북마크는 그대로 유지
                        fprintf(temp_fp, "%s:%s\n", line, colon + 1);
                    }
                }
            }
            fclose(temp_fp);
        }
        fclose(fp);
        
        if (temp_fp != NULL) {
            rename(temp_path, bookmark_path);
        }
    }
    
    // 새 북마크 추가
    if (!found) {
        fp = fopen(bookmark_path, "a");
        if (fp != NULL) {
            fprintf(fp, "%s:%s\n", name, current_dir);
            fclose(fp);
        }
    }
    
    printf("현재 디렉토리를 '%s' 북마크로 저장했습니다\n", name);
}

/**
 * 북마크에서 경로를 찾는 함수
 */
int find_bookmark(const char* name, char* path) {
    char bookmark_path[MAX_PATH_LEN];
    make_file_path(bookmark_path, BOOKMARK_FILE);
    
    FILE* fp = fopen(bookmark_path, "r");
    if (fp == NULL) {
        return 0;
    }
    
    char line[MAX_PATH_LEN];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\0';
            if (strcmp(line, name) == 0) {
                strcpy(path, colon + 1);
                fclose(fp);
                return 1;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

/**
 * 히스토리에서 이전 디렉토리를 찾는 함수
 */
int get_previous_dir(char* prev_dir) {
    char history_path[MAX_PATH_LEN];
    char current_dir[MAX_PATH_LEN];
    
    make_file_path(history_path, HISTORY_FILE);
    
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        return 0;
    }
    
    FILE* fp = fopen(history_path, "r");
    if (fp == NULL) {
        return 0;
    }
    
    char line[MAX_PATH_LEN];
    char last_different[MAX_PATH_LEN] = "";
    
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, current_dir) != 0) {
            strcpy(last_different, line);
        }
    }
    
    fclose(fp);
    
    if (strlen(last_different) > 0) {
        strcpy(prev_dir, last_different);
        return 1;
    }
    
    return 0;
}

/**
 * 디렉토리 정보를 표시하는 함수 (verbose 모드)
 */
void show_directory_info() {
    char current_dir[MAX_PATH_LEN];
    
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd 실패");
        return;
    }
    
    printf("%s\n", current_dir);
    
    // 디렉토리 내용 개수 세기
    DIR* dir = opendir(".");
    if (dir != NULL) {
        struct dirent* entry;
        int total_items = 0;
        int dir_count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                total_items++;

                char full_path[MAX_PATH_LEN];
                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, entry->d_name);

                struct stat st;
                if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                    dir_count++;
                }
            }
        }
        
        printf("%d 개 항목 (%d 디렉토리)\n", total_items, dir_count);
        closedir(dir);
    }
    
    // 디스크 사용량 표시 (du 명령어 실행)
    system("du -sh . 2>/dev/null | cut -f1 | xargs -I {} echo '{} 사용 중'");
}

/**
 * 도움말을 표시하는 함수
 */
void show_help() {
    printf("Enhanced CD (ecd) - 사용법:\n");
    printf("  ecd [옵션] [디렉토리]\n\n");
    printf("옵션:\n");
    printf("  -v, --verbose       디렉토리 변경 시 상세 정보 표시\n");
    printf("  -c, --create        디렉토리가 존재하지 않으면 생성\n");
    printf("  -l, --list          디렉토리 이동 히스토리 표시\n");
    printf("  -b, --back          이전에 방문한 디렉토리로 돌아가기\n");
    printf("  -h, --home          홈 디렉토리로 이동\n");
    printf("  -B, --bookmarks     북마크 목록 표시\n");
    printf("  -a 이름, --add-bookmark 이름  현재 디렉토리를 북마크로 저장\n");
    printf("  -H, --help          이 도움말 표시\n\n");
    printf("예제:\n");
    printf("  ecd ~/Documents     Documents 디렉토리로 이동\n");
    printf("  ecd -c new_folder   new_folder 생성 후 이동\n");
    printf("  ecd -l              디렉토리 히스토리 표시\n");
    printf("  ecd -a docs         현재 디렉토리를 'docs'로 북마크\n");
    printf("  ecd docs            'docs' 북마크로 이동\n");
}

/**
 * 디렉토리를 생성하는 함수 (mkdir -p와 동일)
 */
int create_directory(const char* path) {
    char temp_path[MAX_PATH_LEN];
    char* p = NULL;
    size_t len;
    
    snprintf(temp_path, sizeof(temp_path), "%s", path);
    len = strlen(temp_path);
    
    if (temp_path[len - 1] == '/') {
        temp_path[len - 1] = 0;
    }
    
    for (p = temp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(temp_path, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(temp_path, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    
    return 0;
}

/**
 * 메인 함수
 */
int main(int argc, char* argv[]) {
    int opt;
    char current_dir[MAX_PATH_LEN];
    char bookmark_path[MAX_PATH_LEN];
    
    // 긴 옵션 정의
    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"create", no_argument, 0, 'c'},
        {"list", no_argument, 0, 'l'},
        {"back", no_argument, 0, 'b'},
        {"home", no_argument, 0, 'h'},
        {"bookmarks", no_argument, 0, 'B'},
        {"add-bookmark", required_argument, 0, 'a'},
        {"help", no_argument, 0, 'H'},
        {0, 0, 0, 0}
    };
    
    // 명령행 옵션 파싱
    while ((opt = getopt_long(argc, argv, "vclbhBa:H", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                verbose_mode = 1;
                break;
            case 'c':
                create_mode = 1;
                break;
            case 'l':
                list_history = 1;
                break;
            case 'b':
                go_back = 1;
                break;
            case 'h':
                go_home = 1;
                break;
            case 'B':
                show_bookmarks = 1;
                break;
            case 'a':
                add_bookmark = 1;
                strncpy(bookmark_name, optarg, sizeof(bookmark_name) - 1);
                bookmark_name[sizeof(bookmark_name) - 1] = '\0';
                break;
            case 'H':
                show_help();
                return 0;
            case '?':
                fprintf(stderr, "잘못된 옵션입니다. -H 옵션으로 도움말을 확인하세요.\n");
                return 1;
        }
    }
    
    // 현재 디렉토리 저장
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("현재 디렉토리 정보를 가져올 수 없습니다");
        return 1;
    }
    
    // 히스토리 표시
    if (list_history) {
        show_history();
        return 0;
    }
    
    // 북마크 목록 표시
    if (show_bookmarks) {
        show_bookmark_list();
        return 0;
    }
    
    // 북마크 추가
    if (add_bookmark) {
        if (strlen(bookmark_name) == 0) {
            fprintf(stderr, "오류: 북마크 이름이 지정되지 않았습니다\n");
            return 1;
        }
        add_to_bookmarks(bookmark_name);
        return 0;
    }
    
    // 대상 디렉토리 결정
    if (go_home) {
        char* home = get_home_directory();
        if (home != NULL) {
            strcpy(target_dir, home);
        } else {
            fprintf(stderr, "홈 디렉토리를 찾을 수 없습니다\n");
            return 1;
        }
    } else if (go_back) {
        if (!get_previous_dir(target_dir)) {
            fprintf(stderr, "히스토리에 이전 디렉토리가 없습니다\n");
            return 1;
        }
    } else if (optind < argc) {
        // 명령행 인자로 디렉토리가 지정된 경우
        strcpy(target_dir, argv[optind]);
    } else {
        // 기본값: 홈 디렉토리
        char* home = get_home_directory();
        if (home != NULL) {
            strcpy(target_dir, home);
        } else {
            fprintf(stderr, "홈 디렉토리를 찾을 수 없습니다\n");
            return 1;
        }
    }
    
    // 북마크인지 확인
    if (access(target_dir, F_OK) != 0) {
        if (find_bookmark(target_dir, bookmark_path)) {
            if (verbose_mode) {
                printf("북마크 '%s' 사용 -> %s\n", target_dir, bookmark_path);
            }
            strcpy(target_dir, bookmark_path);
        }
    }
    
    // 디렉토리 생성 (필요한 경우)
    if (create_mode && access(target_dir, F_OK) != 0) {
        if (verbose_mode) {
            printf("디렉토리 생성: %s\n", target_dir);
        }
        if (create_directory(target_dir) != 0) {
            perror("디렉토리 생성 실패");
            return 1;
        }
    }
    
    // 디렉토리 변경
    if (chdir(target_dir) != 0) {
        perror("디렉토리 변경 실패");
        return 1;
    }
    
    // 히스토리에 이전 디렉토리 추가
    add_to_history(current_dir);
    trim_history();
    
    // verbose 모드면 정보 표시
    if (verbose_mode) {
        show_directory_info();
    }
    
    return 0;
}