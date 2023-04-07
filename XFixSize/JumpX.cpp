#include "JumpX.h"
#pragma execution_character_set("utf-8")

const BYTE g_XFileHead[] = {
	0x4A, 0x55, 0x4D, 0x50, 0x58, 0x20, 0x56, 0x35, 0x2E, 0x30, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x57, 0x57, 0x57, 0x2E, 0x4A, 0x55, 0x4D, 0x50, 0x57, 0x2E, 0x43, 0x4F, 0x4D, 0x20, 0x20, 0x20,
	0xB4, 0xAC, 0xB3, 0xA4, 0x20, 0x20, 0xB0, 0xD1, 0xBA, 0xDA, 0xB6, 0xB4, 0xD7, 0xB0, 0xD4, 0xDA,
	0xC6, 0xBF, 0xD7, 0xD3, 0xC0, 0xEF, 0xB5, 0xC4, 0xC8, 0xCB, 0x21, 0x57, 0x45, 0x49, 0x42, 0x4F,
	0x2E, 0x43, 0x4F, 0x4D, 0x2F, 0x57, 0x55, 0x59, 0x41, 0x58, 0x49, 0x54, 0x00, 0x00, 0x00, 0x00 };
const BYTE g_XFileIndexHead[] = {
	0x57, 0x55, 0x59, 0x41, 0x58, 0x49, 0x40, 0x53, 0x49, 0x4E, 0x41, 0x2E, 0x43, 0x4E, 0x00 };

const DWORD g_XFileHeadSize = sizeof(g_XFileHead);
const DWORD g_XFileIndexHeadSize = sizeof(g_XFileIndexHead);

const double g_PI = acos(-1.0);
const double g_EPS = 1e-4;
const DWORD g_limit = 0xA00000;

template<typename T> T* ReallocArray(T* orinArr, int orinSize, int newSize) {
	T* newArr = new T[newSize];
	memcpy(newArr, orinArr, sizeof(T) * orinSize);
	for (int i = orinSize; i < newSize; i++)
		newArr[i] = orinArr[orinSize - 1];
	delete[] orinArr;
	return newArr;
}

