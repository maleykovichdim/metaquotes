// log_reader.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "clog_reader.h"
#include <cstdio>
#include <iostream>
#include <cstring>
//#define _CRT_SECURE_NO_WARNINGS 1


namespace clog_reader {



//////////////////////////////////////////////////////////////////////////////////


	EstimatedString::EstimatedString(CLogReader * clr):s_clr(clr) {}
	EstimatedString::~EstimatedString(void) {
			//todo ?? free s_str
		}


	inline void EstimatedString::setString(char * str, size_t  len) {
			s_str = str; s_len = len;
	};

	//------------------------------------------------------------------------------------------------------------------------------------------------
	// in_start_pos - position in estimated string
	// num_pattern - position pattern in line
	inline bool EstimatedString::wrapper_findWordPattern(size_t  in_start_pos, FilterController * fc)
	{
		return findWordPattern(s_str, in_start_pos, s_len, fc->word, fc->len);
	}


	// find word in place (not after *)
	//output returned bool : true or false
	inline bool EstimatedString::findWordPattern(char * source_, size_t  in_start_pos, size_t lenSource, char * pattern, size_t lenPattern)
	{
		char * source = source_ + in_start_pos;
		if (!memcmp(source, pattern, lenPattern)) return true;
		//todo : check out of range ?????
		return false;
	}

//------------------------------------------------------------------------------------------------------------------------------------------------
	// in_start_pos - position in estimated string
	// num_pattern - position pattern in line
	inline bool EstimatedString::wrapper_findWordPattern_free_count_before(size_t  in_start_pos, FilterController * fc, size_t * pos)
	{
		return findWordPattern_free_count_before(s_str, in_start_pos, s_len, fc->word, fc->len,  pos);
	}



	// find word after *
	//output pos - position of found result only if returned bool is true
	inline bool EstimatedString::findWordPattern_free_count_before(char * source_, size_t  in_start_pos, size_t lenSource, char * pattern, size_t lenPattern, size_t * pos)
	{
		char * source = source_;
		size_t wPos = -1;
		for (size_t i = in_start_pos; i < lenSource - lenPattern ; i++)
		{
			if (source[i] == pattern[0])
			{
				int equal = 1;
				for (int k = 1; k < lenPattern; k++)
				{
					if (source[i + k] != pattern[k]) equal = 0;
					//to do: 
					//? memcmp case for lenPattern >= 16
					//?simd optimization
				}
				if (equal)
				{
					wPos = i;
					break;
				}
			}
		}
		if (wPos != -1)
		{
			*pos = wPos;
			return true;
		}
		return false;
	}



////////////////////////////////////////////////////////////////////////////////////


	CLogReader::d(size_t reading_size) :m_reading_size(reading_size)
		{
			m_buf[0] = (char*)malloc(m_reading_size * 2 + 32);
			m_buf[1] = m_buf[0] + m_reading_size + 16;
			if (!(m_buf[0]))
			{
				free(m_buf[0]);
				reading_size = 1024 * 1024;
				m_buf[0] = (char*)malloc(m_reading_size * 2 + 32);
				m_buf[1] = m_buf[0] + m_reading_size + 16;
			}
			if (!(m_buf[0])) {
				reading_size = 0;
				m_error_status = -1;
			}
			else// set 0 at the end of blocks 
			{
				memset(m_buf[0] + m_reading_size, 0, 16);
				memset(m_buf[1] + m_reading_size, 0, 16);
			}

			container = new ContainerFilters;
			if (!container) m_error_status = -1;

			cout << "CLogReader() done | m_reading_size= " << m_reading_size <<" error:" << m_error_status<<  endl;
		};

//------------------------------------------------------------------------------------------------------------------------------------------------


	CLogReader::~CLogReader() {
			Close();
			if (m_filter)
				free(m_filter);
			if (m_buf[0])
				free(m_buf[0]);
			if (container)
				delete container;
			cout << "~CLogReader() " << endl;
		};

//------------------------------------------------------------------------------------------------------------------------------------------------


