
#ifndef _PRINTF_H_
#define _PRINTF_H_

#ifdef printf
	#undef printf
#endif

#ifdef putchar
	#undef putchar
#endif

#define printf printf_

void printf_(const char* format, ...);
int putchar(int c);

#endif
