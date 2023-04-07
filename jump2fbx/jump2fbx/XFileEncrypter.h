#pragma once

#include "XFileWriter.h"
#include "XFileReader.h"

class XFileEncrypter : public XFileWriter {
public:
	XFileEncrypter() {
	}

	~XFileEncrypter() {
		if (m_FbxTextures != nullptr) delete[] m_FbxTextures;
		if (m_FbxMaterials != nullptr) delete[] m_FbxMaterials;
		if (m_FbxGeometries != nullptr) delete[] m_FbxGeometries;
		if (m_FbxBones != nullptr) delete[] m_FbxBones;
		if (m_bgpInfo != nullptr) delete[] m_bgpInfo;
	}

	void Encrypt(FbxScene *scene);

private:
	void EncryptModel();
	void EncryptTextures();
	void EncryptMaterials();
	void EncryptGeometries();
	void EncryptBones();
	void EncryptBoneGroups();
	void EncryptParticles();
	void EncryptActions();

	FbxScene *fbxScene = nullptr;

	FbxFileTexture **m_FbxTextures = nullptr;
	FbxSurfacePhong **m_FbxMaterials = nullptr;
	FbxNode **m_FbxGeometries = nullptr;
	FbxNode **m_FbxBones = nullptr;
	FbxAnimStack *m_FbxAction = nullptr;
	FbxAnimLayer *m_FbxAnimLayer = nullptr;

	BoneGroupInfo *m_bgpInfo = nullptr;

	DWORD frameCount = 0;
};