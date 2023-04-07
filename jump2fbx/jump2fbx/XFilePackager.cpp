#include "XFilePackager.h"

void XFilePackager::Package(BYTE* xData, DWORD xLength, BYTE* indexData, DWORD indexLength, BYTE* modelData, DWORD modelLength) {
	Read(xData, xLength);

	XFileWriter::m_XFileHead = XFileReader::m_XFileHead;
	XFileWriter::indexBuffer.putBytes(indexData, indexLength);
	XFileWriter::modelBuffer.putBytes(modelData, modelLength);

	Write();
}

void XFilePackager::ReadModel() {

}

void XFilePackager::WriteModel() {

}