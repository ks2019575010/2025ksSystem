#include<stdio.h>
#include <ctype.h>

int main(){
    char ch;

    while (1) {
        printf("문자 입력 : ");
        scanf(" %c", &ch); 

        if (ch == '0') {
            break;  // 0을 입력하면 종료
        }

        if (isupper(ch)) {
            // 대문자일 경우 소문자로 변환
            printf("%c의 소문자는 %c입니다.\n", ch, tolower(ch));
        } else if (islower(ch)) {
            // 소문자일 경우 대문자로 변환
            printf("%c의 대문자는 %c입니다.\n", ch, toupper(ch));
        } 
    }

    return 0;
}