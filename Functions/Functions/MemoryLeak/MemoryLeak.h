#pragma once

#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _CONSOLE
#include <stdlib.h>
#endif

namespace FUNCTIONS {
	namespace MEMORYLEAK {
		class CMemoryLeak {
		public:
			explicit CMemoryLeak() {
				_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#ifdef _CONSOLE
				_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
				_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
				_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
				_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
				_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
				_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

#define DEBUG_NORMALBLOCK new(_NORMAL_BLOCK, __FILE__, __LINE__)

#ifdef new
#undef new
#endif

#define new DEBUG_NORMALBLOCK

#else
				_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
				_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
				_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif
			}
		};

		static CMemoryLeak g_MemoryLeak;

	}
}

#endif
#endif