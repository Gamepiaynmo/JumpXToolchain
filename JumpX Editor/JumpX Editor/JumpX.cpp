#include "stdafx.h"

#include "bf\blowfish.h"

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

template<typename T> T* ReallocArray(T* orinArr, int orinSize, int newSize) {
	T* newArr = new T[newSize];
	memcpy(newArr, orinArr, sizeof(T) * orinSize);
	for (int i = orinSize; i < newSize; i++)
		newArr[i] = orinArr[orinSize - 1];
	delete[] orinArr;
	return newArr;
}

QString XScene::readScript(uint addr) {
	if (addr != 0) {
		dataBuffer.setReadPos(OffsetDecrypt(addr));
		dataBuffer.getInt(); dataBuffer.getInt(); dataBuffer.getInt();
		uint len = dataBuffer.getUInt();
		QString script = dataBuffer.getStringLen(len);
#ifndef NDEBUG
		_cwprintf(L"%s\n", script.constData());
#endif
		return script;
	}
	return "";
}

void XScene::writeScript(uint addr, QString script) {
	if (addr != 0) {
		QByteArray ba = script.toLocal8Bit();
		dataBuffer.setWritePos(OffsetDecrypt(addr));
		dataBuffer.putInt(ba.length() + 12);
		dataBuffer.putBytes("scrp", 4);
		dataBuffer.putInt(ba.length() + 4);
		dataBuffer.putInt(ba.length());
		for (auto &c : ba)
			dataBuffer.putChar(c);
	}
}

void XScene::loadFromFile(QFile &file) {
	if (!file.open(QIODevice::ReadOnly))
		throw QString("打开文件") + file.fileName() + "失败";
	startProgress("读取文件", 12);

	QByteArray qba = file.readAll();
	ByteBuffer fileBuffer(reinterpret_cast<BYTE*>(qba.data()), qba.size());
	file.close();

	BYTE xHead[g_XFileHeadSize];
	fileBuffer.getBytes(xHead, g_XFileHeadSize);
	if (memcmp(xHead, g_XFileHead, g_XFileHeadSize))
		throw QString("无效的文件头");

	int low_version = 5;
	m_head.version = fileBuffer.getInt();
	if (m_head.version < low_version || m_head.version > 8)
		warn("未支持的X版本：" + QString::number(m_head.version));

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

	m_head.headSize = fileBuffer.getInt();
	m_head.dataSize = fileBuffer.getInt();
	m_head.headSizeComp = fileBuffer.getInt();
	m_head.dataSizeComp = fileBuffer.getInt();
	updateProgress();

	DWORD actualHeadSize = m_head.headSize, actualDataSize = m_head.dataSize;
	BYTE *headBuf = new BYTE[actualHeadSize], *dataBuf = new BYTE[actualDataSize],
		*headCompBuf = new BYTE[m_head.headSizeComp], *dataCompBuf = new BYTE[m_head.dataSizeComp];
	fileBuffer.getBytes(headCompBuf, m_head.headSizeComp);
	fileBuffer.getBytes(dataCompBuf, m_head.dataSizeComp);
	uncompress(headBuf, &actualHeadSize, headCompBuf, m_head.headSizeComp);
	uncompress(dataBuf, &actualDataSize, dataCompBuf, m_head.dataSizeComp);

	if (actualHeadSize != m_head.headSize || actualDataSize != m_head.dataSize)
		throw QString("文件数据损坏");

#ifndef NDEBUG
	QFile fhead("head.bin");
	fhead.open(QIODevice::WriteOnly);
	fhead.write((char *) headBuf, actualHeadSize);
	fhead.close();
	QFile fdata("data.bin");
	fdata.open(QIODevice::WriteOnly);
	fdata.write((char *) dataBuf, actualDataSize);
	fdata.close();
#endif

	headBuffer.putBytes(headBuf, actualHeadSize);
	dataBuffer.putBytes(dataBuf, actualDataSize);
	delete[] headBuf; delete[] dataBuf;
	delete[] headCompBuf; delete[] dataCompBuf;

	BYTE xIndexHead[g_XFileIndexHeadSize];
	headBuffer.getBytes(xIndexHead, g_XFileIndexHeadSize);
	if (memcmp(xIndexHead, g_XFileIndexHead, g_XFileIndexHeadSize))
		throw QString("文件数据损坏");
	updateProgress();

	loadTextures();
	updateProgress();

	loadMaterials();
	updateProgress();

	loadGeometries();
	updateProgress();

	loadBones();
	updateProgress();

	loadBoneGroups();
	updateProgress();

	loadAttachments();
	updateProgress();

	loadRibbons();
	updateProgress();

	loadParticles();
	updateProgress();

	loadActions();
	updateProgress();

	if (m_head.bob2 != 0) {
		headBuffer.setReadPos(m_head.bob2);
		m_bob2 = headBuffer.get<XBob2Struct>();
	}

	if (m_head.desc != 0) {
		headBuffer.setReadPos(m_head.desc);
		m_desc = headBuffer.getString();
	}

	m_head.numKey = 0;
	for (XMaterial &mtl : m_materials)
		if (mtl.numColorKey > 0 && m_head.numKey == 0)
			m_head.numKey = mtl.numColorKey;
	if (m_head.numKey == 0)
		for (XBone &bone : m_bones)
			if (bone.numKey > 0 && m_head.numKey == 0)
				m_head.numKey = bone.numKey;

	for (XMaterial &mtl : m_materials) {
		if (mtl.numColorKey > 0 && mtl.numColorKey < m_head.numKey) {
			mtl.colorKeys = ReallocArray<XMaterialAnimDataStruct>(mtl.colorKeys, mtl.numColorKey, m_head.numKey);
			mtl.numColorKey = m_head.numKey;
		}
	}
	for (XBone &bon : m_bones) {
		if (bon.numVisibleKey > 0 && bon.numVisibleKey < m_head.numKey) {
			bon.visibleData = ReallocArray<uint>(bon.visibleData, bon.numVisibleKey, m_head.numKey);
			bon.numVisibleKey = m_head.numKey;
		}
		if (bon.numPosKey > 0 && bon.numPosKey < m_head.numKey) {
			bon.posData = ReallocArray<QVector3D>(bon.posData, bon.numPosKey, m_head.numKey);
			bon.numPosKey = m_head.numKey;
		}
		if (bon.numRotKey > 0 && bon.numRotKey < m_head.numKey) {
			bon.rotData = ReallocArray<QQuaternion>(bon.rotData, bon.numRotKey, m_head.numKey);
			bon.numRotKey = m_head.numKey;
		}
		if (bon.numScaleKey > 0 && bon.numScaleKey < m_head.numKey) {
			bon.scaleData = ReallocArray<QVector3D>(bon.scaleData, bon.numScaleKey, m_head.numKey);
			bon.numScaleKey = m_head.numKey;
		}
	}

	m_head.bobj = m_head.acfg = m_head.cfgs = 0;
	updateProgress();
}

