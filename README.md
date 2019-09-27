# Server Library

 
## 목차
* Functions
* Network
* IOCP
* EventSelect

## Functions
* Circular Queue
  * Test

* Critical Section
  *

* Exception
  *

* Log
  *

* Memory Leak
  *

* Memory Pool
  * 

* Minidump
  * 
 ``` c
 #include <iostream>
 #include <Functions/Functions/Minidump/Minidump.h>

 int main() {
     int* TempPtr = nullptr;
	 
     // nullptr을 참조하여 프로그램이 비정상적으로 종료됩니다.	          
     std::cout << *TempPtr << std::endl;	 
	 
     return 0;
 }
 ```

 ![MiniDumpImage](https://user-images.githubusercontent.com/37787879/65747332-25cd2700-e13c-11e9-971a-a737d09a9ec2.png)
 프로젝트의 폴더 안에 .dmp 파일이 생성됨

 ![MinidumpImage2](https://user-images.githubusercontent.com/37787879/65747485-7a70a200-e13c-11e9-9243-400271791a4e.png)
 .dmp 파일을 실행 했을 때

 ![MiniDumpImage3](https://user-images.githubusercontent.com/37787879/65747545-aee45e00-e13c-11e9-9242-cdeaf58cd0f1.png)
 네이티브 전용으로 디버깅 했을 때

* SocketAddress
  * 

* Uncopyable
  * 특정 클래스의 생성자를 통한 복사 및 대입을 통한 복사가 불가능하도록 만들 때 사용하는 클래스입니다.
 ``` c
 #include <Functions/Functions/Uncopyable/Uncopyable.h>
 
 class Test : private FUNCTIONS::UNCOPYABLE::CUncopyable { 
 public:
     Test() {};
     ~Test() {};
 }
 
 int main() {
     Test T1;      // 성공! 디폴트 생성자가 호출!
     Test T2(T1);  // 에러! 복사생성자에 접근 불가!
     Test T3 = T1; // 에러! 대입연산자에 접근 불가!
     Test T4;      // 성공! 디폴트 생성자가 호출!
     T4 = T1;      // 에러! 대입연산자에 접근 불가!  

     return 0;
 }
 ```

## Network
* Socket



* Session



## IOCP
* IOCP



## EventSelect
* EventSelect


