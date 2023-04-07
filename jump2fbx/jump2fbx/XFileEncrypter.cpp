#include "XFileEncrypter.h"

void XFileEncrypter::Encrypt(FbxScene *scene) {
	fbxScene = scene;
	if (scene->GetSrcObjectCount<FbxAnimStack>() > 0) {
		m_FbxAction = scene->GetSrcObject<FbxAnimStack>();
		if (m_FbxAction->GetMemberCount<FbxAnimLayer>() > 0) {
			m_FbxAnimLayer = m_FbxAction->GetMember<FbxAnimLayer>();
		}
	}

	FbxTimeSpan timeSpan;
	scene->GetGlobalSettings().GetTimelineDefaultTimeSpan(timeSpan);
	frameCount = (DWORD)timeSpan.GetDuration().GetFrameCount();
	cerr << "Frame Count: " << frameCount << endl;
	m_bgpInfo = new BoneGroupInfo[256];

	EncryptModel();

	Write();

	cerr << endl;
}

void XFileEncrypter::EncryptModel() {
	indexBuffer.putBytes(g_XFileIndexHead, g_XFileIndexHeadSize);

	cerr << endl; EncryptTextures();
	cerr << endl; EncryptMaterials();

	m_XFileHead.bob2 = indexBuffer.size();
	Bob2Struct bob2;
	bob2.xmax = bob2.ymax = 10.0f;
	bob2.xmin = bob2.ymin = -10.0f;
	bob2.zmax = 110.0f; bob2.zmin = 0.0f;
	bob2.unknown = 30.0f;
	bob2.zero[0] = bob2.zero[1] = bob2.zero[2] = bob2.zero[3] = 0;
	indexBuffer.put<Bob2Struct>(bob2);

	cerr << endl; EncryptGeometries();
	cerr << endl; EncryptBones();
	cerr << endl; EncryptBoneGroups();
	cerr << endl; EncryptParticles();
	cerr << endl; EncryptActions();
}

void XFileEncrypter::EncryptTextures() {
	m_XFileHead.ntex = fbxScene->GetSrcObjectCount<FbxFileTexture>();
	m_XFileHead.atex = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.atex + m_XFileHead.ntex * sizeof(XTexture));
	indexBuffer.setWritePos(indexBuffer.size());
	m_FbxTextures = new FbxFileTexture*[m_XFileHead.ntex];
	m_XTextures = new XTexture*[m_XFileHead.ntex];

	for (uint32_t i = 0; i < m_XFileHead.ntex; i++) {
		m_FbxTextures[i] = fbxScene->GetSrcObject<FbxFileTexture>(i);
		string texName = m_FbxTextures[i]->GetFileName();

		int sep = max((int) texName.find_last_of('\\'), (int) texName.find_last_of('/'));
		if (sep >= 0) texName = texName.substr(sep + 1);
		cerr << "Texture File: " << texName << endl;

		m_XTextures[i] = new XTexture;
		m_XTextures[i]->zero = 0;
		m_XTextures[i]->nameOffset = indexBuffer.size();
		indexBuffer.putString(texName);
	}
}

