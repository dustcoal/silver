#ifdef WINDOWS
#include <Windows.h>
#endif

/* https://superuser.com/a/1529908 */
void windows_console_enable_colors() {
	#ifdef WINDOWS

		#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
		#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
		#endif


		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(hOut, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, dwMode);

		// References:
		//SetConsoleMode() and ENABLE_VIRTUAL_TERMINAL_PROCESSING?
		//https://stackoverflow.com/questions/38772468/setconsolemode-and-enable-virtual-terminal-processing

		// Windows console with ANSI colors handling
		// https://superuser.com/questions/413073/windows-console-with-ansi-colors-handling
	#endif
}
