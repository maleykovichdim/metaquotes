#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>

namespace clog_reader {

	// global function for definition length of ascii string || use for buffer only !!!
	size_t strlen_ascii(char * str);

	//global function for printing of string - use in debug mode
	void pr(const char * a, int size);//for debug

	//void _printf(const char * j) {};

}