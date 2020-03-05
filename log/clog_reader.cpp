// log_reader.cpp : 
//

#include "pch.h"
#include "clog_reader.h"
#include <cstdio>
#include <iostream>
#include <cstring>



namespace clog_reader {



	CLogReader::CLogReader(size_t reading_size) :
		m_reading_size(reading_size),
		m_error_status(0),
		m_pInFile{ nullptr },
		m_filter{ nullptr }
	{
		data = new (std::nothrow) DataFromFile<char>(4, reading_size);//todo remove /16
	};

	CLogReader::CLogReader(int mt, size_t reading_size ) :
		m_reading_size(reading_size),
		m_error_status(0),
		m_pInFile{ nullptr },
		m_filter{ nullptr }
	{
		data = new (std::nothrow) DataFromFile<char>(4, reading_size, mt);//todo remove /16
	};





	CLogReader::~CLogReader()
	{
		if (data) {
			delete data; data = nullptr;
		};
		Close();
	}

	bool   CLogReader::Open(const char * fname_in)
	{
		fopen_s(&m_pInFile, fname_in, "rb");
		if (m_pInFile) {
			data->setFileInfo(m_pInFile);
			return true;
		}
		m_error_status = -3;
		return false;
	}

	void CLogReader::Close()
	{
		if (m_pInFile) {
			fclose(m_pInFile);
			m_pInFile = nullptr;
			m_error_status = 1;
		}
	}

	bool    CLogReader::SetFilter(const char *filter)
	{
		if (m_error_status && m_error_status != 2) return false;
		if (m_filter) {
			free(m_filter); m_filter = nullptr;
		}
		size_t size = (int)strlen(filter);//
		m_filter = (char*)malloc(size + 1);
		if (!m_filter || size < 1) {
			m_error_status = 2;
			return false;
		}
		memset(m_filter, 0, size);

		size_t cp = 0;
		int isPrevStar = 0;
		for (size_t i = 0; i < size; i++)// removing several stars
		{
			char a = filter[i];
			if (isPrevStar && a == '*') continue;
			m_filter[cp++] = a;
			if (a == '*') 
				isPrevStar = 1;
			else 
				isPrevStar = 0;
		}
		m_filter[cp] = '\0';
		if (m_error_status == 2) m_error_status = 0;
		return true;
	}


	bool    CLogReader::GetNextLine(char *buf, const int bufsize)
	{
		
		if ((m_error_status) || (m_pInFile == nullptr) || (m_filter == nullptr)) return false;

		char *buf_=nullptr;
		size_t  len_=0;
		size_t size_filter = strlen(m_filter);



		while (true)
		{
			bool a = data->GetNewLine(&buf_, &len_);
			if (a == false) {
				return false;
			}

			if (size_filter > len_) continue;

			pr(buf_, (int)len_);

			bool rv = match.consilience(m_filter, buf_);
			if (rv)
			{
				size_t a = (bufsize < len_) ? bufsize - 1 : len_;
				memcpy(buf, buf_, a);
				buf[a] = '\0';
				return true;
			}

		}
		return false;
	}


};//namespace
