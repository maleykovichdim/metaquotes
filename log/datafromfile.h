//class for reading data from file into data-pull buffer

#pragma once

#include <atomic>
#include <future>


#include "datapull.h"
#include "global_function.h"

#include "mt.h"

#include "assert.h"
namespace clog_reader {

	template <typename T>
	struct readed_data
	{
		T * data{ nullptr };
		size_t size{ 0 };
		int status{ 0 };
		//1 - end of file
		//2 no free buffers
		//-1 error data from file

		readed_data & operator=(const readed_data & v)
		{
			if (this != &v) {
				data = v.data;
				size = v.size;
				status = v.status;
			}
			return *this;
		}


	};

	template struct readed_data<char>;

	template <typename T>
	class DataFromFile : public PullBlock<T> //for block buffer from file
	{
	public:

	readed_data<T> (DataFromFile::*p_getData)();//pointer for MT or non_MT

	public:
		DataFromFile(unsigned int chunk_size_, size_t size, int flag_mt = 0) :PullBlock<T>(chunk_size_, size, 0x0A)
		{
			temp = PullBlock<T>::get();//free block for connection
			//mt = MT.Instance();
			if (flag_mt == 1)			
				this->p_getData = &DataFromFile::getData_MT;
			else
				this->p_getData = &DataFromFile::getData_;

		}


		void setFileInfo(FILE * fi) { m_pInFile = fi; };

		readed_data<T> getData()
		{
			//printf("!!!!!!!!!!!!!!!!!!!getData()!!!!!!!!!!!!!!!!!!!!!!!!\n");
			return (this->*p_getData)();
		}

		readed_data<T> getData_()
		{
			readed_data<T> a;

			if (m_error || !m_pInFile) {
				a.status = -1;//error
				return a;
			}
			a.data = PullBlock<T>::get();
			if (!a.data) {
				a.status = 2;//no free buffers
				return a;
			}
			a.size = fread(a.data, 1, PullBlock<T>::m_size, m_pInFile);//read block
			if (a.size < PullBlock<T>::m_size)
			{
				m_error = -2;
				a.status = 1;//end of file
			}
			return a;
		}

		///////////////////////////////////////////////////////
		void readfile_MT()
		{

			fordatafromfile.status = 0;
			fordatafromfile.size = 0;
			int exit = 0;
			m_flag_thread = 1;

			int fm = 0;

			while (true) {


				//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>   TH2 LOOP\n");


				//if (m_mutex_main & 2 == 0)
				{
					mutex_.lock();
					fm = 1;
					//printf(">>>>>>>>>>>>>>>>>>>>>>>>   TH2 LOCK\n");
				}



				//m_mutex_file_reading_int = 1;
				//std::scoped_lock lock(mutex_file_reading, mutex_);


				//if (fm)
				//{
				//	fm = 0;
				//	mutex_file_reading.unlock();
				//}

				//mutex_.lock();

				//printf(">>>>>>>>>>>>>>>>>>>>>>>>   second thread mutex_.lock();\n");

				if (fordatafromfile.data == nullptr)//terminate thread
				{
					exit = 1; 
					m_flag_thread = 2;
					//printf("second thread  m_flag_thread = 2 fordatafromfile.data == nullptr\n");
					fordatafromfile.size = 0;
					fordatafromfile.status = -1;
					break;
				}
				//printf(">>>>>>>>>>>>>>>>>>>>>>>>   second thread READING\n");
				fordatafromfile.size = fread(fordatafromfile.data, 1, PullBlock<T>::m_size, m_pInFile);//read block

				if (fordatafromfile.size < PullBlock<T>::m_size)
				{
					//m_error = -2;
					fordatafromfile.status = 1;//end of file
					exit = 1;
					//printf("second thread size reading less than block\n");
				}
//////////////////////////////////////////////////////////////


				m_flag_thread = 2;
				//printf(">>>>>>>>>>>>>>>>>>>>>>>>   TH2 m_flag_thread = 2\n");
				//printf("second thread m_flag_thread = 2\n");		

				if (exit) break;


				if (fm)
				{
					mutex_.unlock();
					fm = 0;
					//printf(">>>>>>>>>>>>>>>>>>>>>>>>   TH2 UNLOCK\n");
				}

				sem.acquire();

				
			}//while

			if (fm)
			{
				mutex_.unlock();
			}
			//printf(">>>>>>>>>>>>>>>>>>>>>>>>   second thread  FINISH loop\n");
		}
		///////////////////////////////////////////////////////
		readed_data<T> getData_MT()
		{

			readed_data<T> a;

			//printf("GET DATA\n");

			if (m_error || !m_pInFile) {
				a.status = -1;//error
				//m_mutex_main = 0;
				sem.release();
				//printf("ERROR GET DATA\n");
				return a;
			}

			if (m_flag_thread == 0)//once at start
			{

				a.data = PullBlock<T>::get();
				if (!a.data) {
					a.status = 2;//no free buffers
					return a;
				}


				fordatafromfile = a;
				//mutex_file_reading.lock();
				//printf("main thread mutex_file_reading.lock() END\n");


				thread_ = std::async(std::launch::async, &DataFromFile::readfile_MT, this);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				//std::thread t_read2([&]() {
				//	a.read();
				//});
			}
///////////////////////////////////////////////////////////////////////////////////////////// START LOCK
			


///////////////////////////////////////////////////////////////////////////////////////////// 
			int fm = 0;
			if (m_flag_thread != 2)
			{
				mutex_.lock();
				fm = 1;
				//printf("TH1 LOCK\n");
			}
			//else
				//printf("TH1 SKIP LOCK\n");


////////////////////////////////////////////////////////////////////////////////////////////		
			// GET DATA, CHANGE BUFFER
			{
				a = fordatafromfile;//get result 
				
				///////  prepare new buffer ////////////////
				fordatafromfile.data = PullBlock<T>::get();
				if (!fordatafromfile.data) {
					printf("ERROR GET FREE BUFFER\n");
					fordatafromfile.data = nullptr;
					fordatafromfile.status = 2;//no free buffers
				}
				fordatafromfile.status = 0;
				fordatafromfile.size = 0;
				//////////////////////////////////////////////

			}

			m_flag_thread = 1;
			//printf("TH1 m_flag_thread = 1\n");
/////////////////////////////////////////////////////////////////////////////

			if (fm)
			{
				mutex_.unlock();
				fm = 0;
				//printf("TH1 UNLOCK\n");
			}
			//else
				//printf("TH1 SKIP UNLOCK\n");

			//_printf("TH1 REL SEM\n");
			sem.release();
			///////////////////////////////////////////
			if (a.size < PullBlock<T>::m_size)
			{
				m_error = -2;
				a.status = 1;//end of file
			}
			return a;
	
		}