void XFileEncrypter::EncryptMaterials() {
	m_XFileHead.nmtl = fbxScene->GetSrcObjectCount<FbxSurfacePhong>();
	m_XFileHead.amtl = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.amtl + m_XFileHead.nmtl * sizeof(XMaterial));
	indexBuffer.setWritePos(indexBuffer.size());
	m_FbxMaterials = new FbxSurfacePhong*[m_XFileHead.nmtl];
	m_XMaterials = new XMaterial*[m_XFileHead.nmtl];

	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++) {
		m_FbxMaterials[i] = fbxScene->GetSrcObject<FbxSurfacePhong>(i);

		m_XMaterials[i] = new XMaterial;
		m_XMaterials[i]->zero0 = m_XMaterials[i]->zero1 = 0;
		m_XMaterials[i]->flags0 = 0x18000;
		m_XMaterials[i]->flags1 = 1;
		m_XMaterials[i]->textureId = 0xFFFFFFFF;
		m_XMaterials[i]->one0 = 1;
		m_XMaterials[i]->unknownFloat = 1.0f;
		m_XMaterials[i]->uvTransform[0] = 0.0f;
		m_XMaterials[i]->uvTransform[1] = 0.0f;
		m_XMaterials[i]->animFrameCount = frameCount;

		m_XMaterials[i]->offset0 = OffsetEncrypt(modelBuffer.size());
		MaterialBaseInfo baseInfo;
		baseInfo.zeros0[0] = baseInfo.zeros0[1] = 0;
		baseInfo.ffs0[0] = baseInfo.ffs0[1] = 0xFFFFFFFF;
		baseInfo.one = 1.0f;
		baseInfo.ffs1 = 0xFFFFFFFF;
		baseInfo.zeros1[0] = baseInfo.zeros1[1] = baseInfo.zeros1[2] = baseInfo.zeros1[3] = baseInfo.zeros1[4] = 0;
		modelBuffer.put<MaterialBaseInfo>(baseInfo);

		if (m_FbxMaterials[i]->Diffuse.GetSrcObjectCount<FbxFileTexture>() > 0) {
			FbxFileTexture *tex = m_FbxMaterials[i]->Diffuse.GetSrcObject<FbxFileTexture>();
			for (uint32_t j = 0; j < m_XFileHead.ntex; j++) {
				if (m_FbxTextures[j] == tex) {
					m_XMaterials[i]->textureId = j;
					break;
				}
			}
		}

		m_XMaterials[i]->animDataOffset = OffsetEncrypt(modelBuffer.size());
		for (uint32_t j = 0; j < m_XMaterials[i]->animFrameCount; j++) {
			MaterialAnimInfo animInfo;
			animInfo.color[0] = animInfo.color[1] = animInfo.color[2] = 0x95;
			animInfo.color[3] = 0xFF;
			animInfo.flags = 0x100000;
			animInfo.uOffset = animInfo.vOffset = 0.0f;
			modelBuffer.put<MaterialAnimInfo>(animInfo);
		}
	}

	m_XFileHead.mtsi = indexBuffer.size();
	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++) {
		indexBuffer.putInt(OffsetEncrypt(modelBuffer.size()));
		for (uint32_t j = 0; j < m_XMaterials[i]->animFrameCount; j++)
			modelBuffer.putFloat(0);
	}
}

