#pragma once

#include "XUtil.h"

class XFileBase {
public:
	friend class XFileEncrypter;
	friend class XFileDecrypter;

	XFileBase() {
		ZeroMemory(&m_XFileHead, sizeof(XHead));
	}

	~XFileBase() {
		ReleaseAll(m_XTextures, m_XFileHead.ntex);
		ReleaseAll(m_XMaterials, m_XFileHead.nmtl);
		ReleaseAll(m_XGeometries, m_XFileHead.ngeo);
		ReleaseAll(m_XBones, m_XFileHead.nbon);
		ReleaseAll(m_XBoneGroups, m_XFileHead.nbgp);
		ReleaseAll(m_XParticles, m_XFileHead.nprt);
		ReleaseAll(m_XActions, m_XFileHead.nact);
	}

	bool hasWarn() {
		return warn;
	}

protected:

	template<typename T> static void ReleaseAll(T *&array, uint32_t len) {
		if (array != nullptr) {
			for (uint32_t i = 0; i < len; i++) delete array[i];
			delete[] array; array = nullptr;
		}
	}

	ostream& Warn() {
		warn = true;
		return cout << "Warn: ";
	}

	XHead m_XFileHead;
	ByteBuffer indexBuffer, modelBuffer;

	XTexture **m_XTextures = nullptr;
	XMaterial **m_XMaterials = nullptr;
	XGeometry **m_XGeometries = nullptr;
	XBone **m_XBones = nullptr;
	XBoneGroup **m_XBoneGroups = nullptr;
	XParticle **m_XParticles = nullptr;
	XAction **m_XActions = nullptr;

	bool warn = false;

};