		bool    CLogReader::Open(const char * fname_in) {
			fopen_s(&m_pInFile, fname_in, "rb");
			if (m_pInFile)
				return true;
			return false;
		};   
		
		
		
		// открытие файла, false - ошибка

//------------------------------------------------------------------------------------------------------------------------------------------------


		void    CLogReader::Close() {
			if (m_pInFile)
				fclose(m_pInFile);
		};                         // закрытие файла


//------------------------------------------------------------------------------------------------------------------------------------------------


		bool    CLogReader::SetFilter(const char *filter) {
			if (m_filter) {
				free(m_filter); m_filter = nullptr;
			}
			size_t size = (int)strlen(filter);//
			m_filter = (char*)malloc(size + 1);
			if (!m_filter || size < 1) return false;
			memcpy(m_filter, filter, size);
			m_filter[size] = '\0';
			cout << "filter was copied" << endl;
			if (parsing_pattern() != OK) return false;
			cout << "filter was parsed" << endl;
			return true;
		};   // установка фильтра строк, false - ошибка

//------------------------------------------------------------------------------------------------------------------------------------------------

		bool    CLogReader::GetNextLine(char *buf,           // запрос очередной найденной строки,
			const int bufsize) {

			if (m_error_status || container->m_error_status) return false;

			char * found = nullptr;
			bool endReached = false;
			bool endReached_out = false;

			while (true)
			{

				if (m_num_buf_read == -1)
				{
					bool res = read_next_buffer();
					if (!res) {
						m_error_status = -2;
						return false;
					}
					m_current_pointer = m_buf[m_num_buf_read];
					m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
				}

				if (m_current_pointer < m_end_current_buffer_pointer)
				{
					size_t len = 0;
					while (true)
					{
						len = strlen(m_current_pointer);
						found = check_patterns(m_current_pointer, len);
						m_current_pointer += len;
						endReached = (m_current_pointer == m_end_current_buffer_pointer);
						endReached_out = (m_current_pointer >= m_end_current_buffer_pointer + 1);
						if (found || endReached || endReached_out) break;
					}

					if (found && !endReached) {
						memcpy(buf, m_current_pointer - len, bufsize);
						return true;
					}

					return false;//remove
					//TODO point
				}


			}//while
#if 0





				if (m_current_pointer < m_end_current_buffer_pointer)
				{
					size_t len = 0;
					while (true)
					{
						len = strlen(m_current_pointer);
						found = check_pattern(m_current_pointer, len);
						m_current_pointer += len;
						endReached = (m_current_pointer == m_end_current_buffer_pointer);
						endReached_out = (m_current_pointer >= m_end_current_buffer_pointer + 1);
						if (found || endReached || endReached_out) break;
					}

					if (found && !endReached) return StringLog(found, len);// return if found

					if (found && (endReached || endReached_out))
					{
						bool res = read_next_buffer();
						if (!res) 	m_end_process = 1;
						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
						return StringLog(found, len);// return if found
					}


					if (!found && endReached) {
						bool res = read_next_buffer();
						if (!res) {
							m_end_process = 1;
							return StringLog(nullptr, 0);
						}
						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
						continue;
					}

					if (!found && endReached_out) {
						bool res = read_next_buffer();
						if (!res) {
							m_end_process = 1;
							return StringLog(nullptr, 0);
						}
						char * previous = m_current_pointer - len;
						size_t len_prev = len;


						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];

						len = strlen(m_current_pointer);

						size_t full_len = len_prev + len;
						if (m_temp_buffer)
						{
							if (m_size_temp_buffer < full_len)
							{
								m_temp_buffer = (char*)realloc(m_temp_buffer, full_len);
								m_size_temp_buffer = full_len;
							}
						}
						else
						{
							m_temp_buffer = (char*)malloc(m_size_temp_buffer);
							m_size_temp_buffer = full_len;
						}

						if (m_temp_buffer == nullptr)
						{
							m_size_temp_buffer = 0;
							m_end_process = 1; // todo: ???? what to do
							return StringLog(nullptr, 0);
						}

						memcpy(m_temp_buffer, previous, len_prev);
						memcpy(m_temp_buffer + len_prev, m_current_pointer, len);
						if (m_size_temp_buffer > full_len) memset(m_temp_buffer + full_len, 0, m_size_temp_buffer - full_len);

						found = check_pattern(m_temp_buffer, full_len);
						m_current_pointer += len;
						if (found)
							return StringLog(found, full_len);
						else
							return StringLog(nullptr, 0);
					}

				}
			}


