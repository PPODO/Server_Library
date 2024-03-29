/*#include <iostream>
#include <Functions/MySQL/MySQL.hpp>
#include <cppconn/prepared_statement.h>
#include <cppconn/datatype.h>
#include <MySQLDataFormatGenerator/dbtest_jon.h>

using namespace SERVER::FUNCTIONS::MYSQL;

int main() {
	CMySQLPool mysql("tcp://localhost:3306", "root", "a2233212.", 16);
	{
		CMySQLPool::CSQLRealConnection pConnection = mysql.GetConnection("testdb");

		try {
			CDBTEST_JON test_jon(501, "Winter");
			/*test_jon.ExecuteQueryForInsert(pConnection, "dbtest_jon");

			std::vector<CDBTEST_JON> listOfOutput;
			test_jon.ExecuteQueryForSelect(pConnection, "dbtest_jon", listOfOutput, {SQL::CQueryWhereConditional("user_name", "Winter", SQL::CQueryWhereConditional::ELogicalOperator::AND),
																						  SQL::CQueryWhereConditional("id", "501") });
 			test_jon.ExecuteQueryForDelete(pConnection, "dbtest_jon", SQL::CQueryWhereConditional("id", "100"));

			for (auto It : listOfOutput) {
				std::cout << It.m_ID.m_sColumnLabel << '\t' << It.m_ID.m_rawData << '\t';
				std::cout << It.m_user_name.m_sColumnLabel << '\t' << It.m_user_name.m_rawData << std::endl;
			}
		}
		catch (sql::SQLException& exception) {
			std::cout << exception.what() << std::endl;
		}
		//state->setInt(1, 100);

	}

	return 0;
}
*/
/*
#define _USER_PACKET_PROCESSOR_TYPE_
#include <iostream>
#include <NetworkModel/IOCP/IOCP.hpp>
#include <Network/Packet/BasePacket.hpp>

using namespace SERVER::NETWORK::PACKET;

static const uint16_t QUATERNION_ELEMENT_BUFFER_LENGTH = 6;

enum class EPacketType : uint8_t {
	E_Tracking = 1,
	E_Calibration = 2,
	E_Initialization = 3,
	E_SensorInitialization = 4
};

enum class EBoneType : uint8_t {
	E_Head = 1,
	E_Spline1 = 2,
	E_Spline2 = 3,
	E_Spline3 = 4,
	E_Right_Arm = 5,
	E_Left_Arm = 6,
	E_Pelvis = 7,
	E_Right_Leg = 8,
	E_Left_Leg = 9,
	E_Right_Finger = 10,
	E_Left_Finger = 11,
};

enum class EBonePointType : uint8_t {
	E_Upper = 1,
	E_Lower = 2,
	E_End = 3,
	E_Point = 4
};

enum class EFingerPointType : uint8_t {
	E_Thumb = 1,
	E_Index = 2,
	E_Middle = 3,
	E_Ring = 4,
	E_Pinky = 5,
};

struct HandTrackingSensor_Data {
public:
	uint8_t m_iFingerPointType;
	uint16_t m_iFingerAngle;

public:

};

class MyIOCP : public SERVER::NETWORKMODEL::IOCP::IOCP {
public:
	MyIOCP(SERVER::NETWORKMODEL::BASEMODEL::PACKETPROCESSOR& packetProcessorMap, const size_t iWorkerThreadCount = 0) : IOCP(packetProcessorMap, iWorkerThreadCount) {
		packetProcessorMap.insert(std::make_pair(1, std::bind(&MyIOCP::PacketQWE, this, std::placeholders::_1)));
	}

	virtual ~MyIOCP() override {

	}

public:
	void PacketQWE(SERVER::NETWORK::PACKET::PacketQueueData* pData) {
		if (pData) {
			for (int i = 0; i < 5; i++) {
				HandTrackingSensor_Data data = *reinterpret_cast<HandTrackingSensor_Data*>(pData->m_packetData.m_sPacketData);

				std::cout << data.m_iFingerPointType << ":" << data.m_iFingerAngle << '\t';

				MoveMemory(pData->m_packetData.m_sPacketData, pData->m_packetData.m_sPacketData + sizeof(HandTrackingSensor_Data), pData->m_packetData.m_packetInfo.m_iPacketDataSize - (sizeof(HandTrackingSensor_Data) * (i + 1)));
			}
			std::cout << std::endl;
		}
	}

	SERVER::NETWORKMODEL::IOCP::CONNECTION* m_pCommandESPConnection;

};
*/
#include <thread>
#include <Functions/CircularQueue/CircularQueue.hpp>

