#pragma once

#define BB_DEFAULT_SIZE 4096

#include <vector>
#include <Windows.h>
#include <QtCore/QtCore>

using namespace std;

typedef long long llong;
typedef unsigned long long ullong;

class ByteBuffer {
public:
	ByteBuffer(uint size = BB_DEFAULT_SIZE) { buf.reserve(size); }
	ByteBuffer(void *arr, uint size) { buf.assign((byte *) arr, (byte *) arr + size); }
	~ByteBuffer() = default;

	uint remaining() { return size() - rpos; }
	void clear() { rpos = 0; wpos = 0; buf.clear(); }
	void resize(uint newSize) { buf.resize(newSize); rpos = 0; wpos = 0; }
	uint size() { return buf.size(); }

	template<typename T> T get() const {
		uint npos = rpos + sizeof(T);
		if (npos <= buf.size()) {
			T value = *(T *) &buf[rpos];
			rpos = npos;
			return value;
		}
		throw QString("文件数据损坏");
	}

	void getBytes(void *value, uint len) const {
		if (len == 0) return;
		uint npos = rpos + len;
		if (npos <= buf.size()) {
			memcpy(value, &buf[rpos], len);
			rpos = npos;
			return;
		}
		throw QString("文件数据损坏");
	}

	template<typename T> void get(T &value) const { value = get<T>(); }
	char getChar() const { return get<char>(); }
	byte getByte() const { return get<byte>(); }
	short getShort() const { return get<short>(); }
	ushort getUShort() const { return get<ushort>(); }
	int getInt() const { return get<int>(); }
	uint getUInt() const { return get<uint>(); }
	llong getLong() const { return get<llong>(); }
	ullong getULong() const { return get<ullong>(); }
	float getFloat() const { return get<float>(); }
	double getDouble() const { return get<double>(); }
	QString getString() const {
		QByteArray ret;
		char value = getChar();
		while (value) {
			ret.push_back(value);
			value = getChar();
		}
		return QTextCodec::codecForName("GBK")->toUnicode(ret);
	}
	QString getStringLen(uint len) const {
		QByteArray ret(len, Qt::Uninitialized);
		getBytes(ret.data(), len);
		return QTextCodec::codecForName("GBK")->toUnicode(ret);
	}

	template<typename T> void put(const T &value) {
		uint npos = wpos + sizeof(T);
		if (buf.size() < npos)
			buf.resize(npos);
		*(T *) &buf[wpos] = value;
		wpos = npos;
	}

	void putBytes(const void *value, uint len) {
		if (len == 0) return;
		uint npos = wpos + len;
		if (buf.size() < npos)
			buf.resize(npos);
		memcpy(&buf[wpos], value, len);
		wpos = npos;
	}

	void putBuffer(ByteBuffer *value) { putBytes(value->getDataPtr(), value->size()); }
	void putChar(char value) { put<char>(value); }
	void putByte(byte value) { put<byte>(value); }
	void putShort(short value) { put<short>(value); }
	void putUShort(ushort value) { put<ushort>(value); }
	void putInt(int value) { put<int>(value); }
	void putUInt(uint value) { put<uint>(value); }
	void putLong(llong value) { put<llong>(value); }
	void putULong(ullong value) { put<ullong>(value); }
	void putFloat(float value) { put<float>(value); }
	void putDouble(double value) { put<double>(value); }
	void putString(QString value) {
		putStringLen(value);
		putByte(0);
	}
	void putStringLen(QString value) {
		QByteArray bytes = value.toLocal8Bit();
		putBytes(bytes.constData(), bytes.size());
	}

	void setReadPos(uint r) { rpos = r; }
	uint getReadPos() const { return rpos; }
	void setWritePos(uint w) { wpos = w; }
	uint getWritePos() const { return wpos; }

	void *getDataPtr() { return &buf[0]; }

private:
	uint wpos = 0;
	mutable uint rpos = 0;
	std::vector<byte> buf;
};