#endif



			return true;
		};  // buf - буфер, bufsize - максимальная длина
								 // false - конец файла или ошибка
//------------------------------------------------------------------------------------------------------------------------------------------------



		bool CLogReader::read_next_buffer()// one of two
		{
			if (m_reached_end_of_file != -1) return false;

			m_num_buf_read += 1;
			if (m_num_buf_read == 2) m_num_buf_read = 0;

			m_readed_size[m_num_buf_read] = fread(m_buf[m_num_buf_read], m_reading_size, 1, m_pInFile);//read block
			m_end_buf[m_num_buf_read] = m_buf[m_num_buf_read] + m_readed_size[m_num_buf_read] - 1;// end of block(last byte)



			if (m_readed_size[m_num_buf_read] < m_reading_size)
			{
				if (m_readed_size[m_num_buf_read] != 0)
					m_reached_end_of_file = m_num_buf_read;
				else
				{
					int another = 1;
					if (m_num_buf_read) another = 0;
					if (m_readed_size[another])
						m_reached_end_of_file = another;
					else
						m_reached_end_of_file = m_num_buf_read;
				}
			}

			if (m_readed_size[m_num_buf_read] > 0) return true;


			return false;
		}

//------------------------------------------------------------------------------------------------------------------------------------------------

//parsing pattern

		int CLogReader::CLogReader::parsing_pattern()
		{

			if (m_error_status) return m_error_status;
			if (container->m_error_status) return container->m_error_status;


			size_t size = strlen(m_filter);
			size_t np = 0;
			for (size_t i = 0  ;  i < size; )//parsing of pattern
			{
				
				if (np >= container->m_num_patterns)
					container->increase_num_patterns(container->m_num_patterns * 2);

				if (container->m_error_status) return container->m_error_status;


				FilterController * curlexeme = &container->m_patterns[np];
				
				if (m_filter[i] == '*')
				{
					curlexeme->word = &m_filter[i];
					curlexeme->len = 1;
					np++;

					i++;
					while (true) { if (m_filter[i] == '*' && i < size) i++; else break; };
					continue;
				}

				if (m_filter[i] == '?')
				{
					curlexeme->word = &m_filter[i];
					curlexeme->len = 1;
					np++;
					i++;
					continue;
				}

				//words

				curlexeme->word = &m_filter[i];
				curlexeme->len = 1;

				while (true)
				{
					i++;
					if (i >= size || m_filter[i] == '?' || m_filter[i] == '*') break;
					curlexeme->len++;
				}
				np++;
			}

			container->m_num_filled_patterns = np;

			int wasStarsbefore = 0;

			for (int i = 0; i < container->m_num_filled_patterns; i++)
			{
				if (i != 0) container->m_patterns[i].prev = &container->m_patterns[i - 1];
				if (i < container->m_num_filled_patterns - 1)
					container->m_patterns[i].next = &container->m_patterns[i + 1];

				if (i != 0 && container->m_patterns[i - 1].word[0] == '*')
					container->m_patterns[i].isPrevUndefLen = 1;

				if (container->m_patterns[i].word[0] == '?')
					container->m_patterns[i].type_lexeme = 0;
				else if (container->m_patterns[i].word[0] == '*')
					container->m_patterns[i].type_lexeme = 1;
				else
					container->m_patterns[i].type_lexeme = 2;

				if (i == 0)
					container->m_patterns[i].pos_in_list = 0;

				if (i != 0 && i < container->m_num_filled_patterns-1)
					container->m_patterns[i].pos_in_list = 1;

				if ( i == container->m_num_filled_patterns - 1)
					container->m_patterns[i].pos_in_list = 2;


				if (container->m_patterns[i].type_lexeme == 0) wasStarsbefore++;
				if (wasStarsbefore) container->m_posFirstUndefPos = i;

			}

			for (int i = 0; i < container->m_num_filled_patterns; i++)
			{
				cout << "[" << i << "]=";
				for (int j = 0; j < container->m_patterns[i].len; j++) cout << (char)(container->m_patterns[i].word[j]);				
				cout << endl;
			}
			return 0;
		}


