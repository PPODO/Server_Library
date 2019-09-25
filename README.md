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