		void freeData(readed_data<T> & a)
		{
			PullBlock<T>::free(a.data);
			a.data = nullptr;
		}




		bool    GetNewLine(char **buf, size_t * len) {

			if (!cur) {
				
				if (m_error < 0) {
					cur = nullptr;
					return false;
				}
				rd = getData();
				cur = rd.data;
			}
			if (rd.status && rd.status != 1) return false;//!!!!

			assert(rd.data + rd.size >= cur);


			size_t size = 0;
			cur--;
			while (size == 0) {
				cur++;
				size = (size_t)strlen_ascii(cur);

			}
			size++;

			assert(rd.data + rd.size >= cur);

			if (cur + size < rd.data + rd.size)
			{
				*buf = cur;
				//printf("\n!!!!!!!!!!!!!\n%s\n\n", cur);
				*len = size - 1;
				cur += size;
				assert(rd.data + rd.size >= cur);

				return true;
			}

			assert(rd.data + rd.size >= cur);

			if (cur + size == rd.data + rd.size)
			{
				*buf = cur;
				*len = size - 1;

				freeData(rd);

				if (m_error < 0) {
					cur = nullptr;
					return false;
				}
				rd = getData();
				//if (rd.status) return false;
				cur = rd.data;
				assert(rd.data + rd.size >= cur);
				return true;
			}

			assert(rd.data + rd.size >= cur);

			if (cur + size > rd.data + rd.size)
			{

				if (m_error < 0) {
					cur = nullptr;
					return false;
				}
				readed_data<T> rd2 = getData();
				if (rd2.status && rd2.status != 1)
				{
					freeData(rd);
					rd = rd2;
					assert(rd.data + rd.size >= cur);
					return false;
				}
				//temp = PullBlock<T>::get();//free block for connection
				size_t len_2part = strlen_ascii(rd2.data) + 1;//??+1 ??? !!!!!!!!!!!!!!!!!!!!!!!!!
				//assert(len_2part <= rd2.size);
				size_t len_1part = (rd.data + rd.size) - cur;
				
				if (len_2part + len_1part >= rd.size)//buffer too small ! lost all
				{
					freeData(rd);
					freeData(rd2);

					while (true)
					{
						rd = getData();
						int status = rd.status;
						if (status && status != 1)
						{
							freeData(rd);
							return false;
						}
						cur = rd.data;
						size_t len_lost = strlen_ascii(cur);
						if (len_lost < rd.size -2)
						{
							cur += len_lost+1;
							return GetNewLine(buf, len);
						}
						freeData(rd);
						cur = nullptr;
						if (status) return false;
					}
					return false;
				}

				assert(rd.data + rd.size >= cur);
				//printf("%d %d\n", len_1part, len_2part);

				memcpy(temp, cur, len_1part);
				memcpy(temp + len_1part, rd2.data, len_2part);//exeption
				*len = (len_1part + len_2part);
				*buf = temp;//check in mt
				freeData(rd);
				rd = rd2;
				cur = rd.data + len_2part;
				assert(rd.data + rd.size >= cur);
				return true;

			}
			assert(rd.data + rd.size >= cur);
			return true;
		}



		~DataFromFile()
		{
			if (temp)
			{
				PullBlock<T>::free(temp);
				temp = nullptr;
			}

			if (rd.data)
			{
				freeData(rd);
			}
		}


	protected:
		int m_error{ 0 };//-2 - end of file
		FILE * m_pInFile{ nullptr };
		T * cur{ nullptr };
		T * temp{ nullptr };
		readed_data<T> rd;

		//MT

		//MT * mt{ nullptr };
		//decltype(std::async(std::launch::async, &DataFromFile<T>::readfile_MT, DataFromFile<T>(5,5)))  thread_;
		std::future<void>  thread_;


		readed_data<T> fordatafromfile;
		
		mutable std::atomic<int> m_flag_thread{ 0 };
		//mutable std::shared_mutex mutex_;//mutex for MT mode
		//mutable std::shared_mutex mutex_file_reading;//mutex for MT mode
		mutable std::mutex mutex_;//mutex for MT mode
		mutable std::atomic<int> m_mutex_main{ 0 };
		mutable std::mutex mutex_file_reading;//mutex for MT mode
		mutable std::atomic<int> m_mutex_file_reading_int{ 0 };

		mutable Semaphore sem;

	};



	template class DataFromFile<char>;
}