//------------------------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------------------------------------------------------------------








		char * CLogReader::check_patterns(char * str, size_t len, size_t start_pos)
		{
					
			char * ret = nullptr;
			if (len < 1) return ret;

			es.setString(str, len);


			size_t num_pattern = 0;
			FilterController * cur = nullptr;
			bool previous_was_star = false;
			size_t offset = 0;
			int event_status = 0;

			FilterStatus * prevf = nullptr;
			FilterStatus * cf = nullptr;
			
			//number of pattern

			while (true) {

				size_t previous_num_pattern = 0;
				while (true)
				{

					cur = &container->m_patterns[num_pattern];//current pattern(controller)

					if (previous_num_pattern != num_pattern)// if next pattern
					{
						previous_num_pattern = num_pattern;
						//movement
						prevf = cf;
						//cur->num_descriptions++;
						cur->cur_desc++;
						if (cur->cur_desc >= cur->num_descriptions) container->increase_num_descriptions(num_pattern, cur->num_descriptions * 2);
						cf = &cur->data[cur->cur_desc];
						cf->prev = prevf;
						if (cf->prev) cf->prev->next = cf;
					}

					if (offset >= len) {// line was finished
						event_status |= LineWasFinished; break;
					}

					if (cur->type_lexeme == 0)
					{//?
						cf->pos = offset;
						cf->len = 1;
						offset++;

						switch (cur->pos_in_list)
						{
						case 0:
						case 1:
							break;
						case 2:
							if (offset != len) {
								if (cur->isPrevUndefLen) {
									offset = cf->pos + 1;//start searching from next symbol
									continue;
								}
								event_status |= LastNotFound;
							}
							else
								event_status |= FOUND;//OK
							break;
						}//sw
						if (event_status) break;
						num_pattern++;
						continue;
					}//?

					if (cur->type_lexeme == 1)
					{//*
						cf->pos = offset;
						cf->len = 0;

						switch (cur->pos_in_list)
						{
						case 0:
						case 1:
							break;
						case 2:
							if (offset < len)
								event_status |= FOUND;//OK
							else
								event_status |= LastNotFound;//last not found							
							break;
						}//sw
						if (event_status) break;
						num_pattern++;
						continue;
					}//*


					if (cur->type_lexeme == 2)
					{//word

						size_t start_pos = offset;
						bool found = false;
						size_t pos = 0;
						if (cur->isPrevUndefLen)
							found = es.wrapper_findWordPattern_free_count_before(start_pos, cur, &pos);
						else
							found = es.wrapper_findWordPattern(start_pos, cur);

						switch (cur->pos_in_list)
						{
						case 0:
							if (!found)
								event_status |= FirstNotFound;
							else
							{
								cf->pos = offset;
								cf->len = cur->len;
								offset += cur->len;
							}
							break;
						case 1:

							if (found) {
								cf->pos = (cur->isPrevUndefLen) ? pos : offset;
								cf->len = cur->len;
								offset = cf->pos + cf->len;
							}
							else
								event_status |= MiddleNotFound; 

							break;
						case 2:

							if (found) {
								cf->pos = (cur->isPrevUndefLen) ? pos : offset;
								cf->len = cur->len;

								if ((cf->pos + cf->len) == len)//?
									event_status |= FOUND;//OK
								else
								{
									if (cur->isPrevUndefLen) {
										offset = cf->pos + 1;//start searching from next symbol
										continue;
									}
									event_status |= LastFoundBefore; //TODO: check it
								}
							}
							else
								event_status |= LastNotFound;

						}//sw

						//if (cf->hasUndefLenBefore && found)
						//	cf->prev->len = cf->pos - cf->prev->pos;

						if (event_status) break;// found of wrong line
						num_pattern++;
						continue;
					}//word


				}//while
				cf->event_status = event_status;
				//////////////

				//logic block
				if (cf->event_status & FOUND) //OK
					return str;
				if (cf->event_status & FirstNotFound) 
					return nullptr;

				if (( cf->event_status > LastFoundBefore ) && (!container->m_posFirstUndefPos))//no stars and not found
					return nullptr;

				if ((cf->event_status > LastFoundBefore) && (num_pattern <  container->m_posFirstUndefPos))//error until star
					return nullptr;

				///////////////////////////////////////////////////////
				//variation block

				num_pattern = container->m_posFirstUndefPos;
				{

				
				
				}





				//
				///////////////////////////////////////////////////////

				if (cf->event_status & LastFoundBefore)
				{
					if (cf->hasUndefLenBefore)
					{



					}

				}






			}// external while 




			return str;
		}


