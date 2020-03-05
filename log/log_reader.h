/**
* @file clog_reader.h
* search for strings in file matching a filter 
*/

#ifndef __CLOG_READER_H__
#define __CLOG_READER_H__

#include <cstdio>
#include <iostream>
#include "datapull.h"

using namespace std;

namespace clog_reader {

	constexpr auto OK = 0;
	constexpr auto ERROR_ALLOCATION = -1;
	constexpr auto ERROR_PROCESS = -2;

	typedef struct StringLog {
		StringLog(char * str, size_t len) :str(str), len(len) {};
		char * str;
		size_t len;
	}StringLog;




////////////////////////////////////////////////////////////////////
	//estimation string for pattern
	class EstimatedString
	{
	public:
		explicit EstimatedString(CLogReader * clr);
		~EstimatedString();

		inline void setString(char * str, size_t  len);

		// find word in place (not after *)
		inline bool wrapper_findWordPattern(size_t  in_start_pos, FilterController * fc);

		// find word in place (not after *)
		//output returned bool : true or false
		inline bool findWordPattern(char * source_, size_t  in_start_pos, size_t lenSource, char * pattern, size_t lenPattern);


		// find word after * (pos is filled if function returned true)
		inline bool  wrapper_findWordPattern_free_count_before(size_t  in_start_pos, FilterController * fc, size_t * pos);

		// find word after *
		//output pos - position of found result only if returned bool is true
		inline bool findWordPattern_free_count_before(char * source_, size_t  in_start_pos, size_t lenSource, char * pattern, size_t lenPattern, size_t * pos);


	private:
		CLogReader * s_clr;
		char * s_str{ nullptr };
		size_t  s_len{ 0 };
	};



//////////////////////////////////////////////////////////////////////



	//u need TC.data = TD[], TC.num_descriptions
	//TD -descriptions of patterns, TC - patterns(controllers)
	class CLogReader
	{
	public:
		//friend EstimatedString;

		explicit CLogReader(size_t reading_size = 1024 * 1024);
		~CLogReader();

		bool    Open(const char * fname_in);                       // открытие файла, false - ошибка
		void    Close();                         // закрытие файла

		bool    SetFilter(const char *filter);  // установка фильтра строк, false - ошибка
		bool    GetNextLine(char *buf, const int bufsize) ;

	private:

		Factory<char> buffers;


		bool read_next_buffer();// one of two								
		int parsing_pattern();//parsing pattern

		char * start_process_string(char * str, size_t len);
		char * process_string(char * str, size_t len, size_t start_pos, int num_first_pattern);
		char * check_patterns(char * str, size_t len, size_t start_pos = 0);

		StringLog find_next_line();

	private:


		//for input file //////////////////////////////////////
		FILE * m_pInFile{ nullptr };
		int   m_end_process{ 0 };
		int   m_error_status{ 0 };

		///////////////////////////////////////////////////////
		EstimatedString es{ this };
		ContainerFilters * container{ nullptr };


		//for pattern /////////////////////////////////////////
		char * m_filter{ nullptr };
		char ** m_filter_words{ nullptr };
		int  *  m_filter_wordsLength{ nullptr };
		FilterStatus  *  m_filter_status{ nullptr };
		int  m_filter_num_words{ 0 };

		//for buffer  /////////////////////////////////////////
		int   m_num_buf_read{ -1 };
		int   m_num_buf_using{ -1 };
		int   m_reached_end_of_file{ -1 };

		/// tmp buffer ///////////////////////////////////////
		char * m_temp_buffer{ nullptr };
		size_t m_size_temp_buffer{ 0 };


		size_t   m_reading_size{ 0 };
		size_t   m_readed_size[2] = { 0, 0 };
		char * m_buf[2] = { nullptr, nullptr };
		char * m_end_buf[2] = { nullptr, nullptr };

		char * m_current_pointer{ nullptr };
		char * m_end_current_buffer_pointer{ nullptr };

		char * m_rest_pointer{ nullptr };// rest of buffer
		char * m_end_rest_pointer{ nullptr };

		//////////////////////////////////////////////////////





	};



};//namespace


#endif
