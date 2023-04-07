#include "stdafx.h"
#include "DATData.h"

DATData::DATData() {
}

DATData::~DATData() {
}

DATData::DATData(DATData &o) {
	rowCnt = o.rowCnt;
	colCnt = o.colCnt;
	m_type = o.m_type;
	m_data = o.m_data;
	m_indexmethod = o.m_indexmethod;
}

void DATData::loadFromFile(QFile &file) {
	if (!file.open(QIODevice::ReadOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	QByteArray qba = file.readAll();
	ByteBuffer buffer(reinterpret_cast<BYTE*>(qba.data()), qba.size());
	file.close(); qba.clear();

	while (buffer.bytesRemaining() > 0) {
		BYTE head = buffer.getByte();
		if (head == 0x0A) {
			int size = buffer.getCompressedInt();
			int lastcmd = 0;
			int start = buffer.getReadPos();
			QVariant::Type type = QVariant::Int;
			int col = 0;
			m_data.append(QList<QVariant>());
			while (start + size > buffer.getReadPos()) {
				int newcmd = buffer.getCompressedInt();
				int cmd = newcmd - lastcmd;
				lastcmd = newcmd;
loop:
				col++;
				switch (cmd) {
					case 3:
					case 6:
						type = QVariant::Int; break;
					case 5:
					case 10:
						type = QVariant::String; break;
					case 8:
						break;
					case 11:
						if (type == QVariant::Double) {
							type = QVariant::Int;
							goto blank;
						}
						else type = QVariant::Double;
						break;
					case 13:
						if (type == QVariant::Double) {
							type = QVariant::String;
							goto blank;
						}
						else type = QVariant::Double;
						break;
					default:
blank:
						if (cmd > 8 && cmd < 1024) {
							m_data[rowCnt].append(QVariant(QVariant::Invalid));
							cmd -= 8;
							goto loop;
						}

						throw QString("未知命令: %1 位置: %2").arg(cmd).arg(buffer.getReadPos());
				}
				switch (type) {
					case QVariant::Int: m_data[rowCnt].append((int) buffer.getCompressedInt()); break;
					case QVariant::Double: m_data[rowCnt].append((double) buffer.getFloat()); break;
					case QVariant::String: m_data[rowCnt].append(buffer.getStringLength()); break;
				}
			}
			rowCnt++;
			colCnt = max(colCnt, col);
		} else throw QString("非0x0A头部 位置: %1").arg(buffer.getReadPos());
	}

	for (int i = 0; i < colCnt; i++) {
		QVariant::Type type = QVariant::Invalid;
		for (auto &list : m_data) {
			if (list.size() > i && list[i].type() != QVariant::Invalid) {
				type = list[i].type();
				break;
			}
		}
		for (auto &list : m_data) {
			if (list.size() > i && list[i].type() != QVariant::Invalid && list[i].type() != type) {
				throw QString("列%1类型不唯一").arg(i);
			}
		}
		m_type.append(type);
	}

	for (auto &list : m_data) {
		for (int i = list.size(); i < colCnt; i++)
			list.append(QVariant(QVariant::Invalid));
	}
}

void DATData::saveToFile(QFile &file) {
	ByteBuffer buffer, line;
	for (int i = 0; i < rowCnt; i++) {
		buffer.putByte(0x0A);
		int cmd = 0;
		int type = QVariant::Int;
		line.clear();
		for (int j = 0; j < colCnt; j++) {
			while (j < colCnt && (m_type[j] == QVariant::Invalid || (m_type[j] == QVariant::String &&
				m_data[i][j].toString().length() == 0) || m_data[i][j].type() == QVariant::Invalid)) {
				cmd += 8;
				j++;
			}
			if (j == colCnt) break;

			switch (m_type[j]) {
				case QVariant::Int:
					switch (type) {
						case QVariant::Int: cmd += 8; break;
						case QVariant::Double: cmd += 3; break;
						case QVariant::String: cmd += 6; break;
					}
					line.putCompressedInt(cmd);
					line.putCompressedInt(m_data[i][j].toInt());
					break;
				case QVariant::Double:
					switch (type) {
						case QVariant::Int: cmd += 13; break;
						case QVariant::Double: cmd += 8; break;
						case QVariant::String: cmd += 11; break;
					}
					line.putCompressedInt(cmd);
					line.putFloat(m_data[i][j].toDouble());
					break;
				case QVariant::String:
					switch (type) {
						case QVariant::Int: cmd += 10; break;
						case QVariant::Double: cmd += 5; break;
						case QVariant::String: cmd += 8; break;
					}
					line.putCompressedInt(cmd);
					line.putStringLength(m_data[i][j].toString());
					break;
			}
			type = m_type[j];
		}

		buffer.putCompressedInt(line.size());
		buffer.put(&line);
	}

	if (!file.open(QIODevice::WriteOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	BYTE *fileBuf = new BYTE[buffer.size()];
	buffer.setReadPos(0);
	buffer.getBytes(fileBuf, buffer.size());
	file.write(reinterpret_cast<const char*>(fileBuf), buffer.size());
	delete[] fileBuf;
	file.close();
}

bool DATData::canPatch() {
	switch (m_indexmethod) {
		case 0: return m_type[0] == QVariant::Int;
		case 1: return m_type[0] == QVariant::Int && m_type[1] == QVariant::Int;
		case 2: return m_type[0] == QVariant::Int && m_type[2] == QVariant::Int;
		case 3: return m_type[0] == QVariant::String;
		default: return false;
	}
}

void DATData::importFromFile(DATData* orindata, QFile &file) {
	if (!canPatch())
		throw QString("此DAT文件无法制作补丁");

	if (!file.open(QIODevice::ReadOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	int head, pcnt;
	file.read(reinterpret_cast<char*>(&head), 4);
	if (head != 0x4B5044)
		throw QString("未知的文件头");

	file.read(reinterpret_cast<char*>(&m_indexmethod), 4);
	file.read(reinterpret_cast<char*>(&pcnt), 4);
	for (int i = 0; i < pcnt; i++) {
		int row = -1, col;
		file.read(reinterpret_cast<char*>(&col), 4);
		switch (m_indexmethod) {
			case 0: {
				int index;
				file.read(reinterpret_cast<char*>(&index), 4);
				for (int j = 0; j < rowCnt; j++)
					if (m_data[j][0].toInt() == index) {
						row = j;
						break;
					}
				break;
			}
			case 1: {
				int index[2];
				file.read(reinterpret_cast<char*>(&index), 8);
				for (int j = 0; j < rowCnt; j++)
					if (m_data[j][0].toInt() == index[0] && m_data[j][1].toInt() == index[1]) {
						row = j;
						break;
					}
				break;
			}
			case 2: {
				int index[2];
				file.read(reinterpret_cast<char*>(&index), 8);
				for (int j = 0; j < rowCnt; j++)
					if (m_data[j][0].toInt() == index[0] && m_data[j][2].toInt() == index[1]) {
						row = j;
						break;
					}
				break;
			}
			case 3: {
				int len;
				file.read(reinterpret_cast<char*>(&len), 4);
				QString index = QString::fromLocal8Bit(file.read(len));
				for (int j = 0; j < rowCnt; j++)
					if (m_data[j][0].toString() == index) {
						row = j;
						break;
					}
				break;
			}
		}

		BYTE type;
		file.read(reinterpret_cast<char*>(&type), 1);
		QVariant value;
		switch (type) {
			case 0: {
				int data;
				file.read(reinterpret_cast<char*>(&data), 4);
				value = data;
				break;
			}
			case 1: {
				float data;
				file.read(reinterpret_cast<char*>(&data), 4);
				value = data;
				break;
			}
			case 2: {
				int len;
				file.read(reinterpret_cast<char*>(&len), 4);
				value = QString::fromLocal8Bit(file.read(len));
				break;
			}
		}

		BYTE otype = m_type[col] == QVariant::Int ? 0 : m_type[col] == QVariant::Double ? 1
			: m_type[col] == QVariant::String ? 2 : 3;
		if (type == otype && row >= 0)
			m_data[row][col] = value;
	}

	file.close();
}

void DATData::exportToFile(DATData* orindata, QFile &file) {
	if (!canPatch())
		throw QString("此DAT文件无法制作补丁");

	if (!file.open(QIODevice::WriteOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	file.write("DPK", 4);

	int pcnt = 0;
	file.write(reinterpret_cast<char*>(&m_indexmethod), 4);
	file.seek(12);
	for (int i = 0; i < rowCnt; i++) {
		for (int j = 0; j < colCnt; j++) {
			if (m_type[j] == QVariant::Invalid)
				continue;
			if (m_data[i][j] != orindata->m_data[i][j]) {
				pcnt++;
				file.write(reinterpret_cast<char*>(&j), 4);
				switch (m_indexmethod) {
					case 0: {
						int index = m_data[i][0].toInt();
						file.write(reinterpret_cast<char*>(&index), 4);
						break;
					}
					case 1: {
						int index = m_data[i][0].toInt();
						file.write(reinterpret_cast<char*>(&index), 4);
						index = m_data[i][1].toInt();
						file.write(reinterpret_cast<char*>(&index), 4);
						break;
					}
					case 2: {
						int index = m_data[i][0].toInt();
						file.write(reinterpret_cast<char*>(&index), 4);
						index = m_data[i][2].toInt();
						file.write(reinterpret_cast<char*>(&index), 4);
						break;
					}
					case 3: {
						QByteArray data = m_data[i][0].toString().toLocal8Bit();
						int len = data.length();
						file.write(reinterpret_cast<char*>(&len), 4);
						file.write(data.data(), len);
						break;
					}
				}

				BYTE type = (m_type[j] == QVariant::Int ? 0 : m_type[j] == QVariant::Double ? 1 : 2);
				file.write(reinterpret_cast<char*>(&type), 1);
				switch (type) {
					case 0: {
						int data = m_data[i][j].toInt();
						file.write(reinterpret_cast<char*>(&data), 4);
						break;
					}
					case 1: {
						float data = m_data[i][j].toFloat();
						file.write(reinterpret_cast<char*>(&data), 4);
						break;
					}
					case 2: {
						QByteArray data = m_data[i][j].toString().toLocal8Bit();
						int len = data.length();
						file.write(reinterpret_cast<char*>(&len), 4);
						file.write(data.data(), len);
						break;
					}
				}
			}
		}
	}

	file.seek(8);
	file.write(reinterpret_cast<char*>(&pcnt), 4);
	file.close();
}