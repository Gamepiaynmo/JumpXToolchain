#include "XFileDecrypter.h"

void XFileDecrypter::Decrypt(BYTE *fileData, DWORD dataLength, FbxScene *scene) {
	Read(fileData, dataLength);
	fbxScene = scene;

	m_FbxTextures = new FbxFileTexture*[m_XFileHead.ntex];
	m_FbxMaterials = new FbxSurfacePhong*[m_XFileHead.nmtl];
	m_FbxGeometries = new FbxNode*[m_XFileHead.ngeo];
	m_FbxBones = new FbxNode*[m_XFileHead.nbon];

	m_FbxAction = FbxAnimStack::Create(fbxScene, "Animation");
	m_FbxAnimLayer = FbxAnimLayer::Create(fbxScene, "");
	m_FbxAction->AddMember(m_FbxAnimLayer);

	DecryptModel();

	cerr << endl;
}

void XFileDecrypter::DecryptModel() {
	cerr << endl << "Processing Textures ..." << endl; DecryptTextures();
	cerr << endl << "Processing Materials ..." << endl; DecryptMaterials();
	cerr << endl << "Processing Meshes ..." << endl; DecryptGeometries();
	cerr << endl << "Processing Bones ..." << endl; DecryptBones();
	cerr << endl << "Processing Bone Groups ..." << endl; DecryptBoneGroups();
	cerr << endl << "Processing Particles ..." << endl; DecryptParticles();
	cerr << endl << "Processing Animations ..." << endl; DecryptActions();
}

void XFileDecrypter::DecryptTextures() {
	for (uint32_t i = 0; i < m_XFileHead.ntex; i++) {
		indexBuffer.setReadPos(m_XTextures[i]->nameOffset);
		string textureFileName = indexBuffer.getString();
		cerr << "Texture File: " << textureFileName << endl;
		if (m_XTextures[i]->zero != 0) Warn() << "zero is not 0." << endl;

		m_FbxTextures[i] = FbxFileTexture::Create(fbxScene, textureFileName.c_str());
		m_FbxTextures[i]->SetFileName(textureFileName.c_str());
	}
}

void XFileDecrypter::DecryptMaterials() {
	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++) {
		if (m_XMaterials[i]->zero0 != 0) Warn() << "zero0 is not 0." << endl;
		if (m_XMaterials[i]->one0 != 1) Warn() << "one0 is not 1." << endl;
		if (m_XMaterials[i]->zero1 != 0) Warn() << "zero1 is not 0." << endl;

		m_FbxMaterials[i] = FbxSurfacePhong::Create(fbxScene, "");
		if (m_XMaterials[i]->textureId != -1)
			m_FbxMaterials[i]->Diffuse.ConnectSrcObject(m_FbxTextures[m_XMaterials[i]->textureId]);

		if (m_XMaterials[i]->offset0 != 0) {
			modelBuffer.setReadPos(OffsetDecrypt(m_XMaterials[i]->offset0));
			MaterialBaseInfo mbi = modelBuffer.get<MaterialBaseInfo>();
			if (mbi.zeros0[0] != 0) Warn() << "zeros0[0] " << mbi.zeros0[0] << endl;
			if (mbi.zeros0[1] != 0) Warn() << "zeros0[1] " << mbi.zeros0[1] << endl;
			if (mbi.ffs0[0] != 0xffffffff) Warn() << "ffs0[0] " << mbi.ffs0[0] << endl;
			if (mbi.ffs0[1] != 0xffffffff) Warn() << "ffs0[1] " << mbi.ffs0[1] << endl;
			if (mbi.one != 1) Warn() << "one " << mbi.one << endl;
			if (mbi.ffs1 != 0xffffffff) Warn() << mbi.ffs1 << "ffs1 " << endl;
			if (mbi.zeros1[0] != 0) Warn() << "zeros1[0] " << mbi.zeros1[0] << endl;
			if (mbi.zeros1[1] != 0) Warn() << "zeros1[1] " << mbi.zeros1[1] << endl;
			if (mbi.zeros1[2] != 0) Warn() << "zeros1[2] " << mbi.zeros1[2] << endl;
			if (mbi.zeros1[3] != 0) Warn() << "zeros1[3] " << mbi.zeros1[3] << endl;
			if (mbi.zeros1[4] != 0) Warn() << "zeros1[4] " << mbi.zeros1[4] << endl;
		}

		/*if (m_XMaterials[i]->animDataOffset != 0) {
			FbxTime fbxTime; int index = 0;
			FbxAnimCurve *curve = m_FbxMaterials[i]->dis
			modelBuffer.setReadPos(OffsetDecrypt(m_XMaterials[i]->animDataOffset));
			curve->KeyModifyBegin();
			for (uint32_t frame = 0; frame < m_XMaterials[i]->animFrameCount; frame++) {
				MaterialAnimInfo animInfo(modelBuffer.get<MaterialAnimInfo>());
				fbxTime.SetFrame(frame);
				index = curve->KeyAdd(fbxTime);
				curve->KeySetValue(index, animInfo.unknown);
			}
			curve->KeyModifyEnd();
		}*/
	}
}

