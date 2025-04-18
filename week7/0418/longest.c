#include <stdio.h>
#include <string.h>

#define MAXLINE 100

char line[MAXLINE];     // 입력 줄
char longest[MAXLINE];  // 가장 긴 줄

// 함수 선언
void copy(char from[], char to[]);

/* 입력 줄 가운데 가장 긴 줄 출력 */
int main() {
    int len;
    int max = 0;

    while (fgets(line, MAXLINE, stdin) != NULL) {
        len = strlen(line);
        
        // 줄 끝 개행 문자 제거 (선택 사항)
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        if (len > max) {
            max = len;
            copy(line, longest);/*위에 함수선언을 지워버리면 에러터진다.*/
        }
    }

    if (max > 0) { // 입력 줄이 있었다면
        printf("%s\n", longest);
    }

    return 0;
}

/* copy: from을 to에 복사; to가 충분히 크다고 가정 */
void copy(char from[], char to[]) {
    int i = 0;
    while ((to[i] = from[i]) != '\0') {
        i++;
    }
}


/*표준 입력(STDIN)으로부터 여러 줄의 문자열을 입력받아, 그중 가장 긴 줄을 출력하는 C 프로그램
ctrl+d로 종료가능, 종료시 입력받은것중 가장 긴 스트링출력력*/