void XScene::loadFromFile(QFile &file) {
	if (!file.open(QIODevice::ReadOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	QByteArray qba = file.readAll();
	ByteBuffer fileBuffer(reinterpret_cast<BYTE*>(qba.data()), qba.size());
	file.close();

	BYTE xHead[g_XFileHeadSize];
	fileBuffer.getBytes(xHead, g_XFileHeadSize);
	if (memcmp(xHead, g_XFileHead, g_XFileHeadSize))
		throw QString("无效的文件头");

	m_head.fileVersion = fileBuffer.getInt();
	if (m_head.fileVersion < 6 || m_head.fileVersion > 8)
		warn("未支持的X版本：" + QString::number(m_head.fileVersion));

	int headLen = fileBuffer.getInt();
	if (headLen % 12 != 0)
		throw QString("无效的文件头");

	for (; headLen > 0; headLen -= 12) {
		BYTE partName[4];
		fileBuffer.getBytes(partName, 4);
		if (fileBuffer.getInt() != 4)
			throw QString("损坏的文件头");
		int value = fileBuffer.getInt();

		if (!memcmp(partName, "ntex", 4)) m_head.ntex = value;
		if (!memcmp(partName, "atex", 4)) m_head.atex = value;
		if (!memcmp(partName, "nmtl", 4)) m_head.nmtl = value;
		if (!memcmp(partName, "amtl", 4)) m_head.amtl = value;
		if (!memcmp(partName, "ngeo", 4)) m_head.ngeo = value;
		if (!memcmp(partName, "ageo", 4)) m_head.ageo = value;
		if (!memcmp(partName, "nbon", 4)) m_head.nbon = value;
		if (!memcmp(partName, "abon", 4)) m_head.abon = value;
		if (!memcmp(partName, "nbgp", 4)) m_head.nbgp = value;
		if (!memcmp(partName, "abgp", 4)) m_head.abgp = value;
		if (!memcmp(partName, "natt", 4)) m_head.natt = value;
		if (!memcmp(partName, "aatt", 4)) m_head.aatt = value;
		if (!memcmp(partName, "nrib", 4)) m_head.nrib = value;
		if (!memcmp(partName, "arib", 4)) m_head.arib = value;
		if (!memcmp(partName, "nprt", 4)) m_head.nprt = value;
		if (!memcmp(partName, "aprt", 4)) m_head.aprt = value;
		if (!memcmp(partName, "nact", 4)) m_head.nact = value;
		if (!memcmp(partName, "aact", 4)) m_head.aact = value;
		if (!memcmp(partName, "bobj", 4)) m_head.bobj = value;
		if (!memcmp(partName, "bob2", 4)) m_head.bob2 = value;
		if (!memcmp(partName, "acfg", 4)) m_head.acfg = value;
		if (!memcmp(partName, "cfgs", 4)) m_head.cfgs = value;
		if (!memcmp(partName, "desc", 4)) m_head.desc = value;
		if (!memcmp(partName, "rib6", 4)) m_head.rib6 = value;
		if (!memcmp(partName, "mtsi", 4)) m_head.mtsi = value;
	}

	m_head.indexSize = fileBuffer.getInt();
	m_head.dataSize = fileBuffer.getInt();
	m_head.indexSizeComp = fileBuffer.getInt();
	m_head.dataSizeComp = fileBuffer.getInt();

	DWORD actualIndexSize = m_head.indexSize, actualDataSize = m_head.dataSize;
	BYTE *indexBuf = new BYTE[actualIndexSize], *dataBuf = new BYTE[actualDataSize],
		*indexCompBuf = new BYTE[m_head.indexSizeComp], *dataCompBuf = new BYTE[m_head.dataSizeComp];
	fileBuffer.getBytes(indexCompBuf, m_head.indexSizeComp);
	fileBuffer.getBytes(dataCompBuf, m_head.dataSizeComp);
	uncompress(indexBuf, &actualIndexSize, indexCompBuf, m_head.indexSizeComp);
	uncompress(dataBuf, &actualDataSize, dataCompBuf, m_head.dataSizeComp);

	if (actualIndexSize != m_head.indexSize || actualDataSize != m_head.dataSize)
		throw QString("文件数据损坏");

	indexBuffer.putBytes(indexBuf, actualIndexSize);
	dataBuffer.putBytes(dataBuf, actualDataSize);
	delete[] indexBuf; delete[] dataBuf;
	delete[] indexCompBuf; delete[] dataCompBuf;

	BYTE xIndexHead[g_XFileIndexHeadSize];
	indexBuffer.getBytes(xIndexHead, g_XFileIndexHeadSize);
	if (memcmp(xIndexHead, g_XFileIndexHead, g_XFileIndexHeadSize))
		throw QString("文件数据损坏");

	loadTextures();
	loadMaterials();

	if (m_head.bob2 != 0) {
		indexBuffer.setReadPos(m_head.bob2);
		m_bob2 = indexBuffer.get<XBob2Struct>();
	}

	if (m_head.desc != 0) {
		indexBuffer.setReadPos(m_head.desc);
		m_desc = indexBuffer.getString();
	}

	loadGeometries();
	loadBones();
	loadBoneGroups();
	loadTraces();
	loadParticles();
	loadActions();

	m_head.frameCount = 0;
	for (XMaterial &mtl : m_materials)
		if (mtl.animFrameCount > 0 && m_head.frameCount == 0)
			m_head.frameCount = mtl.animFrameCount;
	if (m_head.frameCount == 0)
		for (XBone &bone : m_bones)
			if (bone.frameCount > 0 && m_head.frameCount == 0)
				m_head.frameCount = bone.frameCount;

	for (XMaterial &mtl : m_materials) {
		if (mtl.animFrameCount > 0 && mtl.animFrameCount < m_head.frameCount) {
			mtl.animData = ReallocArray<XMaterialAnimDataStruct>(mtl.animData, mtl.animFrameCount, m_head.frameCount);
			mtl.animFrameCount = m_head.frameCount;
		}
	}
	for (XBone &bon : m_bones) {
		if (bon.visibleFrameCount > 0 && bon.visibleFrameCount < m_head.frameCount) {
			bon.visibleData = ReallocArray<int>(bon.visibleData, bon.visibleFrameCount, m_head.frameCount);
			bon.visibleFrameCount = m_head.frameCount;
		}
		if (bon.posFrameCount > 0 && bon.posFrameCount < m_head.frameCount) {
			bon.posData = ReallocArray<QVector3D>(bon.posData, bon.posFrameCount, m_head.frameCount);
			bon.posFrameCount = m_head.frameCount;
		}
		if (bon.rotFrameCount > 0 && bon.rotFrameCount < m_head.frameCount) {
			bon.rotData = ReallocArray<QQuaternion>(bon.rotData, bon.rotFrameCount, m_head.frameCount);
			bon.rotFrameCount = m_head.frameCount;
		}
		if (bon.scaleFrameCount > 0 && bon.scaleFrameCount < m_head.frameCount) {
			bon.scaleData = ReallocArray<QVector3D>(bon.scaleData, bon.scaleFrameCount, m_head.frameCount);
			bon.scaleFrameCount = m_head.frameCount;
		}
	}

	m_head.natt = m_head.aatt = 0;
	m_head.bobj = m_head.acfg = m_head.cfgs = 0;
}

void XScene::loadTextures() {
	indexBuffer.setReadPos(m_head.atex);
	for (int i = 0; i < m_head.ntex; i++) {
		XTexture tex;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&tex), sizeof(XTextureStruct));
		m_textures.push_back(std::move(tex));
	}

	for (XTexture &tex : m_textures) {
		indexBuffer.setReadPos(tex.nameOffset);
		tex.textureName = indexBuffer.getString();
	}
}

