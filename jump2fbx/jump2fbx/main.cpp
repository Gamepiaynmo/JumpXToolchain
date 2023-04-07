#include "XFileDecrypter.h"
#include "XFileEncrypter.h"
#include "XFilePackager.h"
#include "XFileInspector.h"

#include <fstream>

BYTE *ReadFileAsByteArray(const char *fileName, DWORD *size) {
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	*size = GetFileSize(hFile, NULL);
	BYTE *data = new BYTE[*size];
	ReadFile(hFile, data, *size, size, NULL);
	CloseHandle(hFile);
	return data;
}

void WriteFileAsByteArray(const char *fileName, BYTE *dataBuffer, DWORD size) {
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, dataBuffer, size, nullptr, nullptr);
	CloseHandle(hFile);
}

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename) {
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	FbxExporter* lExporter = FbxExporter::Create(pManager, "");
	int pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_MATERIAL, true);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_TEXTURE, true);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_EMBEDDED, false);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_SHAPE, true);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_GOBO, true);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_ANIMATION, true);
	pManager->GetIOSettings()->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	if (lExporter->Initialize(pFilename, pFileFormat, pManager->GetIOSettings()) == false) {
		cerr << "Export Failed: " << lExporter->GetStatus().GetErrorString() << endl;
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	cerr << "FBX file format version " << lMajor << '.' << lMinor << '.' << lRevision << endl;
	lStatus = lExporter->Export(pScene);
	if (!lStatus)
		cerr << "Export Failed: " << lExporter->GetStatus().GetErrorString() << endl;
	lExporter->Destroy();
	return lStatus;
}

bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFileName) {
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	FbxImporter* lImporter = FbxImporter::Create(pManager, "");
	int pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
	pManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	if (lImporter->Initialize(pFileName, pFileFormat, pManager->GetIOSettings()) == false) {
		cerr << "Import Failed: " << lImporter->GetStatus().GetErrorString() << endl;
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	cerr << "FBX file format version " << lMajor << '.' << lMinor << '.' << lRevision << endl;
	lStatus = lImporter->Import(pScene);
	if (!lStatus)
		cerr << "Import Failed: " << lImporter->GetStatus().GetErrorString() << endl;
	lImporter->Destroy();
	return lStatus;
}

void ConvertToFbx(const char *xFile, const char *fbxFile) {
	try {
		FbxManager *fbxManager = FbxManager::Create();
		FbxScene *fbxScene = FbxScene::Create(fbxManager, "");
		FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ios);

		FbxGlobalSettings& gs = fbxScene->GetGlobalSettings();
		gs.SetAxisSystem(FbxAxisSystem::Max);

		DWORD xSize = 0;
		auto xData = unique_ptr<BYTE>(ReadFileAsByteArray(xFile, &xSize));

		auto xDecrypter = unique_ptr<XFileDecrypter>(new XFileDecrypter());
		xDecrypter->Decrypt(xData.get(), xSize, fbxScene);

		SaveScene(fbxManager, fbxScene, fbxFile);

		fbxManager->Destroy();
	} catch (char* err) {
		cout << "Fatal Error: " << err << endl;
		cout << "Aborted !" << endl;
	}
}

void ConvertToX(const char *fbxFile, const char *xFile) {
	try {
		FbxManager *fbxManager = FbxManager::Create();
		FbxScene *fbxScene = FbxScene::Create(fbxManager, "");
		FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(ios);

		FbxGlobalSettings& gs = fbxScene->GetGlobalSettings();
		gs.SetAxisSystem(FbxAxisSystem::Max);

		DWORD xSize = 0;

		LoadScene(fbxManager, fbxScene, fbxFile);

		auto xEncrypter = unique_ptr<XFileEncrypter>(new XFileEncrypter());
		xEncrypter->Encrypt(fbxScene);
		WriteFileAsByteArray(xFile, xEncrypter->GetDataBuffer(), xEncrypter->GetDataLength());

		fbxManager->Destroy();
	}
	catch (char* err) {
		cerr << "Fatal Error: " << err << endl;
		cerr << "Aborted !" << endl;
	}
}

