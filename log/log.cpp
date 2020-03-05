// log.cpp
//example for testing clog_reader class

#include "pch.h"
#include "clog_reader.h"
#include <cstdio>
#include <stdio.h>
#include <iostream>



int main(int argc, char *argv[])
{
#if 0
	if (argc != 3) {
		cout << "example of using: log.exe \"c:\\\\work\\\\cc.txt\" \"**anytime**\"" << endl;
		return -1;
	}

	const char * name_file = argv[1];// "c:\\work\\cc.txt";
	const char * filter = argv[2]; //"**anytime**";
#endif
	{
		LOG_DURATION("MT");
		const char * name_file = "c:\\work\\ssa.txt";
		//const char * filter = "**Dragons**";
		const char * filter = "**face**";

		clog_reader::CLogReader CL(1, 1024*1024);
		bool open_ = CL.Open(name_file);
		if (open_)
		{
			cout << "file is opened ::" << name_file << endl;
			const int bufsize = 1024;
			char *buf = new  (std::nothrow) char[bufsize];

			bool a = CL.SetFilter(filter);
			if (a)
			{
				cout << "the following filter is set ::" << filter << endl;
				bool b = true;
				int c = 0;
				while (b)
				{
					b = CL.GetNextLine(buf, bufsize);
					if (b)
					{
						//cout << "---------------------------------\n" << buf << "\n---------------------------------\n" << endl;
						c++;
					}
				}
				cout << "found " << c << " matches" << endl;
			}
			else
				cout << "the following filter is not set ::" << filter << "  error" << endl;
			delete[] buf;
		}
		else
		{
			cout << "file is not opened :: " << name_file << " error" << endl;
		}

		cout << "char for exit" << endl;
		//getchar();
	}


//////////////////////////////////////////////////////////////////////////


	{
		LOG_DURATION("ST");
		const char * name_file = "c:\\work\\ssa.txt";
		//const char * filter = "**Dragons**";
		const char * filter = "**face**";

		clog_reader::CLogReader CL;
		bool open_ = CL.Open(name_file);
		if (open_)
		{
			cout << "file is opened ::" << name_file << endl;
			const int bufsize = 1024;
			char *buf = new  (std::nothrow) char[bufsize];

			bool a = CL.SetFilter(filter);
			if (a)
			{
				cout << "the following filter is set ::" << filter << endl;
				bool b = true;
				int c = 0;
				while (b)
				{
					b = CL.GetNextLine(buf, bufsize);
					if (b)
					{
						//cout << "---------------------------------\n" << buf << "\n---------------------------------\n" << endl;
						c++;
					}
				}
				cout << "found " << c << " matches" << endl;
			}
			else
				cout << "the following filter is not set ::" << filter << "  error" << endl;
			delete[] buf;
		}
		else
		{
			cout << "file is not opened :: " << name_file << " error" << endl;
		}

		cout << "char for exit" << endl;
		//getchar();
	}

////////////////////////////////////////////////////////////

	{
		LOG_DURATION("MT");
		const char * name_file = "c:\\work\\ssa.txt";
		//const char * filter = "**Dragons**";
		const char * filter = "**face**";

		clog_reader::CLogReader CL(1, 1024 * 1024);
		bool open_ = CL.Open(name_file);
		if (open_)
		{
			cout << "file is opened ::" << name_file << endl;
			const int bufsize = 1024;
			char *buf = new  (std::nothrow) char[bufsize];

			bool a = CL.SetFilter(filter);
			if (a)
			{
				cout << "the following filter is set ::" << filter << endl;
				bool b = true;
				int c = 0;
				while (b)
				{
					b = CL.GetNextLine(buf, bufsize);
					if (b)
					{
						//cout << "---------------------------------\n" << buf << "\n---------------------------------\n" << endl;
						c++;
					}
				}
				cout << "found " << c << " matches" << endl;
			}
			else
				cout << "the following filter is not set ::" << filter << "  error" << endl;
			delete[] buf;
		}
		else
		{
			cout << "file is not opened :: " << name_file << " error" << endl;
		}

		cout << "char for exit" << endl;
		//getchar();
	}













	return 0;
}