void XScene::loadMaterials() {
	indexBuffer.setReadPos(m_head.amtl);
	for (int i = 0; i < m_head.nmtl; i++) {
		XMaterial mtl;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&mtl), sizeof(XMaterialStruct));
		m_materials.push_back(std::move(mtl));
	}

	for (XMaterial &mtl : m_materials) {
		mtl.isSkillEffect = mtl.flags & 0x4000;
		mtl.useTexAlpha = mtl.flags & 0x8000;
		mtl.doubleFace = mtl.flags & 0x10000;
		dataBuffer.setReadPos(OffsetDecrypt(mtl.baseDataOffset));
		mtl.baseData = dataBuffer.get<XMaterialBaseDataStruct>();
		if (mtl.animFrameCount > 0) {
			dataBuffer.setReadPos(OffsetDecrypt(mtl.animDataOffset));
			mtl.animData = new XMaterialAnimDataStruct[mtl.animFrameCount];
			for (int j = 0; j < mtl.animFrameCount; j++)
				mtl.animData[j] = dataBuffer.get<XMaterialAnimDataStruct>();
		}
	}
}

void XScene::loadGeometries() {
	indexBuffer.setReadPos(m_head.ageo);
	for (int i = 0; i < m_head.ngeo; i++) {
		XGeometry geo;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&geo), sizeof(XGeometryStruct));
		m_geometries.push_back(std::move(geo));
	}

	for (XGeometry &geo : m_geometries) {
		indexBuffer.setReadPos(geo.nameOffset);
		geo.meshName = indexBuffer.getString();

		if (geo.scriptOffset != 0) {
			static char tempScript[1024];
			dataBuffer.setReadPos(OffsetDecrypt(geo.scriptOffset));
			dataBuffer.getInt(); dataBuffer.getInt(); dataBuffer.getInt();
			int len = dataBuffer.getInt();
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(tempScript), len);
			tempScript[len] = 0;
			geo.script = QString(tempScript);
		}

		if (geo.vertexDataOffset[0] == 0) {
			if (geo.vertexDataOffset[1] != 0) {
				throw QString("未支持的数据格式");
			} else throw QString("网格") + geo.meshName + "没有顶点数据";
		} else {
			dataBuffer.setReadPos(OffsetDecrypt(geo.vertexDataOffset[0]));
			geo.vertexData = new QVector3D[geo.vertexCount];
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(geo.vertexData), sizeof(QVector3D) * geo.vertexCount);
		}

		if (geo.normalDataOffset[0] == 0) {
			if (geo.normalDataOffset[1] != 0) {
				throw QString("未支持的数据格式");
			}
		} else {
			dataBuffer.setReadPos(OffsetDecrypt(geo.normalDataOffset[0]));
			geo.normalData = new QVector3D[geo.vertexCount];
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(geo.normalData), sizeof(QVector3D) * geo.vertexCount);
		}

		if (geo.texcoordDataOffset[0] != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.texcoordDataOffset[0]));
			geo.texcoordData[0] = new QVector2D[geo.vertexCount];
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(geo.texcoordData[0]), sizeof(QVector2D) * geo.vertexCount);
		}

		if (geo.texcoordDataOffset[1] != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.texcoordDataOffset[1]));
			geo.texcoordData[1] = new QVector2D[geo.vertexCount];
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(geo.texcoordData[1]), sizeof(QVector2D) * geo.vertexCount);
		}

		if (geo.vertexColorOffset != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.vertexColorOffset));
			geo.vertexColorData = new QRgb[geo.vertexCount];
			BYTE color[4];
			for (int i = 0; i < geo.vertexCount; i++) {
				dataBuffer.getBytes(color, 4);
				geo.vertexColorData[i] = qRgba(color[0], color[1], color[2], color[3]);
			}
		}

		if (geo.indicesOffset != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.indicesOffset));
			geo.indicesData = new int[geo.faceCount * 3];
			for (int i = 0; i < geo.faceCount * 3; i++)
				geo.indicesData[i] = dataBuffer.getShort();
		} else throw QString("网格") + geo.meshName + "没有面数据";

		if (geo.boneDataOffset != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.boneDataOffset));
			geo.boneData = new XBoneDataStructEditor[geo.vertexCount];
			for (int i = 0; i < geo.vertexCount; i++) {
				XBoneDataStruct data = dataBuffer.get<XBoneDataStruct>();
				geo.boneData[i].boneCount = data.boneCount;
				if (data.boneCount > 4) throw QString("网格") + geo.meshName + "骨骼影响限制大于4";
				for (int j = 0; j < geo.boneData[i].boneCount; j++) {
					geo.boneData[i].bones[j] = data.bones[j];
					geo.boneData[i].weight[j] = data.weight[j];
				}
			}
			for (int i = 0; i < geo.vertexCount; i++) {
				for (int j = 0; j < geo.boneData[i].boneCount;) {
					if (geo.boneData[i].weight[j] < g_EPS) {
						geo.boneData[i].boneCount--;
						for (int k = j; k < geo.boneData[i].boneCount; k++) {
							geo.boneData[i].bones[k] = geo.boneData[i].bones[k + 1];
							geo.boneData[i].weight[k] = geo.boneData[i].weight[k + 1];
						}
						continue;
					} else if (geo.boneData[i].bones[j] >= m_head.nbon) {
						geo.boneData[i].bones[j] = 0;
					}
					j++;
				}
			}
		}
	}
}