void PackageX(const char *inXFile, const char *indexFile, const char *modelFile, const char *outXFile) {
	DWORD inXSize = 0, indexSize = 0, modelSize = 0;
	auto inXData = unique_ptr<BYTE>(ReadFileAsByteArray(inXFile, &inXSize));
	auto indexData = unique_ptr<BYTE>(ReadFileAsByteArray(indexFile, &indexSize));
	auto modelData = unique_ptr<BYTE>(ReadFileAsByteArray(modelFile, &modelSize));
	auto xPackager = unique_ptr<XFilePackager>(new XFilePackager());

	xPackager->Package(inXData.get(), inXSize, indexData.get(), indexSize, modelData.get(), modelSize);

	WriteFileAsByteArray(outXFile, xPackager->GetDataBuffer(), xPackager->GetDataLength());
}

bool EvaluateX(const char *xFile) {
	DWORD xSize = 0;
	auto xData = unique_ptr<BYTE>(ReadFileAsByteArray(xFile, &xSize));
	auto xInspector = unique_ptr<XFileInspector>(new XFileInspector());

	try {
		xInspector->Inspect(xData.get(), xSize);
		if (xInspector->hasWarn()) {
			cout << xFile << endl << endl;
		}
	}
	catch (char* err) {
		//cout << "Error: " << err << endl;
		//cout << xFile << endl << endl;
		return false;
	}

	return true;
}

void IterateAllXFiles(bool interrupt) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile = FindFirstFile("xs\\*.x", &findFileData);
	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			string filePath = string("xs\\") + findFileData.cFileName;
			//string fbxPath = filePath + ".fbx";
			//cout << filePath << endl;
			//ConvertToFbx(filePath.c_str(), fbxPath.c_str());
			//EvaluateX(filePath.c_str());
			ifstream xfile(filePath);
			xfile.seekg(g_XFileHeadSize, ios::beg);
			int version;
			xfile.read(reinterpret_cast<char*>(&version), 4);
			cout << version;
			xfile.close();
			if (version >= 6)
				remove(filePath.data());
		}
	} while (FindNextFile(hFindFile, &findFileData));
	FindClose(hFindFile);
}

void Jump2FBX(int argc, char** argv) {
	cout << endl << "JumpX to FBX Convertor by Gamepiaynmo." << endl << endl;
	if (argc < 3) {
		cout << "Usage: jump2fbx.exe <X File Name> <FBX File Name>" << endl << endl;
		return;
	}

	cout << "Link Start !" << endl;
	ConvertToFbx(argv[1], argv[2]);
	cout << "Complete." << endl;

	cout << endl;
}

void FBX2Jump(int argc, char** argv) {
	ConvertToX(argv[1], argv[2]);
}

int main(int argc, char** argv) {
	freopen("out.txt", "w", stdout);
	freopen("debug.txt", "w", stderr);
	//Jump2FBX(argc, argv);
	//FBX2Jump(argc, argv);
	IterateAllXFiles(false);

	//ConvertToFbx("000.x", "000.fbx");
	//ConvertToX("089_phos.fbx", "089_phos.x");
	//ConvertToFbx("089_phos.x", "089_phos_new.fbx");
	//ConvertToFbx("167.x", "167.fbx");
	//ConvertToX("101.fbx", "101_new.x", "101.x");
	//ConvertToFbx("167_new.x", "167_new.fbx");
	//IterateAllXFiles(false);
	//EvaluateX("087.x");

	//ConvertToFbx("167_skin1.x", "167_skin1.fbx");
	//ConvertToX("167_skin1.fbx", "167_new.x", "167_skin1.x");
	//ConvertToFbx("167_new.x", "167_new.fbx");

	//ConvertToX("167.fbx", "167_new.x", "167.x");
	//ConvertToFbx("167_new.x", "167_new.fbx");

	//PackageX("101_T.x", "101_T_index.wuy", "101_T_model.dat", "101_T_new.x");

	//PackageX("167_new.x", "167_skin1_index.wuy", "167_skin1_model.dat", "167_skin1_new.x");
	_CrtDumpMemoryLeaks();
	return 0;
}