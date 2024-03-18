#pragma once
#pragma comment(lib, "Network.lib")
#include <string.h>
#include <Network/Packet/BasePacket.hpp>

using namespace std;
using namespace SERVER::NETWORK::PACKET;

struct UserInfo : public Packet<UserInfo> {
	friend boost::serialization::access;
public:
	int	m_token;
	string	m_user_name;
	string	m_user_id;
	string	m_user_password;

public:
	UserInfo() : Packet(0, 0) {};
	UserInfo(uint8_t iPacketType, uint8_t iMessageType, const int& token, const string& user_name, const string& user_id, const string& user_password) : Packet(iPacketType, iMessageType), m_token(token), m_user_name(user_name), m_user_id(user_id), m_user_password(user_password) {
	};

public:
	 const UserInfo& operator=(const UserInfo& rhs) {
		m_token= rhs.m_token;
		m_user_name= rhs.m_user_name;
		m_user_id= rhs.m_user_id;
		m_user_password= rhs.m_user_password;

		return (*this);
	}

protected:
	template<typename Archive>
	void serialize(Archive& ar, unsigned int Version) {
		ar& m_iPacketType;
		ar& m_iMessageType;

		ar& m_token;
		ar& m_user_name;
		ar& m_user_id;
		ar& m_user_password;
	};

};

