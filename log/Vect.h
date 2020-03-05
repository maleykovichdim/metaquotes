#pragma once
// analog vector.h

struct Consilience;

//////////////////////////////////////////////////////////////////////////////////////////

constexpr int start_size = 128;//default size for vector

template <typename T>
struct RawMemory {
	
	T * buf;
	size_t cp;
	int pseudo_alloc{ 0 };

	inline T * Allocate(size_t n)
	{
		return static_cast<T*>(operator new(n * sizeof(T)));
	}

	inline void ReAllocate(size_t n)
	{
		if (n > cp)
		{
			Deallocate();
			buf = Allocate(n);
			if (buf)
			{
				cp = n;
				pseudo_alloc = 0;
			}
		}
	}

	inline void ReAllocateCopy(size_t n)
	{
		if (n > cp)
		{
			T* buf_ = Allocate(n);
			if (buf_)
			{
				memcpy(buf_, buf, cp * sizeof(T));
				Deallocate();
				buf = buf_;
				cp = n;
				pseudo_alloc = 0;
			}
		}
	}

	inline void Deallocate()
	{
		if (!pseudo_alloc) {
			operator delete(buf);
			buf = nullptr;
		}
	}

	RawMemory(size_t n = start_size)
	{
		buf = Allocate(n);
		cp = n;
	}

	RawMemory(T * buf_, size_t cp_)// constructor for external memory
	{
		buf = buf_;
		cp = cp_;
		pseudo_alloc = 1;
	}



	~RawMemory()
	{
		Deallocate();
	}

	T * operator + (size_t i)
	{
		return buf + i;
	}

	const T * operator + (size_t i) const {
		return buf + i;
	}

	T& operator[] (size_t i) {
		return buf[i];
	}

	const T& operator[] (size_t i) const {
		return buf[i];
	}

	void Swap(RawMemory& other)
	{
		T * buf_;
		size_t cp_;

		buf_ = other.buf;
		other.buf = buf;
		buf = buf_;

		cp_ = other.cp;
		other.cp = cp;
		cp = cp_;
	}
};


template struct RawMemory<Consilience*>;
template struct RawMemory<char*>;
template struct RawMemory<char>;



template <typename T, unsigned int num_element = start_size>
class Vect
{
public: 


	Vect():	size(0)	{}

	Vect(RawMemory<T> * rm) : size(0)	
	{
		data = *rm;
	}


	~Vect()	{}
	
	Vect(const Vect & v):size(v.size)
	{
		if (size > data.cp) data.ReAllocate(size);
		if (data.buf) memcpy(data.buf, v.data.buf, size * sizeof(T));	
	}

	Vect & operator=(const Vect & v)
	{
		if (this != &v) {
			if (data.cp < v.data.cp) 
				data.ReAllocateCopy(data.cp);
			else
				memcpy(data.buf, v.data.buf, size * sizeof(T));
			size = v.size;
		}
		return *this;
	}

	Vect(Vect&& v) noexcept
	{
			size = v.size;
			data.cp = v.data.cp;
			data.buf = v.data.buf;

			v.data.buf = nullptr;
			v.data.cp = 0;
			v.size = 0;
	}

	Vect& operator = (Vect &&v) noexcept
	{

		if (this != &v) {
			if (data.buf) data.Deallocate();
			size = v.size;
			data.cp = v.data.cp;
			data.buf = v.data.buf;

			v.data.buf = nullptr;
			v.data.cp = 0;
			v.size = 0;
		}
		return *this;
	}


	inline void check_size(size_t new_size)
	{
		while (new_size > data.cp)
		{
			size_t new_capacity = (data.cp > 0) ? data.cp * 2 : 1;
			data.ReAllocateCopy(new_capacity);
		}
	}



	inline T& operator[](size_t index) {
		return data[index];
	}



	inline void push_back(const T& value) {
		check_size(size + 1);
		data[size++] = value;
	}

	inline void push_back(T&& value)
	{
		check_size(size + 1);
		data[size++] = std::move(value);
	}

	inline void emplace_back(T&& value)
	{
		check_size(size + 1);
		data[size++] = std::move(value);
	}

	inline void emplace_back()
	{
		check_size(size + 1);
		data[size++] = T();
	}

	void pop_back() {
		if (size > 0) size--;
	};

	T & back() {
		return data[size - 1];
	};


	bool empty()
	{
		if (size == 0)
			return true;
		else
			return false;
	}

	inline T* begin() {
		return data.buf;
	}

	inline T* end() {
		return data.buf + size;
	}

	inline const T* begin() const {
		return data.buf;
	}

	inline const T* end() const {
		return data.buf + size;
	}

	bool isEmpty()
	{
		if (size == 0) return true;
		return false;
	}


private:
	RawMemory<T> data;
	size_t size;

};

template class Vect<Consilience*>;
template class Vect<char*>;
template class Vect<char>;

