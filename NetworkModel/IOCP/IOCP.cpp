#include "IOCP.hpp"
#include <MSWSock.h>

using namespace SERVER::NETWORKMODEL::IOCP;
using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::FUNCTIONS::LOG;

IOCP::IOCP(const BASEMODEL::PACKETPROCESSOR& packetProcessorMap, const size_t iWorkerThreadCount) : BaseNetworkModel(1, packetProcessorMap), m_pServer(nullptr), m_iWorkerThreadCount(iWorkerThreadCount), bEnableUDPAckCheck(false) {
}

IOCP::~IOCP() {
}

bool IOCP::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& serverAddress) {
    if (!BaseNetworkModel::Initialize(protocolType, serverAddress)) return false;

    if (!(m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))) {
        Log::WriteLog(L"Failed To Initialization IOCP Handle");
        return false;
    }

    try {
        using namespace SERVER::NETWORK::USER_SESSION::USER_SERVER;

        m_pServer = std::make_unique<User_Server>(protocolType);
        if (!m_pServer->Initialize(serverAddress) || 
            !m_pServer->RegisterIOCompletionPort(m_hIOCP)) return false;

        if ((protocolType & EPROTOCOLTYPE::EPT_UDP)) 
            if (!m_pServer->ReceiveFrom()) return false;

        if (protocolType & EPROTOCOLTYPE::EPT_TCP) {
            for (size_t i = 0; i < 100; i++) {
                User_Server* pClient = new User_Server(EPROTOCOLTYPE::EPT_TCP);
                if (!pClient->Initialize(*m_pServer)) return false;

                m_clientList.push_back(new CONNECTION(pClient));
            }
        }
    }
    catch (std::bad_alloc& exception) {
        Log::WriteLog(FUNCTIONS::UTIL::MBToUni(std::string(exception.what())).c_str());
        return false;
    }

    if (m_iWorkerThreadCount <= 0)
        m_iWorkerThreadCount = std::thread::hardware_concurrency() * 2;
  
    for (size_t i = 0; i < m_iWorkerThreadCount; i++)
        m_workerThreadList.push_back(std::thread(&IOCP::IOCPWorkerThread, this));

    Log::WriteLog(L"Initialize IOCP : Worker Thread Creation Succeeded! - Thread Count : %d", m_iWorkerThreadCount);

    return true;
}

void IOCP::Run() {
    BaseNetworkModel::Run();
}

void IOCP::Destroy() {
    for (auto& iterator : m_workerThreadList)
        PostQueuedCompletionStatus(m_hIOCP, 0, NULL, NULL);

    for (auto& iterator : m_workerThreadList)
        if (iterator.joinable()) iterator.join();

    m_workerThreadList.clear();

    m_clientListLock.Lock();
    for (auto client : m_clientList)
        if (client && client->m_pUser) delete client;
    m_clientList.clear();
    m_clientListLock.UnLock();
}

void IOCP::IOCPWorkerThread() {
    using namespace NETWORK::USER_SESSION::USER_SERVER;

    while (true) {
        DWORD iRecvBytes = 0;
        void* pCompletionKey = nullptr;
        OVERLAPPED_EX* pOverlappedEx = nullptr;

        bool bIOCPResult = GetQueuedCompletionStatus(m_hIOCP, &iRecvBytes, 
                                                     reinterpret_cast<PULONG_PTR>(&pCompletionKey), 
                                                     reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx), INFINITE);

        if (!pCompletionKey) // stop event
            break;

        if (pOverlappedEx) {
            if (!bIOCPResult || iRecvBytes <= 0) {
                switch (pOverlappedEx->m_IOType) {
                case EIOTYPE::EIT_ACCEPT:
                    OnIOAccept(pOverlappedEx);
                    break;
                case EIOTYPE::EIT_DISCONNECT:
                    OnIODisconnect(pOverlappedEx->m_pOwner);
                    break;
                default:
                    OnIOTryDisconnect(pOverlappedEx->m_pOwner);
                    break;
                }
                continue;
            }

            switch (pOverlappedEx->m_IOType) {
            case EIOTYPE::EIT_READ:
                OnIOReceive(pOverlappedEx, iRecvBytes);
                break;
            case EIOTYPE::EIT_WRITE:
                OnIOWrite(pOverlappedEx->m_pOwner, iRecvBytes);
                break;
            case EIOTYPE::EIT_READFROM:
                OnIOReceiveFrom(pOverlappedEx, iRecvBytes);
                break;
            }
        }
    }
}

CONNECTION* IOCP::GetConnectionFromList(User_Server* const pUser) {
    auto iterator = std::find_if(m_clientList.cbegin(), m_clientList.cend(), [&pUser](CONNECTION* const pConnection) {
        if (pConnection->m_pUser == pUser) return true;
        return false;
    });

    if (iterator != m_clientList.cend())
        return *iterator;
    return nullptr;
}