void XScene::loadBones() {
	indexBuffer.setReadPos(m_head.abon);
	for (int i = 0; i < m_head.nbon; i++) {
		XBone bone;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&bone), sizeof(XBoneStruct));
		m_bones.push_back(std::move(bone));
	}

	for (XBone &bone : m_bones) {
		indexBuffer.setReadPos(bone.nameOffset);
		bone.boneName = indexBuffer.getString();
		bone.invMatrix = QMatrix4x4(bone.invertedMatrix).transposed();
		bone.matrix = bone.invMatrix.inverted();

		if (bone.childCount > 0) {
			bone.childs = new int[bone.childCount];
			indexBuffer.setReadPos(bone.childOffset);
			for (int i = 0; i < bone.childCount; i++)
				bone.childs[i] = indexBuffer.getInt();
		}

		if (bone.frameCount > 0) {
			if (bone.visibleFrameCount > 0) {
				dataBuffer.setReadPos(OffsetDecrypt(bone.visibleDataOffset));
				bone.visibleData = new int[bone.visibleFrameCount];
				dataBuffer.getBytes(reinterpret_cast<BYTE*>(bone.visibleData), sizeof(int) * bone.visibleFrameCount);
			}

			if (bone.posFrameCount > 0) {
				bone.posData = new QVector3D[bone.posFrameCount];
				if (bone.posDataOffset[0] == 0) {
					throw QString("未支持的数据格式");
				} else {
					dataBuffer.setReadPos(OffsetDecrypt(bone.posDataOffset[0]));
					dataBuffer.getBytes(reinterpret_cast<BYTE*>(bone.posData), sizeof(QVector3D) * bone.posFrameCount);
				}
			}

			if (bone.rotFrameCount > 0) {
				bone.rotData = new QQuaternion[bone.rotFrameCount];
				if (bone.rotDataOffset[0] == 0) {
					throw QString("未支持的数据格式");
				} else {
					float quat[4];
					dataBuffer.setReadPos(OffsetDecrypt(bone.rotDataOffset[0]));
					for (int i = 0; i < bone.rotFrameCount; i++) {
						dataBuffer.getBytes(reinterpret_cast<BYTE*>(quat), sizeof(quat));
						bone.rotData[i] = QQuaternion(quat[3], quat[0], quat[1], quat[2]).inverted();
					}
				}
			}

			if (bone.scaleFrameCount > 0) {
				dataBuffer.setReadPos(OffsetDecrypt(bone.scaleDataOffset));
				bone.scaleData = new QVector3D[bone.scaleFrameCount];
				dataBuffer.getBytes(reinterpret_cast<BYTE*>(bone.scaleData), sizeof(QVector3D) * bone.scaleFrameCount);
			}
		}
	}
}

void XScene::loadBoneGroups() {
	indexBuffer.setReadPos(m_head.abgp);
	for (int i = 0; i < m_head.nbgp; i++) {
		XBoneGroup bgp;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&bgp), sizeof(XBoneGroupStruct));
		m_boneGroups.push_back(std::move(bgp));
	}

	for (XBoneGroup &bgp : m_boneGroups) {
		indexBuffer.setReadPos(bgp.boneOffset);
		for (int i = 0; i < bgp.boneCnt; i++)
			bgp.bones[i] = indexBuffer.getInt();
	}
}

void XScene::loadTraces() {
	indexBuffer.setReadPos(m_head.arib);
	for (int i = 0; i < m_head.nrib; i++) {
		XTrace rib;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&rib), sizeof(XTraceStruct));
		m_traces.push_back(std::move(rib));
	}

	if (m_head.rib6 != 0) {
		indexBuffer.setReadPos(m_head.rib6);
		for (int i = 0; i < m_head.nrib; i++) {
			m_traces[i].ribInfo = indexBuffer.get<XTraceInfo>();
		}
	}

	for (XTrace &rib : m_traces) {
		indexBuffer.setReadPos(rib.nameOffset);
		rib.traceName = indexBuffer.getString();
		if (rib.boneId >= m_head.nbon) rib.boneId = -1;
	}
}

