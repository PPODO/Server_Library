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

#include <iostream>
#include <NetworkModel/IOCP/IOCP.hpp>
#include <Network/Packet/BasePacket.hpp>

using namespace SERVER::NETWORK::PACKET;

static const uint16_t QUATERNION_ELEMENT_BUFFER_LENGTH = 6;

enum class EPacketType : uint8_t {
	E_Tracking = 1
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
	E_Left_Leg = 9
};

enum class EBonePointType : uint8_t {
	E_Upper = 1,
	E_Lower = 2,
	E_End = 3,
	E_Point = 4
};

struct Quaternion {
public:
	float w;
	float x;
	float y;
	float z;

public:
	Quaternion() : w(0), x(0), y(0), z(0) {};

};

struct TrackingSensor_Data {
public:
	uint8_t m_iBoneType;
	uint8_t m_iBonePointType;

	Quaternion m_quaternion;

public:
	TrackingSensor_Data() : m_iBoneType((uint8_t)0), m_iBonePointType((uint8_t)0), m_quaternion() {};
	TrackingSensor_Data(EBoneType boneType, EBonePointType bonePointType) : m_iBoneType((uint8_t)boneType), m_iBonePointType((uint8_t)bonePointType), m_quaternion() {};

	/*uint16_t ConvertToString(char* sBuffer) {
		uint16_t iDataSize = sizeof(uint16_t);

		memcpy(sBuffer + iDataSize, &m_iBoneType, sizeof(uint8_t));
		memcpy(sBuffer + iDataSize + sizeof(uint8_t), &m_iBonePointType, sizeof(uint8_t));
		iDataSize += sizeof(uint8_t) * 2;

		dtostrf(m_quaternion.w, 2, 4, sBuffer + iDataSize + (QUATERNION_ELEMENT_BUFFER_LENGTH * 0));
		dtostrf(m_quaternion.x, 2, 4, sBuffer + iDataSize + (QUATERNION_ELEMENT_BUFFER_LENGTH * 1));
		dtostrf(m_quaternion.y, 2, 4, sBuffer + iDataSize + (QUATERNION_ELEMENT_BUFFER_LENGTH * 2));
		dtostrf(m_quaternion.z, 2, 4, sBuffer + iDataSize + (QUATERNION_ELEMENT_BUFFER_LENGTH * 3));
		iDataSize += QUATERNION_ELEMENT_BUFFER_LENGTH * 4;

		memcpy(sBuffer, &iDataSize, sizeof(uint16_t));

		return iDataSize;
	}*/

};

void PacketProcessor(PacketQueueData* const Data) {
	TrackingSensor_Data SensorData[3];
	if (Data) {
		for (int i = 0; i < 3; i++) {
			uint16_t iSensorDataSize = *reinterpret_cast<uint16_t*>(Data->m_packetData.m_sPacketData);
			SensorData[i].m_iBoneType = *reinterpret_cast<uint8_t*>(Data->m_packetData.m_sPacketData + sizeof(uint16_t));
			SensorData[i].m_iBonePointType = *reinterpret_cast<uint8_t*>(Data->m_packetData.m_sPacketData + sizeof(uint16_t) + sizeof(uint8_t));

			SensorData[i].m_quaternion.w = std::stof(Data->m_packetData.m_sPacketData + sizeof(uint16_t) + sizeof(uint16_t) + (QUATERNION_ELEMENT_BUFFER_LENGTH * 0));
			SensorData[i].m_quaternion.x = std::stof(Data->m_packetData.m_sPacketData + sizeof(uint16_t) + sizeof(uint16_t) + (QUATERNION_ELEMENT_BUFFER_LENGTH * 1));
			SensorData[i].m_quaternion.y = std::stof(Data->m_packetData.m_sPacketData + sizeof(uint16_t) + sizeof(uint16_t) + (QUATERNION_ELEMENT_BUFFER_LENGTH * 2));
			SensorData[i].m_quaternion.z = std::stof(Data->m_packetData.m_sPacketData + sizeof(uint16_t) + sizeof(uint16_t) + (QUATERNION_ELEMENT_BUFFER_LENGTH * 3));

			std::cout << (uint16_t)SensorData[i].m_iBonePointType << ' ' << (uint16_t)SensorData[i].m_iBonePointType << '\t';
			std::cout << SensorData[i].m_quaternion.w << ' ' << SensorData[i].m_quaternion.x << ' ' << SensorData[i].m_quaternion.y << ' ' << SensorData[i].m_quaternion.z << std::endl;

			MoveMemory(Data->m_packetData.m_sPacketData, Data->m_packetData.m_sPacketData + iSensorDataSize, Data->m_packetData.m_packetInfo.m_iPacketDataSize - iSensorDataSize);
		}
	}
}

int main() {
	SERVER::NETWORKMODEL::BASEMODEL::PACKETPROCESSOR list;
	list.insert(std::make_pair(1, PacketProcessor));

	SERVER::FUNCTIONS::SOCKETADDRESS::SocketAddress bindAddress("172.30.1.89", 3550);
	SERVER::NETWORKMODEL::IOCP::IOCP iocp(list);
	iocp.Initialize(SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET::EPROTOCOLTYPE::EPT_UDP, bindAddress);
	iocp.EnableAckCheck(false);

	while (true)
		iocp.Run();

	iocp.Destroy();
	return 0;
}