void XFileDecrypter::DecryptGeometries() {
	for (uint32_t i = 0; i < m_XFileHead.ngeo; i++) {
		indexBuffer.setReadPos(m_XGeometries[i]->nameOffset);
		string geometryName = indexBuffer.getString();
		cerr << "Object Name: " << geometryName << " Vertexes Count: " << m_XGeometries[i]->vertexCount << " Triangles Count: " << m_XGeometries[i]->faceCount << endl;
		if (m_XGeometries[i]->unknown0 != 0x40) Warn() << "unknown0 is not 0x40." << endl;
		if (m_XGeometries[i]->unknown1 != 0x3ff) Warn() << "unknown1 is not 0x3ff." << endl;

		FbxMesh *fbxMesh = FbxMesh::Create(fbxScene, geometryName.c_str());
		if (m_XGeometries[i]->vertexDataOffset != 0) {
			fbxMesh->InitControlPoints(m_XGeometries[i]->vertexCount);
			FbxVector4 *vertexes = fbxMesh->GetControlPoints();
			modelBuffer.setReadPos((uint32_t)OffsetDecrypt(m_XGeometries[i]->vertexDataOffset));
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				float x = modelBuffer.getFloat();
				float y = modelBuffer.getFloat();
				float z = modelBuffer.getFloat();
				vertexes[j] = FbxVector4(x, y, z);
			}
		}

		//FILE *fp = fopen((geometryName + "_d.txt").c_str(), "w");
		if (m_XGeometries[i]->normalDataOffset != 0) {
			FbxGeometryElementNormal *normalElement = fbxMesh->CreateElementNormal();
			normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			normalElement->SetReferenceMode(FbxGeometryElement::eDirect);
			modelBuffer.setReadPos((uint32_t)OffsetDecrypt(m_XGeometries[i]->normalDataOffset));
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				float x = modelBuffer.getFloat();
				float y = modelBuffer.getFloat();
				float z = modelBuffer.getFloat();
				normalElement->GetDirectArray().Add(FbxVector4(x, y, z));
				//fprintf(fp, "%d %lf %lf %lf\n", j, x, y, z);
			}
		}
		//fclose(fp);

		if (m_XGeometries[i]->texcoordDataOffset != 0) {
			FbxGeometryElementUV *uvElement = fbxMesh->CreateElementUV("DiffuseUV");
			uvElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			uvElement->SetReferenceMode(FbxGeometryElement::eDirect);
			modelBuffer.setReadPos((uint32_t)OffsetDecrypt(m_XGeometries[i]->texcoordDataOffset));
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				float u = modelBuffer.getFloat();
				float v = modelBuffer.getFloat();
				uvElement->GetDirectArray().Add(FbxVector2(u, 1 - v));
			}
		}

		if (m_XGeometries[i]->vertexColorOffset != 0) {
			FbxGeometryElementVertexColor *vertexColorElement = fbxMesh->CreateElementVertexColor();
			vertexColorElement->SetMappingMode(FbxGeometryElement::eByControlPoint);
			vertexColorElement->SetReferenceMode(FbxGeometryElement::eDirect);
			modelBuffer.setReadPos((uint32_t)OffsetDecrypt(m_XGeometries[i]->vertexColorOffset));
			for (uint32_t j = 0; j < m_XGeometries[i]->vertexCount; j++) {
				float r = modelBuffer.getByte() / 255.0f;
				float g = modelBuffer.getByte() / 255.0f;
				float b = modelBuffer.getByte() / 255.0f;
				float a = modelBuffer.getByte() / 255.0f;
				vertexColorElement->GetDirectArray().Add(FbxColor(r, g, b, a));
			}
		}

		if (m_XGeometries[i]->indicesOffset != 0) {
			modelBuffer.setReadPos((uint32_t)OffsetDecrypt(m_XGeometries[i]->indicesOffset));
			for (uint32_t j = 0; j < m_XGeometries[i]->faceCount; j++) {
				fbxMesh->BeginPolygon(-1, -1, -1, false);
				for (int k = 0; k < 3; k++)
					fbxMesh->AddPolygon(modelBuffer.getShort());
				fbxMesh->EndPolygon();
			}
		}

		m_FbxGeometries[i] = FbxNode::Create(fbxScene, geometryName.c_str());
		m_FbxGeometries[i]->SetNodeAttribute(fbxMesh);
		m_FbxGeometries[i]->SetShadingMode(FbxNode::eTextureShading);
		if (m_XGeometries[i]->materialId != 0xFFFFFFFF) {
			FbxGeometryElementMaterial *materialElement = fbxMesh->CreateElementMaterial();
			materialElement->SetMappingMode(FbxGeometryElement::eAllSame);
			materialElement->GetIndexArray().SetAt(0, 0);
			m_FbxGeometries[i]->AddMaterial(m_FbxMaterials[m_XGeometries[i]->materialId]);
		}

		fbxScene->GetRootNode()->AddChild(m_FbxGeometries[i]);
	}
}

