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
#include <cmath>

#include <Windows.h>
#include <QtNetwork\QtNetwork>
#include "zlib\zlib.h"
#include "ByteBuffer.h"

#define EDITOR_VERSION "V2.2.1"

using namespace std;

extern const BYTE g_XFileHead[], g_XFileIndexHead[];
extern const DWORD g_XFileHeadSize, g_XFileIndexHeadSize;

template<typename T> inline T OffsetEncrypt(T addr) { return addr + 1000000000; }
template<typename T> inline T OffsetDecrypt(T addr) { return addr - 1000000000; }

extern const double g_PI;
extern const double g_EPS;

#define X_SIZE_ERROR "Incorrect XFile Struct Size."

union XCOLOR {
	struct {
		byte r, g, b, a;
	};
	byte v[4];
	byte &operator [](int index) {
		return v[index];
	}
};
static_assert(sizeof(XCOLOR) == 0x4, X_SIZE_ERROR);

inline void setFlag(uint &flag, uint value, bool set) { set ? flag |= value : flag &= ~value; }
inline void clamp(int &id, int count) { if (id < 0 || id >= count) id = -1; }

inline QColor getColor(const byte v[]) { return QColor(v[0], v[1], v[2]); }
inline QColor getColor(XCOLOR v) { return QColor(v[0], v[1], v[2], v[3]); }
inline QColor getColor(const float v[], int size) { return size == 3 ? QColor::fromRgbF(v[0], v[1], v[2]) : QColor::fromRgbF(v[0], v[1], v[2], v[3]); }
inline QColor getColor(const int v[]) { return QColor(v[0], v[1], v[2]); }
inline void setColor(byte v[], const QColor &color) { v[0] = color.red(); v[1] = color.green(); v[2] = color.blue(); }
inline void setColor(XCOLOR &v, const QColor &color) { v[0] = color.red(); v[1] = color.green(); v[2] = color.blue(); v[3] = color.alpha(); }
inline void setColor(float v[], int size, const QColor &color) { v[0] = color.redF(); v[1] = color.greenF(); v[2] = color.blueF(); if (size > 3) v[3] = color.alphaF(); }
inline void setColor(int v[], const QColor &color) { v[0] = color.red(); v[1] = color.green(); v[2] = color.blue(); }

struct XBBOX {
	QVector3D max = { -10000000.0f, -10000000.0f, -10000000.0f }, min = { 10000000.0f, 10000000.0f, 10000000.0f };
	uint numSeg = 0x3ff;
	QVector3D uncompress(uint pos) {
		float x = pos & numSeg; pos >>= 10;
		x = (max.x() - min.x()) * x / numSeg + min.x();
		float y = pos & numSeg; pos >>= 10;
		y = (max.y() - min.y()) * y / numSeg + min.y();
		float z = pos & numSeg; pos >>= 10;
		z = (max.z() - min.z()) * z / numSeg + min.z();
		return QVector3D(x, y, z);
	}
};
static_assert(sizeof(XBBOX) == 0x1c, X_SIZE_ERROR);

struct XDIR {
	ushort sx, sy, sz;
	QVector3D uncompress() {
		float x = sx / 127.0f;
		float y = sy / 127.0f;
		float z = sz / 127.0f;
		return QVector3D(x, y, z).normalized();
	}
};

struct XDATA {
	uint ntex, atex;
	uint nmtl, amtl;
	uint ngeo, ageo;
	uint nbon, abon;
	uint nbgp, abgp;
	uint natt, aatt;
	uint nrib, arib;
	uint nprt, aprt;
	uint nact, aact;
	uint bobj; // bounding box
	uint bob2;
	uint acfg;
	uint cfgs;
	uint desc;
	uint rib6; // m_dwRibbonEmitterSetAddrVer6Ex
	uint mtsi; // m_dwMtlSetAddr_SelfIllminateEx

	uint headSize, headSizeComp;
	uint dataSize, dataSizeComp;

	int version;
	int numKey;
};

