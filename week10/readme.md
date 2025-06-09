
# copy.c

cp

return 0는 엑시트로 만듬

return 0는 성공

쉘에서 0은 성공

# 파일 디스크립터

키보드, 모니터, 에러메시지

파일의 패스가 길기때문에 그 대신에 파일 디스크립터(숫자)를 이용한다.

dup = 파일디스크립터를 복사하는 명령어 

---

# dup

```c
#include <unistd.h>   // dup, write, close 함수 등을 위해 포함
#include <fcntl.h>    // creat 함수 등을 위해 포함
#include <stdlib.h>   // exit 함수 포함
#include <stdio.h>    // perror 함수 포함

int main()
{
    int fd1, fd2;

    // "myfile"이라는 이름의 새 파일을 생성하며, 권한은 0600 (소유자만 읽기/쓰기 가능)
    if ((fd1 = creat("myfile", 0600)) == -1) {
        perror("myfile");  // 파일 생성 실패 시 에러 메시지 출력
    }

    // fd1을 통해 파일에 "Hello! Linux"라는 문자열 12바이트를 씀
    write(fd1, "Hello! Linux", 12);

    // fd1을 복제하여 fd2에 저장 (두 디스크립터는 동일한 파일을 가리킴)
    fd2 = dup(fd1);

    // fd2를 통해 파일에 "Bye! Linux"라는 문자열 10바이트를 씀
    write(fd2, "Bye! Linux", 10);

    // 프로그램 종료
    exit(0);
}
```

dup명령때문에 fd2가 fd1에 쓴것과 동일한 효과를 준다.

---

# 파일 위치 포인터


있음

# lseek() - 포인터 이동

i가 아니고 l이다

0L,100L의 L은 롱타입이라는 것

100L은 100바이트 옮겨서

위치포인터를 찾아 코드를 집어 넣을 수 있다.

이것이 바이러스

## read, write 실행권한까지 있으면 그 파일 그냥 내꺼임

---

# 시스템 콜 = 호출


- 지금 내가 디스크에 있는 파일을 불러와서 사용하고 싶을때, 파일 디스크립터에서 번호를 부여(오픈한순서)

---

# 파일시스템의 구조

부트블록이 망가지면 그냥 부팅이 안된다.

파일을 삭제하면, 디스크에 남아있는다.

앞에 이름만 살짝 바꾸면 부

### ROM이 부팅 시 하는 일

1. **전원 공급 (Power On)**
    - 컴퓨터나 장치에 전원이 들어오면, CPU는 가장 먼저 **ROM에 저장된 부트 코드**를 실행합니다.
2. **POST 수행 (Power-On Self Test)**
    - ROM에 저장된 펌웨어(BIOS 또는 UEFI)는 **CPU, 메모리, 키보드, 그래픽 카드 등 주요 하드웨어를 검사**합니다.
    - 하드웨어에 문제가 없으면 다음 단계로 넘어갑니다.
3. **부트 디바이스 탐색**
    - 하드디스크, SSD, USB 등에서 **운영체제를 찾기 위한 장치를 검색**합니다.
    - ROM은 이때 부트 순서(boot order)에 따라 장치를 확인합니다.
4. **부트로더 실행**
    - 운영체제를 포함하고 있는 디스크를 찾으면, 해당 디스크에 있는 **부트로더(예: GRUB, Windows Boot Manager)**를 메모리로 로드합니다.
5. **제어권 이양**
    - ROM은 부트로더에 제어권을 넘기고, 이후 운영체제의 커널을 메모리에 올려 실행하게 됩니다.

부트 디바이스 = c드라이브

부트 로더 = 주기억 장치 안에 있는 부트블록으로 이루어진것?

부트 스트랩 = 운영체제를 ram에 옮김(c드라이브 안에 있음)

디스크에 운영체제를 옮기는 녀석이 ‘부트블록’

슈퍼블록은 보통 4k

파일에 대한 정보는 i-리스트에 있다.

파일 타일이 i-노드에 있다.

## i-노드(i-Node)

한 파일은 하나의 i-노드를 갖는다. 

파일에 대한 모든 정보를 가지고 있음
 파일 타입: 일반 파일, 디렉터리, 블록 장치, 문자 장치 등
 파일 크기
 사용권한
 파일 소유자 및 그룹
 접근 및 갱신 시간
 데이터 블록에 대한 포인터(주소) 등

ls에 있는 목록은 전부 i-리스트에 있다.

그렇다면 i-node는 어떻게 정보를 관리중일까

# 블록 포인터

데이터 블록을 직접 블록 포인터로 관리한다.

간접 블록포인터는 직접 블록 포인터 1024개

이중 간접(더블 포인터)은 간접 블록 포인터 1024개

파일이 더 크면, 간접블록 포인터를 쓰고

그것보다 더 크면 이중 간접

하나의 파일의 최대사이즈?

한 블록이 4k이고, 10개 지정가능

간접 블록 1024개 지정

이중 간접 블록 1024개중에 1024개

### 📌 총합

- 직접: 40KB
- 간접: 4MB = 4096KB
- 이중 간접: 4GB = 4,194,304KB