void XFileDecrypter::DecryptBones() {
	for (uint32_t i = 0; i < m_XFileHead.nbon; i++) {
		indexBuffer.setReadPos(m_XBones[i]->nameOffset);
		string boneName = indexBuffer.getString();
		cerr << "Bone Name: " << boneName << " Childs Count: " << m_XBones[i]->childCount << endl;
		if (m_XBones[i]->zero0[0] != 0) Warn() << "zero0[0] is not 0." << endl;
		if (m_XBones[i]->zero0[1] != 0) Warn() << "zero0[1] is not 0." << endl;
		if (m_XBones[i]->unknown1 != 1) Warn() << "unknown1 is not 1." << endl;
		if (m_XBones[i]->unknown2 != 0x3ff) Warn() << "unknown2 is not 0x3ff." << endl;
		if (m_XBones[i]->zero1[0] != 0) Warn() << "zero1[0] is not 0." << endl;
		if (m_XBones[i]->zero1[1] != 0) Warn() << "zero1[1] is not 0." << endl;
		if (m_XBones[i]->zero3 != 0) Warn() << "zero3 is not 0." << endl;
		if (m_XBones[i]->zero4 != 0) Warn() << "zero4 is not 0." << endl;

		FbxSkeleton *fbxSkeleton = FbxSkeleton::Create(fbxScene, boneName.c_str());
		m_FbxBones[i] = FbxNode::Create(fbxScene, boneName.c_str());
		m_FbxBones[i]->SetNodeAttribute(fbxSkeleton);
	}

	for (uint32_t i = 0; i < m_XFileHead.nbon; i++) {
		D3DXMATRIX curMatrix = D3DXMATRIX(m_XBones[i]->matrix);
		D3DXMatrixInverse(&curMatrix, nullptr, &curMatrix);
		if (m_XBones[i]->parentId != 0xFFFFFFFF) {
			D3DXMATRIX parentMatrix = D3DXMATRIX(m_XBones[m_XBones[i]->parentId]->matrix);
			D3DXMatrixMultiply(&curMatrix, &curMatrix, &parentMatrix);
			m_FbxBones[m_XBones[i]->parentId]->AddChild(m_FbxBones[i]);
		}

		FbxAMatrix fbxAMatrix;
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				fbxAMatrix[j][k] = curMatrix.m[j][k];

		m_FbxBones[i]->LclTranslation.Set(fbxAMatrix.GetT());
		m_FbxBones[i]->LclRotation.Set(fbxAMatrix.GetR());
		m_FbxBones[i]->LclScaling.Set(fbxAMatrix.GetS());
	}

	FbxCluster **fbxClusters = new FbxCluster*[m_XFileHead.nbon];
	for (uint32_t mesh = 0; mesh < m_XFileHead.ngeo; mesh++) {
		for (uint32_t i = 0; i < m_XFileHead.nbon; i++) {
			FbxCluster *fbxCluster = FbxCluster::Create(fbxScene, "");
			fbxCluster->SetLink(m_FbxBones[i]);
			fbxCluster->SetLinkMode(FbxCluster::eNormalize);
			fbxClusters[i] = fbxCluster;

			modelBuffer.setReadPos(OffsetDecrypt(m_XGeometries[mesh]->bonesOffset));
			for (uint32_t k = 0; k < m_XGeometries[mesh]->vertexCount; k++) {
				VertexBoneWeightInfo wInfo = modelBuffer.get<VertexBoneWeightInfo>();
				for (uint32_t l = 0; l < wInfo.boneCount; l++) {
					if (wInfo.bones[l] == i) {
						fbxCluster->AddControlPointIndex(k, wInfo.weight[l]);
					}
				}
			}
		}

		FbxSkin *fbxSkin = FbxSkin::Create(fbxScene, "");
		for (uint32_t i = 0; i < m_XFileHead.nbon; i++) {
			FbxAMatrix fbxAMatrix = m_FbxBones[i]->EvaluateGlobalTransform();
			fbxClusters[i]->SetTransformLinkMatrix(fbxAMatrix);
			fbxSkin->AddCluster(fbxClusters[i]);
			if (m_XBones[i]->parentId == 0xFFFFFFFF) {
				fbxScene->GetRootNode()->AddChild(m_FbxBones[i]);
			}
		}

		FbxMesh *fbxMesh = (FbxMesh*)m_FbxGeometries[mesh]->GetNodeAttribute();
		fbxMesh->AddDeformer(fbxSkin);
	}

	delete[] fbxClusters;
}

