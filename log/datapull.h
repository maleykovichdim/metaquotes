//class PullBlock - data-pull of char buffers(use)
//class PullObj - data-pull of objects(no use here)


#pragma once

#include "Vect.h"




template <typename T>
class Factory
{
public:
	virtual ~Factory() {};
	virtual T* get() = 0;
	virtual void free(T* obj) = 0;
};

template <typename T>
class PullBlock : public Factory<T> //for block buffer
{

public:

	PullBlock(unsigned int chunk_size , size_t size )
	{
		m_chunk_size = chunk_size;
		m_size = size;
		data = (T*)operator new (size * m_chunk_size * sizeof(T));
		for (int i = 0; i < m_chunk_size; ++i)
		{
			T * p = data + i * m_size;
			free_ptrs.push_back(p);
		}
	}

	PullBlock(unsigned int chunk_size, size_t size, char pad_value )//= 0x0A
	{
		m_chunk_size = chunk_size;
		m_size = size;
		data = (T*)operator new ((size + 16) * m_chunk_size * sizeof(T));
		for (int i = 0; i < m_chunk_size; ++i)
		{
			T * p = data + i * m_size + i * 16;
			memset(p +  m_size, pad_value, 16 * sizeof(T));
			free_ptrs.push_back(p);
		}
	}



	~PullBlock()
	{
		operator delete (data);
	}

	T* get() 
	{
		if (free_ptrs.empty()) return nullptr;
		T* block = free_ptrs.back();
		free_ptrs.pop_back();

#if 0
		for (int i = 0; i < 16; i++)
			if (block[1024 + i] != 0x0A && block[1024 + i] != '\0')
				//printf("%d %d", block[1024 + i],'\0');
				printf("get datapull block problem [%d] %d\n", block[1024 + i],i);

		//printf("\n end get\n");
#endif

		return block;
	}

	void free(T* block)
	{
#if 0
		for (int i = 0; i < 16; i++)
			if (block[1024 + i] != 0x0A && block[1024 + i] != '\0')
				printf("free datapull block problem [%d] %d\n", block[1024 + i],i);
		//printf("end free\n");
#endif

		free_ptrs.push_back(block);
	}

protected:
	Vect<T*> free_ptrs;
	T * data{ nullptr };
	size_t m_chunk_size;
	size_t m_size;
};

template class PullBlock<char>;




template <typename T>
class PullObj : public Factory<T>
{

public:

	PullObj(size_t num_objects_)//for class buffer
	{
		data = operator new (num_objects_ * sizeof(T));
		rm = new RawMemory(data, num_objects_);
		buffer = new Vect<T>(rm);
		num_objects = num_objects_;
		makeMore();
	}

	~PullObj()
	{
		if (buffer) delete buffer;
		if (data) operator delete (data);
	}

	inline T* get()
	{
		if (free_ptrs.isEmpty()) return nullptr;
		//if (free_ptrs.empty())
		//{
		//	makeMore();
		//}
		T* obj = free_ptrs.back();
		free_ptrs.pop_back();
		return obj;
	}



	inline void free(T* obj) {
		free_ptrs.push_back(obj);
	}



private:

	RawMemory<T> * rm;
	Vect<T> * buffer;
	Vect<T*> free_ptrs;
	T * data{ nullptr };

	size_t  num_objects;
	void makeMore() {
		for (int i = 0; i < num_objects; ++i)
		{
			buffer->emplace_back();//place for obj
			free_ptrs.emplace_back(&buffer->back());//pointers
		}
	}
};






