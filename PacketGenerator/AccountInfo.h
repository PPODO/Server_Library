#pragma once
#pragma comment(lib, "Network.lib")
#include <string.h>
#include <Network/Packet/BasePacket.hpp>

using namespace std;
using namespace SERVER::NETWORK::PACKET;

struct AccountInfo : public Packet<AccountInfo> {
	friend boost::serialization::access;
public:
	int	m_token;
	string	m_user_id;
	string	m_user_password;
	string	m_user_name;

public:
	AccountInfo() : Packet(0, 0) {};
	AccountInfo(uint8_t iPacketType, uint8_t iMessageType, const int& token, const string& user_id, const string& user_password, const string& user_name) : Packet(iPacketType, iMessageType), m_token(token), m_user_id(user_id), m_user_password(user_password), m_user_name(user_name) {
	};

public:
	 const AccountInfo& operator=(const AccountInfo& rhs) {
		m_token= rhs.m_token;
		m_user_id= rhs.m_user_id;
		m_user_password= rhs.m_user_password;
		m_user_name= rhs.m_user_name;

		return (*this);
	}

protected:
	template<typename Archive>
	void serialize(Archive& ar, unsigned int Version) {
		ar& m_iMessageType;

		ar& m_token;
		ar& m_user_id;
		ar& m_user_password;
		ar& m_user_name;
	};

};

