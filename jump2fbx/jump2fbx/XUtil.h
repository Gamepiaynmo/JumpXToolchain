#pragma once

#define _CRT_SECURE_NO_WARNINGS

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
#include <d3dx9.h>
#include "zlib\zlib.h"
#include <fbxsdk.h>
#include "ByteBuffer.h"

using namespace std;
using bb::ByteBuffer;

extern const BYTE g_XFileHead[], g_XFileIndexHead[];
extern const DWORD g_XFileHeadSize, g_XFileIndexHeadSize;

template<typename T> inline T OffsetEncrypt(T addr) { return addr + 1000000000; }
template<typename T> inline T OffsetDecrypt(T addr) { return addr - 1000000000; }

extern const double g_PI;
extern const double g_EPS;

struct XHead {
	DWORD ntex, atex; // Texture
	DWORD nmtl, amtl; // Material
	DWORD ngeo, ageo; // Geometry
	DWORD nbon, abon; // Bone
	DWORD nbgp, abgp; // Bone Group
	DWORD natt, aatt; // TODO
	DWORD nrib, arib; // TODO
	DWORD nprt, aprt; // Particle
	DWORD nact, aact; // Action
	DWORD bobj; // TODO
	DWORD bob2; // TODO
	DWORD acfg; // TODO
	DWORD cfgs; // TODO
	DWORD desc; // TODO
	DWORD rib6; // TODO
	DWORD mtsi; // TODO
	DWORD indexSize, modelSize;
	DWORD indexSizeCompressed, modelSizeCompressed;
};

struct XTexture {
	DWORD zero;
	DWORD nameOffset;
};

struct XMaterial {
	DWORD offset0;
	DWORD zero0;
	DWORD flags0;	// 0x8000 Transparent
					// 0x10000 Double face
					// 0x4000 Disable Depth
	DWORD textureId;
	DWORD one0;
	DWORD flags1; // 1
	DWORD zero1;
	float unknownFloat; // 1 or 50
	float uvTransform[2];
	DWORD animFrameCount;
	DWORD animDataOffset;
};

struct XGeometry {
	DWORD headOffset; // scrp
	DWORD unknown0; // 0x40 0x00 0x00 0x00 ?
	DWORD nameOffset;
	DWORD objectId;
	DWORD materialId;
	DWORD flags0[2];
	DWORD vertexCount;
	DWORD faceCount;
	DWORD64 vertexDataOffset;
	DWORD64 normalDataOffset;
	DWORD64 texcoordDataOffset;
	DWORD64 vertexColorOffset;
	DWORD64 boneJointOffset;
	DWORD64 indicesOffset;
	DWORD isInvisible;
	DWORD mainBoneId;
	DWORD bonesOffset;
	float box[6];
	DWORD unknown1; // 0xFF 0x03 0x00 0x00 ?
};

struct XBone {
	DWORD zero0[2];
	DWORD nameOffset;
	DWORD parentId;
	DWORD frameCount;
	DWORD unknown1; // 0x01 0x00 0x00 0x00 ?
	float matrix[16];
	float box[6];
	DWORD unknown2; // 0xFF 0x03 0x00 0x00 ?
	DWORD childCount;
	DWORD childsOffset;
	DWORD zero1[2];
	DWORD visibleFrameCount;
	DWORD visibleOffset;
	DWORD posFrameCount;
	DWORD posActionOffset;
	DWORD zero3;
	DWORD rotFrameCount;
	DWORD rotActionOffset;
	DWORD zero4;
	DWORD scaleFrameCount;
	DWORD scaleActionOffset;
};

struct XBoneGroup {
	DWORD zero;
	DWORD boneCnt;
	DWORD boneOffset;
};

struct XParticle {
	int infoOffset = 0;
	int unknown0 = 0;
	char name[80] = "";
	int boneId = -1;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float offsetZ = 10.0f;
	int unknown1 = 20;
	float speed = 100.0f;
	float randSpeed = 50.0f;
	int unknown2[2] = { 0 };
	float lifeSpan = 1.0f;
	float spawnPerSec = 10.0f;
	float randOffX = 0.8f;
	float randOffZ = 0.9f;
	DWORD flags0 = 0x40000;
	int subImgCntX = 1;
	int subImgCntY = 1;
	DWORD flags1 = 0x800000;
	float unknown3[2] = { 1.0f, 0.5f };
	int startColor[3] = { 255, 255, 255 }; // RGB, 0 ~ 255
	int midColor[3] = { 255, 255, 255 };
	int endColor[3] = { 255, 255, 255 };
	int unknown4[2] = { 0, 255 };
	int unknown5 = 0;
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
	float vect[3] = { 0.0f, 0.0f, 1.0f };
	float vectt[6] = { 0.0f };
	int unknown13[5] = { 0 };
	float unknown14 = 0.0f;
	int unknown15[12] = { 0 };
	BYTE unknown16;
	BYTE enableRandSize = 0;
	WORD unknown17 = 0xcdcd;
	DWORD unknown18[3] = { 0 };
	BYTE unknown19 = 0;
	BYTE unknown20[3] = { 0xcd, 0xcd, 0xcd };
	DWORD unknown21[2] = { 0 };
	BYTE unknown22 = 0;
	BYTE unknown23[0x4f] = { 0xcd };
	DWORD unknown24[3] = { 0 };
	BYTE unknown29 = 1;
	BYTE unknown30[3] = { 0xcd, 0xcd, 0xcd };
	float unknown26[2] = { 0 };
	BYTE unknown27[0x208] = { 0xcd };
	DWORD unknown28 = 0;
};

struct ParticleInfoStruct {
	DWORD zero0;
	DWORD ffs;
	DWORD zero1[8];
};

struct XAction {
	char name[80];
	WORD startFrame;
	WORD endFrame;
	WORD otherFrame;
	DWORD ffs; // 0xff ?
};

struct VertexBoneWeightInfo {
	BYTE boneCount;
	BYTE bones[4];
	BYTE unused[3]; // 0xCC ?
	float weight[4];
};

struct MaterialBaseInfo {
	DWORD zeros0[2];
	DWORD ffs0[2];
	float one;
	DWORD ffs1;
	DWORD zeros1[5];
};

struct MaterialAnimInfo {
	BYTE color[4];
	float uOffset;
	float vOffset;
	DWORD flags;
};

struct Bob2Struct {
	float xmax;
	float ymax;
	float zmax;
	float xmin;
	float ymin;
	float zmin;
	float unknown;
	DWORD zero[4];
};

struct BoneGroupInfo {
	DWORD boneCnt;
	BYTE bones[4];
};

struct XTrace {
	int unknown0 = 0;
	float startPos[3];
	float endPos[3];
	int nameOffset = 0;
	int boneId = -1;
	int textureId = -1;
	int unknown2 = 4;
	int unknown3 = 2;
	int unknown4 = 0;
	int emptyOffset = 0;
	int unknown5 = 0;
};

struct XTraceInfo {
	int unknown0 = 0xffffff;
	int unknown1 = 0;
};