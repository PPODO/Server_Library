#pragma once
#pragma comment(lib, "Rpcrt4.lib")
#include <rpc.h>
#include "../CriticalSection/CriticalSection.hpp"

namespace SERVER {
	namespace FUNCTIONS {
		namespace UUID {
			class UUIDGenerator {
			public:
				static UINT16 Generate() {
					CRITICALSECTION::CriticalSectionGuard lock(m_lock);

					::UUID userUUID;

					m_lock.Lock();

					if (UuidCreate(&userUUID) != RPC_S_OK)
						return 0;

					RPC_STATUS result;
					auto iUUID = UuidHash(&userUUID, &result);

					m_lock.UnLock();

					if (result != RPC_S_OK)
						return 0;

					return iUUID;
				}

			private:
				static CRITICALSECTION::CriticalSection m_lock;

			};

		}
	}
}

__declspec(selectany) SERVER::FUNCTIONS::CRITICALSECTION::CriticalSection SERVER::FUNCTIONS::UUID::UUIDGenerator::m_lock(0);