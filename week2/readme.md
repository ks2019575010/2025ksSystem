# 1장

![image.png](attachment:5b71b2d1-b5e9-49bd-993b-f5314441dcb5:image.png)

 BSD 유닉스(Unix) = 무료 유닉스

리눅스는 원래 오픈소스라 무료

# 유닉스 특징

![image.png](attachment:fa46218d-0d83-43a7-a7fb-db12a818c832:image.png)

- 다중 사용자 - 로그인을 여러명이서 할 수 있음(리눅스 동일)
- 다중 프로세스 - 다른 여러 프로그래밍 동시에 실행가능

쉘 = 명령어 해석기

ls 명령어를 사용한 파워쉘이 그런것

# 유닉스 운영체제

![image.png](attachment:0e8a6453-e12c-4e64-bbd1-33fdad4144a3:image.png)

커널이란 무엇인가? = ?

운영체제는 발표되고 사라지기도 한다.

하드웨어를 만들때 논리게이트(기본적인 게이트)가지고 구성

이걸 제어하기 위해서 함수가 필요하며, 하드웨어 제조회사에서 이것역시 만든다.

초창기엔 어셈블리로 만들었지만 현재는 c나 파이썬으로 제작

이렇게 제공되는 함수는 프로토타입만 제공한다.(안의 코드는 볼 수 없다.)

시스템 호출 = 

운영체제 기능을 마음껏 쓸 수 있게 운영체제도 프로토타입을 제공한다.

이것을 사용하는것을 시스템 호출이라한다.

(운영체제를 사용하는것)

쉘 = cmd, 파워쉘 기타등등

---

운영체제의 역할 = 커널?

# 커널

![image.png](attachment:80358145-5649-4d23-a0ff-8c63bceaf754:image.png)

![image.png](attachment:8f60f034-c753-4b6a-8069-66135b2222a7:image.png)

![image.png](attachment:8683a6b8-ba01-41e3-906d-0a5a70a66268:image.png)

파일관리란

crud = create, read, update, delete

위와같은 프로그램을 모아둔것이 커널이다.

---

gnu is not unix

---

토발즈가 오픈소스로 자기만의 운영체제를 만들었다.

그것이 리눅스

리눅스가 유닉스를 따라한것이다. 헷갈리지 말자

---

mac os = 유닉스 기반, 맥북 터미널 명령어가 비슷하다.

안드로이드 = 리눅스 기반

*참고 리눅스중에는 상업용 리눅스가 따로 존재한다. 업계사람들 말로는 돈값은 한다고

*참고2 오픈소스인 리눅스랑 숨겨진 윈도우즈 중에 리눅스가 더 안전하다.

설치했음으로 1장 나머지와 2장 윈도우는 생략

---

# 3장

이말은 참인가?

“컴퓨터사용자는 항상 언제나 늘 특정한 디렉토리에서 작업한다.”

마우스 포인트 보이는 것은 “사용자의 명령을 기다리는 상황”

사용자가 명령을 내릴때 , 사용자는 항상 특정한 디렉토리에 존재하고 있다.

바탕화면에서 더블 클릭으로 특정 파일을 여는것 역시 명령

## 컴퓨터사용자는 “항상” 특정 디렉토리에서 컴퓨터를 사용하고 있다.

자기가 어느 디렉토리에서 작업하는지 알고싶다면 = pwd

info = 뒤에 명령어를 붙이면 설명이 나온다. q를 누르면 빠져나온다.

nam? x → man으로 info같은 설명 들을 수 있다.

cmd에서 wsl로 리눅스 실행가능

---

# 운영체제에서 다뤄야하는 명령어

![image.png](attachment:d0155f28-06cd-406b-8dd2-4ce91700c942:image.png)

파일

- 실행 파일(.exe)
- 데이터 파일(exe의 실행보조)

root → home → id 디렉토리

id 디렉토리에 abc 파일을 추가하면 home에서도 나온다?

home에서 mkdir로 파일을 생성할 순 없었다.

---

ls, ll

ls - al

.으로 시작하는 것을 봐라

.은 자기자신, ..은 부모

root도 저걸 가지고 있다.

모든 디렉토리는 저것이 생성된다

왜 저걸 만들었냐

디렉토리를 관리하기 위해서

---

## 디렉토리 용어 4가지

![image.png](attachment:5f5f2bcc-1a67-441d-a6f8-29382df1525b:image.png)

/ = root directory

~ = home directory

. = self

.. = parent

그리고 crud관련된것들

---

## 경로

### 절대 경로

root부터 시작해서 가고자 하는 디렉토리까지 전부다 쓰는것

그냥 어느 디렉토리에 있던 그대로 갈 수 있다

cd  /home은 home으로 갈 수 있지만

cd home은 갈 수 없다.

### 상대 경로

자기가 있는 (작업하고 있는) 디렉토리 기준으로 이동하는것

cd home은 내가 /에 위치한 것이 아니라면 갈 수 없다.

./software →내가 있는 디렉토리에서 softare라는 디렉토리로 이동

즉, ./는 생략해도 상관없다.

---

## 파일생성

![image.png](attachment:d1080533-ca97-4b0a-acf3-e8cdb82a4fae:image.png)

cat >

이후 파일 내용을 적고

ctrl + d를 누르면 종료

윈도우즈에선 z

cat sample.txt로 내용확인 가능

ls -f와 ll은 비슷하다?

리눅스 사용자는 ls -f는 사용하지 않는다

---

## ll을 쳤을때 나오는것

![image.png](attachment:7fa84a3e-1a23-4e91-a53b-c85621b27707:image.png)

파일이름, 파일관련된 정보, 파일이 지니고 있는 내용

---

![image.png](attachment:5ea95723-e662-4f2d-9a74-a7db7e4fc5c3:image.png)

echo $path

환경변수?

---

만들 디렉토리

| softarer | → | system | →os |  |  |
| --- | --- | --- | --- | --- | --- |
|  |  |  | → | LP | →compilar,a.txt |
|  |  |  | →util,c.txt |  | →interpreter,b.txt |
|  |  |  |  |  |  |
|  | → | app,d.txt |  |  |  |

---

지금내가 작업하는 디렉토리, software 디렉토리안에서

위 표처럼 만들고 tree 명령어로 캡쳐

![image.png](attachment:99d2e5d2-f8ca-4f63-99fc-45bffb754a07:image.png)
