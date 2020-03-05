#pragma once

#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>

#include <condition_variable>
#include <mutex>
#include <shared_mutex >
#include <thread>
#include <vector>
#include <variant>
#include <future>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

#include <assert.h>
///////////////////////////////////////////////////////////////////////////////

class Semaphore {
public:
	explicit Semaphore(int max_count = 0) : max_count_(max_count) {}

	void acquire() {
		std::unique_lock<std::mutex> lock(mut_);
		while (count_ == max_count_) cv_.wait(lock);
		++count_;
	}

	void release() {
		std::lock_guard<std::mutex> lock(mut_);
		//assert(count_ > 0); //todo check it
		--count_;
		cv_.notify_one();
	}

private:
	int max_count_;
	int count_ = 0;
	std::mutex mut_;
	std::condition_variable cv_;
};

///////////////////////////////////////////////////////////////////


class LogDuration {
public:
	explicit LogDuration(const string& msg = "")
		: message(msg + ": ")
		, start(steady_clock::now())
	{
	}

	~LogDuration() {
		auto finish = steady_clock::now();
		auto dur = finish - start;
		//printf("%s ", message.c_str());
		std::cout 	<< message.c_str() << duration_cast<milliseconds>(dur).count()	<< " ms" << std::endl;


	}
private:
	string message;
	steady_clock::time_point start;
};

#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)

#define LOG_DURATION(message) \
  LogDuration UNIQ_ID(__LINE__){message};







/////////////////////////////////////////////////////////////////////////////
#if 0
constexpr auto MAX_THREADS = 4;

template< typename FirstFunc, typename SecondFunc, typename P1, typename P2, typename OUT>
class MT
{
public:
	//std::variant<int, float> v;

	typedef decltype(std::async(std::launch::async,  FirstFunc, P1, P2)) TH1;
	typedef decltype(std::async(std::launch::async,  SecondFunc, P1, P2)) TH2;
	typedef std::variant<TH1, TH2> OUT;

    


	void run(FirstFunc func, P1 p1, P2 p2, OUT * out)
	{
		auto a = std::async(std::launch::async, func, p1, p2, out);//
		m_threads.push_back(a);
		//return m_threads.size();
	}




public:
	static MT& Instance()
	{
		static MT singleton; 
		//{
		//	std::lock_guard<std::mutex> lock(init_mutex);
		//	if (!singleton.flag) singleton.init();
		//}
		return singleton;
	}
protected:

	std::vector<OUT> m_threads;

	MT() {}                                  // Private constructor
	~MT() {}
	MT(const MT&)=delete;                 // Prevent copy-construction
	MT& operator=(const MT&)=delete;// Prevent assignment

	//int flag{0};
	//std::mutex init_mutex;

	int num_last_mutex{ 0 };
	mutable std::shared_mutex mutex_[MAX_THREADS];

};

#endif