void XScene::loadTextures() {
	headBuffer.setReadPos(m_head.atex);
	for (int i = 0; i < m_head.ntex; i++) {
		XTexture tex;
		headBuffer.get<XTextureStruct>(tex);
		m_textures.push_back(std::move(tex));
	}

	for (XTexture &tex : m_textures) {
		headBuffer.setReadPos(tex.nameAddr);
		tex.name = headBuffer.getString();
	}
}

void XScene::loadMaterials() {
	headBuffer.setReadPos(m_head.amtl);
	for (int i = 0; i < m_head.nmtl; i++) {
		XMaterial mtl;
		headBuffer.get<XMaterialStruct>(mtl);
		m_materials.push_back(std::move(mtl));
	}

	for (XMaterial &mtl : m_materials) {
		dataBuffer.setReadPos(OffsetDecrypt(mtl.dataAddr));
		mtl.exData = dataBuffer.get<XMaterialExStruct>();
		if (mtl.exData.bumpTexId >= m_head.ntex) mtl.exData.flag &= ~EFFECT_BUMP;
		if (mtl.exData.lightTexId >= m_head.ntex) mtl.exData.flag &= ~EFFECT_LIGHT;
		if (mtl.exData.specTexId >= m_head.ntex) mtl.exData.flag &= ~EFFECT_SPECULAR;

		if (mtl.exData.flag & EFFECT_CARTOON && mtl.exData.dataAddr != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(mtl.exData.dataAddr));
			mtl.cartoonData = dataBuffer.get<XMaterialCartoonStruct>();
			if (!(mtl.cartoonData.specTextureID >= 0 && mtl.cartoonData.shadowTextureID >= 0))
				setFlag(mtl.exData.flag, EFFECT_CARTOON, false);
		}
		if (mtl.exData.flag & EFFECT_DISSOLVE && mtl.exData.dissolveDataAddr != 0) {
			dataBuffer.setReadPos(OffsetDecrypt(mtl.exData.dissolveDataAddr));
			mtl.dissolveData = dataBuffer.get<XMaterialDissolveStruct>();
			if (!(mtl.dissolveData.dissolveTextureID >= 0))
				setFlag(mtl.exData.flag, EFFECT_DISSOLVE, false);
			if (mtl.dissolveData.thresholdKeyAddr != 0) {
				dataBuffer.setReadPos(OffsetDecrypt(mtl.dissolveData.thresholdKeyAddr));
				mtl.dissolveKeys = new float[mtl.numColorKey];
				dataBuffer.getBytes(mtl.dissolveKeys, sizeof(float) * mtl.numColorKey);
			}
		}
		if (mtl.numColorKey > 0) {
			dataBuffer.setReadPos(OffsetDecrypt(mtl.colorKeyAddr));
			mtl.colorKeys = new XMaterialAnimDataStruct[mtl.numColorKey];
			for (int j = 0; j < mtl.numColorKey; j++)
				mtl.colorKeys[j] = dataBuffer.get<XMaterialAnimDataStruct>();
		}
	}
}

