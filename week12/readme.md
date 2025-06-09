
# fork()

- 메모리에 똑같은거 하나가 더 생김

지금은 그다지 쓰지 않는다

```c
#include <stdio.h>
#include <unistd.h>
/* 자식 프로세스를 생성한다. */
int main()
{
    int pid;
    printf("[%d] 프로세스 시작 \n", getpid());
    pid = fork();
    printf("[%d] 프로세스 : 리턴값 %d\n", getpid(), pid);
}
//ctrl + i -> 바이브 코딩
```

이 코드를 실행하면서 오른쪽 프로세스가 생성된다.

저 박스아래에 pid아래 printf가 있다면

왼쪽과 오른쪽에서 동시에 실행된다. → 부모 먼저

(똑같이 복사된)자식에게는 0을 리턴

부모에겐 원래 pid 값을 리턴한다 x → 자식 프로세스id를 리턴한다 o


## 근데 자기를 복사해서 어따써…?

0을 리턴하면, 가장 먼저 실행되는거 아닌가

쓰레드 스케쥴링이 가능? 생명주기

실행되는 순서가 중요한것은 맞다.

하지만 부모가 자식보다 먼저 실행된다.

---

# 부모 프로세스와 자식 프로세스 구분

- `fork()` 호출 후에 리턴값이 다르므로 이 리턴값을 이용하여
- 부모 프로세스와 자식 프로세스를 구별하고
- 서로 다른 일을 하도록 할 수 있다.

---

```c
c
복사편집
pid = fork();
if (pid == 0)
{
    // 자식 프로세스의 실행 코드
}
else
{
    // 부모 프로세스의 실행 코드
}

```

# fork2

```c
#include <stdlib.h>
#include <stdio.h>
#include<unistd.h>
/* 부모 프로세스가 자식 프로세스를 생성하고 서로 다른 메시지를 프린트 */
int main() 
{
    int pid;
    pid = fork();
    if (pid ==0) 
    { // 자식 프로세스
        printf("[Child] : Hello, world pid=%d\n", getpid());
    }
    else { // 부모 프로세스
    printf("[Parent] : Hello, world pid=%d\n", getpid());
    }
}
```

---

# 두개의 자식 프로세스 생성

```c
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>/*오류가 없길 바란다면 넣어라*/
/* 부모 프로세스가 두 개의 자식 프로세스를 생성한다. */
int main() 
{
    int pid1, pid2;
    pid1 = fork();
    if (pid1 == 0) {
    printf("[Child 1] : Hello, world ! pid=%d\n", getpid());
    exit(0);
    }
    pid2 = fork();
    if (pid2 == 0) {
    printf("[Child 2] : Hello, world ! pid=%d\n", getpid());
    exit(0);
    }
    printf("[PARENT] : Hello, world ! pid=%d\n",getpid());
}
```

어떻게 실행될까

pid1은 자식 1

pid2는 자식 2

exit가 없으면 그냥 계속 생성됨

재귀적으로 

---

# 프로세스 기다리기 : wait()

```c
#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <unistd.h>     // fork, getpid
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait

int main() 
{
    int pid, child, status;
    printf("[%d] 부모 프로세스 시작 \n", getpid());
    pid = fork();
    if (pid == 0) {
        printf("[%d] 자식 프로세스 시작 \n", getpid());
        exit(1);
    }
    child = wait(&status); // 자식 프로세스가 끝나기를 기다린다.
    printf("[%d] 자식 프로세스 %d 종료 \n", getpid(), child);
    printf("\t종료 코드 %d\n", status >> 8);
}

이렇게 하면 자식이 먼저 실행된다.
```

