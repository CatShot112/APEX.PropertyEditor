#pragma once

#include <vector>
#include <string>
#include <cassert>

using std::vector;
using std::string;
using std::ostream;

class DataBuf {
	// Typedefs
	typedef vector<uint8_t> Data;

	// Variables
	Data buf;
	size_t readOffset;

public:
	// Typedefs
	typedef Data::iterator iter;
	typedef Data::reference ref;
	typedef Data::const_iterator const_iter;
	typedef Data::const_reference const_ref;

	// Constructors
	DataBuf();
	DataBuf(DataBuf& oth);
	DataBuf(const DataBuf& oth);
	DataBuf(const DataBuf& oth, size_t off);
	DataBuf(const string& str);

	// Iterators
	iter begin();
	iter end();
	const_iter begin() const;
	const_iter end() const;

	// Operators
	DataBuf& operator<< (string data);
	DataBuf& operator<< (DataBuf& data);
	DataBuf& operator<< (const DataBuf& data);

	DataBuf& operator>> (string& data);
	DataBuf& operator>> (DataBuf& data);

	DataBuf& operator= (DataBuf&& data) noexcept;
	DataBuf& operator= (const DataBuf& data);

	ref operator[] (Data::size_type i);
	const_ref operator[] (Data::size_type i) const;

	// Templates
	template <typename T>
	DataBuf& operator<< (T data) {
		std::reverse((uint8_t*)&data, (uint8_t*)&data + sizeof(T));
		Append(data);

		return *this;
	}

	template <typename T>
	DataBuf& operator>> (T& data) {
		assert(readOffset + sizeof(T) <= GetSize());
		data = *(T*)&buf[readOffset];
		std::reverse((uint8_t*)&data, (uint8_t*)&data + sizeof(T));
		readOffset += sizeof(T);

		return *this;
	}

	template <typename T>
	void Append(T data) {
		size_t size = sizeof(data);
		size_t endPos = buf.size();

		buf.resize(endPos + size);
		memcpy(&buf[endPos], &data, size);
	}

	// Member Functions
	size_t GetSize() const;
	size_t GetRemaining() const;
	size_t GetReadOffset() const;

	string ToString() const;
	bool IsFinished() const;
	bool IsEmpty() const;

	void Reserve(size_t amount);
	void Resize(size_t size);
	void Erase(iter it);

	void Clear();
	void SetReadOffset(size_t pos);

	void Read(char* buffer, size_t amount);
	void Read(uint8_t* buffer, size_t amount);
	void Read(DataBuf& buffer, size_t amount);
	void Read(string& buffer, size_t amount);
};

ostream& operator<< (ostream& os, const DataBuf& buf);
