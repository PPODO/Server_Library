#include "EventSelect.hpp"
#include <Functions/Log/Log.hpp>

using namespace SERVER::NETWORKMODEL::EVENTSELECT;
using namespace SERVER::FUNCTIONS::LOG;

EventSelect::EventSelect(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap) : BaseNetworkModel(iPacketProcessorLoopCount, packetProcessorMap) {
}

EventSelect::~EventSelect() {
}

bool EventSelect::Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) {
	BaseNetworkModel::Initialize(protocolType, bindAddress);

	return true;
}

void EventSelect::Run() {
	BaseNetworkModel::Run();
}

void EventSelect::Destroy() {
	BaseNetworkModel::Destroy();
}