void XScene::loadGeometries() {
	headBuffer.setReadPos(m_head.ageo);
	for (int i = 0; i < m_head.ngeo; i++) {
		XGeometry geo;
		headBuffer.get<XGeometryStruct>(geo);
		m_geometries.push_back(std::move(geo));
	}

	for (XGeometry &geo : m_geometries) {
		headBuffer.setReadPos(geo.nameAddr);
		geo.name = headBuffer.getString();
		geo.script = readScript(geo.dataAddr);

		if (geo.saveFlag & XFGEOCHUNKSAVE_COMPRESSED_VERTEX && geo.vertexCompAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.vertexCompAddr));
			geo.vertexData = new QVector3D[geo.numVertex];
			for (int i = 0; i < geo.numVertex; i++)
				geo.vertexData[i] = geo.bbox.uncompress(dataBuffer.getUInt());
		} else if (geo.vertexAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.vertexAddr));
			geo.vertexData = new QVector3D[geo.numVertex];
			dataBuffer.getBytes(geo.vertexData, sizeof(QVector3D) * geo.numVertex);
		} else throw QString("网格") + geo.name + "没有顶点数据";

		for (int i = 0; i < geo.numVertex; i++)
			geo.center += geo.vertexData[i];
		geo.center /= geo.numVertex;

		if (geo.saveFlag & XFGEOCHUNKSAVE_COMPRESSED_NORMAL && geo.normalCompAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.normalCompAddr));
			geo.normalData = new QVector3D[geo.numVertex];
			for (int i = 0; i < geo.numVertex; i++) {
				QVector3D &vec = geo.normalData[i];
				vec.setX(dataBuffer.getChar() / 127.0f);
				vec.setY(dataBuffer.getChar() / 127.0f);
				vec.setZ(dataBuffer.getChar() / 127.0f);
				vec.normalize();
			}
		} else if (geo.normalAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.normalAddr));
			geo.normalData = new QVector3D[geo.numVertex];
			dataBuffer.getBytes(geo.normalData, sizeof(QVector3D) * geo.numVertex);
		} else throw QString("网格") + geo.name + "没有法线数据";

		if (geo.uvAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.uvAddr));
			geo.uvData[0] = new QVector2D[geo.numVertex];
			dataBuffer.getBytes(geo.uvData[0], sizeof(QVector2D) * geo.numVertex);
		}

		if (geo.saveFlag & XFGEOCHUNKSAVE_ENABLE_UV2 && geo.uv1Addr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.uv1Addr));
			geo.uvData[1] = new QVector2D[geo.numVertex];
			dataBuffer.getBytes(geo.uvData[1], sizeof(QVector2D) * geo.numVertex);
		}

		if (geo.vertColAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.vertColAddr));
			geo.vertColData = new XCOLOR[geo.numVertex];
			dataBuffer.getBytes(geo.vertColData, sizeof(XCOLOR) * geo.numVertex);
		}

		if (geo.indicesAddr) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.indicesAddr));
			geo.indicesData = new ushort[geo.numFace * 3];
			dataBuffer.getBytes(geo.indicesData, sizeof(ushort) * 3 * geo.numFace);
		} else throw QString("网格") + geo.name + "没有面数据";

		if (geo.saveFlag & XFGEOCHUNKSAVE_ENABLE_BONE_PALETTE) {
			dataBuffer.setReadPos(OffsetDecrypt(geo.bonePaletteAddr));
			geo.boneData = new XBonePaletteEditor[geo.numVertex];
			for (int i = 0; i < geo.numVertex; i++) {
				XBonePaletteStruct data = dataBuffer.get<XBonePaletteStruct>();
				geo.boneData[i].numBone = data.numBone;
				if (data.numBone > 4) throw QString("网格") + geo.name + "骨骼影响限制大于4";
				for (int j = 0; j < geo.boneData[i].numBone; j++) {
					geo.boneData[i].bones[j] = data.bones[j];
					geo.boneData[i].weight[j] = data.weight[j];
				}
			}
			for (int i = 0; i < geo.numVertex; i++) {
				for (int j = 0; j < geo.boneData[i].numBone;) {
					if (geo.boneData[i].weight[j] < g_EPS) {
						geo.boneData[i].numBone--;
						for (int k = j; k < geo.boneData[i].numBone; k++) {
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
	headBuffer.setReadPos(m_head.abon);
	for (int i = 0; i < m_head.nbon; i++) {
		XBone bone;
		headBuffer.get<XBoneStruct>(bone);
		m_bones.push_back(std::move(bone));
	}

	for (XBone &bone : m_bones) {
		headBuffer.setReadPos(bone.nameAddr);
		bone.name = headBuffer.getString();
		bone.matrixInv = QMatrix4x4(bone.matInv).transposed();
		bone.matrix = bone.matrixInv.inverted();

		if (bone.numChild > 0) {
			bone.children = new int[bone.numChild];
			headBuffer.setReadPos(bone.childAddr);
			for (int i = 0; i < bone.numChild; i++)
				bone.children[i] = headBuffer.getInt();
		}

		if (bone.numKey > 0) {
			if (bone.numVisibleKey > 0) {
				dataBuffer.setReadPos(OffsetDecrypt(bone.visibleKeyAddr));
				bone.visibleData = new uint[bone.numVisibleKey];
				dataBuffer.getBytes(bone.visibleData, sizeof(uint) * bone.numVisibleKey);
			}

			if (bone.numPosKey > 0) {
				bone.posData = new QVector3D[bone.numPosKey];
				if (bone.saveFlag & XFGEOCHUNKSAVE_COMPRESSED_VERTEX && bone.posKeyAddr == 0) {
					dataBuffer.setReadPos(OffsetDecrypt(bone.posKeyCompAddr));
					for (int i = 0; i < bone.numPosKey; i++)
						bone.posData[i] = bone.transBbox.uncompress(dataBuffer.getUInt());
				} else {
					dataBuffer.setReadPos(OffsetDecrypt(bone.posKeyAddr));
					dataBuffer.getBytes(bone.posData, sizeof(QVector3D) * bone.numPosKey);
				}
			}

			if (bone.numRotKey > 0) {
				bone.rotData = new QQuaternion[bone.numRotKey];
				if (bone.saveFlag & XFGEOCHUNKSAVE_COMPRESSED_NORMAL && bone.rotKeyAddr == 0) {
					dataBuffer.setReadPos(OffsetDecrypt(bone.rotKeyCompAddr));
					for (int i = 0; i < bone.numRotKey; i++) {
						ullong rot = dataBuffer.getULong();
						float a = short(rot & 0xfff) * g_PI / 180; rot >>= 16;
						float x = short(rot & 0xfff) / 2048.0f; rot >>= 16;
						float y = short(rot & 0xfff) / 2048.0f; rot >>= 16;
						float z = short(rot & 0xfff) / 2048.0f; rot >>= 16;
						bone.rotData[i] = QQuaternion::fromAxisAndAngle(x, y, z, a).inverted();
					}
				} else {
					float quat[4];
					dataBuffer.setReadPos(OffsetDecrypt(bone.rotKeyAddr));
					for (int i = 0; i < bone.numRotKey; i++) {
						dataBuffer.getBytes(quat, sizeof(quat));
						bone.rotData[i] = QQuaternion(quat[3], quat[0], quat[1], quat[2]).inverted();
					}
				}
			}

			if (bone.numScaleKey > 0) {
				dataBuffer.setReadPos(OffsetDecrypt(bone.scaleKeyAddr));
				bone.scaleData = new QVector3D[bone.numScaleKey];
				dataBuffer.getBytes(bone.scaleData, sizeof(QVector3D) * bone.numScaleKey);
			}
		}
	}
}

void XScene::loadBoneGroups() {
	headBuffer.setReadPos(m_head.abgp);
	for (int i = 0; i < m_head.nbgp; i++) {
		XBoneGroup bgp;
		headBuffer.get<XBoneGroupStruct>(bgp);
		m_boneGroups.push_back(std::move(bgp));
	}

	for (XBoneGroup &bgp : m_boneGroups) {
		headBuffer.setReadPos(bgp.boneAddr);
		for (int i = 0; i < bgp.numBone; i++)
			bgp.bones[i] = headBuffer.getInt();
	}
}

void XScene::loadAttachments() {
	headBuffer.setReadPos(m_head.aatt);
	for (int i = 0; i < m_head.natt; i++) {
		XAttachment att;
		headBuffer.get<XAttachmentStruct>(att);
		m_attachments.push_back(std::move(att));
	}

	for (XAttachment &att : m_attachments) {
		att.name = QString(att.attachName);
		att.matrix = QMatrix4x4(att.matInit);
	}
}

void XScene::loadRibbons() {
	headBuffer.setReadPos(m_head.arib);
	for (int i = 0; i < m_head.nrib; i++) {
		XRibbon rib;
		headBuffer.get<XRibbonStruct>(rib);
		m_ribbons.push_back(std::move(rib));
	}

	if (m_head.rib6 != 0) {
		headBuffer.setReadPos(m_head.rib6);
		for (int i = 0; i < m_head.nrib; i++) {
			m_ribbons[i].ribEx = headBuffer.get<XRibbonExStruct>();
		}
	}

	for (XRibbon &rib : m_ribbons) {
		headBuffer.setReadPos(rib.nameAddr);
		rib.name = headBuffer.getString();
		if (rib.boneId >= m_head.nbon) rib.boneId = -1;
	}
}

void XScene::loadParticles() {
	headBuffer.setReadPos(m_head.aprt);
	for (int i = 0; i < m_head.nprt; i++) {
		XParticle prt;
		m_head.version >= 8 ? headBuffer.get<XParticleStruct>(prt) : headBuffer.get<XParticleStruct_6>(prt);
		m_particles.push_back(std::move(prt));
	}

	for (XParticle &prt : m_particles) {
		prt.name = QString(prt.partName);
		headBuffer.setReadPos(prt.dataAddr);
		prt.partEx = headBuffer.get<XParticleExStruct>();
		if (prt.boneId >= m_head.nbon) prt.boneId = -1;
		prt.script = readScript(prt.partEx.scriptAddr);
	}
}

void XScene::loadActions() {
	headBuffer.setReadPos(m_head.aact);
	for (int i = 0; i < m_head.nact; i++) {
		XAction act;
		headBuffer.get<XActionStruct>(act);
		m_actions.push_back(std::move(act));
	}

	for (XAction &act : m_actions) {
		act.name = QString(act.actionName);
	}
}

void XScene::saveToFile(QFile &file) {
	headBuffer.clear(); dataBuffer.clear();

	startProgress("保存文件", 12);
	headBuffer.putBytes(g_XFileIndexHead, g_XFileIndexHeadSize);

	saveTextures();
	updateProgress();

	saveMaterials();
	updateProgress();

	m_head.mtsi = headBuffer.size();
	for (int i = 0; i < m_head.nmtl; i++) {
		headBuffer.putInt(OffsetEncrypt(dataBuffer.size()));
		for (int j = 0; j < m_head.numKey; j++)
			dataBuffer.putFloat(0);
	}

	m_head.bob2 = headBuffer.size();
	headBuffer.put<XBob2Struct>(m_bob2);

	if (!m_desc.isEmpty()) {
		m_head.desc = headBuffer.size();
		headBuffer.putString(m_desc);
	}

	saveGeometries();
	updateProgress();

	saveBones();
	updateProgress();

	saveBoneGroups();
	updateProgress();

	saveAttachments();
	updateProgress();

	saveRibbons();
	updateProgress();

	saveParticles();
	updateProgress();

	saveActions();
	updateProgress();


	if (m_head.ntex == 0) m_head.atex = 0;
	if (m_head.nmtl == 0) m_head.amtl = m_head.mtsi = 0;
	if (m_head.ngeo == 0) m_head.ageo = 0;
	if (m_head.nbon == 0) m_head.abon = 0;
	if (m_head.nbgp == 0) m_head.abgp = 0;
	if (m_head.natt == 0) m_head.aatt = 0;
	if (m_head.nrib == 0) m_head.arib = m_head.rib6 = 0;
	if (m_head.nprt == 0) m_head.aprt = 0;
	if (m_head.nact == 0) m_head.aact = 0;
	m_head.bobj = m_head.acfg = m_head.cfgs = 0;

	ByteBuffer fileBuffer;
	fileBuffer.putBytes(g_XFileHead, g_XFileHeadSize);
	fileBuffer.putInt(m_head.version);
	fileBuffer.putInt(300);

	fileBuffer.putBytes("ntex", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ntex);
	fileBuffer.putBytes("atex", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.atex);
	fileBuffer.putBytes("nmtl", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nmtl);
	fileBuffer.putBytes("amtl", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.amtl);
	fileBuffer.putBytes("ngeo", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ngeo);
	fileBuffer.putBytes("ageo", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.ageo);
	fileBuffer.putBytes("nbon", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nbon);
	fileBuffer.putBytes("abon", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.abon);
	fileBuffer.putBytes("nbgp", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nbgp);
	fileBuffer.putBytes("abgp", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.abgp);
	fileBuffer.putBytes("natt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.natt);
	fileBuffer.putBytes("aatt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aatt);
	fileBuffer.putBytes("nrib", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nrib);
	fileBuffer.putBytes("arib", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.arib);
	fileBuffer.putBytes("nprt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nprt);
	fileBuffer.putBytes("aprt", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aprt);
	fileBuffer.putBytes("nact", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.nact);
	fileBuffer.putBytes("aact", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.aact);
	fileBuffer.putBytes("bobj", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.bobj);
	fileBuffer.putBytes("bob2", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.bob2);
	fileBuffer.putBytes("acfg", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.acfg);
	fileBuffer.putBytes("cfgs", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.cfgs);
	fileBuffer.putBytes("desc", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.desc);
	fileBuffer.putBytes("rib6", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.rib6);
	fileBuffer.putBytes("mtsi", 4); fileBuffer.putInt(4); fileBuffer.putInt(m_head.mtsi);

	m_head.headSize = headBuffer.size();
	m_head.dataSize = dataBuffer.size();
	m_head.headSizeComp = compressBound(m_head.headSize);
	m_head.dataSizeComp = compressBound(m_head.dataSize);
	updateProgress();

	BYTE *headBuf = new BYTE[m_head.headSize], *dataBuf = new BYTE[m_head.dataSize],
		*headCompBuf = new BYTE[m_head.headSizeComp], *dataCompBuf = new BYTE[m_head.dataSizeComp];
	headBuffer.setReadPos(0); dataBuffer.setReadPos(0);
	headBuffer.getBytes(headBuf, headBuffer.size());
	dataBuffer.getBytes(dataBuf, dataBuffer.size());
	compress(headCompBuf, (ulong *) &m_head.headSizeComp, headBuf, m_head.headSize);
	compress(dataCompBuf, (ulong *) &m_head.dataSizeComp, dataBuf, m_head.dataSize);
	updateProgress();

	fileBuffer.putInt(m_head.headSize);
	fileBuffer.putInt(m_head.dataSize);
	fileBuffer.putInt(m_head.headSizeComp);
	fileBuffer.putInt(m_head.dataSizeComp);
	fileBuffer.putBytes(headCompBuf, m_head.headSizeComp);
	fileBuffer.putBytes(dataCompBuf, m_head.dataSizeComp);

	delete[] headBuf; delete[] dataBuf;
	delete[] headCompBuf; delete[] dataCompBuf;
	updateProgress();

	if (!file.open(QIODevice::WriteOnly))
		throw QString("打开文件") + file.fileName() + "失败";
	file.write((char *) fileBuffer.getDataPtr(), fileBuffer.size());
	file.close();
}

void XScene::saveTextures() {
	m_head.atex = headBuffer.size();
	headBuffer.resize(m_head.atex + sizeof(XTextureStruct) * m_head.ntex);
	headBuffer.setWritePos(headBuffer.size());
	for (XTexture &tex : m_textures) {
		tex.nameAddr = headBuffer.size();
		headBuffer.putString(tex.name);
	}

	headBuffer.setWritePos(m_head.atex);
	for (XTexture &tex : m_textures)
		headBuffer.put<XTextureStruct>(tex);
}

void XScene::saveMaterials() {
	m_head.amtl = headBuffer.size();
	headBuffer.resize(m_head.amtl + sizeof(XMaterialStruct) * m_head.nmtl);
	headBuffer.setWritePos(headBuffer.size());
	for (XMaterial &mtl : m_materials) {
		mtl.dataAddr = OffsetEncrypt(dataBuffer.size());
		uint writePos = dataBuffer.getWritePos() + sizeof(XMaterialExStruct);
		if (mtl.exData.flag & EFFECT_CARTOON) {
			mtl.exData.dataAddr = OffsetEncrypt(writePos);
			writePos += sizeof(XMaterialCartoonStruct);
		}
		if (mtl.exData.flag & EFFECT_DISSOLVE) {
			mtl.exData.dissolveDataAddr = OffsetEncrypt(writePos);
			writePos += sizeof(XMaterialDissolveStruct);
			mtl.dissolveData.thresholdKeyAddr = OffsetEncrypt(writePos);
		}

		dataBuffer.put<XMaterialExStruct>(mtl.exData);
		if (mtl.exData.flag & EFFECT_CARTOON)
			dataBuffer.put<XMaterialCartoonStruct>(mtl.cartoonData);
		if (mtl.exData.flag & EFFECT_DISSOLVE) {
			dataBuffer.put<XMaterialDissolveStruct>(mtl.dissolveData);
			dataBuffer.putBytes(mtl.dissolveKeys, sizeof(float) * mtl.numColorKey);
		}

		mtl.numColorKey = m_head.numKey;
		mtl.colorKeyAddr = OffsetEncrypt(dataBuffer.size());
		if (mtl.colorKeys == nullptr) {
			XMaterialAnimDataStruct animData;
			for (int i = 0; i < m_head.numKey; i++)
				dataBuffer.put<XMaterialAnimDataStruct>(animData);
		} else dataBuffer.putBytes(mtl.colorKeys, sizeof(XMaterialAnimDataStruct) * m_head.numKey);
	}

	headBuffer.setWritePos(m_head.amtl);
	for (XMaterial &mtl : m_materials)
		headBuffer.put<XMaterialStruct>(mtl);
}

void XScene::saveGeometries() {
	m_head.ageo = headBuffer.size();
	headBuffer.resize(m_head.ageo + sizeof(XGeometryStruct) * m_head.ngeo);
	headBuffer.setWritePos(headBuffer.size());

	m_head.nbgp = 0;
	m_boneGroups.clear();

	for (XGeometry &geo : m_geometries) {
		geo.nameAddr = headBuffer.size();
		headBuffer.putString(geo.name);
		geo.saveFlag = 0;

		geo.vertexAddr = OffsetEncrypt(dataBuffer.size());
		dataBuffer.putBytes(geo.vertexData, sizeof(QVector3D) * geo.numVertex);
		geo.vertexCompAddr = 0;

		if (geo.normalData != nullptr) {
			geo.normalAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(geo.normalData, sizeof(QVector3D) * geo.numVertex);
		} else geo.normalAddr = 0;
		geo.normalCompAddr = 0;

		if (geo.uvData[0] != nullptr) {
			geo.uvAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(geo.uvData[0], sizeof(QVector2D) * geo.numVertex);
		} else geo.uvAddr = 0;

		if (geo.uvData[1] != nullptr) {
			geo.uv1Addr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(geo.uvData[1], sizeof(QVector2D) * geo.numVertex);
			geo.saveFlag |= XFGEOCHUNKSAVE_ENABLE_UV2;
		} else geo.uv1Addr = 0;

		if (geo.vertColData != nullptr) {
			geo.vertColAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(geo.vertColData, sizeof(XCOLOR) * geo.numVertex);
		} else geo.vertColAddr = 0;

		if (geo.boneData != nullptr) {
			geo.boneGroupAddr = OffsetEncrypt(dataBuffer.size());
			for (int i = 0; i < geo.numVertex; i++) {
				int foundbgp = -1;
				for (int j = 0; j < m_head.nbgp; j++) {
					if (m_boneGroups[j].numBone != geo.boneData[i].numBone)
						continue;
					bool equal = true;
					for (int k = 0; k < m_boneGroups[j].numBone; k++) {
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
					newBgp.numBone = geo.boneData[i].numBone;
					for (int j = 0; j < newBgp.numBone; j++)
						newBgp.bones[j] = geo.boneData[i].bones[j];
					foundbgp = m_head.nbgp++;
					m_boneGroups.push_back(std::move(newBgp));
				}
				foundbgp = min(foundbgp, 255);
				dataBuffer.putByte(static_cast<BYTE>(foundbgp));
			}
		} else geo.boneGroupAddr = 0;

		geo.indicesAddr = OffsetEncrypt(dataBuffer.size());
		dataBuffer.putBytes(geo.indicesData, sizeof(ushort) * 3 * geo.numFace);

		if (geo.boneData != nullptr) {
			geo.bonePaletteAddr = OffsetEncrypt(dataBuffer.size());
			for (int i = 0; i < geo.numVertex; i++) {
				XBonePaletteStruct data;
				data.numBone = geo.boneData[i].numBone;
				for (int j = 0; j < data.numBone; j++) {
					data.bones[j] = geo.boneData[i].bones[j];
					data.weight[j] = geo.boneData[i].weight[j];
				}
				dataBuffer.put<XBonePaletteStruct>(data);
			}
			geo.saveFlag |= XFGEOCHUNKSAVE_ENABLE_BONE_PALETTE;
		} else geo.bonePaletteAddr = 0;

		if (!geo.script.isEmpty()) {
			geo.dataAddr = OffsetEncrypt(dataBuffer.size());
			writeScript(geo.dataAddr, geo.script);
		} else geo.dataAddr = 0;
	}

	headBuffer.setWritePos(m_head.ageo);
	for (XGeometry &geo : m_geometries)
		headBuffer.put<XGeometryStruct>(geo);
}

void XScene::saveBones() {
	m_head.abon = headBuffer.size();
	headBuffer.resize(m_head.abon + sizeof(XBoneStruct) * m_head.nbon);
	headBuffer.setWritePos(headBuffer.size());

	for (int i = 0; i < m_head.nbon; i++) {
		XBone &bone = m_bones[i];
		bone.nameAddr = headBuffer.size();
		headBuffer.putString(bone.name);
		bone.numKey = m_head.numKey;
		bone.saveFlag = 0;

		bone.numChild = 0;
		bone.childAddr = headBuffer.size();
		for (int j = 0; j < m_head.nbon; j++) {
			if (m_bones[j].parentId == i) {
				bone.numChild++;
				headBuffer.putInt(j);
			}
		}

		if (bone.visibleData != nullptr) {
			bone.numVisibleKey = m_head.numKey;
			bone.visibleKeyAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(bone.visibleData, sizeof(int) * m_head.numKey);
		} else bone.numVisibleKey = bone.visibleKeyAddr = 0;

		bone.posKeyCompAddr = 0;
		if (bone.posData != nullptr) {
			bone.numPosKey = m_head.numKey;
			bone.posKeyAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(bone.posData, sizeof(QVector3D) * m_head.numKey);
		} else bone.numPosKey = bone.posKeyAddr = 0;

		bone.rotKeyCompAddr = 0;
		if (bone.rotData != nullptr) {
			bone.numRotKey = m_head.numKey;
			bone.rotKeyAddr = OffsetEncrypt(dataBuffer.size());
			for (int i = 0; i < m_head.numKey; i++) {
				QQuaternion quat = bone.rotData[i].inverted();
				dataBuffer.putFloat(quat.x());
				dataBuffer.putFloat(quat.y());
				dataBuffer.putFloat(quat.z());
				dataBuffer.putFloat(quat.scalar());
			}
		} else bone.numRotKey = bone.rotKeyAddr = 0;

		if (bone.scaleData != nullptr) {
			bone.numScaleKey = m_head.numKey;
			bone.scaleKeyAddr = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(bone.scaleData, sizeof(QVector3D) * m_head.numKey);
		} else bone.numScaleKey = bone.scaleKeyAddr = 0;
	}

	headBuffer.setWritePos(m_head.abon);
	for (XBone &bone : m_bones)
		headBuffer.put<XBoneStruct>(bone);
}

void XScene::saveBoneGroups() {
	m_head.abgp = headBuffer.size();
	headBuffer.resize(m_head.abgp + sizeof(XBoneGroupStruct) * m_head.nbgp);
	headBuffer.setWritePos(headBuffer.size());
	for (XBoneGroup &bgp : m_boneGroups) {
		bgp.boneAddr = headBuffer.size();
		for (int i = 0; i < bgp.numBone; i++)
			headBuffer.putInt(bgp.bones[i]);
	}

	headBuffer.setWritePos(m_head.abgp);
	for (XBoneGroup &bgp : m_boneGroups)
		headBuffer.put<XBoneGroupStruct>(bgp);
}

void XScene::saveAttachments() {
	m_head.aatt = headBuffer.size();
	headBuffer.resize(m_head.aatt + sizeof(XAttachment) * m_head.natt);
	for (XAttachment &att : m_attachments)
		headBuffer.put<XAttachment>(att);
}

void XScene::saveRibbons() {
	m_head.arib = headBuffer.size();
	headBuffer.resize(m_head.arib + sizeof(XRibbonStruct) * m_head.nrib);
	m_head.rib6 = headBuffer.size();
	headBuffer.setWritePos(m_head.rib6);
	for (XRibbon &rib : m_ribbons) {
		headBuffer.put<XRibbonExStruct>(rib.ribEx);
	}

	for (XRibbon &rib : m_ribbons) {
		rib.nameAddr = headBuffer.size();
		headBuffer.putString(rib.name);
		rib.bindPartNameAddr = headBuffer.size();
		headBuffer.putString("");
	}

	headBuffer.setWritePos(m_head.arib);
	for (XRibbon &rib : m_ribbons)
		headBuffer.put<XRibbonStruct>(rib);
}

void XScene::saveParticles() {
	m_head.aprt = headBuffer.size();
	headBuffer.resize(m_head.aprt + (m_head.version >= 8 ? sizeof(XParticleStruct) : sizeof(XParticleStruct_6)) * m_head.nprt);
	headBuffer.setWritePos(headBuffer.size());
	for (XParticle &prt : m_particles) {
		if (!prt.script.isEmpty()) {
			prt.partEx.scriptAddr = OffsetEncrypt(dataBuffer.size());
			writeScript(prt.partEx.scriptAddr, prt.script);
		} else prt.partEx.scriptAddr = 0;

		QByteArray name = prt.name.toLocal8Bit();
		if (name.size() > 78) name = name.left(78);
		strcpy(prt.partName, name.constData());
		prt.dataAddr = headBuffer.size();
		headBuffer.put<XParticleExStruct>(prt.partEx);
	}

	headBuffer.setWritePos(m_head.aprt);
	for (XParticle &prt : m_particles)
		m_head.version >= 8 ? headBuffer.put<XParticleStruct>(prt) : headBuffer.put<XParticleStruct_6>(prt);
}

void XScene::saveActions() {
	m_head.aact = headBuffer.size();
	headBuffer.setWritePos(m_head.aact);
	for (XAction &act : m_actions) {
		QByteArray name = act.name.toLocal8Bit();
		if (name.size() > 78) name = name.left(78);
		strcpy(act.actionName, name.constData());
		headBuffer.put<XActionStruct>(act);
	}
}

void XScene::combineScene(XScene *oScene) {
	for (XTexture &tex : oScene->m_textures)
		m_textures.push_back(move(tex));
	for (XMaterial &mtl : oScene->m_materials) {
		if (mtl.textureId >= 0) mtl.textureId += m_head.ntex;
		if (mtl.exData.bumpTexId >= 0) mtl.exData.bumpTexId += m_head.ntex;
		if (mtl.exData.specTexId >= 0) mtl.exData.specTexId += m_head.ntex;
		if (mtl.exData.lightTexId >= 0) mtl.exData.lightTexId += m_head.ntex;
		if (mtl.cartoonData.shadowTextureID >= 0) mtl.cartoonData.shadowTextureID += m_head.ntex;
		if (mtl.cartoonData.specTextureID >= 0) mtl.cartoonData.specTextureID += m_head.ntex;
		if (mtl.dissolveData.dissolveTextureID >= 0) mtl.dissolveData.dissolveTextureID += m_head.ntex;
		m_materials.push_back(move(mtl));
	}
	for (XGeometry &geo : oScene->m_geometries) {
		geo.objectId += m_head.ngeo;
		if (geo.materialId >= 0) geo.materialId += m_head.nmtl;
		if (geo.ancestorBone >= 0) geo.ancestorBone += m_head.nbon;
		if (geo.boneData != nullptr)
			for (int i = 0; i < geo.numVertex; i++)
				for (int j = 0; j < geo.boneData[i].numBone; j++)
					geo.boneData[i].bones[j] += m_head.nbon;
		m_geometries.push_back(move(geo));
	}
	for (XBone &bon : oScene->m_bones) {
		if (bon.parentId >= 0) bon.parentId += m_head.nbon;
		m_bones.push_back(move(bon));
	}
	for (XAttachment &att : oScene->m_attachments) {
		if (att.boneID >= 0) att.boneID += m_head.nbon;
		m_attachments.push_back(move(att));
	}
	for (XRibbon &rib : oScene->m_ribbons) {
		if (rib.boneId >= 0) rib.boneId += m_head.nbon;
		if (rib.textureId >= 0) rib.textureId += m_head.ntex;
		m_ribbons.push_back(move(rib));
	}
	for (XParticle &prt : oScene->m_particles) {
		if (prt.boneId >= 0) prt.boneId += m_head.nbon;
		if (prt.textureId >= 0) prt.textureId += m_head.ntex;
		if (prt.partEx.bumpTexId >= 0) prt.partEx.bumpTexId += m_head.ntex;
		m_particles.push_back(move(prt));
	}
	for (XAction &act : oScene->m_actions)
		m_actions.push_back(move(act));

	m_head.ntex += oScene->m_head.ntex;
	m_head.nmtl += oScene->m_head.nmtl;
	m_head.ngeo += oScene->m_head.ngeo;
	m_head.nbon += oScene->m_head.nbon;
	m_head.natt += oScene->m_head.natt;
	m_head.nrib += oScene->m_head.nrib;
	m_head.nprt += oScene->m_head.nprt;
	m_head.nact += oScene->m_head.nact;
	m_head.version = max(m_head.version, oScene->m_head.version);
	m_head.numKey = max(m_head.numKey, oScene->m_head.numKey);
	if (m_desc.isEmpty()) m_desc = oScene->m_desc;
	if (m_head.nbon >= 255)
		warn("骨骼数量超过255上限，保存可能出错");

	for (XMaterial &mtl : m_materials) {
		if (mtl.numColorKey > 0 && mtl.numColorKey < m_head.numKey) {
			mtl.colorKeys = ReallocArray<XMaterialAnimDataStruct>(mtl.colorKeys, mtl.numColorKey, m_head.numKey);
			mtl.numColorKey = m_head.numKey;
		}
	}
	for (XBone &bon : m_bones) {
		if (bon.numVisibleKey > 0 && bon.numVisibleKey < m_head.numKey) {
			bon.visibleData = ReallocArray<uint>(bon.visibleData, bon.numVisibleKey, m_head.numKey);
			bon.numVisibleKey = m_head.numKey;
		}
		if (bon.numPosKey > 0 && bon.numPosKey < m_head.numKey) {
			bon.posData = ReallocArray<QVector3D>(bon.posData, bon.numPosKey, m_head.numKey);
			bon.numPosKey = m_head.numKey;
		}
		if (bon.numRotKey > 0 && bon.numRotKey < m_head.numKey) {
			bon.rotData = ReallocArray<QQuaternion>(bon.rotData, bon.numRotKey, m_head.numKey);
			bon.numRotKey = m_head.numKey;
		}
		if (bon.numScaleKey > 0 && bon.numScaleKey < m_head.numKey) {
			bon.scaleData = ReallocArray<QVector3D>(bon.scaleData, bon.numScaleKey, m_head.numKey);
			bon.numScaleKey = m_head.numKey;
		}
	}
}

void XScene::startProgress(QString name, int range) {
	if (m_showProgress && range > 0) {
		m_progressDlg = new QProgressDialog();
		m_progressDlg->setWindowTitle(name);
		m_progressDlg->setFixedWidth(300);
		m_progressDlg->setRange(0, range);
		m_progressDlg->setValue(0);
		m_progressDlg->show();
		m_progressRange = range;
		QCoreApplication::processEvents();
	}
}

bool XScene::updateProgress() {
	if (m_progressDlg) {
		m_progressDlg->setValue(m_progressDlg->value() + 1);
		QCoreApplication::processEvents();
		if (m_progressDlg->value() >= m_progressRange) {
			endProgress();
			return true;
		}
		return m_progressDlg->wasCanceled();
	}
	return true;
}

void XScene::endProgress() {
	if (m_progressDlg) {
		delete m_progressDlg;
		m_progressDlg = nullptr;
	}
}