//------------------------------------------------------------------------------------------------------------------------------------------------


		StringLog CLogReader::find_next_line()
		{

			while (true)
			{
				char * found = nullptr;
				bool endReached = false;
				bool endReached_out = false;

				if (m_num_buf_read == -1)
				{
					read_next_buffer();
					bool res = read_next_buffer();
					if (!res) {
						m_end_process = 1;
						return StringLog(nullptr, 0);
					}
					m_current_pointer = m_buf[m_num_buf_read];
					m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
				}


				if (m_current_pointer < m_end_current_buffer_pointer)
				{
					size_t len = 0;
					while (true)
					{
						len = strlen(m_current_pointer);
						found = check_patterns(m_current_pointer, len);
						m_current_pointer += len;
						endReached = (m_current_pointer == m_end_current_buffer_pointer);
						endReached_out = (m_current_pointer >= m_end_current_buffer_pointer + 1);
						if (found || endReached || endReached_out) break;
					}

					if (found && !endReached) return StringLog(found, len);// return if found

					if (found && (endReached || endReached_out))
					{
						bool res = read_next_buffer();
						if (!res) 	m_end_process = 1;
						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
						return StringLog(found, len);// return if found
					}


					if (!found && endReached) {
						bool res = read_next_buffer();
						if (!res) {
							m_end_process = 1;
							return StringLog(nullptr, 0);
						}
						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];
						continue;
					}

					if (!found && endReached_out) {
						bool res = read_next_buffer();
						if (!res) {
							m_end_process = 1;
							return StringLog(nullptr, 0);
						}
						char * previous = m_current_pointer - len;
						size_t len_prev = len;


						m_current_pointer = m_buf[m_num_buf_read];
						m_end_current_buffer_pointer = m_end_buf[m_num_buf_read];

						len = strlen(m_current_pointer);

						size_t full_len = len_prev + len;
						if (m_temp_buffer)
						{
							if (m_size_temp_buffer < full_len)
							{
								m_temp_buffer = (char*)realloc(m_temp_buffer, full_len);
								m_size_temp_buffer = full_len;
							}
						}
						else
						{
							m_temp_buffer = (char*)malloc(m_size_temp_buffer);
							m_size_temp_buffer = full_len;
						}

						if (m_temp_buffer == nullptr)
						{
							m_size_temp_buffer = 0;
							m_end_process = 1; // todo: ???? what to do
							return StringLog(nullptr, 0);
						}

						memcpy(m_temp_buffer, previous, len_prev);
						memcpy(m_temp_buffer + len_prev, m_current_pointer, len);
						if (m_size_temp_buffer > full_len) memset(m_temp_buffer + full_len, 0, m_size_temp_buffer - full_len);

						found = check_patterns(m_temp_buffer, full_len);
						m_current_pointer += len;
						if (found)
							return StringLog(found, full_len);
						else
							return StringLog(nullptr, 0);
					}

				}
			}

		}

//------------------------------------------------------------------------------------------------------------------------------------------------
		//template class CLogReader<FilterStatus, FilterController>;


};//namespace
