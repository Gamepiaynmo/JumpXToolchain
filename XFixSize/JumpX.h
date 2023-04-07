#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define ZLIB_WINAPI

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <memory>
#include <map>
#include <functional>

#include <Windows.h>
#include <QtCore>
#include <QtGui>
#include "zlib\zlib.h"
#include "ByteBuffer.h"

typedef long long llong;

using namespace std;
using bb::ByteBuffer;

extern const BYTE g_XFileHead[], g_XFileIndexHead[];
extern const DWORD g_XFileHeadSize, g_XFileIndexHeadSize;

template<typename T> inline T OffsetEncrypt(T addr) { return addr + 1000000000; }
template<typename T> inline T OffsetDecrypt(T addr) { return addr - 1000000000; }

extern const double g_PI;
extern const double g_EPS;
extern const DWORD g_limit;

#define X_SIZE_ERROR "Incorrect XFile Struct Size."

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

	DWORD indexSize, indexSizeComp;
	DWORD dataSize, dataSizeComp;

	int fileVersion;
	int frameCount;
};

#pragma pack(1)
struct XTextureStruct {
	int zero = 0;
	int nameOffset = 0;
};
static_assert(sizeof(XTextureStruct) == 0x8, X_SIZE_ERROR);
#pragma pack()

struct XTexture : XTextureStruct {
	QString textureName;
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
static_assert(sizeof(XMaterialStruct) == 0x30, X_SIZE_ERROR);

struct XMaterialBaseDataStruct {
	int unknown0 = 0;
	int enableEffect = 0;
	int refractTexId = -1;
	int specularTexId = -1;
	float intensity = 1.0f;
	int unknown3 = -1;
	int unknown4[5] = { 0 };
};
static_assert(sizeof(XMaterialBaseDataStruct) == 0x2C, X_SIZE_ERROR);

struct XMaterialAnimDataStruct {
	BYTE color[4] = { 255, 255, 255, 255 };
	float uvOffset[2] = { 0.0f, 0.0f };
	/*
		0x40000 black transparent
		0x80000 unknown
		0x100000 show black
		0x800000 unknown
		0x4000000 uv show
	*/
	DWORD flags = 0;
};
static_assert(sizeof(XMaterialAnimDataStruct) == 0x10, X_SIZE_ERROR);
#pragma pack()

struct XMaterial : XMaterialStruct {
	bool isSkillEffect = false;
	bool useTexAlpha = false;
	bool doubleFace = false;
	XMaterialBaseDataStruct baseData;
	XMaterialAnimDataStruct* animData = nullptr;

	XMaterial() = default;
	XMaterial(XMaterial &&o) {
		memcpy(this, &o, sizeof(XMaterial));
		o.animData = nullptr;
	}
	XMaterial(const XMaterial &o) {
		memcpy(this, &o, sizeof(XMaterial));
		if (o.animData != nullptr) {
			animData = new XMaterialAnimDataStruct[animFrameCount];
			memcpy(animData, o.animData, sizeof(XMaterialAnimDataStruct) * animFrameCount);
		}
	}
	XMaterial& operator =(const XMaterial &o) {
		memcpy(this, &o, sizeof(XMaterial));
		if (o.animData != nullptr) {
			animData = new XMaterialAnimDataStruct[animFrameCount];
			memcpy(animData, o.animData, sizeof(XMaterialAnimDataStruct) * animFrameCount);
		}
		return *this;
	}
	XMaterial& operator =(XMaterial &&o) {
		memcpy(this, &o, sizeof(XMaterial));
		o.animData = nullptr;
		return *this;
	}

