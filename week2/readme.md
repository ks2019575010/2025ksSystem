# 1장

![image](https://github.com/user-attachments/assets/db8d8cfd-b490-4a6c-a1d6-292ba08d92ce)


 BSD 유닉스(Unix) = 무료 유닉스

리눅스는 원래 오픈소스라 무료

# 유닉스 특징

![image (1)](https://github.com/user-attachments/assets/8f7eae3a-c81d-4e96-8237-35e24bc66512)


- 다중 사용자 - 로그인을 여러명이서 할 수 있음(리눅스 동일)
- 다중 프로세스 - 다른 여러 프로그래밍 동시에 실행가능

쉘 = 명령어 해석기

ls 명령어를 사용한 파워쉘이 그런것

# 유닉스 운영체제

![image (2)](https://github.com/user-attachments/assets/6670afdc-9ef1-4492-9483-9ca312e67a22)

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

![image (3)](https://github.com/user-attachments/assets/02c65a72-05a2-4e49-a0d6-8d82791d370c)


![image (4)](https://github.com/user-attachments/assets/bd44fc4a-02c9-459f-ad6b-ccbac8c0fba1)

![image (5)](https://github.com/user-attachments/assets/f89fa1e0-3420-4e48-89db-073a3c85ecac)

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

![image (6)](https://github.com/user-attachments/assets/6eb34b08-0ca7-4ee0-be38-ba666e51451f)

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

![image (7)](https://github.com/user-attachments/assets/4ec8227c-7e54-4c21-b8cf-6ba9e8c99601)

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

![image (8)](https://github.com/user-attachments/assets/885a5793-c7e5-425f-b76f-1c08bd47c50d)

cat >

이후 파일 내용을 적고

ctrl + d를 누르면 종료

윈도우즈에선 z

cat sample.txt로 내용확인 가능

ls -f와 ll은 비슷하다?

리눅스 사용자는 ls -f는 사용하지 않는다

---

## ll을 쳤을때 나오는것

![image (9)](https://github.com/user-attachments/assets/c918b280-3a8f-4552-8ae5-b958ede962d8)


파일이름, 파일관련된 정보, 파일이 지니고 있는 내용

---

![image (10)](https://github.com/user-attachments/assets/acb4144f-8179-41e9-b30c-b2f2bf114b88)

echo $path

환경변수?

---

만들 디렉토리

| softarer | → | system | →os |  |  |
| --- | --- | --- | --- | --- | --- |
|  |  |  | → | LP | →compilar,a.txt |
|  |  |  | →util,c.txt |  | →interpreter,b.txt |
|  | → | app,d.txt |  |  |  |

---

지금내가 작업하는 디렉토리, software 디렉토리안에서

위 표처럼 만들고 tree 명령어로 캡쳐

![image (11)](https://github.com/user-attachments/assets/39b9c9f6-338e-41fe-802b-149672761b9f)
