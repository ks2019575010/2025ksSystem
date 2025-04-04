#include <stdio.h>

int main() {
    int num, i;
    int count = 0;
    int binary[8] = {0}; // 8비트 이진수 저장 배열
    
    // 0부터 255 사이의 정수 입력 받기
    printf("0부터 255 사이의 정수를 입력하세요: ");
    scanf("%d", &num);
    
    // 입력값 범위 확인
    if (num < 0 || num > 255) {
        printf("입력 범위를 벗어났습니다. 0에서 255 사이의 값을 입력하세요.\n");
        return 1;
    }
    
    // 2진수로 변환
    for (i = 7; i >= 0; i--) {
        binary[i] = num % 2;
        num /= 2;
    }
    
    // 1의 개수 계산 및 출력
    for (i = 0; i < 8; i++) {
        if (binary[i] == 1) {
            count++;
        }
    }
    printf("1의 개수: %d\n", count);
    
    // 상위 4비트 출력
    printf("상위 4비트: ");
    for (i = 0; i < 4; i++) {
        printf("%d", binary[i]);
    }
    printf("\n");
    
    return 0;
}