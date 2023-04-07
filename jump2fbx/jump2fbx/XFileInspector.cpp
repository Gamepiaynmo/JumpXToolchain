#include "XFileInspector.h"
#include <set>

void XFileInspector::Inspect(BYTE *fileData, DWORD dataLength) {
	Read(fileData, dataLength);

	InspectModel();
}

void XFileInspector::InspectModel() {
	if (m_XFileHead.ntex != 0) InspectTextures();
	if (m_XFileHead.nmtl != 0) InspectMaterials();
	if (m_XFileHead.ngeo != 0) InspectGeometries();
	if (m_XFileHead.nbon != 0) InspectBones();
	if (m_XFileHead.nbgp != 0) InspectBoneGroups();
	if (m_XFileHead.natt != 0) Warn() << "natt " << m_XFileHead.natt << endl;
	if (m_XFileHead.nrib != 0) InspectTraces();
	if (m_XFileHead.nprt != 0) InspectParticles();
	if (m_XFileHead.nact != 0) InspectActions();

	if (m_XFileHead.bobj != 0) Warn() << "bobj != 0" << endl;
	if (m_XFileHead.bob2 != 0);
	if (m_XFileHead.acfg != 0) Warn() << "acfg != 0" << endl;;
	if (m_XFileHead.cfgs != 0) Warn() << "cfgs != 0" << endl;;
	if (m_XFileHead.desc != 0);
	if (m_XFileHead.rib6 != 0);
	if (m_XFileHead.mtsi != 0);
}

void XFileInspector::InspectTextures() {
	for (uint32_t i = 0; i < m_XFileHead.ntex; i++) {
		if (m_XTextures[i]->zero != 0) Warn() << "tex zero " << m_XTextures[i]->zero << endl;
	}
}

void XFileInspector::InspectMaterials() {
	for (uint32_t i = 0; i < m_XFileHead.nmtl; i++) {
		if (m_XMaterials[i]->zero0 != 0) Warn() << "mtl zero0 " << m_XMaterials[i]->zero0 << endl;
		if (m_XMaterials[i]->one0 != 1) Warn() << "mtl one0 " << m_XMaterials[i]->one0 << endl;
		if (m_XMaterials[i]->zero1 != 0) Warn() << "mtl zero1 " << m_XMaterials[i]->zero1 << endl;
		if (m_XMaterials[i]->animDataOffset != 0) {
			modelBuffer.setReadPos(OffsetDecrypt(m_XMaterials[i]->animDataOffset));
			MaterialAnimInfo info = modelBuffer.get<MaterialAnimInfo>();
			Warn() << i << " Anim Flag " << hex << info.flags << dec << endl;
		}
		/*
		if (m_XMaterials[i]->animDataOffset != 0) {
			modelBuffer.setReadPos(OffsetDecrypt(m_XMaterials[i]->animDataOffset));
			for (int j = 0; j < m_XMaterials[i]->animFrameCount; j++) {
				MaterialAnimInfo info = modelBuffer.get<MaterialAnimInfo>();
				static set<DWORD> flags;
				if (flags.find(info.flags) == flags.end()) {
					flags.insert(info.flags);
					for (DWORD flag : flags)
						Warn() << hex << flag << dec << endl;
				}
			}
		}
		*/
	}
}

void XFileInspector::InspectGeometries() {
	static set<string> scrps;
	for (uint32_t i = 0; i < m_XFileHead.ngeo; i++) {
		if (m_XGeometries[i]->unknown0 != 0x40) Warn() << "unknown0 != 0x40 " << hex << m_XGeometries[i]->unknown0 << dec << endl;
		if (m_XGeometries[i]->unknown1 != 0x3ff) Warn() << "unknown1 != 0x3FF " << hex << m_XGeometries[i]->unknown1 << dec << endl;
		//if (m_XGeometries[i]->flags0[0] != 0) Warn() << "flags0[0] != 0 " << hex << m_XGeometries[i]->flags0[0] << dec << endl;
		//if (m_XGeometries[i]->flags0[1] != 0) Warn() << "flags0[1] != 0 " << hex << m_XGeometries[i]->flags0[1] << dec << endl;
		/*
		if (m_XGeometries[i]->headOffset != 0) {
			indexBuffer.setReadPos(m_XGeometries[i]->nameOffset);
			Warn() << indexBuffer.getString() << endl;
			modelBuffer.setReadPos(OffsetDecrypt(m_XGeometries[i]->headOffset));
			int size = modelBuffer.getInt();
			while (size > 0) {
				BYTE title[4];
				modelBuffer.getBytes(title, 4);
				int len = modelBuffer.getInt();
				size -= 8;
				size -= len;
				if (memcmp(title, "scrp", 4) == 0) {
					len = modelBuffer.getInt();
					char str[1024];
					modelBuffer.getBytes((BYTE*)str, len);
					str[len] = 0;
					Warn() << str << endl;
				}
			}
		}
		*/
		
	}
}

