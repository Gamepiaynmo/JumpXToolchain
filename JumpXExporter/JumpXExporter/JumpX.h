#pragma once

//#define DEBUG_OUTPUT

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <queue>
#include <conio.h>
#include <fstream>
#include <zlibdll.h>

#include <Windows.h>
#include "ByteBuffer.h"
#include "JumpXExporter.h"

typedef long long llong;

using std::string;
using bb::ByteBuffer;

extern const BYTE g_XFileHead[], g_XFileIndexHead[];
extern const DWORD g_XFileHeadSize, g_XFileIndexHeadSize;

template<typename T> inline T OffsetEncrypt(T addr) { return addr + 1000000000; }
template<typename T> inline T OffsetDecrypt(T addr) { return addr - 1000000000; }

extern const double g_PI;
extern const double g_EPS;

struct XFileHead {

	int ntex, atex;
	int nmtl, amtl;
	int ngeo, ageo;
	int nbon, abon;
	int nbgp, abgp;
	int natt, aatt;
	int nrib, arib;
	int nprt, aprt;
	int nact, aact;
	int bobj;
	int bob2;
	int acfg;
	int cfgs;
	int desc;
	int rib6;
	int mtsi;

	uLong indexSize, indexSizeComp;
	uLong dataSize, dataSizeComp;

	int frameCount;
};

#pragma pack(1)
struct XTextureStruct {
	int zero = 0;
	int nameOffset = 0;
};
#pragma pack()

struct XTexture : XTextureStruct {
	string textureName;
};

#pragma pack(1)
struct XMaterialStruct {
	int baseDataOffset = 0;
	int unknoun0 = 0;
	// 0x4000 Disable Depth
	// 0x8000 Transparent
	// 0x10000 Double face
	DWORD flags = 0;
	int textureId = -1;
	int unknown1 = 1;
	int unknown2 = 1;
	int unknown3 = 0;
	float unknown4 = 1.0f; // or 50
	float uvTransform[2] = { 0.0f, 0.0f };
	int animFrameCount = 0;
	int animDataOffset = 0;
};

struct XMaterialBaseDataStruct {
	int unknown0[2] = { 0, 0 };
	int unknown1[2] = { -1, -1 };
	float unknown2 = 1.0f;
	int unknown3 = -1;
	int unknown4[5] = { 0, 0, 0, 0, 0 };
};

struct XMaterialAnimDataStruct {
	BYTE color[4] = { 255, 255, 255, 255 };
	float uvOffset[2] = { 0.0f, 0.0f };
	DWORD flags = 0x100000;
};
#pragma pack()

struct XMaterial : XMaterialStruct {
	bool disableDepth = false;
	bool transparent = false;
	bool doubleFace = false;
	XMaterialBaseDataStruct baseData;
	XMaterialAnimDataStruct* animData = nullptr;

	XMaterial() = default;
	XMaterial(XMaterial &&o) {
		*this = o;
		o.animData = nullptr;
	}

	~XMaterial() { if (animData != nullptr) delete[] animData; }

private:
	XMaterial &XMaterial::operator =(const XMaterial &) = default;
};

#pragma pack(1)
struct XGeometryStruct {
	int scriptOffset = 0;
	DWORD unknown0 = 0x40;
	int nameOffset = 0;
	int objectId = 0;
	int materialId = -1;
	int flags[2] = { 0, 0 };
	int vertexCount = 0;
	int faceCount = 0;
	llong vertexDataOffset = 0;
	llong normalDataOffset = 0;
	llong texcoordDataOffset = 0;
	llong vertexColorOffset = 0;
	llong boneGroupOffset = 0;
	llong indicesOffset = 0;
	int unknown1 = 1;
	int unknown2 = 0;
	int boneDataOffset = 0;
	float box[6] = { -10000000.0f, -10000000.0f, -10000000.0f, 10000000.0f, 10000000.0f, 10000000.0f };
	DWORD unknown3 = 0x3ff;
};

struct XBoneDataStruct {
	INT8 boneCount = 0;
	INT8 bones[4] = { 0, 0, 0, 0 };
	BYTE unknown[3] = { 0xCC, 0xCC, 0xCC };	// 0xCC
	float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
#pragma pack()

struct XGeometry : XGeometryStruct {
	string meshName;
	Point3* vertexData = nullptr;
	Point3* normalData = nullptr;
	Point2* texcoordData = nullptr;
	BMM_Color_32* vertexColorData = nullptr;
	int* indicesData = nullptr;
	XBoneDataStruct* boneData = nullptr;
	IGameNode* node;

