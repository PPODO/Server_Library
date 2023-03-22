#include "IOCP.hpp"

using namespace SERVER::NETWORKMODEL::IOCP;
using namespace SERVER::NETWORKMODEL::BASEMODEL;
using namespace SERVER::FUNCTIONS::LOG;

IOCP::IOCP(const BASEMODEL::PACKETPROCESSOR& packetProcessorMap, const int iPacketProcessorLoopCount) : BaseNetworkModel(iPacketProcessorLoopCount, packetProcessorMap), m_pServer(nullptr) {
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
        if (!m_pServer->Initialize(serverAddress) && 
            !m_pServer->RegisterIOCompletionPort(m_hIOCP)) return false;

        if ((protocolType & EPROTOCOLTYPE::EPT_UDP) && !m_pServer->ReceiveFrom()) return false;

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

    const size_t iWorkerThreadCount = std::thread::hardware_concurrency() * 2;
    for (size_t i = 0; i < iWorkerThreadCount; i++)
        m_workerThreadList.push_back(std::thread(&IOCP::IOCPWorkerThread, this));

    Log::WriteLog(L"Initialize IOCP : Worker Thread Creation Succeeded! - Thread Count : %d", iWorkerThreadCount);

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

    while (true) {

    }
}