struct XTextureStruct {
	uint dataAddr = 0;
	uint nameAddr = 0;
};
static_assert(sizeof(XTextureStruct) == 0x8, X_SIZE_ERROR);

struct XTexture : XTextureStruct {
	QString name;
};

enum RenderFlag {
	RENDER_SPECULARENABLE = 0x1000,
	RENDER_SORTBYFARZ = 0x2000,
	RENDER_ALPHABLEND = 0x4000, // Disable Depth
	RENDER_ALPHATEST = 0x8000, // Transparent
	RENDER_TWOSIDED = 0x10000, // Double face
	RENDER_BLEND = 0x20000,
	RENDER_ADD = 0x40000, // black transparent
	RENDER_MODULATE = 0x80000, //
	RENDER_MODULATE2X = 0x100000, // show black
	RENDER_MODULATE4X = 0x200000,
	RENDER_ALPHAKEY = 0x400000,
	RENDER_UNSHADED = 0x800000, //
	RENDER_UNFOGGED = 0x1000000,
	RENDER_ZWRITEENABLE = 0x2000000,
	RENDER_UVCLAMP = 0x4000000, // uv show
};

enum EffectFlag {
	EFFECT_BUMP = 1,
	EFFECT_SPECULAR = 2,
	EFFECT_LIGHT = 4,
	EFFECT_CARTOON = 8,
	EFFECT_DISSOLVE = 16,
};

struct XMaterialStruct {
	uint dataAddr = 0;
	uint saveFlag = 0;
	uint flag = 0; // RenderFlag
	int textureId = -1;
	int uTile = 1;
	int vTile = 1;
	int startFrame = 0;
	float playbackRate = 1.0f; // or 50
	float uvSpeed[2] = { 0.0f, 0.0f };
	int numColorKey = 0;
	uint colorKeyAddr = 0;
};
static_assert(sizeof(XMaterialStruct) == 0x30, X_SIZE_ERROR);

struct XMaterialExStruct {
	uint dataAddr = 0;
	uint flag = 0; // EffectFlag
	int bumpTexId = -1;
	int specTexId = -1;
	float bumpAmount = 1.0f;
	int lightTexId = -1;
	uint dissolveDataAddr = 0;
	int reserved[4] = { 0 };
};
static_assert(sizeof(XMaterialExStruct) == 0x2C, X_SIZE_ERROR);

struct XMaterialCartoonStruct {
	uint dataAddr = 0;
	float ambientColor[3] = { 1, 1, 1 };
	float shadowColor[3] = { 1, 1, 1 };
	float ambientIntensity = 1.0f;
	float shadowThreshold = 0.0f;
	float specularSmoothness = 0.0f;
	int specTextureID = -1;
	int shadowTextureID = -1;
};
static_assert(sizeof(XMaterialCartoonStruct) == 0x30, X_SIZE_ERROR);

struct XMaterialDissolveStruct {
	uint dataAddr = 0;
	int dissolveTextureID = -1;
	XCOLOR dissolveColor = { 255, 255, 255, 255 };
	XCOLOR dissolveEdgeColor = { 255, 255, 255, 255 };
	uint thresholdKeyAddr = 0;
};
static_assert(sizeof(XMaterialDissolveStruct) == 0x14, X_SIZE_ERROR);

struct XMaterialAnimDataStruct {
	XCOLOR color = { 255, 255, 255, 255 };
	float uvOffset[2] = { 0.0f, 0.0f };
	uint blend = 0; // RenderFlag
};
static_assert(sizeof(XMaterialAnimDataStruct) == 0x10, X_SIZE_ERROR);

struct XMaterial : XMaterialStruct {
	XMaterialExStruct exData;
	XMaterialCartoonStruct cartoonData;
	XMaterialDissolveStruct dissolveData;
	XMaterialAnimDataStruct* colorKeys = nullptr;
	float *dissolveKeys = nullptr;