void XFileEncrypter::EncryptGeometries() {
	m_XFileHead.ngeo = fbxScene->GetSrcObjectCount<FbxMesh>();
	m_XFileHead.ageo = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.ageo + m_XFileHead.ngeo * sizeof(XGeometry));
	indexBuffer.setWritePos(indexBuffer.size());
	m_FbxGeometries = new FbxNode*[m_XFileHead.ngeo];
	m_XGeometries = new XGeometry*[m_XFileHead.ngeo];

	m_FbxBones = new FbxNode*[256];
	for (int i = 0; i < fbxScene->GetSrcObjectCount<FbxNode>(); i++) {
		FbxNode *fbxNode = fbxScene->GetSrcObject<FbxNode>(i);
		auto attr = fbxNode->GetNodeAttribute();
		if (attr && (attr->GetClassId() == FbxNull::ClassId || attr->GetClassId() == FbxSkeleton::ClassId)) {
			cerr << "Bone " << fbxNode->GetName() << endl;
			bool newOne = true;
			for (uint32_t j = 0; j < m_XFileHead.nbon; j++) {
				if (fbxNode == m_FbxBones[j]) {
					newOne = false;
					break;
				}
			}

			if (newOne) {
				if (m_XFileHead.nbon == 256)
					throw "Bone Count Limit Exceeded.";
				m_FbxBones[m_XFileHead.nbon++] = fbxNode;
			}
		}
	}

	for (uint32_t i = 0; i < m_XFileHead.ngeo; i++) {
		FbxMesh *fbxMesh = fbxScene->GetSrcObject<FbxMesh>(i);
		string meshName = fbxMesh->GetName();

		m_XGeometries[i] = new XGeometry;
		m_XGeometries[i]->headOffset = 0;
		m_XGeometries[i]->unknown0 = 0x40;
		m_XGeometries[i]->nameOffset = indexBuffer.size();
		m_XGeometries[i]->objectId = i;
		m_XGeometries[i]->flags0[0] = m_XGeometries[i]->flags0[1] = 0;
		m_XGeometries[i]->vertexCount = fbxMesh->GetControlPointsCount();
		m_XGeometries[i]->faceCount = fbxMesh->GetPolygonCount();

		/*
		if (m_OrinXFixer->m_XGeometries[i]->headOffset != 0) {
			m_XGeometries[i]->headOffset = OffsetEncrypt(modelBuffer.size());
			m_OrinXFixer->modelBuffer.setReadPos(OffsetDecrypt(m_OrinXFixer->m_XGeometries[i]->headOffset));
			DWORD length = m_OrinXFixer->modelBuffer.getInt();
			modelBuffer.putInt(length);
			for (uint32_t j = 0; j < length; j++) {
				BYTE data = m_OrinXFixer->modelBuffer.getByte();
				modelBuffer.putByte(data);
			}
		}
		*/

		for (int j = fbxScene->GetNodeCount() - 1; j >= 0; j--) {
			FbxNode *fbxNode = fbxScene->GetNode(j);
			FbxNodeAttribute *fbxNodeAttribute = fbxNode->GetNodeAttribute();
			if (fbxNodeAttribute == fbxMesh) {
				meshName = fbxNode->GetName();
				FbxSurfaceMaterial *fbxMaterial = fbxNode->GetMaterial(0);
				m_XGeometries[i]->materialId = 0xFFFFFFFF;
				for (uint32_t k = 0; k < m_XFileHead.nmtl; k++) {
					if (m_FbxMaterials[k] == fbxMaterial) {
						m_XGeometries[i]->materialId = k;
						break;
					}
				}
				break;
			}
		}

		indexBuffer.putString(meshName);
		cerr << "Object Name: " << meshName << " Vertexes Count: " << m_XGeometries[i]->vertexCount << " Triangles Count: " << m_XGeometries[i]->faceCount << endl;
		m_XGeometries[i]->vertexDataOffset = OffsetEncrypt(modelBuffer.size());
		for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
			FbxVector4 pos = fbxMesh->GetControlPointAt(j);
			modelBuffer.putFloat((float)pos[0]);
			modelBuffer.putFloat((float)pos[1]);
			modelBuffer.putFloat((float)pos[2]);
		}

		//FILE *fp = fopen((meshName + "_e.txt").c_str(), "w");
		m_XGeometries[i]->normalDataOffset = 0;
		if (fbxMesh->GetElementNormalCount() > 0) {
			m_XGeometries[i]->normalDataOffset = OffsetEncrypt(modelBuffer.size());
			FbxGeometryElementNormal *normalElement = fbxMesh->GetElementNormal();
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				FbxVector4 normal = normalElement->GetDirectArray().GetAt(j);
				modelBuffer.putFloat((float)normal[0]);
				modelBuffer.putFloat((float)normal[1]);
				modelBuffer.putFloat((float)normal[2]);
				//fprintf(fp, "%d %lf %lf %lf\n", j, normal[0], normal[1], normal[2]);
			}
		}
		//fclose(fp);

		m_XGeometries[i]->texcoordDataOffset = 0;
		if (fbxMesh->GetElementUVCount() > 0) {
			m_XGeometries[i]->texcoordDataOffset = OffsetEncrypt(modelBuffer.size());
			FbxGeometryElementUV *uvElement = fbxMesh->GetElementUV();
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				FbxVector2 uv = uvElement->GetDirectArray().GetAt(j);
				modelBuffer.putFloat((float)uv[0]);
				modelBuffer.putFloat(1 - (float)uv[1]);
			}
		}

		m_XGeometries[i]->vertexColorOffset = 0;
		if (fbxMesh->GetElementVertexColorCount() > 0) {
			m_XGeometries[i]->vertexColorOffset = OffsetEncrypt(modelBuffer.size());
			FbxGeometryElementVertexColor *colorElement = fbxMesh->GetElementVertexColor();
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				FbxColor color = colorElement->GetDirectArray().GetAt(j);
				modelBuffer.putByte((BYTE)(color[0] * 255.0f));
				modelBuffer.putByte((BYTE)(color[1] * 255.0f));
				modelBuffer.putByte((BYTE)(color[2] * 255.0f));
				modelBuffer.putByte((BYTE)(color[3] * 255.0f));
			}
		}

		m_XGeometries[i]->boneJointOffset = 0;
		m_XGeometries[i]->isInvisible = 1;
		m_XGeometries[i]->mainBoneId = 0;
		m_XGeometries[i]->box[0] = m_XGeometries[i]->box[1] = m_XGeometries[i]->box[2] = -10000000.0f;
		m_XGeometries[i]->box[3] = m_XGeometries[i]->box[4] = m_XGeometries[i]->box[5] = 10000000.0f;
		m_XGeometries[i]->unknown1 = 0x3FF;

		VertexBoneWeightInfo *boneInfo = new VertexBoneWeightInfo[m_XGeometries[i]->vertexCount];
		for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
			boneInfo[j].boneCount = 0;
			boneInfo[j].bones[0] = boneInfo[j].bones[1] = boneInfo[j].bones[2] = boneInfo[j].bones[3] = 0;
			boneInfo[j].unused[0] = boneInfo[j].unused[1] = boneInfo[j].unused[2] = 0xCC;
			boneInfo[j].weight[0] = boneInfo[j].weight[1] = boneInfo[j].weight[2] = boneInfo[j].weight[3] = 0.0f;
		}

		if (fbxMesh->GetDeformer(0) && fbxMesh->GetDeformer(0)->GetClassId() == FbxSkin::ClassId) {
			FbxSkin *fbxSkin = (FbxSkin*)fbxMesh->GetDeformer(0);
			for (int bone = 0; bone < fbxSkin->GetClusterCount(); bone++) {
				FbxCluster *fbxCluster = fbxSkin->GetCluster(bone);
				FbxNode *fbxNode = fbxCluster->GetLink();
				int boneIndex = -1;
				for (uint32_t j = 0; j < m_XFileHead.nbon; j++) {
					if (m_FbxBones[j] == fbxNode) {
						boneIndex = j;
						break;
					}
				}
				if (boneIndex >= 0) {
					for (int j = 0; j < fbxCluster->GetControlPointIndicesCount(); j++) {
						int vertex = fbxCluster->GetControlPointIndices()[j];
						int index = boneInfo[vertex].boneCount;
						if (index == 4)
							continue;// throw "Bone Weight Count Limit Exceeded.";
						boneInfo[vertex].bones[index] = boneIndex;
						boneInfo[vertex].weight[index] = (float) fbxCluster->GetControlPointWeights()[j];
						boneInfo[vertex].boneCount = index + 1;
					}
				}
			}

			m_XGeometries[i]->boneJointOffset = OffsetEncrypt(modelBuffer.size());
			for (uint32_t v = 0; v < m_XGeometries[i]->vertexCount; v++) {
				int bgp = -1;
				for (uint32_t g = 0; g < m_XFileHead.nbgp; g++) {
					bool equal = true;
					if (m_bgpInfo[g].boneCnt == boneInfo[v].boneCount) {
						for (uint32_t j = 0; j < m_bgpInfo[g].boneCnt; j++) {
							if (m_bgpInfo[g].bones[j] != boneInfo[v].bones[j]) {
								equal = false;
								break;
							}
						}
					}
					else equal = false;

					if (equal) {
						bgp = g;
						break;
					}
				}

				if (bgp == -1) {
					if (m_XFileHead.nbgp == 256)
						throw "Bone Group Limit Exceeded.";
					m_bgpInfo[m_XFileHead.nbgp].boneCnt = boneInfo[v].boneCount;
					for (uint32_t k = 0; k < boneInfo[v].boneCount; k++) {
						m_bgpInfo[m_XFileHead.nbgp].bones[k] = boneInfo[v].bones[k];
					}
					bgp = m_XFileHead.nbgp++;
				}

				modelBuffer.putByte((BYTE)bgp);
			}
		}
		else m_XGeometries[i]->boneJointOffset = 0;

		m_XGeometries[i]->indicesOffset = OffsetEncrypt(modelBuffer.size());
		for (uint32_t j = 0; j < m_XGeometries[i]->faceCount; j++) {
			if (fbxMesh->GetPolygonSize(j) != 3)
				throw "Non-Triangle Faces are not supported.";
			modelBuffer.putShort((WORD)fbxMesh->GetPolygonVertex(j, 0));
			modelBuffer.putShort((WORD)fbxMesh->GetPolygonVertex(j, 1));
			modelBuffer.putShort((WORD)fbxMesh->GetPolygonVertex(j, 2));
		}

		m_XGeometries[i]->bonesOffset = OffsetEncrypt(modelBuffer.size());
		modelBuffer.putBytes((BYTE*)boneInfo, sizeof(VertexBoneWeightInfo) * m_XGeometries[i]->vertexCount);
		delete[] boneInfo;
	}
}

