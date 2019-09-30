#pragma once
#include <Network/Socket/Socket.h>
#include <Functions/Functions/Log/Log.h>
#include <memory>

namespace NETWORK {
	namespace SOCKET {
		namespace TCPIP {
			class CTCPIPSocket : public BASESOCKET::CBaseSocket {
			private:
				bool WriteProcess(const char* const SendData, const size_t& DataLength, WSAOVERLAPPED* const SendOverlapped);
				bool ReadProcess(char* const ReadBuffer, size_t& ReadedSize);
				bool ReadProcess(char* const ReadBuffer, size_t&& ReadedSize, WSAOVERLAPPED* const RecvOverlapped);

			public:
				explicit CTCPIPSocket();
				virtual ~CTCPIPSocket() override;

			public:
				bool Listen(const size_t BackLogCount = SOMAXCONN);
				bool Connect(const FUNCTIONS::SOCKADDR::CSocketAddress& ConnectAddress);
				bool Accept(CTCPIPSocket& ListenSocket, UTIL::BASESOCKET::OVERLAPPED_EX& AcceptOverlapped);

			public:
				inline bool Write(const char* const SendData, const size_t& DataLength) {
					return WriteProcess(SendData, DataLength, nullptr);
				}
				inline bool Write(const char* const SendData, const size_t& DataLength, UTIL::BASESOCKET::OVERLAPPED_EX& SendOverlapped) {
					return WriteProcess(SendData, DataLength, &SendOverlapped.m_Overlapped);
				}

			public:
				inline bool Read(char* const ReadBuffer, size_t& ReadedSize) {
					return ReadProcess(ReadBuffer, ReadedSize);
				}
				inline bool Read(UTIL::BASESOCKET::OVERLAPPED_EX& RecvOverlapped) {
					return ReadProcess(nullptr, size_t(), &RecvOverlapped.m_Overlapped);
				}

			};
		}
	}

	namespace UTIL {
		namespace TCPIP {
			static bool Recv(const ::SOCKET& Socket, char* const ReceiveBuffer, const size_t& ReceiveBufferSize, DWORD* const RecvBytes, WSAOVERLAPPED* const RecvOverlapped) {
				DWORD Flag = 0;
				WSABUF RecvBuffer;
				RecvBuffer.buf = ReceiveBuffer;
				RecvBuffer.len = ReceiveBufferSize;

				if (WSARecv(Socket, &RecvBuffer, 1, RecvBytes, &Flag, RecvOverlapped, nullptr) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING && WSAGetLastError() != WSAEWOULDBLOCK) {
						FUNCTIONS::LOG::CLog::WriteLog(L"WSA Recv : Failed To WSA Recv! - %d", WSAGetLastError());
						return false;
					}
				}
				return true;
			}

			bool ReUseSocket(const ::SOCKET& Socket, UTIL::BASESOCKET::OVERLAPPED_EX& DisconnectOverlapped);
		}
	}
}