void XScene::loadParticles() {
	indexBuffer.setReadPos(m_head.aprt);
	for (int i = 0; i < m_head.nprt; i++) {
		XParticle prt;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&prt), m_head.fileVersion >= 8 ? sizeof(XParticleStruct_8) : sizeof(XParticleStruct_6));
		m_particles.push_back(std::move(prt));
	}

	for (XParticle &prt : m_particles) {
		prt.particleName = QString(prt.name);
		indexBuffer.setReadPos(prt.infoOffset);
		prt.partInfo = indexBuffer.get<XParticleInfoStruct>();
		prt.blink = prt.flags & 0x2000;
		prt.keepTranslation = prt.flags & 0x40000;
		prt.keepRotation = prt.flags & 0x80000;
		prt.fixedPosition = prt.flags & 0x100000;
		prt.randomDirection = prt.flags & 0x400000;
		if (prt.boneId >= m_head.nbon) prt.boneId = -1;

		if (prt.partInfo.scriptOffset != 0) {
			static char tempScript[1024];
			dataBuffer.setReadPos(OffsetDecrypt(prt.partInfo.scriptOffset));
			dataBuffer.getInt(); dataBuffer.getInt(); dataBuffer.getInt();
			int len = dataBuffer.getInt();
			dataBuffer.getBytes(reinterpret_cast<BYTE*>(tempScript), len);
			tempScript[len] = 0;
			prt.script = QString(tempScript);
		}
	}
}

void XScene::loadActions() {
	indexBuffer.setReadPos(m_head.aact);
	for (int i = 0; i < m_head.nact; i++) {
		XAction act;
		indexBuffer.getBytes(reinterpret_cast<BYTE*>(&act), sizeof(XActionStruct));
		m_actions.push_back(std::move(act));
	}

	for (XAction &act : m_actions) {
		act.actionName = QString(act.name);
	}
}

void XScene::saveToFile(QFile &file) {
	indexBuffer.clear(); dataBuffer.clear();
	indexBuffer.putBytes(g_XFileIndexHead, g_XFileIndexHeadSize);

	saveTextures();
	saveMaterials();

	indexBuffer.setWritePos(indexBuffer.size());
	m_head.mtsi = indexBuffer.size();
	for (int i = 0; i < m_head.nmtl; i++) {
		indexBuffer.putInt(OffsetEncrypt(dataBuffer.size()));
		for (int j = 0; j < m_head.frameCount; j++)
			dataBuffer.putFloat(0);
	}

	m_head.bob2 = indexBuffer.size();
	indexBuffer.put<XBob2Struct>(m_bob2);

	if (!m_desc.isEmpty()) {
		m_head.desc = indexBuffer.size();
		indexBuffer.putString(m_desc);
	}

	saveGeometries();
	saveBones();
	saveBoneGroups();
	saveTraces();
	saveParticles();
	saveActions();

	if (m_head.ntex == 0) m_head.atex = 0;
	if (m_head.nmtl == 0) m_head.amtl = m_head.mtsi = 0;
	if (m_head.ngeo == 0) m_head.ageo = 0;
	if (m_head.nbon == 0) m_head.abon = 0;
	if (m_head.nbgp == 0) m_head.abgp = 0;
	if (m_head.nrib == 0) m_head.arib = m_head.rib6 = 0;
	if (m_head.nprt == 0) m_head.aprt = 0;
	if (m_head.nact == 0) m_head.aact = 0;
	m_head.natt = m_head.aatt = 0;
	m_head.bobj = m_head.acfg = m_head.cfgs = 0;

	ByteBuffer fileBuffer;
	fileBuffer.putBytes(g_XFileHead, g_XFileHeadSize);
	fileBuffer.putInt(m_head.fileVersion);
	fileBuffer.putInt(300);

	fileBuffer.putChars("ntex", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ntex);
	fileBuffer.putChars("atex", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.atex);
	fileBuffer.putChars("nmtl", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nmtl);
	fileBuffer.putChars("amtl", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.amtl);
	fileBuffer.putChars("ngeo", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ngeo);
	fileBuffer.putChars("ageo", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ageo);
	fileBuffer.putChars("nbon", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nbon);
	fileBuffer.putChars("abon", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.abon);
	fileBuffer.putChars("nbgp", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nbgp);
	fileBuffer.putChars("abgp", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.abgp);
	fileBuffer.putChars("natt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.natt);
	fileBuffer.putChars("aatt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aatt);
	fileBuffer.putChars("nrib", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nrib);
	fileBuffer.putChars("arib", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.arib);
	fileBuffer.putChars("nprt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nprt);
	fileBuffer.putChars("aprt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aprt);
	fileBuffer.putChars("nact", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nact);
	fileBuffer.putChars("aact", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aact);
	fileBuffer.putChars("bobj", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.bobj);
	fileBuffer.putChars("bob2", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.bob2);
	fileBuffer.putChars("acfg", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.acfg);
	fileBuffer.putChars("cfgs", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.cfgs);
	fileBuffer.putChars("desc", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.desc);
	fileBuffer.putChars("rib6", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.rib6);
	fileBuffer.putChars("mtsi", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.mtsi);

	m_head.indexSize = indexBuffer.size();
	m_head.dataSize = dataBuffer.size();
	m_head.indexSizeComp = compressBound(m_head.indexSize);
	m_head.dataSizeComp = compressBound(m_head.dataSize);

	BYTE *indexBuf = new BYTE[m_head.indexSize], *dataBuf = new BYTE[m_head.dataSize],
		*indexCompBuf = new BYTE[m_head.indexSizeComp], *dataCompBuf = new BYTE[m_head.dataSizeComp];
	indexBuffer.setReadPos(0); dataBuffer.setReadPos(0);
	indexBuffer.getBytes(indexBuf, indexBuffer.size());
	dataBuffer.getBytes(dataBuf, dataBuffer.size());
	compress(indexCompBuf, &m_head.indexSizeComp, indexBuf, m_head.indexSize);
	compress(dataCompBuf, &m_head.dataSizeComp, dataBuf, m_head.dataSize);

	fileBuffer.putInt(m_head.indexSize);
	fileBuffer.putInt(m_head.dataSize);
	fileBuffer.putInt(m_head.indexSizeComp);
	fileBuffer.putInt(m_head.dataSizeComp);
	fileBuffer.putBytes(indexCompBuf, m_head.indexSizeComp);
	fileBuffer.putBytes(dataCompBuf, m_head.dataSizeComp);

	delete[] indexBuf; delete[] dataBuf;
	delete[] indexCompBuf; delete[] dataCompBuf;

	if (!file.open(QIODevice::WriteOnly))
		throw QString("打开文件") + file.fileName() + "失败";

	BYTE *fileBuf = new BYTE[fileBuffer.size()];
	fileBuffer.setReadPos(0);
	fileBuffer.getBytes(fileBuf, fileBuffer.size());
	file.write(reinterpret_cast<const char*>(fileBuf), fileBuffer.size());
	delete[] fileBuf;
	file.close();
}

void XScene::saveTextures() {
	m_head.atex = indexBuffer.size();
	indexBuffer.resize(m_head.atex + sizeof(XTextureStruct) * m_head.ntex);
	indexBuffer.setWritePos(indexBuffer.size());
	for (XTexture &tex : m_textures) {
		tex.nameOffset = indexBuffer.size();
		indexBuffer.putString(tex.textureName);
	}

	indexBuffer.setWritePos(m_head.atex);
	for (XTexture &tex : m_textures)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&tex), sizeof(XTextureStruct));
}

void XScene::saveMaterials() {
	m_head.amtl = indexBuffer.size();
	indexBuffer.resize(m_head.amtl + sizeof(XMaterialStruct) * m_head.nmtl);
	indexBuffer.setWritePos(indexBuffer.size());
	for (XMaterial &mtl : m_materials) {
		mtl.baseDataOffset = OffsetEncrypt(dataBuffer.size());
		dataBuffer.put<XMaterialBaseDataStruct>(mtl.baseData);

		mtl.flags = 0;
		if (mtl.isSkillEffect) mtl.flags |= 0x4000;
		if (mtl.useTexAlpha) mtl.flags |= 0x8000;
		if (mtl.doubleFace) mtl.flags |= 0x10000;

		auto buffer = selectBuffer();
		mtl.animFrameCount = m_head.frameCount;
		mtl.animDataOffset = buffer.first;
		if (mtl.animData == nullptr) {
			XMaterialAnimDataStruct animData;
			for (int i = 0; i < m_head.frameCount; i++)
				buffer.second.put<XMaterialAnimDataStruct>(animData);
		} else buffer.second.putBytes(reinterpret_cast<BYTE*>(mtl.animData), sizeof(XMaterialAnimDataStruct) * m_head.frameCount);
	}

	indexBuffer.setWritePos(m_head.amtl);
	for (XMaterial &mtl : m_materials)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&mtl), sizeof(XMaterialStruct));
}