CONNECTION* IOCP::GetConnectionFromList(FUNCTIONS::SOCKETADDRESS::SocketAddress& peerAddress) {
    auto iterator = std::find_if(m_clientList.cbegin(), m_clientList.cend(), [&peerAddress](CONNECTION* const pConnection) {
        if (peerAddress == pConnection->m_peerInformation.m_remoteAddress) return true;
        return false;
    });

    if (iterator != m_clientList.cend())
        return *iterator;
    else if (GetProtocolType() & EPROTOCOLTYPE::EPT_UDP) {
        auto pNewConnection = new CONNECTION(m_pServer.get(), PeerInfo(peerAddress, 0));
        m_clientList.emplace_back(pNewConnection);
        return pNewConnection;
    }
    return nullptr;
}


CONNECTION* IOCP::OnIOAccept(OVERLAPPED_EX* const pAcceptOverlapped) {
    using namespace FUNCTIONS::SOCKETADDRESS;

    if (auto pUser = pAcceptOverlapped->m_pOwner) {
        const DWORD iAddrSize = SocketAddress::GetSize() + 16;
        sockaddr* pLocalAddr = nullptr, * pRemoteAddr = nullptr;
        int iLocalLength = 0, iRemoteLength = 0;
        GetAcceptExSockaddrs(pAcceptOverlapped->m_pReceiveBuffer, 0, iAddrSize, iAddrSize, &pLocalAddr, &iLocalLength, &pRemoteAddr, &iRemoteLength);

        if (auto pRemoteAddr_in = reinterpret_cast<sockaddr_in*>(pRemoteAddr)) {
            if (auto pConnection = GetConnectionFromList(pUser)) {
                pConnection->m_peerInformation = PeerInfo(SocketAddress(*pRemoteAddr_in), 0);
                 if (pUser->RegisterIOCompletionPort(m_hIOCP) && pUser->Receive()) {
                    Log::WriteLog(L"Accept New Client!");
                    return pConnection;
                }
            }
        }
        Log::WriteLog(L"Accept Failed!");
        pUser->SocketRecycle();
    }
    return nullptr;
}

CONNECTION* IOCP::OnIOTryDisconnect(User_Server* const pClient) {
    if (auto pConnection = GetConnectionFromList(pClient)) {
        if (pConnection->m_pUser->SocketRecycle())
            Log::WriteLog(L"Try Disconnect Client!");
        else
            OnIODisconnect(pClient);
        return pConnection;
    }
    return nullptr;
}

CONNECTION* IOCP::OnIODisconnect(User_Server* const pClient) {
    if (auto pConnection = GetConnectionFromList(pClient)) {
        if (pClient->Initialize(*m_pServer.get())) {
            Log::WriteLog(L"Disconnect Client!");
            return pConnection;
        }
    }
    return nullptr;
}

CONNECTION* IOCP::OnIOWrite(User_Server* const pClient, const uint16_t iWriteBytes) {
    pClient->SendCompletion(EPROTOCOLTYPE::EPT_TCP, iWriteBytes);
    return GetConnectionFromList(pClient);
}

CONNECTION* IOCP::OnIOReceive(OVERLAPPED_EX* const pReceiveOverlapped, const uint16_t iRecvBytes) {
    if (auto pConnection = GetConnectionFromList(pReceiveOverlapped->m_pOwner)) {

        pReceiveOverlapped->m_iRemainReceiveBytes += iRecvBytes;
        ReceiveDataProcessing(EPROTOCOLTYPE::EPT_TCP, pReceiveOverlapped->m_pReceiveBuffer, pReceiveOverlapped->m_iRemainReceiveBytes, pReceiveOverlapped->m_iLastReceivedPacketNumber, pConnection);
        if (!pConnection->m_pUser->Receive()) {
            pConnection->m_pUser->SocketRecycle();
        }
        return pConnection;
    }
    return nullptr;
}

CONNECTION* IOCP::OnIOReceiveFrom(OVERLAPPED_EX* const pReceiveFromOverlapped, const uint16_t iRecvBytes) {
    CONNECTION* pConnection = GetConnectionFromList(pReceiveFromOverlapped->m_pOwner);
    if (!pConnection)
        pConnection = GetConnectionFromList(pReceiveFromOverlapped->m_remoteAddress);

    if (pConnection) {
        pReceiveFromOverlapped->m_iRemainReceiveBytes += iRecvBytes;
        pReceiveFromOverlapped->m_iLastReceivedPacketNumber = pConnection->m_peerInformation.m_iLastPacketNumber;

        if (bEnableUDPAckCheck && NETWORK::PROTOCOL::UTIL::UDP::CheckAck(*pReceiveFromOverlapped)) {
            ReceiveDataProcessing(EPROTOCOLTYPE::EPT_UDP, pReceiveFromOverlapped->m_pReceiveBuffer, pReceiveFromOverlapped->m_iRemainReceiveBytes, pReceiveFromOverlapped->m_iLastReceivedPacketNumber, pConnection);
            pConnection->m_peerInformation.m_iLastPacketNumber = pReceiveFromOverlapped->m_iLastReceivedPacketNumber;
        }
        else {
            ReceiveDataProcessing(EPROTOCOLTYPE::EPT_UDP, pReceiveFromOverlapped->m_pReceiveBuffer, pReceiveFromOverlapped->m_iRemainReceiveBytes, pReceiveFromOverlapped->m_iLastReceivedPacketNumber, pConnection);
        }

        if (!pConnection->m_pUser->ReceiveFrom())
            pConnection->m_pUser->SocketRecycle();
        return pConnection;
    }
    return nullptr;
}