	XMaterial() = default;
	XMaterial(XMaterial &&o) {
		*this = move(o);
	}
	XMaterial& operator =(XMaterial &&o) {
		clearData();
		*this = o;
		o.colorKeys = nullptr;
		o.dissolveKeys = nullptr;
		return *this;
	}
	~XMaterial() { clearData(); }

private:
	XMaterial(const XMaterial &o) = default;
	XMaterial &operator =(const XMaterial &o) = default;
	void clearData() {
		if (colorKeys != nullptr) delete[] colorKeys;
		if (dissolveKeys != nullptr) delete[] dissolveKeys;
	}
};

enum XGEO_TYPE {
	XGEO_NORMAL_MESH, //
	XGEO_BILLBOARD, //
	XGEO_LASER,
	XGEO_DECAL,
	XGEO_FLOOR, //
	XGEO_COLLISION,
	XGEO_ALPHA_MESH, //
	XGEO_LIGHT_MAP,
	XGEO_BOUND_MESH, //
	XGEO_BBOXEX,
	XGEO_CLOSEOBJ,
	XGEO_MAX,
};

enum XGEO_FLAG {
	XGEO_FLAG_ALWAYSELOOKATCAMERA = 1,
	XGEO_FLAG_ALWAYSESTAND = 2, //
	XGEO_FLAG_VERTICALGROUND = 4, //
	XGEO_FLAG_LIGHTDISABLE = 8,
	XGEO_FLAG_ADDONE = 16,
	XGEO_FLAG_CLAMP = 32,
	XGEO_FLAG_APERTURE = 64,
	XGEO_FLAG_HEADLIGHTCASTERDISABLE = 128,
	XGEO_FLAG_SHADOWCASTER_ENABLE = 256,
	XGEO_FLAG_USEDBYUE4SCENE = 512, //
};

enum XGEOSAVE_TYPE {
	XFGEOCHUNKSAVE_COMPRESSED_VERTEX = 1, //
	XFGEOCHUNKSAVE_COMPRESSED_NORMAL = 2, //
	XFGEOCHUNKSAVE_COMPRESSED_ROTATION = 4,
	XFGEOCHUNKSAVE_COMPRESSED_UV = 8,
	XFGEOCHUNKSAVE_COMPRESSED_VERTCOLOR = 16,
	XFGEOCHUNKSAVE_COMPRESSED_INDICES = 32,
	XFGEOCHUNKSAVE_ENABLE_BONE_PALETTE = 64, //
	XFGEOCHUNKSAVE_ENABLE_UV2 = 128, //
};

struct XGeometryStruct {
	uint dataAddr = 0;
	uint saveFlag = 0x40; // XGEOSAVE_TYPE
	uint nameAddr = 0;
	int objectId = 0;
	int materialId = -1;
	uint type = 0; // XGEO_TYPE
	uint flag = 0; // XGEO_FLAG
	int numVertex = 0;
	int numFace = 0;
	uint vertexAddr = 0;
	uint vertexCompAddr = 0;
	uint normalAddr = 0;
	uint normalCompAddr = 0;
	uint uvAddr = 0;
	uint uv1Addr = 0;
	uint vertColAddr = 0; // vertex color
	uint vertColCompAddr = 0;
	uint boneGroupAddr = 0;
	uint boneGroupCompAddr = 0;
	uint indicesAddr = 0;
	uint indicesCompAddr = 0;
	int singleBoneInfl = 1;
	int ancestorBone = -1;
	uint bonePaletteAddr = 0;
	XBBOX bbox;
};
static_assert(sizeof(XGeometryStruct) == 0x7C, X_SIZE_ERROR);

struct XBonePaletteStruct {
	byte numBone = 0;
	byte bones[4] = { 0 };
	float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};
static_assert(sizeof(XBonePaletteStruct) == 0x18, X_SIZE_ERROR);