	~XMaterial() { if (animData != nullptr) delete[] animData; }
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
	DWORD vertexDataOffset[2] = { 0 };
	DWORD normalDataOffset[2] = { 0 };
	DWORD texcoordDataOffset[2] = { 0 };
	llong vertexColorOffset = 0;
	llong boneGroupOffset = 0;
	llong indicesOffset = 0;
	int unknown1 = 1;
	int visibleBone = -1;
	int boneDataOffset = 0;
	float box[6] = { -10000000.0f, -10000000.0f, -10000000.0f, 10000000.0f, 10000000.0f, 10000000.0f };
	DWORD unknown3 = 0x3ff;
};
static_assert(sizeof(XGeometryStruct) == 0x7C, X_SIZE_ERROR);

struct XBoneDataStruct {
	INT8 boneCount = 0;
	BYTE bones[4] = { 0 };
	BYTE unknown[3] = { 0xCC, 0xCC, 0xCC };	// 0xCC
	float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
static_assert(sizeof(XBoneDataStruct) == 0x18, X_SIZE_ERROR);
#pragma pack()

struct XBoneDataStructEditor {
	int boneCount = 0;
	int bones[4] = { 0 };
	float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct XGeometry : XGeometryStruct {
	QString meshName;
	QString script;
	QVector3D* vertexData = nullptr;
	QVector3D* normalData = nullptr;
	QVector2D* texcoordData[2] = { 0 };
	QRgb* vertexColorData = nullptr;
	int* indicesData = nullptr;
	XBoneDataStructEditor* boneData = nullptr;

	XGeometry() = default;
	XGeometry(XGeometry &&o) {
		memcpy(this, &o, sizeof(XGeometryStruct));
		meshName = o.meshName;
		script = o.script;
		vertexData = o.vertexData; o.vertexData = nullptr;
		normalData = o.normalData; o.normalData = nullptr;
		texcoordData[0] = o.texcoordData[0]; o.texcoordData[0] = nullptr;
		texcoordData[1] = o.texcoordData[1]; o.texcoordData[1] = nullptr;
		vertexColorData = o.vertexColorData; o.vertexColorData = nullptr;
		indicesData = o.indicesData; o.indicesData = nullptr;
		boneData = o.boneData; o.boneData = nullptr;
	}
	XGeometry(const XGeometry &o) {
		memcpy(this, &o, sizeof(XGeometryStruct));
		meshName = o.meshName;
		script = o.script;
		if (o.vertexData != nullptr) { vertexData = new QVector3D[vertexCount]; memcpy(vertexData, o.vertexData, sizeof(QVector3D) * vertexCount); }
		if (o.normalData != nullptr) { normalData = new QVector3D[vertexCount]; memcpy(normalData, o.normalData, sizeof(QVector3D) * vertexCount); }
		if (o.texcoordData[0] != nullptr) { texcoordData[0] = new QVector2D[vertexCount]; memcpy(texcoordData[0], o.texcoordData[0], sizeof(QVector2D) * vertexCount); }
		if (o.texcoordData[1] != nullptr) { texcoordData[1] = new QVector2D[vertexCount]; memcpy(texcoordData[1], o.texcoordData[1], sizeof(QVector2D) * vertexCount); }
		if (o.vertexColorData != nullptr) { vertexColorData = new QRgb[vertexCount]; memcpy(vertexColorData, o.vertexColorData, sizeof(QRgb) * vertexCount); }
		if (o.indicesData != nullptr) { indicesData = new int[faceCount * 3]; memcpy(indicesData, o.indicesData, sizeof(int) * faceCount * 3); }
		if (o.boneData != nullptr) { boneData = new XBoneDataStructEditor[vertexCount]; memcpy(boneData, o.boneData, sizeof(XBoneDataStructEditor) * vertexCount); }
	}
	XGeometry& operator =(const XGeometry &o) {
		memcpy(this, &o, sizeof(XGeometryStruct));
		meshName = o.meshName;
		script = o.script;
		if (o.vertexData != nullptr) { vertexData = new QVector3D[vertexCount]; memcpy(vertexData, o.vertexData, sizeof(QVector3D) * vertexCount); }
		if (o.normalData != nullptr) { normalData = new QVector3D[vertexCount]; memcpy(normalData, o.normalData, sizeof(QVector3D) * vertexCount); }
		if (o.texcoordData[0] != nullptr) { texcoordData[0] = new QVector2D[vertexCount]; memcpy(texcoordData[0], o.texcoordData[0], sizeof(QVector2D) * vertexCount); }
		if (o.texcoordData[1] != nullptr) { texcoordData[1] = new QVector2D[vertexCount]; memcpy(texcoordData[1], o.texcoordData[1], sizeof(QVector2D) * vertexCount); }
		if (o.vertexColorData != nullptr) { vertexColorData = new QRgb[vertexCount]; memcpy(vertexColorData, o.vertexColorData, sizeof(QRgb) * vertexCount); }
		if (o.indicesData != nullptr) { indicesData = new int[faceCount * 3]; memcpy(indicesData, o.indicesData, sizeof(int) * faceCount * 3); }
		if (o.boneData != nullptr) { boneData = new XBoneDataStructEditor[vertexCount]; memcpy(boneData, o.boneData, sizeof(XBoneDataStructEditor) * vertexCount); }
		return *this;
	}
	XGeometry& operator =(XGeometry &&o) {
		memcpy(this, &o, sizeof(XGeometryStruct));
		meshName = o.meshName;
		script = o.script;
		vertexData = o.vertexData; o.vertexData = nullptr;
		normalData = o.normalData; o.normalData = nullptr;
		texcoordData[0] = o.texcoordData[0]; o.texcoordData[0] = nullptr;
		texcoordData[1] = o.texcoordData[1]; o.texcoordData[1] = nullptr;
		vertexColorData = o.vertexColorData; o.vertexColorData = nullptr;
		indicesData = o.indicesData; o.indicesData = nullptr;
		boneData = o.boneData; o.boneData = nullptr;
		return *this;
	}

	~XGeometry() {
		if (vertexData != nullptr) delete[] vertexData;
		if (normalData != nullptr) delete[] normalData;
		if (texcoordData[0] != nullptr) delete[] texcoordData[0];
		if (texcoordData[1] != nullptr) delete[] texcoordData[1];
		if (vertexColorData != nullptr) delete[] vertexColorData;
		if (indicesData != nullptr) delete[] indicesData;
		if (boneData != nullptr) delete[] boneData;
	}
};

#pragma pack(1)
struct XBoneStruct {
	int unknown0[2] = { 0 };
	int nameOffset = 0;
	int parentId = -1;
	int frameCount = 0;
	DWORD unknown1 = 1;
	float invertedMatrix[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	float box[6] = { -10000000.0f, -10000000.0f, -10000000.0f, 10000000.0f, 10000000.0f, 10000000.0f };
	DWORD unknown2 = 0x3ff;
	int childCount = 0;
	int childOffset = 0;
	int unknown3[2] = { 0 };
	int visibleFrameCount = 0;
	int visibleDataOffset = 0;
	int posFrameCount = 0;
	DWORD posDataOffset[2] = { 0 };
	int rotFrameCount = 0;
	DWORD rotDataOffset[2] = { 0 };
	int scaleFrameCount = 0;
	int scaleDataOffset = 0;
};
static_assert(sizeof(XBoneStruct) == 0xAC, X_SIZE_ERROR);
#pragma pack()

struct XBone : XBoneStruct {
	QString boneName;
	QMatrix4x4 matrix, invMatrix;
	int* visibleData = nullptr;
	QVector3D* posData = nullptr;
	QQuaternion* rotData = nullptr;
	QVector3D* scaleData = nullptr;
	int* childs = nullptr;

