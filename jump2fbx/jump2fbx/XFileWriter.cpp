#include "XFileWriter.h"

void XFileWriter::Write() {
	ByteBuffer *buffer = new ByteBuffer();
	buffer->putBytes(g_XFileHead, g_XFileHeadSize);
	buffer->putInt(6);
	buffer->putInt(300);

	WriteModel();

	buffer->putChars("ntex", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.ntex);
	buffer->putChars("atex", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.atex);
	buffer->putChars("nmtl", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nmtl);
	buffer->putChars("amtl", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.amtl);
	buffer->putChars("ngeo", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.ngeo);
	buffer->putChars("ageo", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.ageo);
	buffer->putChars("nbon", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nbon);
	buffer->putChars("abon", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.abon);
	buffer->putChars("nbgp", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nbgp);
	buffer->putChars("abgp", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.abgp);
	buffer->putChars("natt", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.natt);
	buffer->putChars("aatt", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.aatt);
	buffer->putChars("nrib", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nrib);
	buffer->putChars("arib", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.arib);
	buffer->putChars("nprt", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nprt);
	buffer->putChars("aprt", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.aprt);
	buffer->putChars("nact", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.nact);
	buffer->putChars("aact", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.aact);
	buffer->putChars("bobj", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.bobj);
	buffer->putChars("bob2", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.bob2);
	buffer->putChars("acfg", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.acfg);
	buffer->putChars("cfgs", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.cfgs);
	buffer->putChars("desc", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.desc);
	buffer->putChars("rib6", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.rib6);
	buffer->putChars("mtsi", 4); buffer->putInt(4); buffer->putInt(m_XFileHead.mtsi);

	DWORD indexSizeActual = indexBuffer.size(), modelSizeActual = modelBuffer.size();
	BYTE *indexData = new BYTE[indexSizeActual], *modelData = new BYTE[modelSizeActual];
	indexBuffer.setReadPos(0); indexBuffer.getBytes(indexData, indexBuffer.size());
	modelBuffer.setReadPos(0); modelBuffer.getBytes(modelData, modelBuffer.size());

	DWORD indexSizeCompressed = indexSizeActual * 2, modelSizeCompressed = modelSizeActual * 2;
	BYTE *indexDataCompressed = new BYTE[indexSizeCompressed],
		*modelDataCompressed = new BYTE[modelSizeCompressed];
	compress(indexDataCompressed, &indexSizeCompressed, indexData, indexSizeActual);
	compress(modelDataCompressed, &modelSizeCompressed, modelData, modelSizeActual);

	buffer->putInt(indexSizeActual);
	buffer->putInt(modelSizeActual);
	buffer->putInt(indexSizeCompressed);
	buffer->putInt(modelSizeCompressed);

	buffer->putBytes(indexDataCompressed, indexSizeCompressed);
	buffer->putBytes(modelDataCompressed, modelSizeCompressed);

	dataLength = buffer->size();
	fileData = new BYTE[dataLength];
	buffer->setReadPos(0); buffer->getBytes(fileData, dataLength);

	delete[] indexData; delete[] modelData;
	delete[] indexDataCompressed; delete[] modelDataCompressed;
	delete buffer;
}

void XFileWriter::WriteModel() {
	WriteTextures();
	WriteMaterials();
	WriteGeometries();
	WriteBones();
	WriteBoneGroups();
	WriteParticles();
	WriteActions();
}

void XFileWriter::WriteTextures() {
	cerr << "Textures Count: " << m_XFileHead.ntex << endl;
	indexBuffer.setWritePos(m_XFileHead.atex);
	for (uint32_t i = 0; i < m_XFileHead.ntex; i++)
		indexBuffer.put<XTexture>(*m_XTextures[i]);
}

void XFileWriter::WriteMaterials() {
	cerr << "Materials Count: " << m_XFileHead.nmtl << endl;
	indexBuffer.setWritePos(m_XFileHead.amtl);
	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++)
		indexBuffer.put<XMaterial>(*m_XMaterials[i]);
}

void XFileWriter::WriteGeometries() {
	cerr << "Geometries Count: " << m_XFileHead.ngeo << endl;
	indexBuffer.setWritePos(m_XFileHead.ageo);
	for (uint32_t i = 0; i < m_XFileHead.ngeo; i++)
		indexBuffer.put<XGeometry>(*m_XGeometries[i]);
}

void XFileWriter::WriteBones() {
	cerr << "Bones Count: " << m_XFileHead.nbon << endl;
	indexBuffer.setWritePos(m_XFileHead.abon);
	for (uint32_t i = 0; i < m_XFileHead.nbon; i++)
		indexBuffer.put<XBone>(*m_XBones[i]);
}

void XFileWriter::WriteBoneGroups() {
	cerr << "Bone Groups Count: " << m_XFileHead.nbgp << endl;
	indexBuffer.setWritePos(m_XFileHead.abgp);
	for (uint32_t i = 0; i < m_XFileHead.nbgp; i++)
		indexBuffer.put<XBoneGroup>(*m_XBoneGroups[i]);
}

void XFileWriter::WriteParticles() {
	cerr << "Particles Count: " << m_XFileHead.nprt << endl;
	indexBuffer.setWritePos(m_XFileHead.aprt);
	for (uint32_t i = 0; i < m_XFileHead.nprt; i++)
		indexBuffer.put<XParticle>(*m_XParticles[i]);
}

void XFileWriter::WriteActions() {
	cerr << "Actions Count: " << m_XFileHead.nact << endl;
	indexBuffer.setWritePos(m_XFileHead.aact);
	for (uint32_t i = 0; i < m_XFileHead.nact; i++)
		indexBuffer.put<XAction>(*m_XActions[i]);
}