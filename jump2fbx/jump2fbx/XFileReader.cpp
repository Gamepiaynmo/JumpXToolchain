#include "XFileReader.h"

void XFileReader::Read(BYTE *fileData, DWORD dataLength) {
	ByteBuffer *buffer = new ByteBuffer(fileData, dataLength);

	unique_ptr<BYTE> xFileHead(new BYTE[g_XFileHeadSize]);
	buffer->getBytes(xFileHead.get(), g_XFileHeadSize);
	if (memcmp(xFileHead.get(), g_XFileHead, g_XFileHeadSize))
		throw "Invalid File Head.";

	int xVersion = buffer->getInt();
	cerr << "X File Version: " << xVersion << endl;
	if (xVersion != 6) {
		Warn() << "Unsupported X File Version " << xVersion << endl;
	}

	int xHeadLen = buffer->getInt();
	if (xHeadLen % 12 != 0)
		throw "Corrupted X File Head Size.";

	for (; xHeadLen > 0; xHeadLen -= 12) {
		BYTE partName[4];
		buffer->getBytes(partName, 4);
		if (buffer->getInt() != 4)
			throw "Corrupted X File Head.";
		DWORD value = buffer->getInt();

		if (!memcmp(partName, "ntex", 4)) m_XFileHead.ntex = value;
		if (!memcmp(partName, "atex", 4)) m_XFileHead.atex = value;
		if (!memcmp(partName, "nmtl", 4)) m_XFileHead.nmtl = value;
		if (!memcmp(partName, "amtl", 4)) m_XFileHead.amtl = value;
		if (!memcmp(partName, "ngeo", 4)) m_XFileHead.ngeo = value;
		if (!memcmp(partName, "ageo", 4)) m_XFileHead.ageo = value;
		if (!memcmp(partName, "nbon", 4)) m_XFileHead.nbon = value;
		if (!memcmp(partName, "abon", 4)) m_XFileHead.abon = value;
		if (!memcmp(partName, "nbgp", 4)) m_XFileHead.nbgp = value;
		if (!memcmp(partName, "abgp", 4)) m_XFileHead.abgp = value;
		if (!memcmp(partName, "natt", 4)) m_XFileHead.natt = value;
		if (!memcmp(partName, "aatt", 4)) m_XFileHead.aatt = value;
		if (!memcmp(partName, "nrib", 4)) m_XFileHead.nrib = value;
		if (!memcmp(partName, "arib", 4)) m_XFileHead.arib = value;
		if (!memcmp(partName, "nprt", 4)) m_XFileHead.nprt = value;
		if (!memcmp(partName, "aprt", 4)) m_XFileHead.aprt = value;
		if (!memcmp(partName, "nact", 4)) m_XFileHead.nact = value;
		if (!memcmp(partName, "aact", 4)) m_XFileHead.aact = value;
		if (!memcmp(partName, "bobj", 4)) m_XFileHead.bobj = value;
		if (!memcmp(partName, "bob2", 4)) m_XFileHead.bob2 = value;
		if (!memcmp(partName, "acfg", 4)) m_XFileHead.acfg = value;
		if (!memcmp(partName, "cfgs", 4)) m_XFileHead.cfgs = value;
		if (!memcmp(partName, "desc", 4)) m_XFileHead.desc = value;
		if (!memcmp(partName, "rib6", 4)) m_XFileHead.rib6 = value;
		if (!memcmp(partName, "mtsi", 4)) m_XFileHead.mtsi = value;
	}

	m_XFileHead.indexSize = buffer->getInt();
	m_XFileHead.modelSize = buffer->getInt();
	m_XFileHead.indexSizeCompressed = buffer->getInt();
	m_XFileHead.modelSizeCompressed = buffer->getInt();

	DWORD indexSizeActual = m_XFileHead.indexSize, modelSizeActual = m_XFileHead.modelSize;
	BYTE *indexData = new BYTE[indexSizeActual], *modelData = new BYTE[modelSizeActual];
	BYTE *indexDataCompressed = new BYTE[m_XFileHead.indexSizeCompressed],
		*modelDataCompressed = new BYTE[m_XFileHead.modelSizeCompressed];
	buffer->getBytes(indexDataCompressed, m_XFileHead.indexSizeCompressed);
	buffer->getBytes(modelDataCompressed, m_XFileHead.modelSizeCompressed);
	uncompress(indexData, &indexSizeActual, indexDataCompressed, m_XFileHead.indexSizeCompressed);
	uncompress(modelData, &modelSizeActual, modelDataCompressed, m_XFileHead.modelSizeCompressed);

	if (indexSizeActual != m_XFileHead.indexSize) throw "Corrupted Index Data.";
	if (modelSizeActual != m_XFileHead.modelSize) throw "Corrupted Model Data.";

	cerr << "Index Data Size: " << m_XFileHead.indexSize << ", After Compression: " << m_XFileHead.indexSizeCompressed << endl;
	cerr << "Model Data Size: " << m_XFileHead.modelSize << ", After Compression: " << m_XFileHead.modelSizeCompressed << endl;

	indexBuffer.putBytes(indexData, indexSizeActual);
	modelBuffer.putBytes(modelData, modelSizeActual);

	ReadModel();

	delete[] indexData; delete[] modelData;
	delete[] indexDataCompressed; delete[] modelDataCompressed;
	delete buffer;
}

