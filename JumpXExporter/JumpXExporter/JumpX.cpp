#include "JumpX.h"

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

const Point3 g_orinScale = Point3(1, 1, 1);

IParamBlock2* GetParamBlock2(ReferenceMaker* obj) {
	int nRefs = obj->NumRefs();
	for (int i = 0; i < nRefs; ++i) {
		ReferenceTarget* ref = obj->GetReference(i);

		if (ref && ref->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID) {
			return dynamic_cast<IParamBlock2*>(ref);
		}
	}
	return NULL;
}

IParamBlock* GetParamBlock(ReferenceMaker* obj) {
	int nRefs = obj->NumRefs();
	for (int i = 0; i < nRefs; ++i) {
		ReferenceTarget* ref = obj->GetReference(i);

		if (ref && ref->SuperClassID() == PARAMETER_BLOCK_CLASS_ID) {
			return dynamic_cast<IParamBlock*>(ref);
		}
	}
	return NULL;
}

std::string WChar2Ansi(LPCWSTR pwszSrc) {
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);
	delete[] pszDst;

	return strTemp;
}

IGameSkin* GetSkinModifier(IGameObject *obj) {
	for (int i = 0; i < obj->GetNumModifiers(); i++) {
		IGameModifier *mod = obj->GetIGameModifier(i);
		if (mod->IsSkin()) return (IGameSkin*) mod;
	}
	return nullptr;
}

void XScene::saveToFile(const TCHAR* fileName, DWORD options) {
	ByteBuffer fileBuffer;
	fileBuffer.putBytes(g_XFileHead, g_XFileHeadSize);
	fileBuffer.putInt(8);
	fileBuffer.putInt(300);

	indexBuffer.clear(); dataBuffer.clear();
	indexBuffer.putBytes(g_XFileIndexHead, g_XFileIndexHeadSize);

	m_igame = GetIGameInterface();
	IGameConversionManager* cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::CoordSystem::IGAME_MAX);
	m_igame->InitialiseIGame(options & SCENE_EXPORT_SELECTED);
	m_igame->SetStaticFrame(0);
	m_head.frameCount = 1 + (m_igame->GetSceneEndTime() - m_igame->GetSceneStartTime()) / GetTicksPerFrame();

	for (int i = 0; i < m_igame->GetRootMaterialCount(); i++) {
		IGameMaterial *mtl = m_igame->GetRootMaterial(i);
		processMaterial(mtl);
	}

	for (int i = 0; i < m_igame->GetTopLevelNodeCount(); i++) {
		IGameNode *node = m_igame->GetTopLevelNode(i);
		IGameObject *obj = node->GetIGameObject();
		if (obj->GetIGameType() != IGameObject::IGAME_MESH) {
			processBone(node, false, false);
		}
	}

	for (int i = 0; i < m_head.nbon; i++)
		boneIndex[m_bones[i].node] = i;

	for (int i = 0; i < m_igame->GetTopLevelNodeCount(); i++) {
		IGameNode *node = m_igame->GetTopLevelNode(i);
		IGameObject *obj = node->GetIGameObject();
#ifdef DEBUG_OUTPUT
		fout << node->GetName() << ' ' << obj->GetIGameType() << endl;
#endif
		if (obj->GetIGameType() == IGameObject::IGAME_MESH) {
			processMesh(node);
		}
	}

	m_igame->ReleaseIGame();

	saveTextures();
	saveMaterials();

	m_head.mtsi = indexBuffer.size();
	for (int i = 0; i < m_head.nmtl; i++) {
		indexBuffer.putInt(OffsetEncrypt(dataBuffer.size()));
		for (int j = 0; j < m_head.frameCount; j++)
			dataBuffer.putFloat(0);
	}

	m_head.bob2 = indexBuffer.size();
	indexBuffer.put<XBob2Struct>(m_bob2);

	saveGeometries();
	saveBones();
	saveParticles();
	saveTraces();

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
	m_head.indexSizeComp = m_head.indexSize * 2;
	m_head.dataSizeComp = m_head.dataSize * 2;

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

	BYTE *fileBuf = new BYTE[fileBuffer.size()];
	fileBuffer.setReadPos(0);
	fileBuffer.getBytes(fileBuf, fileBuffer.size());

	fstream outFile(fileName, ios::out | ios::binary);
	outFile.write((char*) fileBuf, fileBuffer.size());

	delete[] fileBuf;

	MessageBox(nullptr, L"导出成功", L"JumpX", MB_OK);
}