void XFileEncrypter::EncryptBones() {
	m_XFileHead.abon = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.abon + m_XFileHead.nbon * sizeof(XBone));
	indexBuffer.setWritePos(indexBuffer.size());
	m_XBones = new XBone*[m_XFileHead.nbon];
	memset(m_XBones, 0, sizeof(m_XBones) * m_XFileHead.nbon);

	FbxTime fbxTime;
	FbxAMatrix* bindMatrix = new FbxAMatrix[m_XFileHead.nbon];
	FbxAMatrix** transformMatrix = new FbxAMatrix*[m_XFileHead.nbon];
	std::function<void(int)> EvaluateTransform = [&](int bone) {
		if (m_XBones[bone] == nullptr) {
			m_XBones[bone] = new XBone;
			string boneName = m_FbxBones[bone]->GetName();
			cerr << "Processing Bone " << boneName << endl;
			memset(m_XBones[bone], 0, sizeof(XBone));
			m_XBones[bone]->nameOffset = indexBuffer.size();
			indexBuffer.putString(boneName);
			m_XBones[bone]->frameCount = frameCount;
			m_XBones[bone]->unknown1 = 1;

			FbxNode *parent = m_FbxBones[bone]->GetParent();
			transformMatrix[bone] = new FbxAMatrix[frameCount];
			if (parent == fbxScene->GetRootNode()) {
				m_XBones[bone]->parentId = 0xFFFFFFFF;
				bindMatrix[bone] = m_FbxBones[bone]->EvaluateLocalTransform().Inverse();
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					fbxTime.SetFrame(frame);
					transformMatrix[bone][frame] = FbxAMatrix(
						m_FbxBones[bone]->LclTranslation.EvaluateValue(fbxTime),
						m_FbxBones[bone]->LclRotation.EvaluateValue(fbxTime),
						m_FbxBones[bone]->LclScaling.EvaluateValue(fbxTime));
				}
			}
			else {
				for (uint32_t j = 0; j < m_XFileHead.nbon; j++)
					if (m_FbxBones[j] == parent) {
						m_XBones[bone]->parentId = j;
						break;
					}
				EvaluateTransform(m_XBones[bone]->parentId);
				bindMatrix[bone] = m_FbxBones[bone]->EvaluateLocalTransform().Inverse() * bindMatrix[m_XBones[bone]->parentId];
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					fbxTime.SetFrame(frame);
					transformMatrix[bone][frame] = transformMatrix[m_XBones[bone]->parentId][frame] * FbxAMatrix(
							m_FbxBones[bone]->LclTranslation.EvaluateValue(fbxTime),
							m_FbxBones[bone]->LclRotation.EvaluateValue(fbxTime),
							m_FbxBones[bone]->LclScaling.EvaluateValue(fbxTime));
				}
			}

			m_XBones[bone]->box[0] = m_XBones[bone]->box[1] = m_XBones[bone]->box[2] = -10000000.0f;
			m_XBones[bone]->box[3] = m_XBones[bone]->box[4] = m_XBones[bone]->box[5] = 10000000.0f;
			m_XBones[bone]->unknown2 = 0x3FF;
			for (uint32_t i = 0; i < 4; i++)
				for (uint32_t j = 0; j < 4; j++)
					m_XBones[bone]->matrix[i * 4 + j] = (float)bindMatrix[bone][i][j];

			//if (m_FbxBones[bone]->LclTranslation.IsAnimated(m_FbxAnimLayer)) {
				m_XBones[bone]->posFrameCount = frameCount;
				m_XBones[bone]->posActionOffset = OffsetEncrypt(modelBuffer.size());
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					FbxVector4 pos = transformMatrix[bone][frame].GetT();
					modelBuffer.putFloat((float)pos[0]);
					modelBuffer.putFloat((float)pos[1]);
					modelBuffer.putFloat((float)pos[2]);
				}
			//}

			//if (m_FbxBones[bone]->LclRotation.IsAnimated(m_FbxAnimLayer)) {
				m_XBones[bone]->rotFrameCount = frameCount;
				m_XBones[bone]->rotActionOffset = OffsetEncrypt(modelBuffer.size());
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					FbxQuaternion quat = transformMatrix[bone][frame].GetQ();
					quat.Inverse();
					modelBuffer.putFloat((float)quat[0]);
					modelBuffer.putFloat((float)quat[1]);
					modelBuffer.putFloat((float)quat[2]);
					modelBuffer.putFloat((float)quat[3]);
				}
			//}

			if (m_FbxBones[bone]->LclScaling.IsAnimated(m_FbxAnimLayer)) {
				m_XBones[bone]->scaleFrameCount = frameCount;
				m_XBones[bone]->scaleActionOffset = OffsetEncrypt(modelBuffer.size());
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					FbxVector4 scale = transformMatrix[bone][frame].GetS();
					modelBuffer.putFloat((float)scale[0]);
					modelBuffer.putFloat((float)scale[1]);
					modelBuffer.putFloat((float)scale[2]);
				}
			}

			if (m_FbxBones[bone]->Visibility.IsAnimated(m_FbxAnimLayer)) {
				m_XBones[bone]->visibleFrameCount = frameCount;
				m_XBones[bone]->visibleOffset = OffsetEncrypt(modelBuffer.size());
				for (uint32_t frame = 0; frame < frameCount; frame++) {
					fbxTime.SetFrame(frame);
					modelBuffer.putInt(m_FbxBones[bone]->Visibility.EvaluateValue(fbxTime) > 0.5 ? 1 : 0);
				}
			}

			m_XBones[bone]->childCount = m_FbxBones[bone]->GetChildCount();
			if (m_XBones[bone]->childCount > 0) {
				m_XBones[bone]->childsOffset = indexBuffer.size();
				for (uint32_t j = 0; j < m_XBones[bone]->childCount; j++) {
					FbxNode *child = m_FbxBones[bone]->GetChild(j);
					for (uint32_t j = 0; j < m_XFileHead.nbon; j++) {
						if (m_FbxBones[j] == child) {
							indexBuffer.putInt(j);
							break;
						}
					}
				}
			}
		}
	};

	for (uint32_t i = 0; i < m_XFileHead.nbon; i++)
		EvaluateTransform(i);

	for (uint32_t i = 0; i < m_XFileHead.nbon; i++)
		delete[] transformMatrix[i];
	delete[] transformMatrix;
	delete[] bindMatrix;
}

