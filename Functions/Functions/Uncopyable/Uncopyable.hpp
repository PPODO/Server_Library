#pragma once

namespace FUNCTIONS {
	namespace UNCOPYABLE {
		class CUncopyable {
		private:
			CUncopyable(const CUncopyable& rhs) {}
			const CUncopyable& operator=(const CUncopyable& rhs) {}

		public:
			explicit CUncopyable() {}
			~CUncopyable() {}

		};
	}
}