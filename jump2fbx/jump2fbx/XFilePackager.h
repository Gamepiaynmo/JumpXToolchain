#pragma once

#include "XFileReader.h"
#include "XFileWriter.h"

class XFilePackager : public XFileReader, public XFileWriter {
public:
	XFilePackager() {}

	~XFilePackager() {}

	void Package(BYTE* xData, DWORD xLength, BYTE* indexData, DWORD indexLength, BYTE* modelData, DWORD modelLength);

protected:
	virtual void ReadModel() override;
	virtual void WriteModel() override;
};