void XFileReader::ReadModel() {
	m_XTextures = new XTexture*[m_XFileHead.ntex];
	m_XMaterials = new XMaterial*[m_XFileHead.nmtl];
	m_XGeometries = new XGeometry*[m_XFileHead.ngeo];
	m_XBones = new XBone*[m_XFileHead.nbon];
	m_XBoneGroups = new XBoneGroup*[m_XFileHead.nbgp];
	m_XParticles = new XParticle*[m_XFileHead.nprt];
	m_XActions = new XAction*[m_XFileHead.nact];

	unique_ptr<BYTE> xFileIndexHead(new BYTE[g_XFileIndexHeadSize]);
	indexBuffer.getBytes(xFileIndexHead.get(), g_XFileIndexHeadSize);
	if (memcmp(xFileIndexHead.get(), g_XFileIndexHead, g_XFileIndexHeadSize))
		throw "Invalid X Index File Head.";

	ReadTextures();
	ReadMaterials();
	ReadGeometries();
	ReadBones();
	ReadBoneGroups();
	ReadParticles();
	ReadActions();
}

void XFileReader::ReadTextures() {
	cerr << "Textures Count: " << m_XFileHead.ntex << endl;
	indexBuffer.setReadPos(m_XFileHead.atex);
	for (uint32_t i = 0; i < m_XFileHead.ntex; i++)
		m_XTextures[i] = new XTexture(indexBuffer.get<XTexture>());
}

void XFileReader::ReadMaterials() {
	cerr << "Materials Count: " << m_XFileHead.nmtl << endl;
	indexBuffer.setReadPos(m_XFileHead.amtl);
	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++)
		m_XMaterials[i] = new XMaterial(indexBuffer.get<XMaterial>());
}

void XFileReader::ReadGeometries() {
	cerr << "Geometries Count: " << m_XFileHead.ngeo << endl;
	indexBuffer.setReadPos(m_XFileHead.ageo);
	for (uint32_t i = 0; i < m_XFileHead.ngeo; i++)
		m_XGeometries[i] = new XGeometry(indexBuffer.get<XGeometry>());
}

void XFileReader::ReadBones() {
	cerr << "Bones Count: " << m_XFileHead.nbon << endl;
	indexBuffer.setReadPos(m_XFileHead.abon);
	for (uint32_t i = 0; i < m_XFileHead.nbon; i++)
		m_XBones[i] = new XBone(indexBuffer.get<XBone>());
}

void XFileReader::ReadBoneGroups() {
	cerr << "Bone Groups Count: " << m_XFileHead.nbgp << endl;
	indexBuffer.setReadPos(m_XFileHead.abgp);
	for (uint32_t i = 0; i < m_XFileHead.nbgp; i++)
		m_XBoneGroups[i] = new XBoneGroup(indexBuffer.get<XBoneGroup>());
}

void XFileReader::ReadParticles() {
	cerr << "Particles Count: " << m_XFileHead.nprt << endl;
	indexBuffer.setReadPos(m_XFileHead.aprt);
	for (uint32_t i = 0; i < m_XFileHead.nprt; i++)
		m_XParticles[i] = new XParticle(indexBuffer.get<XParticle>());
}

void XFileReader::ReadActions() {
	cerr << "Actions Count: " << m_XFileHead.nact << endl;
	indexBuffer.setReadPos(m_XFileHead.aact);
	for (uint32_t i = 0; i < m_XFileHead.nact; i++)
		m_XActions[i] = new XAction(indexBuffer.get<XAction>());
}