	XGeometry() = default;
	XGeometry(XGeometry &&o) {
		*this = o;
		o.vertexData = nullptr;
		o.normalData = nullptr;
		o.texcoordData = nullptr;
		o.vertexColorData = nullptr;
		o.indicesData = nullptr;
		o.boneData = nullptr;
	}

	~XGeometry() {
		if (vertexData != nullptr) delete[] vertexData;
		if (normalData != nullptr) delete[] normalData;
		if (texcoordData != nullptr) delete[] texcoordData;
		if (vertexColorData != nullptr) delete[] vertexColorData;
		if (indicesData != nullptr) delete[] indicesData;
		if (boneData != nullptr) delete[] boneData;
	}

private:
	XGeometry& operator =(const XGeometry &) = default;
};

#pragma pack(1)
struct XBoneStruct {
	int unknown0[2] = { 0, 0 };
	int nameOffset = 0;
	int parentId = -1;
	int frameCount = 0;
	DWORD unknown1 = 1;
	float invertedMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	float box[6] = { -10000000.0f, -10000000.0f, -10000000.0f, 10000000.0f, 10000000.0f, 10000000.0f };
	DWORD unknown2 = 0x3ff;
	int childCount = 0;
	int childOffset = 0;
	int unknown3[2] = { 0, 0 };
	int visibleFrameCount = 0;
	int visibleDataOffset = 0;
	int posFrameCount = 0;
	llong posDataOffset = 0;
	int rotFrameCount = 0;
	llong rotDataOffset = 0;
	int scaleFrameCount = 0;
	int scaleDataOffset = 0;
};
#pragma pack()

struct XBone : XBoneStruct {
	string boneName;
	GMatrix matrix, invMatrix;
	int* visibleData = nullptr;
	Point3* posData = nullptr;
	Quat* rotData = nullptr;
	Point3* scaleData = nullptr;
	std::vector<int> childs;
	IGameNode* node;

	XBone() = default;
	XBone(XBone &&o) {
		*this = o;
		o.visibleData = nullptr;
		o.posData = nullptr;
		o.rotData = nullptr;
		o.scaleData = nullptr;
	}

	~XBone() {
		if (visibleData != nullptr) delete[] visibleData;
		if (posData != nullptr) delete[] posData;
		if (rotData != nullptr) delete[] rotData;
		if (scaleData != nullptr) delete[] scaleData;
	}

private:
	XBone& operator =(const XBone &) = default;
};

#pragma pack(1)
struct XBob2Struct {
	float xmax = 10.0f;
	float ymax = 10.0f;
	float zmax = 110.0f;
	float xmin = -10.0f;
	float ymin = -10.0f;
	float zmin = 0.0f;
	float unknown = 30.0f;
	DWORD zero[4] = { 0, 0, 0, 0 };
};
#pragma pack()

#pragma pack(1)
struct XParticleStruct {
	int infoOffset = 0;
	/*	0x5c2000
	0x2000 invisible
	0x40000 move with character
	0x80000 rotate with character
	0x100000 fixed position
	0x400000 random direction
	*/
	int flags = 0;
	char name[80] = "";
	int boneId = -1;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float offsetZ = 0.0f;
	int unknown0 = 20;
	float speed = 20.0f;
	float randSpeed = 0.0f;
	float randAngle = 0.0f;
	int unknown1 = 0;
	float lifeSpan = 1.0f;
	float spawnPerSec = 10.0f;
	float randOffX = 1.0f;
	float randOffY = 1.0f;
	DWORD texDirRand = 0x40000; // 0x20000 fixed 0x40000 random
	int subImgCntX = 1;
	int subImgCntY = 1;
	DWORD texDirSpeed = 0x8000; // 0x10000 speed 0x8000 fixed
	float unknown4[2] = { 1.0f, 0.5f };
	int startColor[3] = { 255, 255, 255 }; // RGB, 0 ~ 255
	int midColor[3] = { 255, 255, 255 };
	int endColor[3] = { 255, 255, 255 };
	int unknown5[3] = { 0, 255, 0 };
	float startSize = 5.0f;
	float midSize = 5.0f;
	float endSize = 5.0f;
	int unknown6 = 5;
	int unknown7[2] = { 0 };
	int unknown8 = 1;
	int unknown9[2] = { 0 };
	int unknown10 = 1;
	int unknown11[6] = { 0 };
	int textureId = -1;
	int unknown12 = 0xcdcdcdcd;
	Point3 direction[3];
	int unknown13[5] = { 0 };
	float unknown14 = 0.0f;
	int unknown15[12] = { 0 };
	WORD unknown16 = 0;
	WORD unknown17 = 0xcdcd;
	float unknownAngle[2] = { 0.0f, 0.0f };
	DWORD unknown18 = 0;
	BYTE unknown19 = 0;
	BYTE unknown20[3] = { 0xcd, 0xcd, 0xcd };
	DWORD unknown21[2] = { 0 };
	BYTE unknown22 = 0;
	BYTE unknown23[0x4f] = { 0xcd };
	DWORD unknown24[3] = { 0 };
	DWORD unknown25 = 1;
	DWORD unknown26[2] = { 0 };
	BYTE unknown27[0x208] = { 0xcd };
	DWORD unknown28 = 0;
	BYTE unknown29 = 1;
	BYTE unknown30[3] = { 0xcd, 0xcd, 0xcd };
	float unknown31[2] = { 0.5f, 1.0f };
	int unknown32[2] = { 0, 0 };

