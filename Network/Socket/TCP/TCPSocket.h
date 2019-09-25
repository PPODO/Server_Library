#pragma once
#include <Network/Socket/Socket.h>
#include <memory>

namespace NETWORK {
	namespace SOCKET {
		namespace TCPIP {
			class CTCPIPSocket : public BASESOCKET::CBaseSocket {
			private:
				bool WriteProcess(const char* const SendData, const size_t& DataLength, WSAOVERLAPPED* const SendOverlapped);
				bool ReadProcess(char* const ReadBuffer, size_t& ReadedSize, WSAOVERLAPPED* const RecvOverlapped);

			public:
				explicit CTCPIPSocket();
				virtual ~CTCPIPSocket() override;

			public:
				bool Listen(const size_t BackLogCount = SOMAXCONN);
				bool Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress);
				bool Accept(std::shared_ptr<CTCPIPSocket> ListenSocket, UTIL::BASESOCKET::OVERLAPPED_EX& AcceptOverlapped);

			public:
				inline bool Write(const char* const SendData, const size_t& DataLength) {
					return WriteProcess(SendData, DataLength, nullptr);
				}
				inline bool Write(const char* const SendData, const size_t& DataLength, UTIL::BASESOCKET::OVERLAPPED_EX& SendOverlapped) {
					return WriteProcess(SendData, DataLength, &SendOverlapped.m_Overlapped);
				}

			public:
				inline bool Read(char* const ReadBuffer, size_t& ReadedSize) {
					return ReadProcess(ReadBuffer, ReadedSize, nullptr);
				}
				inline bool Read(char* const ReadBuffer, size_t& ReadedSize, UTIL::BASESOCKET::OVERLAPPED_EX& RecvOverlapped) {
					return ReadProcess(ReadBuffer, ReadedSize, &RecvOverlapped.m_Overlapped);
				}

			};
		}
	}

	namespace UTIL {
		namespace TCPIP {


		}
	}
}