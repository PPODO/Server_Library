#pragma once
#include "../NetworkProtocol/TCPSocket.hpp"
#include "../NetworkProtocol/UDPSocket.hpp"
#include <memory>

using namespace SERVER::NETWORK::PROTOCOL::TCP;
using namespace SERVER::NETWORK::PROTOCOL::UDP;
using namespace SERVER::NETWORK::PROTOCOL::UTIL::BSD_SOCKET;

namespace SERVER {
	namespace NETWORK {
		namespace USER_SESSION {
			class User {
			protected:
				std::unique_ptr<TCPIPSocket> m_pTCPSocekt;
				std::unique_ptr<UDPIPSocket> m_pUDPSocket;

				EPROTOCOLTYPE m_protocolType;

			protected:
				User(const EPROTOCOLTYPE protocolType);

			public:
				virtual ~User();

			public:
				virtual bool Initialize(FUNCTIONS::SOCKETADDRESS::SocketAddress& toAddress);

				bool Receive(char* const sReceiveBuffer, uint16_t& iReceiveBytes);
				bool ReceiveFrom(char* const sReceiveBuffer, uint16_t& iReceiveBytes);

				bool Send(char* const sSendData, const uint16_t iDataLength);
				bool SendTo(const FUNCTIONS::SOCKETADDRESS::SocketAddress& sendAddress, char* const sSendData, const uint16_t iDataLength);

				inline bool SendCompletion(const EPROTOCOLTYPE protocolType, const uint16_t iSendBytes) {
					if (protocolType & EPROTOCOLTYPE::EPT_TCP)
						m_pTCPSocekt->SendCompletion(iSendBytes);
					if (protocolType & EPROTOCOLTYPE::EPT_UDP)
						m_pUDPSocket->SendCompletion(iSendBytes);
				}
			};
		}
	}
}