#pragma once
#include <Network/Socket/Socket.h>

namespace NETWORK {
	namespace SESSION {
		namespace PACKETSESSION {
			class CPacketSession;
		}
	}

	namespace SESSION {
		namespace PACKETSESSION {
			static const size_t MAX_PACKET_BUFFER_SIZE = 1024;

			class CPacketSession {
			public:
				explicit CPacketSession();
				virtual ~CPacketSession();

			};
		}
	}
}