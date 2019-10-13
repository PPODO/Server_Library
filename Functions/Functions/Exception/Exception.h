#pragma once
#include <WinSock2.h>
#include <string>
#include <exception>

namespace FUNCTIONS {
	namespace EXCEPTION {
		struct bad_wsastart : public std::exception {
		public:
			bad_wsastart() : std::exception(std::string("Exception : Failed To WSAStartup" + WSAGetLastError()).c_str()) {};
		};

		struct bad_tcpalloc : public std::exception {
		public:
			bad_tcpalloc() : std::exception(std::string("Exception : Failed To Allocate TCP Socket").c_str()) {};
		};

		struct bad_udpalloc : public std::exception {
		public:
			bad_udpalloc() : std::exception(std::string("Exception : Failed To Allocate UDP Socket").c_str()) {};
		};
	}
}