```
markdown
복사편집
총합 = 40KB + 4096KB + 4,194,304KB
     = 4,198,440KB
     ≈ 4.198GB

```

실제론 3중 간접역시 존재(테라바이트)

---

# 파일 입출력 구현

파일 입출력 구현을 위한 커널 내 자료구조
 파일 디스크립터 배열(Fd array)
 열린 파일 테이블(Open File Table)
 동적 i-노드 테이블(Active i-node table)

# 커널 자료구조

# 동적 i-노드 테이블

같은 파일을 두번 오픈해야할 필요성이 있는경우

dup(3)을 (4)로 쓸거야

# 파일 상태

# 상태정보 : stat

상태 정보는 i-node에 있다.

i-node에 있는 정보를 불러올 수 있다.

# stat 구조체 (중간고사때 나온 이유)

파일의 목록을 보는 명령어를 가장 먼저 배우는데

그 파일의 정보가 i-node에 있고

위와 같은 구조체로 가져온다.

디스크에 있는 정보는 이정도

# 파일 타입

| 파일 타입 | 설명 |
| --- | --- |
| 일반 파일 | 데이터를 갖고 있는 텍스트 파일 또는 이진 파일 |
| 디렉터리 파일 | 파일의 이름들과 파일 정보에 대한 포인터를 포함하는 파일 |
| 문자 장치 파일 | 문자 단위로 데이터를 전송하는 장치를 나타내는 파일 |
| 블록 장치 파일 | 블록 단위로 데이터를 전송하는 장치를 나타내는 파일 |
| FIFO 파일 | 프로세스 간 통신에 사용되는 파일로, 이름 있는 파이프 |
| 소켓 | 네트워크를 통한 프로세스 간 통신에 사용되는 파일 |
| 심볼릭 링크 | 다른 파일을 가리키는 포인터 역할을 하는 파일 |

---

# ftype.c

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* 파일 타입을 검사한다. */
int main(int argc, char *argv[])
{
    int i;
    struct stat buf;
    for (i = 1; i < argc; i++) {
        printf("%s: ", argv[i]);
        if (lstat(argv[i], &buf) < 0) {
        perror("lstat()");
        continue;
        }
        if (S_ISREG(buf.st_mode))
        printf("%s \n", "일반 파일");
        if (S_ISDIR(buf.st_mode))
        printf("%s \n", "디렉터리");
        if (S_ISCHR(buf.st_mode))
        printf("%s \n", "문자 장치 파일");
        if (S_ISBLK(buf.st_mode))
        printf("%s \n", "블록 장치 파일");
        if (S_ISFIFO(buf.st_mode))
        printf("%s \n", "FIFO 파일");
        if (S_ISLNK(buf.st_mode))
        printf("%s \n", "심볼릭 링크");
        if (S_ISSOCK(buf.st_mode))
        printf("%s \n", "소켓");
        }
    exit(0);
}
```

---

# 파일 사용권한(open할때 함)

파일에 대한 권한
 읽기 r
 쓰기 w
 실행 x

## 암기할것

디렉토리에 write 권한과 execute 권한이 있어야
 그 디렉토리에 파일을 생성할 수 있고
 그 디렉토리의 파일을 삭제할 수 있다
 삭제할 때 그 파일에 대한 read write 권한은 없어도 됨

8진수를 기억해?

16비트 중에, 파일타입이 4비트

4비트를 직접 코딩해도 되지만, 미리 만들었다.

S_ISREG() → 파일 타입 검사 함수

# fchmod()

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
/* 파일 사용권한을 변경한다. */
main(int argc, char *argv[])
{
long strtol( );
int newmode;
newmode = (int) strtol(argv[1], (char **) NULL, 8);
if (chmod(argv[2], newmode) == -1) {
perror(argv[2]);
exit(1);
}
exit(0);
}

이중포인터 사용됨
```

# chown()

```c
#include <sys/types.h>
#include <unistd.h>
int chown (const char *path, uid_t owner, gid_t group );
int fchown (int filedes, uid_t owner, gid_t group );
int lchown (const char *path, uid_t owner, gid_t group );
```

파일의 user ID와 group ID를 변경한다

성공하면 0, 실패하면 -1

# utime()

```c
#include <sys/types.h>
#include <utime.h>
int utime (const char *filename, const struct utimbuf *times );
```

파일의 최종 접근 시간과 최종 변경 시간을 조정한다.

touch랑 동

# 디렉토리 구현

디렉토리조차 파일로 취급

디렉토리에 i-리스트가 있고

i-리스트에 i-노드가 있고

그 i-node가 데이터 블록의 정보를 가짐

# 파일 이름/크기 출력

디렉터리 내에 있는 파일 이름과 그 파일의 크기(블록의 수)를 출력하도록 확장

```c
while ((d = readdir(dp)) != NULL) { //디렉터리 내의 각 파일
sprintf(path, "%s/%s", dir, d->d_name); // 파일경로명 만들기
if (lstat(path, &st) < 0) // 파일 상태 정보 가져오기
perror(path);
printf("%5d %s", st->st_blocks, d->name); // 블록 수, 파일 이름 출력
putchar('\n');
}
```