	XParticleStruct() {
		memset(unknown23, 0xcd, sizeof(unknown23));
		memset(unknown27, 0xcd, sizeof(unknown27));
	}
};

struct XParticleInfoStruct {
	int unknown0 = 0;
	int unknown1 = -1;
	int unknown2[8] = { 0 };
};
#pragma pack()

struct XParticle : XParticleStruct {
	string particleName;
	XParticleInfoStruct partInfo;
	bool keepTranslation;
	bool keepRotation;
	bool fixedPosition;
	bool randomDirection;
};

#pragma pack(1)
struct XTraceStruct {
	int unknown0 = 0;
	float startPos[3];
	float endPos[3];
	int nameOffset = 0;
	int boneId = -1;
	int textureId = -1;
	int unknown2 = 0;
	int length = 2;
	int unknown4 = 0;
	int emptyOffset = 0;
	int unknown5 = 0;
};

struct XTraceInfo {
	BYTE color[3] = { 0xff, 0xff, 0xff };
	BYTE zero = 0;
	int alpha = 0xff;
};
#pragma pack()

struct XTrace : XTraceStruct {
	string traceName;
	XTraceInfo ribInfo;
};

class XScene {
public:
	XScene() {
		ZeroMemory(&m_head, sizeof(XFileHead));
#ifdef DEBUG_OUTPUT
		fout.imbue(std::locale("chs"));
#endif
	}

	void saveToFile(const TCHAR* fileName, DWORD options);

	bool haveWarning() { return !warnMsg.empty(); }
	string nextWarning() { string ret = warnMsg.front(); warnMsg.pop(); return ret; }

	XFileHead m_head;
	XBob2Struct m_bob2;

	std::vector<XTexture> m_textures;
	std::vector<XMaterial> m_materials;
	std::vector<XGeometry> m_geometries;
	std::vector<XBone> m_bones;
	std::vector<XParticle> m_particles;
	std::vector<XTrace> m_traces;

private:
	void processMesh(IGameNode *mesh);
	void processBone(IGameNode *bone, bool visible, bool scale);
	void processMaterial(IGameMaterial *mtl);
	void processParticle(IGameNode *part);
	void processTrace(IGameNode *rib);

	void saveTextures();
	void saveMaterials();
	void saveGeometries();
	void saveBones();
	void saveParticles();
	void saveTraces();

	void warn(string msg) { warnMsg.push(msg); }

	std::queue<string> warnMsg;
	ByteBuffer indexBuffer, dataBuffer;
#ifdef DEBUG_OUTPUT
	wofstream fout = wofstream(L"E:\\GPIAY\\Games\\300\\Max Plugin\\JumpXExporter\\debug.txt", ios::out);
#endif

	map<IGameNode*, int> boneIndex;
	map<IGameMaterial*, int> mtlIndex;
	map<string, int> texIndex;

	IGameScene* m_igame;
};