void XScene::processMaterial(IGameMaterial *material) {
	XMaterial mtl;
	mtl.animFrameCount = m_head.frameCount;
	mtl.animData = new XMaterialAnimDataStruct[m_head.frameCount];
#ifdef DEBUG_OUTPUT
	fout << material->GetMaterialName() << endl;
#endif
	if (material->GetDiffuseData() != nullptr && material->GetOpacityData() != nullptr) {
		for (int i = 0; i < m_head.frameCount; i++) {
			Point3 color;
			int time = m_igame->GetSceneStartTime() + i * GetTicksPerFrame();
			material->GetDiffuseData()->GetPropertyValue(color, time);
			color *= 255.0f;
			mtl.animData[i].color[0] = (BYTE) color.x;
			mtl.animData[i].color[1] = (BYTE) color.y;
			mtl.animData[i].color[2] = (BYTE) color.z;
			float opacity;
			material->GetOpacityData()->GetPropertyValue(opacity, time);
			mtl.animData[i].color[3] = (BYTE) (opacity * 255.0f);
		}
	} else {
		memset(mtl.animData, 0xff, sizeof(XMaterialAnimDataStruct) * m_head.frameCount);
	}

	if (material->GetNumberOfTextureMaps() > 0) {
		IGameTextureMap *texture = material->GetIGameTextureMap(0);
		string texName = WChar2Ansi(texture->GetBitmapFileName());
		int sep = max((int) texName.find_last_of('\\'), (int) texName.find_last_of('/'));
		if (sep >= 0) texName = texName.substr(sep + 1);

		auto iter = texIndex.find(texName);
		if (iter == texIndex.end()) {
			XTexture tex;
			tex.textureName = texName;
			mtl.textureId = m_head.ntex++;
			m_textures.push_back(tex);
			texIndex[texName] = mtl.textureId;
		} else mtl.textureId = iter->second;

		IGameUVGen *uv = texture->GetIGameUVGen();
		for (int i = 0; i < m_head.frameCount; i++) {
			int time = m_igame->GetSceneStartTime() + i * GetTicksPerFrame();
			uv->GetUOffsetData()->GetPropertyValue(mtl.animData[i].uvOffset[0], time);
			uv->GetVOffsetData()->GetPropertyValue(mtl.animData[i].uvOffset[1], time);
		}
	}

	mtlIndex[material] = m_head.nmtl++;
	m_materials.push_back(move(mtl));
}