struct XBonePaletteEditor {
	int numBone = 0;
	int bones[4] = { 0 };
	float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct XGeometry : XGeometryStruct {
	QString name;
	QString script;
	QVector3D* vertexData = nullptr;
	QVector3D* normalData = nullptr;
	QVector2D* uvData[2] = { 0 };
	XCOLOR* vertColData = nullptr;
	ushort* indicesData = nullptr;
	XBonePaletteEditor* boneData = nullptr;
	QVector3D center = { 0.0f, 0.0f, 0.0f };

	XGeometry() = default;
	XGeometry(XGeometry &&o) {
		*this = move(o);
	}
	XGeometry& operator =(XGeometry &&o) {
		clearData();
		*this = o;
		o.vertexData = nullptr;
		o.normalData = nullptr;
		o.uvData[0] = nullptr;
		o.uvData[1] = nullptr;
		o.vertColData = nullptr;
		o.indicesData = nullptr;
		o.boneData = nullptr;
		return *this;
	}
	~XGeometry() { clearData(); }

private:
	XGeometry(const XGeometry &o) = default;
	XGeometry &operator =(const XGeometry &o) = default;
	void clearData() {
		if (vertexData != nullptr) delete[] vertexData;
		if (normalData != nullptr) delete[] normalData;
		if (uvData[0] != nullptr) delete[] uvData[0];
		if (uvData[1] != nullptr) delete[] uvData[1];
		if (vertColData != nullptr) delete[] vertColData;
		if (indicesData != nullptr) delete[] indicesData;
		if (boneData != nullptr) delete[] boneData;
	}
};

struct XBoneStruct {
	uint dataAddr = 0;
	uint saveFlag = 0; // XGEOSAVE_TYPE
	uint nameAddr = 0;
	int parentId = -1;
	int numKey = 0;
	bool hasKeyframe = true;
	float matInv[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	XBBOX transBbox;
	int numChild = 0;
	uint childAddr = 0;
	int numMatrix = 0;
	uint matrixAddr = 0;
	int numVisibleKey = 0;
	uint visibleKeyAddr = 0;
	int numPosKey = 0;
	uint posKeyAddr = 0;
	uint posKeyCompAddr = 0;
	int numRotKey = 0;
	uint rotKeyAddr = 0;
	uint rotKeyCompAddr = 0;
	int numScaleKey = 0;
	uint scaleKeyAddr = 0;
};
static_assert(sizeof(XBoneStruct) == 0xAC, X_SIZE_ERROR);

struct XBone : XBoneStruct {
	QString name;
	QMatrix4x4 matrix, matrixInv;
	uint* visibleData = nullptr;
	QVector3D* posData = nullptr;
	QQuaternion* rotData = nullptr;
	QVector3D* scaleData = nullptr;
	int* children = nullptr;

