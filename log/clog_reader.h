/**
* @file clog_reader.h
* search for strings in file matching a filter 
*/

#pragma once

#include <cstdio>
#include <iostream>
#include "datafromfile.h"
#include "match.h"

using namespace std;

namespace clog_reader {


	class CLogReader
	{
	public:


		explicit CLogReader(size_t reading_size = 1024 * 1024);//1024 * 1024
		CLogReader(int mt, size_t reading_size);
		~CLogReader();

		bool    Open(const char * fname_in);        // opening of file, false - error
		void    Close() ;                         //   file closing 

		bool    SetFilter(const char *filter) ;  // set of pattern, false - error
		bool    GetNextLine(char *buf, const int bufsize);

	private:
		int m_error_status;
		FILE * m_pInFile;
		size_t m_reading_size;
		DataFromFile<char> * data;
		char * m_filter;

		DefaultConsilience match;

	};



};//namespace



