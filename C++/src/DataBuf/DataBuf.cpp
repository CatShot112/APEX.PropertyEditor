#include "DataBuf.hpp"

#include <iomanip>

// Constructors
DataBuf::DataBuf() : readOffset(0) {}

DataBuf::DataBuf(DataBuf& oth) {
	buf = std::move(oth.buf);
	readOffset = std::move(oth.readOffset);
}

DataBuf::DataBuf(const DataBuf& oth) {
	buf = oth.buf;
	readOffset = oth.readOffset;
}

DataBuf::DataBuf(const DataBuf& oth, size_t off) {
	buf.reserve(oth.GetSize() - off);
	std::copy(oth.buf.begin() + off, oth.buf.end(), std::back_inserter(buf));
}

DataBuf::DataBuf(const string& str) : buf(str.begin(), str.end()), readOffset(0) {}

// Iterators
DataBuf::iter DataBuf::begin() { return buf.begin(); }
DataBuf::iter DataBuf::end() { return buf.end(); }
DataBuf::const_iter DataBuf::begin() const { return buf.begin(); }
DataBuf::const_iter DataBuf::end() const { return buf.end(); }

// Operators
DataBuf& DataBuf::operator<< (string data) {
	buf.insert(buf.end(), data.begin(), data.end());
	return *this;
}

DataBuf& DataBuf::operator<< (DataBuf& data) {
	buf.insert(buf.end(), data.begin(), data.end());
	return *this;
}

DataBuf& DataBuf::operator<< (const DataBuf& data) {
	buf.insert(buf.end(), data.begin(), data.end());
	return *this;
}

DataBuf& DataBuf::operator>> (string& data) {
	data.resize(GetSize() - readOffset);
	std::copy(buf.begin() + readOffset, buf.end(), data.begin());
	readOffset = buf.size();

	return *this;
}

DataBuf& DataBuf::operator>> (DataBuf& data) {
	data.Resize(GetSize() - readOffset);
	std::copy(buf.begin() + readOffset, buf.end(), data.begin());
	readOffset = buf.size();

	return *this;
}

DataBuf& DataBuf::operator= (DataBuf&& data) noexcept {
	buf = std::move(data.buf);
	readOffset = std::move(data.readOffset);

	return *this;
}

DataBuf& DataBuf::operator= (const DataBuf& data) {
	buf = data.buf;
	readOffset = data.readOffset;

	return *this;
}

DataBuf::ref DataBuf::operator[] (Data::size_type i) { return buf[i]; }
DataBuf::const_ref DataBuf::operator[] (Data::size_type i) const { return buf[i]; }

// Member functions
size_t DataBuf::GetSize() const { return buf.size(); }
size_t DataBuf::GetRemaining() const { return buf.size() - readOffset; }
size_t DataBuf::GetReadOffset() const { return readOffset; }

string DataBuf::ToString() const { return string(buf.begin(), buf.end()); }
bool DataBuf::IsFinished() const { return readOffset >= buf.size(); }
bool DataBuf::IsEmpty() const { return buf.empty(); }

void DataBuf::Reserve(size_t amount) { buf.reserve(amount); }
void DataBuf::Resize(size_t size) { buf.resize(size); }
void DataBuf::Erase(iter it) { buf.erase(it); }

void DataBuf::Clear() {
	buf.clear();
	readOffset = 0;
}

void DataBuf::SetReadOffset(size_t pos) {
	assert(pos <= GetSize());
	readOffset = pos;
}

void DataBuf::Read(char* buffer, size_t amount) {
	assert(readOffset + amount <= GetSize());
	std::copy_n(buf.begin() + readOffset, amount, buffer);
	readOffset += amount;
}

void DataBuf::Read(uint8_t* buffer, size_t amount) {
	assert(readOffset + amount <= GetSize());
	std::copy_n(buf.begin() + readOffset, amount, buffer);
	readOffset += amount;
}

void DataBuf::Read(DataBuf& buffer, size_t amount) {
	assert(readOffset + amount <= GetSize());
	buffer.Resize(amount);
	std::copy_n(buf.begin() + readOffset, amount, buffer.begin());
	readOffset += amount;
}

void DataBuf::Read(string& buffer, size_t amount) {
	assert(readOffset + amount <= GetSize());
	buffer.resize(amount);
	std::copy_n(buf.begin() + readOffset, amount, buffer.begin());
	readOffset += amount;
}

ostream& operator<< (ostream& os, const DataBuf& buf) {
	for (uint8_t u : buf)
		os << std::hex << std::setfill('0') << std::setw(2) << (int)u << " ";

	os << std::dec;
	return os;
}