	XBone() = default;
	XBone(XBone &&o) {
		*this = move(o);
	}
	XBone& operator =(XBone &&o) {
		clearData();
		*this = o;
		o.visibleData = nullptr;
		o.posData = nullptr;
		o.rotData = nullptr;
		o.scaleData = nullptr;
		o.children = nullptr;
		return *this;
	}
	~XBone() { clearData(); }

private:
	XBone(const XBone &o) = default;
	XBone &operator =(const XBone &o) = default;
	void clearData() {
		if (visibleData != nullptr) delete[] visibleData;
		if (posData != nullptr) delete[] posData;
		if (rotData != nullptr) delete[] rotData;
		if (scaleData != nullptr) delete[] scaleData;
		if (children != nullptr) delete[] children;
	}
};

struct XBoneGroupStruct {
	uint dataAddr = 0;
	int numBone = 0;
	uint boneAddr = 0;
};
static_assert(sizeof(XBoneGroupStruct) == 0xC, X_SIZE_ERROR);

struct XBoneGroup : XBoneGroupStruct {
	int bones[4] = { -1, -1, -1, -1 };
};

struct XAttachmentStruct {
	uint dataAddr = 0;
	int boneID = -1;
	char attachName[80] = "";
	float matInit[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
};
static_assert(sizeof(XAttachmentStruct) == 0x98, X_SIZE_ERROR);

struct XAttachment : XAttachmentStruct {
	QString name;
	QMatrix4x4 matrix;
};

struct XRibbonStruct {
	uint dataAddr = 0;
	QVector3D startPos = { 0.0f, 0.0f, 0.0f };
	QVector3D endPos = { 0.0f, 0.0f, 0.0f };
	uint nameAddr = 0;
	int boneId = -1;
	int textureId = -1;
	uint edgePerSec = 0;
	uint edgeLifeSec = 1;
	uint blendMode = 0;
	uint bindPartNameAddr = 0;
	int maxEdgeCount = 0;
};
static_assert(sizeof(XRibbonStruct) == 0x3C, X_SIZE_ERROR);

struct XRibbonExStruct {
	byte color[3] = { 0xff, 0xff, 0xff };
	int alpha = 0xff;
};
static_assert(sizeof(XRibbonExStruct) == 0x8, X_SIZE_ERROR);

struct XRibbon : XRibbonStruct {
	QString name;
	XRibbonExStruct ribEx;
};

enum ParticleFlag {
	PARTICLE_SQUIRT = 0x2000, //
	PARTICLE_LINEEMITTER = 0x4000,
	PARTICLE_HEAD = 0x8000,
	PARTICLE_TAIL = 0x10000,
	PARTICLE_BOTH = 0x20000,
	PARTICLE_PARTICLEINMODELSPACE = 0x40000, //
	PARTICLE_XYQUAD = 0x80000, //
	PARTICLE_LOCKEMITTER = 0x100000, //
	PARTICLE_APERTURE = 0x200000,
	PARTICLE_MOVEALONEEMITTERPLANE = 0x400000, //
	PARTICLE_ENABLEZWRITE = 0x800000, //
};

struct XParticleStruct_6 {
	uint dataAddr = 0;
	uint flag = 0; // ParticleFlag
	char partName[80] = "";
	int boneId = -1;
	QVector3D pivot = { 0.0f, 0.0f, 100.0f };
	int count = 20;
	float speed = 100.0f;
	float speedVar = 50.0f;
	float coneAngle = 0.0f;
	float gravity = 0;
	float lifeSpan = 1.0f;
	float emissionRate = 10.0f;
	float width = 1.0f;
	float height = 1.0f;
	uint blendMode = 0x40000; // 0x20000 fixed 0x40000 random
	int row = 1;
	int col = 1;
	uint partFlag = 0x8000; // 0x10000 speed 0x8000 fixed
	float tailLength = 1.0f;
	float middleTime = 0.5f;
	int startColor[3] = { 255, 255, 255 }; // RGB, 0 ~ 255
	int midColor[3] = { 255, 255, 255 };
	int endColor[3] = { 255, 255, 255 };
	int alpha[3] = { 255, 255, 0 };
	float startSize = 10.0f;
	float midSize = 10.0f;
	float endSize = 10.0f;
	uint uvAnimFps = 5;
	uint lifeSpanHeadUVAnim[3] = { 0, 0, 1 };
	uint decayHeadUVAnim[3] = { 0, 0, 1 };
	uint lifeSpanTailUVAnim[3] = { 0, 0, 0 };
	uint decayTailUVAnim[3] = { 0, 0, 0 };
	int textureId = -1;
	int priorityPlane = 0;
	QVector3D normal = { 0.0f, 0.0f, 1.0f };
	QVector3D xAxis = { 1.0f, 0.0f, 0.0f };
	QVector3D yAxis = { 0.0f, 1.0f, 0.0f };
	QVector3D rotVec = { 0.0f, 0.0f, 0.0f };
	QVector3D rotVel = { 0.0f, 0.0f, 0.0f };
	float rotVecVar[6] = { 0 }; // randDirection
	float rotVelVar[6] = { 0 }; // randRotSpeed
	bool randRotVec = 0; // randSequenceTexDir
	bool randRotVel = 0; // randTexDir
	float tailStartLength = 0.0f;
	float tailSpeed = 0.0f;
	int tailMode = 0;
	bool enableRandSize = 0;
	float randSize[2] = { 1.0f, 1.0f };
	char bindPartName[80] = "";
	bool useTimeBasedCell = 0;
	int seqLoopTimes = 0;
	bool matchLife = 0;
	int numLoop = 1;
	int useEmitModel = 0;
	int useHitGroundModel = 0;
	char emitModelFile[260] = { 0 };
	char explodeModelFile[260] = { 0 };
	int numPart = 0;
};
static_assert(sizeof(XParticleStruct_6) == 0x418, X_SIZE_ERROR);

struct XParticleStruct : XParticleStruct_6 {
	bool enableLifeRandom = 0;
	float lifeRandom[2] = { 0.5f, 1.0f };
	float gravityX = 0.0f;
	float gravityY = 0.0f;
};
static_assert(sizeof(XParticleStruct) == 0x42C, X_SIZE_ERROR);

struct XParticleExStruct {
	uint flag = 0; // EffectFlag
	int bumpTexId = -1;
	uint scriptAddr = 0;
	float bumpAmount = 0.0f;
	ushort coneAngleMin;
	int reserved[5] = { 0 };
};
static_assert(sizeof(XParticleExStruct) == 0x28, X_SIZE_ERROR);

struct XParticle : XParticleStruct {
	QString name;
	XParticleExStruct partEx;
	QString script;
};

struct XActionStruct {
	char actionName[80] = "";
	short startFrame = 0;
	short endFrame = 0;
	short hitPoint = 0;
	short ribStartFrame = 0;
	short ribEndFrame = 0;
};
static_assert(sizeof(XActionStruct) == 0x5A, X_SIZE_ERROR);

struct XAction : XActionStruct {
	QString name;
};

struct XBobjStruct {
	float width = 20.0f;
	float height = 20.0f;
	float length = 110.0f;
	float lift = 0.0f;
	float radius = 30.0f;
	float reserved[4] = { 0 };
};
static_assert(sizeof(XBobjStruct) == 0x24, X_SIZE_ERROR);

struct XBob2Struct {
	QVector3D max = { 10.0f, 10.0f, 110.0f };
	QVector3D min = { -10.0f, -10.0f, 0.0f };
	float radius = 30.0f;
	float reserved[4] = { 0 };
};
static_assert(sizeof(XBob2Struct) == 0x2C, X_SIZE_ERROR);

class XScene {
public:
	XScene(bool showProgress = true) : m_showProgress(showProgress) { ZeroMemory(&m_head, sizeof(XDATA)); }

