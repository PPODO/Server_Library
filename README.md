# Server Library

 
## 목차
 Functions
  * [Circular Queue](https://github.com/PPODO/Server_Library#circular-queue)
  * [Critical Section](https://github.com/PPODO/Server_Library#critical-section)
  * [Exception](https://github.com/PPODO/Server_Library#exception)
  * [Log](https://github.com/PPODO/Server_Library#log)
  * [Memory Leak](https://github.com/PPODO/Server_Library#memory-leak)
  * [Memory Pool](https://github.com/PPODO/Server_Library#memory-pool)
  * [Minidump](https://github.com/PPODO/Server_Library#minidump)
  * [MySQL](https://github.com/PPODO/Server_Library#mysql)
  * [Socket Address](https://github.com/PPODO/Server_Library#socketaddress)
  * [Uncopyable](https://github.com/PPODO/Server_Library#uncopyable)

 Network


 IOCP


 EventSelect


## Functions
### Circular Queue
  * 멀티스레드 환경에서 사용 가능한 원형 큐입니다.
  * 큐 내부에 CS 객체가 있음으로 따로 동기화 작업을 해줄 필요가 없습니다.
  * QueueData는 따로 정의해야합니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/CircularQueue/CircularQueue.hpp>

 namespace FUNCTIONS::CIRCULARQUEUE::QUEUEDATA {
      struct TESTDATA : public DETAIL::BaseData<TESTDATA> {
      public:
	  int A;

      public:
	  TESTDATA() : A(0) {}
	  TESTDATA(const int a) : A(a) {

	  }

      };
 }

 int main() {
     FUNCTIONS::CIRCULARQUEUE::CCircularQueue<FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::TESTDATA> Queue;

     Queue.Push(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::TESTDATA(1));
     auto ReturnVal = Queue.Push(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::TESTDATA(1));

     if (Queue.Pop()) {}
     if (FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::TESTDATA Value; Queue.Pop(Value)) {}

     if (Queue.IsEmpty()) {}	 
	 
     return 0;
 }
 ```

### Critical Section
  * 기존의 CRITICAL_SECTION을 조금 더 사용하기 쉽게 래핑한 클래스입니다.
  * CCriticalSectionGuard를 사용하면 데드락을 예방할 수 있습니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/CriticalSection/CriticalSection.h>

 int main() {
     FUNCTIONS::CRITICALSECTION::DETAIL::CCriticalSection Lock;

     Lock.Lock();
     Lock.UnLock();

     {
	  FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock2(Lock);
          // 생성자가 호출됨과 동시에 Lock
		
	  // 소멸자가 호출됨과 동시에 UnLock
     }	 
	 
     return 0;
 }
 ```
### Exception
  * 예외들이 정의되어 있습니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/Exception/Exception.h>

 namespace FUNCTIONS::EXCEPTION {
     struct test_exception : public std::exception {
     public:
	 test_exception(const char* const Message) : std::exception(Message) {};
     };
 }

 int main() {
     try {
	 throw FUNCTIONS::EXCEPTION::test_exception("Test!");
     }
     catch (const std::exception& What) {
	 std::cout << What.what();
     } 
	 
     return 0;
 }
 ```

### Log
  * 텍스트를 콘솔에 출력해 주는 용도로 사용합니다.
  * 멀티스레드에서 사용 가능합니다.
  * 출력된 텍스트는 .log 파일로 프로젝트 폴더에 생성됩니다.

### Memory Leak
  * 메모리 누수 체크를 위해 사용됩니다. 헤더파일만 포함하면 됩니다.

### Memory Pool
  * 메모리 할당이 빈번할 때 사용할 수 있습니다. new/delete의 오버헤드를 줄여 프로그램의 실행속도를 높입니다.

### Minidump
  * 프로그램이 모종의 이유로 종료되었을 때의 상황을 기록한 덤프파일을 생성해줍니다.
  * 클래스 선언 없이 헤더파일만 include하면 됩니다.
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

### MySQL
  * 

### SocketAddress
  * WinSock의 sockaddr 구조체를 사용하기 편하게 래핑한 클래스입니다.
 ``` c

 ```

### Uncopyable
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
### Socket



### Session



## IOCP
### IOCP



## EventSelect
### EventSelect