void XScene::processMesh(IGameNode *node) {
	IGameMesh *mesh = (IGameMesh*) node->GetIGameObject();
	IGameSkin *skin = GetSkinModifier(mesh);
#ifdef DEBUG_OUTPUT
	fout << node->GetName() << endl;
#endif
	if (skin != nullptr) {
#ifdef DEBUG_OUTPUT
		fout << node->GetName() << endl;
#endif
		if (mesh->InitializeData()) {
			XGeometry geo;
			geo.node = node;
			geo.meshName = WChar2Ansi(node->GetName());
			geo.faceCount = mesh->GetNumberOfFaces();
			geo.vertexData = new Point3[geo.vertexCount];
#ifdef DEBUG_OUTPUT
			fout << node->GetName() << endl;
			fout << mesh->GetNumberOfVerts() << ' ' << mesh->GetNumberOfFaces() << ' ' << mesh->GetNumberOfNormals() << ' '
				<< mesh->GetNumberOfTexVerts() << ' ' << mesh->GetNumberOfColorVerts() << ' ' << skin->GetNumOfSkinnedVerts() << endl;
#endif

			/*
			fout << skin->GetTotalBoneCount() << endl;
			for (int i = 0; i < skin->GetTotalBoneCount(); i++) {
				GMatrix mat;
				IGameNode *bone = skin->GetIGameBone(i, true);
				fout << bone->GetName() << endl;
				if (skin->GetInitBoneTM(bone, mat)) {
					mat = mat.Inverse();
					CopyMemory(m_bones[boneIndex[bone]].invertedMatrix, &mat, sizeof(GMatrix));
				}
			}
			*/

			XBoneDataStruct *boneData = new XBoneDataStruct[skin->GetNumOfSkinnedVerts()];
			for (int i = 0; i < skin->GetNumOfSkinnedVerts(); i++) {
				boneData[i].boneCount = skin->GetNumberOfBones(i);
				for (int j = 0; j < boneData[i].boneCount; j++) {
					boneData[i].bones[j] = boneIndex[skin->GetIGameBone(i, j)];
					boneData[i].weight[j] = skin->GetWeight(i, j);
				}
			}

			map<tuple<int, int, int, int>, int> vertexIndex;
			geo.indicesData = new int[mesh->GetNumberOfFaces() * 3];
			for (int i = 0; i < mesh->GetNumberOfFaces(); i++) {
				FaceEx *face = mesh->GetFace(i);
				for (int j = 0; j < 3; j++) {
					int vert = face->vert[j];
					int norm = face->norm[j];
					int texc = face->texCoord[j];
					int color = face->color[j];
					auto vertTuple = make_tuple(vert, norm, texc, color);
					auto iter = vertexIndex.find(vertTuple);
					int vertex = geo.vertexCount;
					if (iter == vertexIndex.end()) {
						vertexIndex[vertTuple] = vertex;
						geo.vertexCount++;
					} else vertex = iter->second;
					geo.indicesData[i * 3 + j] = vertex;
				}
			}

			geo.vertexData = new Point3[geo.vertexCount];
			geo.normalData = new Point3[geo.vertexCount];
			geo.texcoordData = new Point2[geo.vertexCount];
			geo.boneData = new XBoneDataStruct[geo.vertexCount];
			if (mesh->GetNumberOfColorVerts() > 0) geo.vertexColorData = new BMM_Color_32[geo.vertexCount];
			for (auto element : vertexIndex) {
				Point2 texcoord;
				mesh->GetVertex(get<0>(element.first), geo.vertexData[element.second]);
				mesh->GetNormal(get<1>(element.first), geo.normalData[element.second]);
				mesh->GetTexVertex(get<2>(element.first), texcoord);
				geo.texcoordData[element.second] = Point2(texcoord.x, 1 - texcoord.y);
				geo.boneData[element.second] = boneData[get<0>(element.first)];
				//geo.vertexData[element.second] = geo.vertexData[element.second] * node->GetWorldTM();
				if (geo.vertexColorData != nullptr) {
					Point3 colorP;
					mesh->GetColorVertex(get<3>(element.first), colorP);
					colorP *= 255.0f;
					geo.vertexColorData[element.second] = BMM_Color_32((BYTE) colorP.x, (BYTE) colorP.y, (BYTE) colorP.z, 255);
				}
			}

			delete[] boneData;
			geo.objectId = m_head.ngeo++;
			auto iter = mtlIndex.find(node->GetNodeMaterial());
			if (iter == mtlIndex.end()) geo.materialId = -1;
			else geo.materialId = iter->second;
			m_geometries.push_back(move(geo));
		}
	}
}

void XScene::processBone(IGameNode *node, bool visible, bool scale) {
	XBone bone;
	bone.node = node;
	bone.boneName = WChar2Ansi(node->GetName());
	bone.frameCount = m_head.frameCount;
	bone.matrix = node->GetWorldTM();
	bone.invMatrix = bone.matrix.Inverse();
	CopyMemory(bone.invertedMatrix, &bone.invMatrix, sizeof(GMatrix));
	IGameNode *parent = node->GetNodeParent();
	if (parent == nullptr) bone.parentId = -1;
	else for (int i = 0; i < m_head.nbon; i++)
		if (m_bones[i].node == parent) {
			bone.parentId = i;
			break;
		}

	INode *mnode = node->GetMaxNode();
#ifdef DEBUG_OUTPUT
	fout << node->GetName() << endl;
#endif
	bone.posFrameCount = bone.rotFrameCount = m_head.frameCount;
	bone.posData = new Point3[m_head.frameCount];
	bone.rotData = new Quat[m_head.frameCount];
	if (visible) bone.visibleData = new int[m_head.frameCount];
	if (scale) bone.scaleData = new Point3[m_head.frameCount];
	for (int frame = 0; frame < m_head.frameCount; frame++) {
		int time = m_igame->GetSceneStartTime() + frame * GetTicksPerFrame();

		GMatrix mat = node->GetWorldTM(time);
		bone.posData[frame] = mat.Translation();
		bone.rotData[frame] = mat.Rotation();
		
		int vis = mnode->GetVisibility(time) > 0.5f ? 1 : 0;
		if (visible) bone.visibleData[frame] = vis;
		else if (!vis) {
			visible = true;
			bone.visibleFrameCount = m_head.frameCount;
			bone.visibleData = new int[m_head.frameCount];
			ZeroMemory(bone.visibleData, sizeof(int) * frame);
			bone.visibleData[frame] = vis;
		}

		Point3 scl = mat.Scaling();
		if (scale) bone.scaleData[frame] = scl;
		else if (!scl.Equals(g_orinScale)) {
			scale = true;
			bone.scaleFrameCount = m_head.frameCount;
			bone.scaleData = new Point3[m_head.frameCount];
			for (int i = 0; i < frame; i++)
				bone.scaleData[i] = g_orinScale;
			bone.scaleData[frame] = scl;
		}
	}

#ifdef DEBUG_OUTPUT
	fout << visible << ' ' << scale << endl;
#endif

	m_bones.push_back(move(bone));
	boneIndex[node] = m_head.nbon++;

	for (int i = 0; i < node->GetChildCount(); i++) {
		IGameNode *child = node->GetNodeChild(i);
		IGameObject::ObjectTypes type = child->GetIGameObject()->GetIGameType();
#ifdef DEBUG_OUTPUT
		fout << child->GetName() << ' ' << type << endl;
#endif
		if (type == IGameObject::IGAME_LIGHT)
			processParticle(child);
		else if (type == IGameObject::IGAME_SPLINE)
			processTrace(child);
		else if (type != IGameObject::IGAME_MESH)
			processBone(child, visible, scale);
	}
}

