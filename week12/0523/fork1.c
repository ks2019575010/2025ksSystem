#include <stdio.h>
#include <unistd.h>
/* 자식 프로세스를 생성한다. */
int main()
{
    int pid;
    printf("[%d] 프로세스 시작 \n", getpid());
    pid = fork();
    printf("[%d] 프로세스 : 리턴값 %d\n", getpid(), pid);
    printf("[%d] 프로세스 : 리턴값 %d\n", getpid(), pid);//print를 여러번한다면?
    printf("[%d] 프로세스 : 리턴값 %d\n", getpid(), pid);
}
//ctrl + i -> 바이브 코딩