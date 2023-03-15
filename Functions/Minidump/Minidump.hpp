#pragma once
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>

namespace SERVER {
	namespace FUNCTIONS {
		namespace MINIDUMP {
			class Minidump {
				typedef bool (WINAPI* MINIDUMPWRITEDUMP)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, 
														 const PMINIDUMP_EXCEPTION_INFORMATION, const PMINIDUMP_USER_STREAM_INFORMATION, 
														 const PMINIDUMP_CALLBACK_INFORMATION);
			private:
				LPTOP_LEVEL_EXCEPTION_FILTER m_pPrevExceptionFilter;

			private:
				static LONG WINAPI UnHandleExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo) {
					LONG iResult = EXCEPTION_CONTINUE_SEARCH;
					HMODULE hDLLHandle = LoadLibrary(TEXT("DBGHELP.DLL"));

					if (hDLLHandle) {
						MINIDUMPWRITEDUMP dump = (MINIDUMPWRITEDUMP)GetProcAddress(hDLLHandle, "MiniDumpWriteDump");

						if (dump) {
							TCHAR sDumpPath[MAX_PATH] = { TEXT("\0") };
							SYSTEMTIME systemTime;

							GetLocalTime(&systemTime);

							swprintf_s(sDumpPath, MAX_PATH, 
									   TEXT("%d-%d-%d %d_%d_%d.dmp"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
									   systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

							HANDLE hFileHandler = CreateFile(sDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
							if (hFileHandler != INVALID_HANDLE_VALUE) {
								_MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;

								dumpExceptionInfo.ThreadId = GetCurrentThreadId();
								dumpExceptionInfo.ExceptionPointers = pExceptionInfo;
								dumpExceptionInfo.ClientPointers = NULL;

								if (dump(GetCurrentProcess(), GetCurrentProcessId(), hFileHandler, MiniDumpNormal, &dumpExceptionInfo, nullptr, nullptr))
									iResult = EXCEPTION_EXECUTE_HANDLER;

								CloseHandle(hFileHandler);
							}
						}
					}
					return iResult;

				}

			public:
				Minidump() {
					SetErrorMode(SEM_FAILCRITICALERRORS);

					m_pPrevExceptionFilter = SetUnhandledExceptionFilter(Minidump::UnHandleExceptionFilter);
				}

				~Minidump() {
					SetUnhandledExceptionFilter(m_pPrevExceptionFilter);
				}
			};

			static Minidump g_MinidumpInst;
		}
	}
}