void XScene::processParticle(IGameNode *part) {
	XParticle prt;
	prt.particleName = WChar2Ansi(part->GetName());
	prt.boneId = boneIndex[part->GetNodeParent()];
	GMatrix mat = part->GetWorldTM();
	Point4 pos = mat.GetRow(3);
	Point4 vec[3] = { mat.GetRow(2), mat.GetRow(0), mat.GetRow(1) };
	prt.offsetX = pos.x;
	prt.offsetY = pos.y;
	prt.offsetZ = pos.z;
	prt.direction[0] = Point3(vec[0].x, vec[0].y, vec[0].z);
	prt.direction[1] = Point3(vec[1].x, vec[1].y, vec[1].z);
	prt.direction[2] = Point3(vec[2].x, vec[2].y, vec[2].z);
	m_particles.push_back(prt);
	m_head.nprt++;
}

void XScene::processTrace(IGameNode *rib) {
	IGameSpline *spline = (IGameSpline*) rib->GetIGameObject();
	if (spline->InitializeData()) {
#ifdef DEBUG_OUTPUT
		fout << spline->GetNumberOfSplines() << endl;
#endif
		if (spline->GetNumberOfSplines() > 0) {
			IGameSpline3D *sp3d = spline->GetIGameSpline3D(0);
#ifdef DEBUG_OUTPUT
			fout << sp3d->GetIGameKnotCount() << endl;
#endif
			if (sp3d->GetIGameKnotCount() >= 2) {
				XTrace trace;
				trace.traceName = WChar2Ansi(rib->GetName());
				trace.boneId = boneIndex[rib->GetNodeParent()];
				GMatrix mat = rib->GetWorldTM();
				Point3 p0 = sp3d->GetIGameKnot(0)->GetKnotPoint() * mat;
				Point3 p1 = sp3d->GetIGameKnot(1)->GetKnotPoint() * mat;
				trace.startPos[0] = p0.x;
				trace.startPos[1] = p0.y;
				trace.startPos[2] = p0.z;
				trace.endPos[0] = p1.x;
				trace.endPos[1] = p1.y;
				trace.endPos[2] = p1.z;
				m_traces.push_back(trace);
				m_head.nrib++;
			}
		}
	}
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
		if (mtl.disableDepth) mtl.flags |= 0x4000;
		if (mtl.transparent) mtl.flags |= 0x8000;
		if (mtl.doubleFace) mtl.flags |= 0x10000;

		mtl.animFrameCount = m_head.frameCount;
		mtl.animDataOffset = OffsetEncrypt(dataBuffer.size());
		if (mtl.animData == nullptr) {
			XMaterialAnimDataStruct animData;
			for (int i = 0; i < m_head.frameCount; i++)
				dataBuffer.put<XMaterialAnimDataStruct>(animData);
		} else dataBuffer.putBytes(reinterpret_cast<BYTE*>(mtl.animData), sizeof(XMaterialAnimDataStruct) * m_head.frameCount);
	}

	indexBuffer.setWritePos(m_head.amtl);
	for (XMaterial &mtl : m_materials)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&mtl), sizeof(XMaterialStruct));
}