void XScene::saveGeometries() {
	m_head.ageo = indexBuffer.size();
	indexBuffer.resize(m_head.ageo + sizeof(XGeometryStruct) * m_head.ngeo);
	indexBuffer.setWritePos(indexBuffer.size());

	m_head.nbgp = 0;
	m_boneGroups.clear();

	for (XGeometry &geo : m_geometries) {
		geo.scriptOffset = 0;
		geo.nameOffset = indexBuffer.size();
		indexBuffer.putString(geo.meshName);

		auto buffer = selectBuffer();
		geo.vertexDataOffset[0] = buffer.first;
		buffer.second.putBytes(reinterpret_cast<BYTE*>(geo.vertexData), sizeof(QVector3D) * geo.vertexCount);
		geo.vertexDataOffset[1] = 0;

		if (geo.normalData != nullptr) {
			auto buffer = selectBuffer();
			geo.normalDataOffset[0] = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(geo.normalData), sizeof(QVector3D) * geo.vertexCount);
		} else geo.normalDataOffset[0] = 0;
		geo.normalDataOffset[1] = 0;

		if (geo.texcoordData[0] != nullptr) {
			auto buffer = selectBuffer();
			geo.texcoordDataOffset[0] = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(geo.texcoordData[0]), sizeof(QVector2D) * geo.vertexCount);
		} else geo.texcoordDataOffset[0] = 0;

		if (geo.texcoordData[1] != nullptr) {
			auto buffer = selectBuffer();
			geo.texcoordDataOffset[1] = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(geo.texcoordData[1]), sizeof(QVector2D) * geo.vertexCount);
		} else geo.texcoordDataOffset[1] = 0;

		if (geo.vertexColorData != nullptr) {
			auto buffer = selectBuffer();
			geo.vertexColorOffset = buffer.first;
			for (int i = 0; i < geo.vertexCount; i++) {
				buffer.second.putByte(qRed(geo.vertexColorData[i]));
				buffer.second.putByte(qGreen(geo.vertexColorData[i]));
				buffer.second.putByte(qBlue(geo.vertexColorData[i]));
				buffer.second.putByte(qAlpha(geo.vertexColorData[i]));
			}
		} else geo.vertexColorOffset = 0;

		if (geo.boneData != nullptr) {
			auto buffer = selectBuffer();
			geo.boneGroupOffset = buffer.first;
			for (int i = 0; i < geo.vertexCount; i++) {
				int foundbgp = -1;
				for (int j = 0; j < m_head.nbgp; j++) {
					if (m_boneGroups[j].boneCnt != geo.boneData[i].boneCount)
						continue;
					bool equal = true;
					for (int k = 0; k < m_boneGroups[j].boneCnt; k++) {
						if (m_boneGroups[j].bones[k] != geo.boneData[i].bones[k]) {
							equal = false;
							break;
						}
					}
					if (equal) {
						foundbgp = j;
						break;
					}
				}
				if (foundbgp == -1) {
					XBoneGroup newBgp;
					newBgp.boneCnt = geo.boneData[i].boneCount;
					for (int j = 0; j < newBgp.boneCnt; j++)
						newBgp.bones[j] = geo.boneData[i].bones[j];
					foundbgp = m_head.nbgp++;
					m_boneGroups.push_back(std::move(newBgp));
				}
				foundbgp = min(foundbgp, 255);
				buffer.second.putByte(static_cast<BYTE>(foundbgp));
			}
		} else geo.boneGroupOffset = 0;

		{
			auto buffer = selectBuffer();
			geo.indicesOffset = buffer.first;
			for (int i = 0; i < geo.faceCount * 3; i++)
				buffer.second.putShort(static_cast<WORD>(geo.indicesData[i]));
		}

		if (geo.boneData != nullptr) {
			auto buffer = selectBuffer();
			geo.boneDataOffset = buffer.first;
			for (int i = 0; i < geo.vertexCount; i++) {
				XBoneDataStruct data;
				data.boneCount = geo.boneData[i].boneCount;
				for (int j = 0; j < data.boneCount; j++) {
					data.bones[j] = geo.boneData[i].bones[j];
					data.weight[j] = geo.boneData[i].weight[j];
				}
				buffer.second.put<XBoneDataStruct>(data);
			}
		}

		if (!geo.script.isEmpty()) {
			QByteArray ba = geo.script.toLocal8Bit();
			geo.scriptOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putInt(ba.length() + 12);
			dataBuffer.putChars("scrp", 4);
			dataBuffer.putInt(ba.length() + 4);
			dataBuffer.putInt(ba.length());
			for (auto &c : ba)
				dataBuffer.putChar(c);
		} else geo.scriptOffset = 0;
	}

	indexBuffer.setWritePos(m_head.ageo);
	for (XGeometry &geo : m_geometries)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&geo), sizeof(XGeometryStruct));
}

