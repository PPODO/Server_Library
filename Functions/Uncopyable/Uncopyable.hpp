#pragma once

namespace SERVER {
	namespace FUNCTIONS {
		namespace UNCOPYABLE {
			class Uncopyable {
			private:
				Uncopyable(const Uncopyable& rhs) {};
				const Uncopyable& operator=(const Uncopyable& rhs) {};

			public:
				Uncopyable() {}
				~Uncopyable() {}

			};
		}
	}
}