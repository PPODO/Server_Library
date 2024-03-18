#pragma once
#pragma comment(lib, "Network.lib")
#include <string.h>
#include <Network/Packet/BasePacket.hpp>

using namespace std;
using namespace SERVER::NETWORK::PACKET;

struct test : public Packet<test> {
	friend boost::serialization::access;
public:
	int	m_name;

public:
	test() : Packet(0, 0) {};
	test(uint8_t iPacketType, uint8_t iMessageType, const int& name) : Packet(iPacketType, iMessageType), m_name(name) {
	};

public:
	 const test& operator=(const test& rhs) {
		m_name= rhs.m_name;

		return (*this);
	}

protected:
	template<typename Archive>
	void serialize(Archive& ar, unsigned int Version) {
		ar& m_iPacketType;
		ar& m_iMessageType;

		ar& m_name;
	};

};

