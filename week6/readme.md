우리가 쓰는거 vim인데 vi라 부름

shift + z + z해야 파일에서 나올 수 있음

# vi

vi test 라고 쳐서 vi 만듬

실행시 → 명령모드

vi에는 3가지 모드가 있다.

### 입력모드(편집모드)

![image.png](attachment:9506faf1-e06f-4b91-82e8-33293c3e5930:image.png)

a,i,o,s 중 하나 누르면 명령모드에서 입력모드

### 명령모드

입력모드에서 다시 명령모드로 가려면 esc

### 실행모드 → :를 누르면 실행모드

---

# 삭제

숫자로 원하는만큼 지우기 가능

# 복사 붙여넣기

## yy누르고 p 누르면 ctrl+c,ctrl+v와 같다.

# 블록지정

드래그해서 문자블록이 지정되어 있을때 사용가능

# vim 정규표현식

![image.png](attachment:18ae69ef-3842-4ca8-82c4-14e3a8ab7162:image.png)

---

# vi의 실행모드에!를 붙이는 것으로

리눅스 명령어?를 바로 쓸 수 있다.

:! gcc -o hello hello.c가 가능

Linux Command Line and Shell Scripting Bible, 2nd Edition

의 9장에 vim이 있다.

![image.png](attachment:8e8f3285-8c72-47c4-820d-16aa64d1035f:image.png)

---

.bat - 윈도우에서 명령어가 여럿 들어있는 실행파일

일괄처리

리눅스는 rwx중 x가 있다면 실행파일인것을 알 수 있다.

x가 곧 bat이랑 같다.

chmod로 x를 추가해 줄 수 있다.

![image.png](attachment:c516b8a7-322c-4dca-b612-fe13b9db7a53:image.png)

test1에 date, ls만 적었다.

---

![image.png](attachment:4758544b-0a14-4374-a064-5e1a1039cbd5:image.png)

g로 시작하는 명령어중 가장 최신의 것

![image.png](attachment:ab5e36bf-8aa3-493d-acd1-c40a5f3c2e12:image.png)

main은 cm

return은 clm?

# ehco $?로 return값을 알 수 있다.

0이 아니면 오류가 났다는 것

---

test1

![image.png](attachment:180244ef-029b-49a0-9668-b9e914bf17a1:image.png)

test 2

![image.png](attachment:886d5f95-31b8-4180-bc4a-2dc3cb83cfcc:image.png)

![image.png](attachment:7c35bbcf-fdf5-4070-9f1b-ba4a16b50631:image.png)

---

vi에서

입력은

shift insert

ctrl v

컨트롤 c 두번하고

윈도우 v하면 여러번 복사가능?

전세계적으로 shell 프로그래머가 잘 없다.

---

# vi 쓰는법

vi 파일이름으로 vi를 만든다.

만든 vi를 chmod

st로 만듬

---

리눅스는 켜는 순간 자동으로 3가지 파일이 오픈된다.

- 모니터
- 키보드
- 오류메세지

모든 os에서 자동으로 열어주는 파일 - 표준 입력 os

---

# 리눅스 수학연산

![image.png](attachment:28d17a9c-59a8-497f-8285-0aab4aa84783:image.png)

위는 1+5를 문자열로 받았다

리눅스는 상황에따라 띄어쓰기를 해야할때와 아닐때가 다르다.

![image.png](attachment:a1bd4135-3f46-4689-9b6c-956eba29ca3b:image.png)

![image.png](attachment:7d6420b6-aaff-4176-bb8a-780539262e6c:image.png)

=은 붙이고 +같은 오퍼레이터는 띄어쓴다.

작은 따옴’표가 아니라 ` 이거 써야한다.

![image.png](attachment:ee0a917a-ae93-446e-a366-aa461aa0bd84:image.png)

exit 5라서 echo $?가 5로 나온다.

---

# if 문

if command
then
commands
fi

---

cat /etc/passwd

![image.png](attachment:d1fbc162-2b92-4942-baa4-5e946d18278d:image.png)

### 🔒 `x`의 의미

- `x`는 **비밀번호가 `/etc/shadow` 파일에 저장되어 있음**을 의미합니다.
- 예전에는 비밀번호가 `/etc/passwd`에 암호화되어 저장되었지만, 보안상의 이유로 이제는 대부분의 리눅스 시스템에서 암호화된 비밀번호를 `/etc/shadow` 파일에 별도로 저장합니다.
- 그래서 `/etc/passwd`에는 단순히 `x`라고 적혀 있고, 실제 비밀번호는 `/etc/shadow`에서 확인됩니다 (이 파일은 루트만 읽을 수 있습니다).

### 전체 필드 구조

`/etc/passwd`의 각 줄은 다음과 같은 형식입니다:

```
ruby
복사편집
username:password:UID:GID:comment:home_directory:shell

```

예시로 풀어 보면:

```
yaml
복사편집
username : 사용자 이름
x        : shadow 파일에 암호가 있음
1000     : UID (User ID)
1000     : GID (Group ID)
comment  : 사용자 정보 (예: 이름 등)
/home/username : 홈 디렉토리
/bin/bash : 로그인 셸

```

---

# nesting if

if아래 if

elif

---

![image.png](attachment:bc2dc267-5351-4958-9d20-955593866835:image.png)

여기서 testing이 널인지 아닌지 파일마다 다르다

---

vi에 자동적으로 환경설정가능한 파일들이 있다
