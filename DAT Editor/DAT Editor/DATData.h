#pragma once

class DATData {
public:
	DATData();
	DATData(DATData &o);
	~DATData();

	void loadFromFile(QFile &file);
	void saveToFile(QFile &file);

	bool canPatch();
	void importFromFile(DATData* orindata, QFile &file);
	void exportToFile(DATData* orindata, QFile &file);

	void setDatType(int type) { m_indexmethod = type; }

	int getColumnCount() { return colCnt; }
	int getRowCount() { return rowCnt; }
	int getType(int col) { return m_type[col]; }
	QVariant getData(int row, int col) {
		return m_data[row][col];
	}

	void setData(int row, int col, QVariant data) {
		if (data.type() != m_type[col]) return;
		m_data[row][col] = data;
	}

private:
	int rowCnt = 0, colCnt = 0;
	QList<QList<QVariant>> m_data;
	QList<int> m_type;
	int m_indexmethod;
};

