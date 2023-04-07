#pragma once

#include "XFileBase.h"

class XFileWriter : public XFileBase {
public:
	XFileWriter() {}

	~XFileWriter() {
		if (fileData != nullptr)
			delete[] fileData;
	}

	void Write();

	BYTE* GetDataBuffer() { return fileData; }
	DWORD GetDataLength() { return dataLength; }

protected:
	virtual void WriteModel();

private:
	void WriteTextures();
	void WriteMaterials();
	void WriteGeometries();
	void WriteBones();
	void WriteBoneGroups();
	void WriteParticles();
	void WriteActions();

	BYTE *fileData = nullptr;
	DWORD dataLength = 0;

};