```c
#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <unistd.h>     // fork, getpid, sleep
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid

/* 부모 프로세스가 자식 프로세스를 생성하고 끝나기를 기다린다. */
int main() 
{
    int pid1, pid2, child, status;

    printf("[%d] 부모 프로세스 시작 \n", getpid());
    
    pid1 = fork();
    if (pid1 == 0) {
        printf("[%d] 자식 프로세스[1] 시작 \n", getpid());
        sleep(1);
        printf("[%d] 자식 프로세스[1] 종료 \n", getpid());
        exit(1);
    }

    pid2 = fork();
    if (pid2 == 0) {
        printf("[%d] 자식 프로세스 #2 시작 \n", getpid());
        sleep(2);
        printf("[%d] 자식 프로세스 #2 종료 \n", getpid());
        exit(2);
    }

    // 자식 프로세스 #1의 종료를 기다린다.
    child = waitpid(pid1, &status, 0); 
    printf("[%d] 자식 프로세스 #1 %d 종료 \n", getpid(), child);
    printf("\t종료 코드 %d\n", status >> 8);
}
```

부모가 끝나고 나서 무조건 자식이 끝난다

이 코드에서 맨처음 실행되는것은  printf, 이후 자기 복제

복제되면서 30번쯤 라인이 실행되고 그럼으로 15번으로 올라가는데

쉬는걸로 다시 자식한테 내려간다

시험치면 이런거 실행결과같은것을 물을듯

```

int main()
{
int pid1, pid2, child, status;
printf("[%d] 부모 프로세스 시작 \\n", getpid());
// 부모 프로세스에서 첫 번째 자식 프로세스를 생성
pid1 = fork();
if (pid1 == 0) {
    // 이 블록은 첫 번째 자식 프로세스만 실행
    printf("[%d] 자식 프로세스[1] 시작 \\n", getpid());
    sleep(1);  // 1초 대기
    printf("[%d] 자식 프로세스[1] 종료 \\n", getpid());
    exit(1);   // 종료 코드 1로 종료
}

// 부모 프로세스에서 두 번째 자식 프로세스를 생성
pid2 = fork();
if (pid2 == 0) {
    // 이 블록은 두 번째 자식 프로세스만 실행
    printf("[%d] 자식 프로세스 #2 시작 \\n", getpid());
    sleep(2);  // 2초 대기
    printf("[%d] 자식 프로세스 #2 종료 \\n", getpid());
    exit(2);   // 종료 코드 2로 종료
}

// 부모 프로세스는 자식 프로세스 #1이 종료될 때까지 대기
child = waitpid(pid1, &status, 0);
// pid1에 해당하는 자식 프로세스가 종료되면 아래 코드 실행
printf("[%d] 자식 프로세스 #1 %d 종료 \\n", getpid(), child);
printf("\\t종료 코드 %d\\n", status >> 8);  // 종료 코드 추출 및 출력

```

}
주요 동작 순서 요약
부모 프로세스가 시작된다.

fork()로 첫 번째 자식 프로세스(pid1)를 생성한다.

자식 프로세스[1]은 1초 후 종료되며 종료 코드 1을 반환한다.

부모는 다시 fork()로 두 번째 자식 프로세스(pid2)를 생성한다.

자식 프로세스[2]는 2초 후 종료되며 종료 코드 2를 반환한다.

부모 프로세스는 waitpid(pid1)를 통해 자식[1]이 종료될 때까지 대기한다.

자식[1]이 종료되면 부모는 자식[1]의 PID와 종료 코드를 출력한다.

자식[2]는 부모가 waitpid(pid2)를 호출하지 않았기 때문에 좀비 프로세스로 남을 수 있다.

---

# RSA cryptosystem

그래 뭐, 암호화에 쓰지

그래서 가장 큰 소수가 중요하기도 하고

밀러 라빈 - 소수 판별법

페르마의 소정리 = 단하나도 조건에 맞지 않으면 합성수

모든 조건이 맞는다고 소수라는것은 아니다

결정론적 소수 판별 알고리즘

## 본론 - 확률적 인수분해 알고리즘

큰 합성수의 약수 찾기

사이클 탐지 기반

모든 케이스를 고려하는것은 말이 안된다.(불가능하다)

생일문제 

23명만 모여도 생일이 겹칠 확률이 50프로가 넘어간다.

## 팩토리제이션 알고리즘

폴라드 알고리즘으로 n의 약수 d찾기

d가 소수인지 밀러라빈의 소수판별법으로 판별

n을 d로 나눔

---

# 궁금증

프로세스 가 0이면 init이나 systemd 보다 빠른거 아닌가?