void XScene::saveBones() {
	m_head.abon = indexBuffer.size();
	indexBuffer.resize(m_head.abon + sizeof(XBoneStruct) * m_head.nbon);
	indexBuffer.setWritePos(indexBuffer.size());

	for (int i = 0; i < m_head.nbon; i++) {
		XBone &bone = m_bones[i];
		bone.nameOffset = indexBuffer.size();
		indexBuffer.putString(bone.boneName);
		bone.frameCount = m_head.frameCount;
		bone.unknown0[1] = 0;

		bone.childCount = 0;
		bone.childOffset = indexBuffer.size();
		for (int j = 0; j < m_head.nbon; j++) {
			if (m_bones[j].parentId == i) {
				bone.childCount++;
				indexBuffer.putInt(j);
			}
		}

		if (bone.visibleData != nullptr) {
			auto buffer = selectBuffer();
			bone.visibleFrameCount = m_head.frameCount;
			bone.visibleDataOffset = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(bone.visibleData), sizeof(int) * m_head.frameCount);
		} else bone.visibleFrameCount = bone.visibleDataOffset = 0;

		bone.posDataOffset[1] = 0;
		if (bone.posData != nullptr) {
			auto buffer = selectBuffer();
			bone.posFrameCount = m_head.frameCount;
			bone.posDataOffset[0] = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(bone.posData), sizeof(QVector3D) * m_head.frameCount);
		} else bone.posDataOffset[0] = bone.posFrameCount = 0;

		bone.rotDataOffset[1] = 0;
		if (bone.rotData != nullptr) {
			auto buffer = selectBuffer();
			bone.rotFrameCount = m_head.frameCount;
			bone.rotDataOffset[0] = buffer.first;
			for (int i = 0; i < m_head.frameCount; i++) {
				QQuaternion quat = bone.rotData[i].inverted();
				buffer.second.putFloat(quat.x());
				buffer.second.putFloat(quat.y());
				buffer.second.putFloat(quat.z());
				buffer.second.putFloat(quat.scalar());
			}
		} else bone.rotDataOffset[0] = bone.rotFrameCount = 0;

		if (bone.scaleData != nullptr) {
			auto buffer = selectBuffer();
			bone.scaleFrameCount = m_head.frameCount;
			bone.scaleDataOffset = buffer.first;
			buffer.second.putBytes(reinterpret_cast<BYTE*>(bone.scaleData), sizeof(QVector3D) * m_head.frameCount);
		} else bone.scaleDataOffset = bone.scaleFrameCount = 0;
	}

	indexBuffer.setWritePos(m_head.abon);
	for (XBone &bone : m_bones)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&bone), sizeof(XBoneStruct));
}