void XFileInspector::InspectBones() {
	for (uint32_t i = 0; i < m_XFileHead.nbon; i++) {
		if (m_XBones[i]->zero0[0] != 0) Warn() << "zero0[0] " << m_XBones[i]->zero0[0] << endl;
		if (m_XBones[i]->zero0[1] != 0) Warn() << "zero0[1] " << m_XBones[i]->zero0[1] << endl;
		if (m_XBones[i]->unknown1 != 1) Warn() << "unknown1 != 1 " << m_XBones[i]->unknown1 << endl;
		if (m_XBones[i]->unknown2 != 0x3ff) Warn() << "unknown2 != 0x3FF " << m_XBones[i]->unknown2 << endl;
		if (m_XBones[i]->zero1[0] != 0) Warn() << "zero1[0] " << m_XBones[i]->zero1[0] << endl;
		if (m_XBones[i]->zero1[1] != 0) Warn() << "zero1[1] " << m_XBones[i]->zero1[1] << endl;
		if (m_XBones[i]->zero3 != 0) Warn() << "zero3 " << m_XBones[i]->zero3 << endl;
		if (m_XBones[i]->zero4 != 0) Warn() << "zero4 " << m_XBones[i]->zero4 << endl;

		/*
		if (m_XBones[i]->zero0[0] != 0) {
			indexBuffer.setReadPos(m_XBones[i]->nameOffset);
			Warn() << indexBuffer.getString() << endl;
			modelBuffer.setReadPos(OffsetDecrypt(m_XBones[i]->zero0[0]));
			int size = modelBuffer.getInt();
			while (size > 0) {
				BYTE title[4];
				modelBuffer.getBytes(title, 4);
				int len = modelBuffer.getInt();
				size -= 8;
				size -= len;
				if (memcmp(title, "scrp", 4) == 0) {
					len = modelBuffer.getInt();
					char str[1024];
					modelBuffer.getBytes((BYTE*) str, len);
					str[len] = 0;
					Warn() << str << endl;
				}
			}
		}
		*/
	}
}

void XFileInspector::InspectBoneGroups() {
	for (uint32_t i = 0; i < m_XFileHead.nbgp; i++) {
	}
}

void XFileInspector::InspectParticles() {
	static DWORD u0 = 0;
	for (uint32_t i = 0; i < m_XFileHead.nprt; i++) {
		XParticle &prt = *m_XParticles[i];
		u0 |= prt.flags1;

		/*
		if (prt.unknown16 != 0) Warn() << "unknown16 " << (int) prt.unknown16 << endl;
		if (prt.unknown24[0] != 0) Warn() << "unknown24[0] " << prt.unknown24[0] << endl;
		if (prt.unknown24[1] != 0) Warn() << "unknown24[1] " << prt.unknown24[1] << endl;
		if (prt.unknown24[2] != 0) Warn() << "unknown24[2] " << prt.unknown24[2] << endl;
		*/

		/*
		if (prt.unknown29 != 0 && prt.unknown26[0] != 0.0f && prt.unknown26[1] != 0.0f) {
			Warn() << "unknown29 " << (int) prt.unknown29 << endl;
			Warn() << "unknown26[0] " << prt.unknown26[0] << endl;
			Warn() << "unknown26[1] " << prt.unknown26[1] << endl;
		}
		*/

		/*
		if (prt.infoOffset != 0) {
			indexBuffer.setReadPos(prt.infoOffset);
			ParticleInfoStruct info = indexBuffer.get<ParticleInfoStruct>();
			if (info.zero0 != 0) Warn() << "zero0 " << info.zero0 << endl;
			if (info.ffs != 0xffffffff) Warn() << "ffs " << info.ffs << endl;
			if (info.zero1[0] != 0) Warn() << "zero1[0] " << hex << info.zero1[0] << endl;
			if (info.zero1[1] != 0) Warn() << "zero1[1] " << info.zero1[1] << endl;
			if (info.zero1[2] != 0) Warn() << "zero1[2] " << info.zero1[2] << endl;
			if (info.zero1[3] != 0) Warn() << "zero1[3] " << info.zero1[3] << endl;
			if (info.zero1[4] != 0) Warn() << "zero1[4] " << info.zero1[4] << endl;
			if (info.zero1[5] != 0) Warn() << "zero1[5] " << info.zero1[5] << endl;
			if (info.zero1[6] != 0) Warn() << "zero1[6] " << info.zero1[6] << endl;
			if (info.zero1[7] != 0) Warn() << "zero1[7] " << info.zero1[7] << dec << endl;
		}
		*/
		if (prt.infoOffset != 0) {
			indexBuffer.setReadPos(prt.infoOffset);
			ParticleInfoStruct info = indexBuffer.get<ParticleInfoStruct>();
			if (info.zero1[0] != 0) {
				static char tempScript[1024];
				modelBuffer.setReadPos(OffsetDecrypt(info.zero1[0]));
				modelBuffer.getInt(); modelBuffer.getInt(); modelBuffer.getInt();
				int len = modelBuffer.getInt();
				modelBuffer.getBytes(reinterpret_cast<BYTE*>(tempScript), len);
				tempScript[len] = 0;
				Warn() << "Part scrp " << tempScript << endl;
			}
		}
	}
}

void XFileInspector::InspectActions() {
	for (uint32_t i = 0; i < m_XFileHead.nact; i++) {
		if (m_XActions[i]->ffs != 0xffffffff)
			Warn() << m_XActions[i]->ffs;
	}
}

void XFileInspector::InspectTraces() {
	for (uint32_t i = 0; i < m_XFileHead.nrib; i++) {
		indexBuffer.setReadPos(m_XFileHead.arib + i * sizeof(XTrace));
		XTrace rib = indexBuffer.get<XTrace>();
		//Warn() << rib.unknown0 << ' ' << rib.unknown2 << ' ' << rib.unknown4 << ' ' << rib.unknown5 << endl;
	}
}