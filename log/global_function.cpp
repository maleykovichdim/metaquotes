#include "pch.h"
#include "global_function.h"


namespace clog_reader {

	//global function for printing of string - use only in debug mode
	void pr(const char * a, int size)
	{
		if (size == 0) return;
		char *b = (char*)malloc(size + 16);
		memset(b, 0, size + 16);
		memcpy(b, a, size);
		b[size] = '\0';
		free(b);
	}

	// global function for definition length of ascii string || use for buffer only !!!
	size_t strlen_ascii(char * str)
	{
		size_t a = 0;
		while (true)
		{
			if (str[a] == 0x0A || str[a] == 0x0D || str[a] == '\0')
			{
				str[a] = '\0';
				break;
			}
			a++;
		}
		return a;
	}

}