void XScene::saveGeometries() {
	m_head.ageo = indexBuffer.size();
	indexBuffer.resize(m_head.ageo + sizeof(XGeometryStruct) * m_head.ngeo);
	indexBuffer.setWritePos(indexBuffer.size());

	for (XGeometry &geo : m_geometries) {
		geo.scriptOffset = 0;
		geo.nameOffset = indexBuffer.size();
		indexBuffer.putString(geo.meshName);

		geo.vertexDataOffset = OffsetEncrypt(dataBuffer.size());
		dataBuffer.putBytes(reinterpret_cast<BYTE*>(geo.vertexData), sizeof(Point3) * geo.vertexCount);

		if (geo.normalData != nullptr) {
			geo.normalDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(geo.normalData), sizeof(Point3) * geo.vertexCount);
		} else geo.normalDataOffset = 0;

		if (geo.texcoordData != nullptr) {
			geo.texcoordDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(geo.texcoordData), sizeof(Point2) * geo.vertexCount);
		} else geo.texcoordDataOffset = 0;

		if (geo.vertexColorData != nullptr) {
			geo.vertexColorOffset = OffsetEncrypt(dataBuffer.size());
			for (int i = 0; i < geo.vertexCount; i++) {
				dataBuffer.putByte(geo.vertexColorData[i].r);
				dataBuffer.putByte(geo.vertexColorData[i].g);
				dataBuffer.putByte(geo.vertexColorData[i].b);
				dataBuffer.putByte(geo.vertexColorData[i].a);
			}
		} else geo.vertexColorOffset = 0;

		geo.boneGroupOffset = 0;
		geo.indicesOffset = OffsetEncrypt(dataBuffer.size());
		for (int i = 0; i < geo.faceCount * 3; i++)
			dataBuffer.putShort(static_cast<WORD>(geo.indicesData[i]));

		if (geo.boneData != nullptr) {
			geo.boneDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(geo.boneData), sizeof(XBoneDataStruct) * geo.vertexCount);
		}
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

		bone.childCount = 0;
		bone.childOffset = indexBuffer.size();
		for (int j = 0; j < m_head.nbon; j++) {
			if (m_bones[j].parentId == i) {
				bone.childCount++;
				indexBuffer.putInt(j);
			}
		}

		if (bone.visibleData != nullptr) {
			bone.visibleFrameCount = m_head.frameCount;
			bone.visibleDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(bone.visibleData), sizeof(int) * m_head.frameCount);
		} else bone.visibleFrameCount = bone.visibleDataOffset = 0;

		if (bone.posData != nullptr) {
			bone.posFrameCount = m_head.frameCount;
			bone.posDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(bone.posData), sizeof(Point3) * m_head.frameCount);
		} else bone.posFrameCount = bone.posDataOffset = 0;

		if (bone.rotData != nullptr) {
			bone.rotFrameCount = m_head.frameCount;
			bone.rotDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(bone.rotData), sizeof(Quat) * m_head.frameCount);
		} else bone.rotDataOffset = bone.rotFrameCount = 0;

		if (bone.scaleData != nullptr) {
			bone.scaleFrameCount = m_head.frameCount;
			bone.scaleDataOffset = OffsetEncrypt(dataBuffer.size());
			dataBuffer.putBytes(reinterpret_cast<BYTE*>(bone.scaleData), sizeof(Point3) * m_head.frameCount);
		} else bone.scaleDataOffset = bone.scaleFrameCount = 0;
	}

	indexBuffer.setWritePos(m_head.abon);
	for (XBone &bone : m_bones)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&bone), sizeof(XBoneStruct));
}

void XScene::saveParticles() {
	m_head.aprt = indexBuffer.size();
	indexBuffer.resize(m_head.aprt + sizeof(XParticleStruct) * m_head.nprt);
	indexBuffer.setWritePos(indexBuffer.size());
	for (XParticle &prt : m_particles) {
		if (prt.particleName.size() > 75) prt.particleName = prt.particleName.substr(0, 75);
		strcpy(prt.name, prt.particleName.c_str());
		prt.infoOffset = indexBuffer.size();
		indexBuffer.put<XParticleInfoStruct>(prt.partInfo);
	}

	indexBuffer.setWritePos(m_head.aprt);
	for (XParticle &prt : m_particles)
		indexBuffer.putBytes(reinterpret_cast<BYTE*>(&prt), sizeof(XParticleStruct));
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