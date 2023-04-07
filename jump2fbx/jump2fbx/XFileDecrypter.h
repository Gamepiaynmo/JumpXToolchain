#pragma once

#include "XFileReader.h"

class XFileDecrypter : public XFileReader {
public:
	XFileDecrypter() {
	}

	~XFileDecrypter() {
		if (m_FbxTextures != nullptr) delete[] m_FbxTextures;
		if (m_FbxMaterials != nullptr) delete[] m_FbxMaterials;
		if (m_FbxGeometries != nullptr) delete[] m_FbxGeometries;
		if (m_FbxBones != nullptr) delete[] m_FbxBones;
	}

	void Decrypt(BYTE *fileData, DWORD dataLength, FbxScene *scene);

private:
	void DecryptModel();
	void DecryptTextures();
	void DecryptMaterials();
	void DecryptGeometries();
	void DecryptBones();
	void DecryptBoneGroups();
	void DecryptParticles();
	void DecryptActions();

	FbxScene *fbxScene = nullptr;

	FbxFileTexture **m_FbxTextures = nullptr;
	FbxSurfacePhong **m_FbxMaterials = nullptr;
	FbxNode **m_FbxGeometries = nullptr;
	FbxNode **m_FbxBones = nullptr;
	FbxAnimStack *m_FbxAction = nullptr;
	FbxAnimLayer *m_FbxAnimLayer = nullptr;
};