void XScene::saveBoneGroups() {
	m_head.abgp = indexBuffer.size();
	indexBuffer.resize(m_head.abgp + sizeof(XBoneGroupStruct) * m_head.nbgp);
	indexBuffer.setWritePos(indexBuffer.size());
	for (XBoneGroup &bgp : m_boneGroups) {
		auto buffer = selectBuffer();
		bgp.boneOffset = buffer.first;
		for (int i = 0; i < bgp.boneCnt; i++)
			buffer.second.putInt(bgp.bones[i]);
	}

	indexBuffer.setWritePos(m_head.abgp);
	for (XBoneGroup &bgp : m_boneGroups)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&bgp), sizeof(XBoneGroupStruct));
}

void XScene::saveTraces() {
	m_head.arib = indexBuffer.size();
	indexBuffer.resize(m_head.arib + sizeof(XTraceStruct) * m_head.nrib);
	m_head.rib6 = indexBuffer.size();
	indexBuffer.setWritePos(m_head.rib6);
	for (XTrace &rib : m_traces) {
		indexBuffer.put<XTraceInfo>(rib.ribInfo);
	}

	for (XTrace &rib : m_traces) {
		rib.nameOffset = indexBuffer.size();
		indexBuffer.putString(rib.traceName);
		rib.emptyOffset = indexBuffer.size();
		indexBuffer.putString("");
	}

	indexBuffer.setWritePos(m_head.arib);
	for (XTrace &rib : m_traces)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&rib), sizeof(XTraceStruct));
}

void XScene::saveParticles() {
	m_head.aprt = indexBuffer.size();
	indexBuffer.resize(m_head.aprt + (m_head.fileVersion >= 8 ? sizeof(XParticleStruct_8) : sizeof(XParticleStruct_6)) * m_head.nprt);
	indexBuffer.setWritePos(indexBuffer.size());
	for (XParticle &prt : m_particles) {
		if (!prt.script.isEmpty()) {
			QByteArray ba = prt.script.toLocal8Bit();
			prt.partInfo.scriptOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putInt(ba.length() + 12);
			dataBuffer.putChars("scrp", 4);
			dataBuffer.putInt(ba.length() + 4);
			dataBuffer.putInt(ba.length());
			for (auto &c : ba)
				dataBuffer.putChar(c);
		} else prt.partInfo.scriptOffset = 0;

		QByteArray name = prt.particleName.toLocal8Bit();
		if (name.size() > 78) name = name.left(78);
		strcpy(prt.name, name.constData());
		prt.infoOffset = indexBuffer.size();
		indexBuffer.put<XParticleInfoStruct>(prt.partInfo);
		prt.flags = 0;
		if (prt.blink) prt.flags |= 0x2000;
		if (prt.keepTranslation) prt.flags |= 0x40000;
		if (prt.keepRotation) prt.flags |= 0x80000;
		if (prt.fixedPosition) prt.flags |= 0x100000;
		if (prt.randomDirection) prt.flags |= 0x400000;
	}

	indexBuffer.setWritePos(m_head.aprt);
	for (XParticle &prt : m_particles)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&prt), m_head.fileVersion >= 8 ? sizeof(XParticleStruct_8) : sizeof(XParticleStruct_6));
}

void XScene::saveActions() {
	m_head.aact = indexBuffer.size();
	indexBuffer.setWritePos(m_head.aact);
	for (XAction &act : m_actions) {
		QByteArray name = act.actionName.toLocal8Bit();
		if (name.size() > 78) name = name.left(78);
		strcpy(act.name, name.constData());
		act.midFrame = -1;
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&act), sizeof(XActionStruct));
	}
}

pair<int, ByteBuffer&> XScene::selectBuffer() {
	if (indexBuffer.size() > dataBuffer.size())
		return { OffsetEncrypt(dataBuffer.size()), dataBuffer };
	else return { indexBuffer.size(), indexBuffer };
}