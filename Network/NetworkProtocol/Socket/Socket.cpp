#include "Socket.hpp"
#include "../../../Functions/Log/Log.hpp"
#include <MSWSock.h>
#pragma comment(lib, "mswsock.lib")

using namespace SERVER::NETWORK::PROTOCOL::BSD_SOCKET;
using namespace SERVER::FUNCTIONS::LOG;

BaseSocket::BaseSocket(const UTIL::BSD_SOCKET::EPROTOCOLTYPE protocolType) {
    m_hSocket = UTIL::BSD_SOCKET::CreateSocketByProtocolType(protocolType);

    ZeroMemory(m_sReceiveMessageBuffer, MAX_RECEIVE_BUFFER_SIZE);
}

BaseSocket::~BaseSocket() {
    if (m_hSocket != INVALID_SOCKET) {
        shutdown(m_hSocket, SD_BOTH);
        closesocket(m_hSocket);
    }
}

bool BaseSocket::Bind(const FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddres) {
    if (bind(m_hSocket, &bindAddres, bindAddres.GetSize()) == SOCKET_ERROR) {
        Log::WriteLog(L"Bind Failed!");
        return false;
    }
    return true;
}