#pragma once

#include "XFileReader.h"

class XFileInspector : public XFileReader {
public:
	XFileInspector() {}

	~XFileInspector() {}

	void Inspect(BYTE *fileData, DWORD dataLength);

protected:
	virtual void InspectModel();

private:
	void InspectTextures();
	void InspectMaterials();
	void InspectGeometries();
	void InspectBones();
	void InspectBoneGroups();
	void InspectTraces();
	void InspectParticles();
	void InspectActions();
};