	XBone() = default;
	XBone(XBone &&o) {
		memcpy(this, &o, sizeof(XBoneStruct));
		boneName = o.boneName;
		matrix = o.matrix;
		invMatrix = o.invMatrix;
		visibleData = o.visibleData; o.visibleData = nullptr;
		posData = o.posData; o.posData = nullptr;
		rotData = o.rotData; o.rotData = nullptr;
		scaleData = o.scaleData; o.scaleData = nullptr;
		childs = o.childs; o.childs = nullptr;
	}
	XBone(const XBone &o) {
		memcpy(this, &o, sizeof(XBoneStruct));
		boneName = o.boneName;
		matrix = o.matrix;
		invMatrix = o.invMatrix;
		if (o.visibleData != nullptr) { visibleData = new int[frameCount]; memcpy(visibleData, o.visibleData, sizeof(int) * frameCount); }
		if (o.posData != nullptr) { posData = new QVector3D[frameCount]; memcpy(posData, o.posData, sizeof(QVector3D) * frameCount); }
		if (o.rotData != nullptr) { rotData = new QQuaternion[frameCount]; memcpy(rotData, o.rotData, sizeof(QQuaternion) * frameCount); }
		if (o.scaleData != nullptr) { scaleData = new QVector3D[frameCount]; memcpy(scaleData, o.scaleData, sizeof(QVector3D) * frameCount); }
		if (o.childs != nullptr) { childs = new int[childCount]; memcpy(childs, o.childs, sizeof(int) * childCount); }
	}
	XBone& operator =(const XBone &o) {
		memcpy(this, &o, sizeof(XBoneStruct));
		boneName = o.boneName;
		matrix = o.matrix;
		invMatrix = o.invMatrix;
		if (o.visibleData != nullptr) { visibleData = new int[frameCount]; memcpy(visibleData, o.visibleData, sizeof(int) * frameCount); }
		if (o.posData != nullptr) { posData = new QVector3D[frameCount]; memcpy(posData, o.posData, sizeof(QVector3D) * frameCount); }
		if (o.rotData != nullptr) { rotData = new QQuaternion[frameCount]; memcpy(rotData, o.rotData, sizeof(QQuaternion) * frameCount); }
		if (o.scaleData != nullptr) { scaleData = new QVector3D[frameCount]; memcpy(scaleData, o.scaleData, sizeof(QVector3D) * frameCount); }
		if (o.childs != nullptr) { childs = new int[childCount]; memcpy(childs, o.childs, sizeof(int) * childCount); }
		return *this;
	}
	XBone& operator =(XBone &&o) {
		memcpy(this, &o, sizeof(XBoneStruct));
		boneName = o.boneName;
		matrix = o.matrix;
		invMatrix = o.invMatrix;
		visibleData = o.visibleData; o.visibleData = nullptr;
		posData = o.posData; o.posData = nullptr;
		rotData = o.rotData; o.rotData = nullptr;
		scaleData = o.scaleData; o.scaleData = nullptr;
		childs = o.childs; o.childs = nullptr;
		return *this;
	}

