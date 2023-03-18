#pragma once
#include "../NetworkProtocol/TCPSocket.hpp"
#include "../NetworkProtocol/UDPSocket.hpp"
#include <memory>

using namespace SERVER::NETWORK::PROTOCOL::TCP;
using namespace SERVER::NETWORK::PROTOCOL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::SOCKET;

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			class User {
			protected:
				std::unique_ptr<TCPIPSocket> m_pTCPSocekt;
				std::unique_ptr<UDPIPSocket> m_pUDPSocket;

				EPROTOCOLTYPE m_protocolType;

			public:
				User(const EPROTOCOLTYPE protocolType);
				virtual ~User();

			public:
				virtual bool Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress);

				bool Receive();
				bool ReceiveFrom();

				bool Send();
				bool SendTo();

			};
		}
	}
}