# Server Library

 
## 목차
 Functions
  * [Circular Queue](https://github.com/PPODO/Server_Library#circular-queue)
  * [Critical Section](https://github.com/PPODO/Server_Library#critical-section)
  * [Log](https://github.com/PPODO/Server_Library#log)
  * [Memory Leak](https://github.com/PPODO/Server_Library#memory-leak)
  * [Memory Pool](https://github.com/PPODO/Server_Library#memory-pool)
  * [Minidump](https://github.com/PPODO/Server_Library#minidump)
  * [MySQL](https://github.com/PPODO/Server_Library#mysql)
  * [Socket Address](https://github.com/PPODO/Server_Library#socketaddress)
  * [Uncopyable](https://github.com/PPODO/Server_Library#uncopyable)

 Network
  * [Packet](https://github.com/PPODO/Server_Library#packet)
  * [Session](https://github.com/PPODO/Server_Library#session)
  * [Socket](https://github.com/PPODO/Server_Library#socket)


 NetworkModel
  * [EventSelect](https://github.com/PPODO/Server_Library#eventselect)
  * [IOCP](https://github.com/PPODO/Server_Library#iocp)


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
     if (decltype(ReturnVal) Value; Queue.Pop(Value)) {}

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
          FUNCTIONS::CRITICALSECTION::CCriticalSectionGuard Lock2(&Lock);
          // 생성자가 호출됨과 동시에 Lock
		
          // 소멸자가 호출됨과 동시에 UnLock
     }	 
	 
     return 0;
 }
 ```


### Log
  * 텍스트를 콘솔에 출력해 주는 용도로 사용합니다.
  * 멀티스레드에서 사용 가능합니다.
  * 출력된 텍스트는 .log 파일로 프로젝트 폴더에 생성됩니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/Log/Log.h>

 int main() {

     // 멀티바이트
     FUNCTIONS::LOG::CLog::WriteLog("HI %d", 1);
     FUNCTIONS::LOG::CLog::WriteLog("HI %f", 1.2f);
     FUNCTIONS::LOG::CLog::WriteLog("HI %c", 'A');
     FUNCTIONS::LOG::CLog::WriteLog("HI %s", "ASD");

     // 유니코드
     FUNCTIONS::LOG::CLog::WriteLog(L"HI %d", 1);
     FUNCTIONS::LOG::CLog::WriteLog(L"HI %f", 1.663f);
     FUNCTIONS::LOG::CLog::WriteLog(L"HI %C", 'B');
     FUNCTIONS::LOG::CLog::WriteLog(L"HI %S", "QWE");	 
	 
     return 0;
 }
 ```

 ![1](https://user-images.githubusercontent.com/37787879/67821767-7bad3a00-fb01-11e9-9c62-321c8f0e6392.png)
 
 콘솔에 로그가 출력됨

 ![2](https://user-images.githubusercontent.com/37787879/67799475-c199dc00-fac8-11e9-8e37-0220f7ff88cc.png)

 프로젝트 폴더에 .log 파일이 생성됨(M - 멀티바이트, U - 유니코드)

 ![3](https://user-images.githubusercontent.com/37787879/67799522-d8d8c980-fac8-11e9-957b-af94f37b2d43.png)


### Memory Leak
  * 메모리 누수 체크를 위해 사용됩니다. 헤더파일만 포함하면 됩니다.


### Memory Pool
  * 메모리 할당이 빈번할 때 사용할 수 있습니다. new/delete의 오버헤드를 줄여 프로그램의 실행속도를 높입니다.
  * 스마트포인터 사용이 가능합니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/MemoryPool/MemoryPool.hpp>

 class TEST : public FUNCTIONS::MEMORYMANAGER::CMemoryManager<TEST> {
 private:
     int a;
     float b;
     char c;

 public:
     TEST(int _a, float _b, char _c) : a(_a), b(_b), c(_c) {}

 public:
     void Print() {
         std::cout << a << '\t' << b << '\t' << c << std::endl;
     }

 };

 int main() {
     TEST* T1 = new TEST(1, 5.f, 'A');
     std::unique_ptr<TEST> T2 = std::make_unique<TEST>(64, 1235.f, 'B');
     std::shared_ptr<TEST> T3 = std::make_shared<TEST>(123, 565.f, 'C');

     T1->Print();
     T2->Print();
     T3->Print();

     delete T1;	 
	 
     return 0;
 }
 ```


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
 ``` c
 #include <iostream>
 #include <Functions/Functions/MySQL/MySQL.h>

 FUNCTIONS::MYSQL::CMySQLPool g_DBPool("tcp://127.0.0.1:3306", "root", "", 16, 32);

 void SearchResult(sql::ResultSet* Result) {
     while (Result && Result->next()) {
          for (unsigned int i = 1; i <= Result->getMetaData()->getColumnCount(); i++) {
               std::cout << Result->getString(i) << '\t';
          }
          std::cout << std::endl;
     }
 }

 int main() {
     using namespace FUNCTIONS::UTIL;

     auto Connection(g_DBPool.GetRawConnection("SchemaName"));
     auto Connection2(g_DBPool.GetConnection("SchemaName"));

     std::vector<MYSQL::DETAIL::INSERTDATA> InsertDatas{ MYSQL::DETAIL::ROW("FieldName", 1), MYSQL::DETAIL::ROW("FieldName2", 2.f), MYSQL::DETAIL::ROW("FieldName3", "hi!") };
     MYSQL::ExecuteQuery(Connection, MYSQL::InsertNewData("TableName", InsertDatas));

     MYSQL::DETAIL::UPATEDATA UpdateData({ MYSQL::DETAIL::ROW("FieldName2", 100.f) }, { MYSQL::MakeCondition(MYSQL::DETAIL::ECONDITIONTYPE::ECT_LIKE,  MYSQL::DETAIL::ELOGICALTYPE::ELT_NONE,"FieldName3", "hi!") });
     MYSQL::ExecuteQuery(Connection, MYSQL::UpdateData("TableName", UpdateData));

     MYSQL::DETAIL::SELECTDATA SelectData({ "*" }, { MYSQL::MakeCondition(MYSQL::DETAIL::ECONDITIONTYPE::ECT_BETAND, MYSQL::DETAIL::ELOGICALTYPE::ELT_NONE,"FieldName2", 0.f, 5.f) });
     MYSQL::ExecuteQuery(Connection, MYSQL::SearchData("TableName", SelectData), SearchResult);

     g_DBPool.ReleaseConnectionToPool(Connection);	 
	 
     return 0;
 }
 ```

### SocketAddress
  * WinSock의 sockaddr 구조체를 사용하기 편하게 래핑한 클래스입니다.
 ``` c
 #include <iostream>
 #include <Functions/Functions/SocketAddress/SocketAddress.h>

 int main() {
     using namespace FUNCTIONS::SOCKADDR;

     // IP는 자동으로 localhost(127.0.0.1)로 설정됩니다.
     CSocketAddress Addr(1234);
     CSocketAddress Addr1("127.0.0.1", 1234);

     const sockaddr* sockaddress = &Addr;
     sockaddr_in sockaddress_in(Addr);

     bind(INVALID_SOCKET, &Addr1, Addr.GetSize());	 
	 
     return 0;
 }
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
### Packet
  * 패킷의 직렬화, 역직렬화 및 패킷과 관련된 구조체들이 정의되어 있습니다. 


### Session
  * IOCP에서만 사용 가능한 클래스입니다. 


### Socket
  * TCP, UDP프로토콜이 정의되어 있습니다.


## NetworkModel
### EventSelect
  * 클라이언트에서 사용 가능한 네트워크 모델입니다.
 ``` c
 #include <NetworkModel/NetworkModel/IOCP/IOCP.hpp>
 
 void PacketProcessor(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const Data) {
    // 처리
     
    if(auto Owner = reinterpret_cast<NETWORKMODEL::EVENTSELECT::CEventSelect*>(Data->m_Owner); Owner) {
       CTEST TEST;
       auto PacketStructure = NETWORK::UTIL::PACKET::Serialize<CTEST>(TEST);

       Owner->Send(PacketStructure);
    }
 }
 
 int main() {
     NETWORKMODEL::DETAIL::PACKETPROCESSORLIST List;
     List.insert(std::make_pair(EPACKETPROTOCOL::EPPT_RESULT, &PacketProcessor));

     FUNCTIONS::SOCKADDR::CSocketAddress Address("127.0.0.1", 3550);
     std::unique_ptr<NETWORKMODEL::EVENTSELECT::CEventSelect> Event = std::make_unique<NETWORKMODEL::EVENTSELECT::CEventSelect>(12, List);

     std::thread T1([&]() {
         while (true) {
             std::string Test1, Test2;

             std::getline(std::cin, Test1);
             std::getline(std::cin, Test2);

             CTEST TEST(0, Test1, Test2);
             auto PacketStructure = NETWORK::UTIL::PACKET::Serialize<CTEST>(TEST);

             Event->SendTo(PacketStructure);
         }
     });

     if (Event->Initialize(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_BOTH, Address)) {
         while (true) {
             Event->Run();
         }
     }

     T1.join();
     return 0;
 }
 ```


### IOCP
  * 서버에서 사용 가능한 네트워크 모델입니다. 
 ``` c
 #include <NetworkModel/NetworkModel/IOCP/IOCP.hpp>
 
 void PacketProcessor(FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::CPacketQueueData* const Data) {
    // 처리
     
    if(auto Owner = reinterpret_cast<IOCP::CONNECTION*>(Data->Owner); Owner) { 
        if (Owner->m_Client.m_Session) {
            CTEST TEST;
            auto Packet = NETWORK::UTIL::PACKET::Serialize(TEST);

            Owner->m_Client.m_Session->SendTo(Packet);
        }
    }
 }
 
 int main() {
     NETWORKMODEL::DETAIL::PACKETPROCESSORLIST List;
     List.insert(std::make_pair(EPACKETPROTOCOL::EPPT_LOGIN, &PacketProcessor));

     NETWORKMODEL::IOCP::CIOCP IOCP(List);

     FUNCTIONS::SOCKADDR::CSocketAddress BindAddress("127.0.0.1", 3550);
     if (IOCP.Initialize(NETWORK::UTIL::BASESOCKET::EPROTOCOLTYPE::EPT_BOTH, BindAddress)) {
          IOCP.Run();
     }

     return 0;
 }
 ```
