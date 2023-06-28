#pragma once
#include <NetworkModel/BaseModel/BaseModel.hpp>

using namespace SERVER::NETWORKMODEL::BASEMODEL;

namespace SERVER {
	namespace NETWORKMODEL {
		namespace EVENTSELECT {

			class EventSelect : public BaseNetworkModel {
			private:


			public:
				EventSelect(const int iPacketProcessorLoopCount, const PACKETPROCESSOR& packetProcessorMap);
				virtual ~EventSelect() override;

				virtual bool Initialize(const EPROTOCOLTYPE protocolType, FUNCTIONS::SOCKETADDRESS::SocketAddress& bindAddress) override;
				virtual void Run() override;
				virtual void Destroy() override;

			};
		}
	}
}