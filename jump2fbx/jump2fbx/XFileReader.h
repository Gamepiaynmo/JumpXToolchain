#pragma once

#include "XFileBase.h"

class XFileReader : public XFileBase {
public:
	XFileReader() {}

	~XFileReader() {}

	void Read(BYTE *fileData, DWORD dataLength);

protected:
	virtual void ReadModel();

private:
	void ReadTextures();
	void ReadMaterials();
	void ReadGeometries();
	void ReadBones();
	void ReadBoneGroups();
	void ReadParticles();
	void ReadActions();

};