struct QWE : SERVER::FUNCTIONS::CIRCULARQUEUE::QUEUEDATA::BaseData<QWE> {
public:
	int Data;

public:
	QWE() {}
	QWE(int newData) :Data(newData) {};
};

class A {
public:
	SERVER::FUNCTIONS::CIRCULARQUEUE::CircularQueue<QWE> queue;

	std::vector<std::thread> listofVector;


public:
	A() {
		for (int i = 0; i < 2; i++) {
			listofVector.push_back(std::thread(&A::Run, this));
		}
	}

	~A() {
		for (auto& It : listofVector) {
			It.join();
		}
	}

	void Run() {
		while (true) {
			if (queue.IsEmpty())
				continue;

			QWE data;
			if (queue.Pop(data)) {

				std::cout << data.Data;
			}
		}
	}

	void AddNewData(int data) {
		queue.Push(QWE(data));
	}

};

int main() {
	/*SERVER::NETWORKMODEL::BASEMODEL::PACKETPROCESSOR list;

	SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress("172.30.1.21", 3550);
	MyIOCP iocp(list);
	iocp.Initialize(SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_BOTH, bindAddress);
	iocp.EnableAckCheck(false);

	while (true)
		iocp.Run();

	iocp.Destroy();*/

/*	SERVER::FUNCTIONS::MYSQL::CMySQLPool pool("tcp://localhost:3306", "root", "a2233212.", 2);

	{
		auto connection = pool.GetConnection("testdb");

		CDBTEST_JON table(51, "Aespa is me");
		CDBTEST_JON table1(100, "We can't become two");
		CDBTEST_JON table2(100, "Monochrome to colors");
		CDBTEST_JON table3(123128, "This is evo, evolution");

		table.ExecuteQueryForInsert(connection.get());
		table1.ExecuteQueryForInsert(connection.get());
		table2.ExecuteQueryForInsert(connection.get());
		table3.ExecuteQueryForInsert(connection.get());


		{	// 테이블에 존재하는 모든 데이터 조회
			std::vector<CDBTEST_JON> select_result;
			CDBTEST_JON::ExecuteQueryForSelect(connection.get(), select_result);
			std::cout << "Select!\n" << "ID\tUSER_NAME\n\n";

			for (auto& It : select_result) {
				std::cout << It.m_ID.m_rawData << '\t' << It.m_user_name.m_rawData << std::endl;
			}
		}

		std::cout << std::endl;

		{	// 테이블에 존재하는 모든 데이터 조회 - 조건 : USER_NAME만 조회
			std::vector<CDBTEST_JON> select_result;
			CDBTEST_JON::ExecuteQueryForSelect(connection.get(), select_result, { "user_name" });
			std::cout << "Select! - Only USER_NAME\n" << "ID\tUSER_NAME\n\n";

			for (auto& It : select_result) {
				std::cout << It.m_ID.m_rawData << '\t' << It.m_user_name.m_rawData << std::endl;
			}
		}

		std::cout << std::endl;

		{	// 테이블에 존재하는 데이터 삭제 - 조건 : ID가 100인 행만 삭제
			using namespace SERVER::FUNCTIONS::MYSQL::SQL;
			std::cout << "Delete! - Only ID is 100\n";
			CDBTEST_JON::ExecuteQueryForDelete(connection.get(), CQueryWhereConditional("ID", "100"));
		}

		std::cout << std::endl;

		{	// 테이블에 존재하는 모든 데이터 조회
			std::vector<CDBTEST_JON> select_result;
			CDBTEST_JON::ExecuteQueryForSelect(connection.get(), select_result);
			std::cout << "Select!\n" << "ID\tUSER_NAME\n\n";

			for (auto& It : select_result) {
				std::cout << It.m_ID.m_rawData << '\t' << It.m_user_name.m_rawData << std::endl;
			}
		}
	}*/
	A a;

	while (true) {
		for (int i = 0; i < 5; i++) {
			a.AddNewData(i);
		}
	}
	return 0;
}