void XFileEncrypter::EncryptBoneGroups() {
	m_XFileHead.abgp = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.abgp + m_XFileHead.nbgp * sizeof(XBoneGroup));
	indexBuffer.setWritePos(indexBuffer.size());

	m_XBoneGroups = new XBoneGroup*[m_XFileHead.nbgp];
	for (uint32_t i = 0; i < m_XFileHead.nbgp; i++) {
		m_XBoneGroups[i] = new XBoneGroup;
		m_XBoneGroups[i]->zero = 0;
		m_XBoneGroups[i]->boneCnt = m_bgpInfo[i].boneCnt;
		m_XBoneGroups[i]->boneOffset = indexBuffer.size();
		for (uint32_t j = 0; j < m_bgpInfo[i].boneCnt; j++)
			indexBuffer.putInt(m_bgpInfo[i].bones[j]);
	}
}

void XFileEncrypter::EncryptParticles() {
	m_XFileHead.nprt = m_XFileHead.aprt = 0;
	/*
	m_XFileHead.nprt = m_OrinXFixer->m_XFileHead.nprt;
	m_XFileHead.aprt = indexBuffer.size();
	indexBuffer.resize(m_XFileHead.aprt + m_XFileHead.nprt * sizeof(XParticle));
	indexBuffer.setWritePos(indexBuffer.size());

	m_XParticles = new XParticle*[m_XFileHead.nprt];
	for (uint32_t i = 0; i < m_XFileHead.nprt; i++) {
		m_XParticles[i] = new XParticle;
		memcpy(m_XParticles[i], m_OrinXFixer->m_XParticles[i], sizeof(XParticle));
		ParticleInfoStruct info;
		memset(&info, 0, sizeof(info));
		info.ffs = 0xFFFFFFFF;
		m_XParticles[i]->unknownOff = indexBuffer.size();
		indexBuffer.put<ParticleInfoStruct>(info);
	}
	*/
}

void XFileEncrypter::EncryptActions() {
	m_XFileHead.nact = m_XFileHead.aact = 0;
	/*
	m_XFileHead.nact = m_OrinXFixer->m_XFileHead.nact;
	m_XFileHead.aact = indexBuffer.size();

	m_XActions = new XAction*[m_XFileHead.nact];
	for (uint32_t i = 0; i < m_XFileHead.nact; i++) {
		m_XActions[i] = new XAction;
		memcpy(m_XActions[i], m_OrinXFixer->m_XActions[i], sizeof(XAction));
	}
	*/
}