	~XBone() {
		if (visibleData != nullptr) delete[] visibleData;
		if (posData != nullptr) delete[] posData;
		if (rotData != nullptr) delete[] rotData;
		if (scaleData != nullptr) delete[] scaleData;
		if (childs != nullptr) delete[] childs;
	}
};

#pragma pack(1)
struct XBoneGroupStruct {
	int zero = 0;
	int boneCnt = 0;
	int boneOffset = 0;
};
static_assert(sizeof(XBoneGroupStruct) == 0xC, X_SIZE_ERROR);
#pragma pack()

struct XBoneGroup : XBoneGroupStruct {
	int bones[4] = { -1, -1, -1, -1 };
};

#pragma pack(1)
struct XActionStruct {
	char name[80] = "";
	short startFrame = 0;
	short endFrame = 0;
	short midFrame = 0;
	BYTE unknown[4] = { 0xff, 0xff, 0xff, 0xff };
};
static_assert(sizeof(XActionStruct) == 0x5A, X_SIZE_ERROR);
#pragma pack()

struct XAction : XActionStruct {
	QString actionName;
};

#pragma pack(1)
struct XParticleStruct_6 {
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
	float offsetZ = 100.0f;
	int unknown0 = 20;
	float speed = 100.0f;
	float randSpeed = 50.0f;
	float randAngle = 0.0f;
	int unknown1 = 0;
	float lifeSpan = 1.0f;
	float spawnPerSec = 10.0f;
	float randOffX = 0.0f;
	float randOffY = 0.0f;
	DWORD effectTrans = 0x40000; // 0x20000 fixed 0x40000 random
	int subImgCntX = 1;
	int subImgCntY = 1;
	DWORD texDirSpeed = 0x8000; // 0x10000 speed 0x8000 fixed
	float unknown4[2] = { 1.0f, 0.5f };
	int startColor[3] = { 255, 255, 255 }; // RGB, 0 ~ 255
	int midColor[3] = { 255, 255, 255 };
	int endColor[3] = { 255, 255, 255 };
	int alpha[3] = { 0, 255, 0 };
	float startSize = 50.0f;
	float midSize = 50.0f;
	float endSize = 50.0f;
	int unknown6 = 5;
	int unknown7[2] = { 0 };
	int unknown8 = 1;
	int unknown9[2] = { 0 };
	int unknown10 = 1;
	int unknown11[6] = { 0 };
	int textureId = -1;
	int unknown12 = 0xcdcdcdcd;
	QVector3D direction[3] = { { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
	int unknown13[5] = { 0 };
	float rotSpeed = 0.0f;
	float unknown15[5] = { 0 };
	float randDirection = 360.0f;
	float unknown15_2[4] = { 0 };
	float randRotSpeed[2] = { 0 };
	BYTE randSequenceTexDir = 0;
	BYTE randTexDir = 0;
	WORD unknown17 = 0xcdcd;
	float unknownAngle[2] = { 0.0f, 0.0f };
	DWORD unknown18 = 0;
	BYTE enableRandSize = 0;
	BYTE unknown20[3] = { 0xcd, 0xcd, 0xcd };
	float randSize[2] = { 0 };
	BYTE unknown22 = 0;
	BYTE unknown23[0x4f] = { 0xcd };
	DWORD sequence[3] = { 0 };
	DWORD unknown25 = 1;
	DWORD unknown26[2] = { 0 };
	BYTE unknown27[0x208] = { 0xcd };
	DWORD unknown28 = 0;

	XParticleStruct_6() {
		memset(unknown23, 0xcd, sizeof(unknown23));
		memset(unknown27, 0xcd, sizeof(unknown27));
	}
};
static_assert(sizeof(XParticleStruct_6) == 0x418, X_SIZE_ERROR);

struct XParticleStruct_8 : XParticleStruct_6 {
	BYTE unknown29 = 1;
	BYTE unknown30[3] = { 0xcd, 0xcd, 0xcd };
	float unknown31[2] = { 0.5f, 1.0f };
	int unknown32[2] = { 0, 0 };
};
static_assert(sizeof(XParticleStruct_8) == 0x42C, X_SIZE_ERROR);

struct XParticleInfoStruct {
	int enableEffect = 0;
	int texId = -1;
	int scriptOffset = 0;
	float intensity = 0.0f;
	int unknown1 = 0;
	int unknown2[5] = { 0 };
};
static_assert(sizeof(XParticleInfoStruct) == 0x28, X_SIZE_ERROR);
#pragma pack()

struct XParticle : XParticleStruct_8 {
	QString particleName;
	XParticleInfoStruct partInfo;
	bool blink;
	bool keepTranslation;
	bool keepRotation;
	bool fixedPosition;
	bool randomDirection;
	QString script;
};

#pragma pack(1)
struct XTraceStruct {
	int unknown0 = 0;
	float startPos[3];
	float endPos[3];
	int nameOffset = 0;
	int boneId = -1;
	int textureId = -1;
	int segment = 0;
	int length = 2;
	int transparent = 0;
	int emptyOffset = 0;
	int unknown5 = 0;
};
static_assert(sizeof(XTraceStruct) == 0x3C, X_SIZE_ERROR);

struct XTraceInfo {
	BYTE color[3] = { 0xff, 0xff, 0xff };
	BYTE zero = 0;
	int alpha = 0xff;
};
static_assert(sizeof(XTraceInfo) == 0x8, X_SIZE_ERROR);
#pragma pack()

struct XTrace : XTraceStruct {
	QString traceName;
	XTraceInfo ribInfo;
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
static_assert(sizeof(XBob2Struct) == 0x2C, X_SIZE_ERROR);
#pragma pack()

class XScene {
public:
	XScene() { ZeroMemory(&m_head, sizeof(XFileHead)); }

	void loadFromFile(QFile &file);
	void saveToFile(QFile &file);

	bool haveWarning() { return !warnMsg.isEmpty(); }
	QString nextWarning() { return warnMsg.dequeue(); }

	XFileHead m_head;
	XBob2Struct m_bob2;
	QString m_desc;

	vector<XTexture> m_textures;
	vector<XMaterial> m_materials;
	vector<XGeometry> m_geometries;
	vector<XBone> m_bones;
	vector<XBoneGroup> m_boneGroups;
	vector<XAction> m_actions;
	vector<XParticle> m_particles;
	vector<XTrace> m_traces;

private:

	void loadTextures();
	void loadMaterials();
	void loadGeometries();
	void loadBones();
	void loadBoneGroups();
	void loadActions();
	void loadParticles();
	void loadTraces();

	void saveTextures();
	void saveMaterials();
	void saveGeometries();
	void saveBones();
	void saveBoneGroups();
	void saveActions();
	void saveParticles();
	void saveTraces();

	void warn(QString msg) { warnMsg.enqueue(msg); }

	pair<int, ByteBuffer&> selectBuffer();

	QQueue<QString> warnMsg;
	ByteBuffer indexBuffer, dataBuffer;
};