void XFileDecrypter::DecryptBoneGroups() {
	for (uint32_t i = 0; i < m_XFileHead.nbgp; i++) {
		cerr << "Bones Count: " << m_XBoneGroups[i]->boneCnt << ": ";
		for (uint32_t j = 0; j < m_XBoneGroups[i]->boneCnt; j++) {
			indexBuffer.setReadPos(m_XBoneGroups[i]->boneOffset + j * sizeof(DWORD));
			DWORD bone = indexBuffer.getInt();
			if (bone != 0xFFFFFFFF) {
				indexBuffer.setReadPos(m_XBones[bone]->nameOffset);
				cerr << indexBuffer.getString() << ' ';
			}
		}
		cerr << endl;
	}
}

void XFileDecrypter::DecryptParticles() {
	for (uint32_t i = 0; i < m_XFileHead.nprt; i++) {
		cerr << "Particle Name: " << m_XParticles[i]->name << endl;
	}
}

void XFileDecrypter::DecryptActions() {
	for (uint32_t i = 0; i < m_XFileHead.nact; i++) {
		cerr << "Action Name: " << m_XActions[i]->name << " Start Frame: " << m_XActions[i]->startFrame << " End Frame: " << m_XActions[i]->endFrame << endl;
	}

	FbxTime fbxTime; int index = 0;
	FbxTime timeStart, timeStop;
	timeStart.SetFrame(0);
	timeStop.SetFrame(m_XBones[0]->frameCount);
	fbxScene->GetGlobalSettings().SetTimelineDefaultTimeSpan(FbxTimeSpan(timeStart, timeStop));
	for (uint32_t bone = 0; bone < m_XFileHead.nbon; bone++) {
		FbxAnimCurve *posAnimCurve[3], *rotAnimCurve[3], *scaleAnimCurve[3], *visibleCurve;
		bool hasPos = false, hasRot = false, hasScale = false, hasVisible = false;
		const char components[][4] = { FBXSDK_CURVENODE_COMPONENT_X, FBXSDK_CURVENODE_COMPONENT_Y, FBXSDK_CURVENODE_COMPONENT_Z };

		if (m_XBones[bone]->posActionOffset != 0) {
			hasPos = true;
			for (int i = 0; i < 3; i++) {
				posAnimCurve[i] = m_FbxBones[bone]->LclTranslation.GetCurve(m_FbxAnimLayer, components[i], true);
				posAnimCurve[i]->KeyModifyBegin();
			}
		}

		if (m_XBones[bone]->rotActionOffset != 0) {
			hasRot = true;
			for (int i = 0; i < 3; i++) {
				rotAnimCurve[i] = m_FbxBones[bone]->LclRotation.GetCurve(m_FbxAnimLayer, components[i], true);
				rotAnimCurve[i]->KeyModifyBegin();
			}
		}

		if (m_XBones[bone]->scaleActionOffset != 0) {
			hasScale = true;
			for (int i = 0; i < 3; i++) {
				scaleAnimCurve[i] = m_FbxBones[bone]->LclScaling.GetCurve(m_FbxAnimLayer, components[i], true);
				scaleAnimCurve[i]->KeyModifyBegin();
			}
		}

		if (m_XBones[bone]->visibleOffset != 0) {
			hasVisible = true;
			visibleCurve = m_FbxBones[bone]->Visibility.GetCurve(m_FbxAnimLayer, true);
			visibleCurve->KeyModifyBegin();
		}

		for (uint32_t frame = 0; frame < m_XBones[bone]->frameCount; frame++) {
			fbxTime.SetFrame(frame);
			FbxVector4 posVector(0, 0, 0), scaleVector(1, 1, 1); FbxQuaternion rotQuat;
			FbxVector4 parentPosVector(0, 0, 0), parentScaleVector(1, 1, 1); FbxQuaternion parentRotQuat;

			if (hasPos) {
				modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR3) + OffsetDecrypt(m_XBones[bone]->posActionOffset));
				float x = modelBuffer.getFloat();
				float y = modelBuffer.getFloat();
				float z = modelBuffer.getFloat();
				posVector = FbxVector4(x, y, z);
			}

			if (hasRot) {
				modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR4) + OffsetDecrypt(m_XBones[bone]->rotActionOffset));
				float x = modelBuffer.getFloat();
				float y = modelBuffer.getFloat();
				float z = modelBuffer.getFloat();
				float w = modelBuffer.getFloat();
				rotQuat = FbxQuaternion(x, y, z, w);
				rotQuat.Inverse();
			}

			if (hasScale) {
				modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR3) + OffsetDecrypt(m_XBones[bone]->scaleActionOffset));
				float x = modelBuffer.getFloat();
				float y = modelBuffer.getFloat();
				float z = modelBuffer.getFloat();
				scaleVector = FbxVector4(x, y, z);
			}

			if (hasVisible) {
				modelBuffer.setReadPos(frame * sizeof(DWORD) + OffsetDecrypt(m_XBones[bone]->visibleOffset));
				index = visibleCurve->KeyAdd(fbxTime);
				visibleCurve->KeySetValue(index, modelBuffer.getInt() == 0 ? 0.0f : 1.0f);
			}

			FbxAMatrix transMatrix(posVector, rotQuat, scaleVector);
			if (m_XBones[bone]->parentId != 0xFFFFFFFF) {
				int parent = m_XBones[bone]->parentId;
				if (m_XBones[parent]->posActionOffset != 0) {
					modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR3) + OffsetDecrypt(m_XBones[parent]->posActionOffset));
					float x = modelBuffer.getFloat();
					float y = modelBuffer.getFloat();
					float z = modelBuffer.getFloat();
					parentPosVector = FbxVector4(x, y, z);
				}

				if (m_XBones[parent]->rotActionOffset != 0) {
					modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR4) + OffsetDecrypt(m_XBones[parent]->rotActionOffset));
					float x = modelBuffer.getFloat();
					float y = modelBuffer.getFloat();
					float z = modelBuffer.getFloat();
					float w = modelBuffer.getFloat();
					parentRotQuat = FbxQuaternion(x, y, z, w);
					parentRotQuat.Inverse();
				}

				if (m_XBones[parent]->scaleActionOffset != 0) {
					modelBuffer.setReadPos(frame * sizeof(D3DXVECTOR3) + OffsetDecrypt(m_XBones[parent]->scaleActionOffset));
					float x = modelBuffer.getFloat();
					float y = modelBuffer.getFloat();
					float z = modelBuffer.getFloat();
					parentScaleVector = FbxVector4(x, y, z);
				}

				FbxAMatrix parentTransMatrix(parentPosVector, parentRotQuat, parentScaleVector);
				transMatrix = parentTransMatrix.Inverse() * transMatrix;
			}

			FbxVector4 lclPosVector = transMatrix.GetT();
			FbxVector4 lclRotVector = transMatrix.GetR();
			FbxVector4 lclScaleVector = transMatrix.GetS();

			if (hasPos) {
				for (int i = 0; i < 3; i++) {
					index = posAnimCurve[i]->KeyAdd(fbxTime);
					posAnimCurve[i]->KeySetValue(index, (float)lclPosVector[i]);
				}
			}

			if (hasRot) {
				for (int i = 0; i < 3; i++) {
					index = rotAnimCurve[i]->KeyAdd(fbxTime);
					rotAnimCurve[i]->KeySetValue(index, (float)lclRotVector[i]);
				}
			}

			if (hasScale) {
				for (int i = 0; i < 3; i++) {
					index = scaleAnimCurve[i]->KeyAdd(fbxTime);
					scaleAnimCurve[i]->KeySetValue(index, (float)lclScaleVector[i]);
				}
			}
		}

		if (hasPos)
			for (int i = 0; i < 3; i++)
				posAnimCurve[i]->KeyModifyEnd();
		if (hasRot)
			for (int i = 0; i < 3; i++)
				rotAnimCurve[i]->KeyModifyEnd();
		if (hasScale)
			for (int i = 0; i < 3; i++)
				scaleAnimCurve[i]->KeyModifyEnd();
		if (hasVisible)
			visibleCurve->KeyModifyEnd();
	}
}