	void loadFromFile(QFile &file);
	void saveToFile(QFile &file);
	void combineScene(XScene *oScene);

	bool haveWarning() { return !warnMsg.isEmpty(); }
	QString nextWarning() { return warnMsg.dequeue(); }

	void startProgress(QString name, int range);
	bool updateProgress();
	void endProgress();

	XDATA m_head;
	XBobjStruct m_bobj;
	XBob2Struct m_bob2;
	QString m_desc;

	vector<XTexture> m_textures;
	vector<XMaterial> m_materials;
	vector<XGeometry> m_geometries;
	vector<XBone> m_bones;
	vector<XBoneGroup> m_boneGroups;
	vector<XAttachment> m_attachments;
	vector<XRibbon> m_ribbons;
	vector<XParticle> m_particles;
	vector<XAction> m_actions;

private:

	void loadTextures();
	void loadMaterials();
	void loadGeometries();
	void loadBones();
	void loadBoneGroups();
	void loadAttachments();
	void loadRibbons();
	void loadParticles();
	void loadActions();

	void saveTextures();
	void saveMaterials();
	void saveGeometries();
	void saveBones();
	void saveBoneGroups();
	void saveAttachments();
	void saveRibbons();
	void saveParticles();
	void saveActions();

	QString readScript(uint addr);
	void writeScript(uint addr, QString script);

	void warn(QString msg) { warnMsg.enqueue(msg); }

	QQueue<QString> warnMsg;
	ByteBuffer headBuffer, dataBuffer;

	QProgressDialog *m_progressDlg = nullptr;
	int m_progressRange = 0;
	bool m_showProgress;
};