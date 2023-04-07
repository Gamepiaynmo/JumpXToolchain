//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "JumpXExporter.h"
#include "JumpX.h"

#define JumpXExporter_CLASS_ID	Class_ID(0xae2b9fc9, 0xcd3bc499)

class JumpXExporter : public SceneExport {
public:
	//Constructor/Destructor
	JumpXExporter();
	~JumpXExporter();

	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	BOOL SupportsOptions(int ext, DWORD options);
	int  DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
};



class JumpXExporterClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new JumpXExporter(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return JumpXExporter_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("JumpXExporter"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};


ClassDesc2* GetJumpXExporterDesc() { 
	static JumpXExporterClassDesc JumpXExporterDesc;
	return &JumpXExporterDesc; 
}





INT_PTR CALLBACK JumpXExporterOptionsDlgProc(HWND hWnd,UINT message,WPARAM,LPARAM lParam) {
	static JumpXExporter* imp = nullptr;

	switch(message) {
		case WM_INITDIALOG:
			imp = (JumpXExporter *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;
	}
	return 0;
}


//--- JumpXExporter -------------------------------------------------------
JumpXExporter::JumpXExporter()
{

}

JumpXExporter::~JumpXExporter() 
{

}

int JumpXExporter::ExtCount()
{
	#pragma message(TODO("Returns the number of file name extensions supported by the plug-in."))
	return 1;
}

const TCHAR *JumpXExporter::Ext(int /*i*/)
{		
	#pragma message(TODO("Return the 'i-th' file name extension (i.e. \"3DS\")."))
	return _T("X");
}

const TCHAR *JumpXExporter::LongDesc()
{
	#pragma message(TODO("Return long ASCII description (i.e. \"Targa 2.0 Image File\")"))
	return _T("JumpX Model File");
}
	
const TCHAR *JumpXExporter::ShortDesc() 
{			
	#pragma message(TODO("Return short ASCII description (i.e. \"Targa\")"))
	return _T("JumpX");
}

const TCHAR *JumpXExporter::AuthorName()
{			
	#pragma message(TODO("Return ASCII Author name"))
	return _T("Gamepiaynmo");
}

const TCHAR *JumpXExporter::CopyrightMessage() 
{	
	#pragma message(TODO("Return ASCII Copyright message"))
	return _T("Gamepiaynmo");
}

const TCHAR *JumpXExporter::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *JumpXExporter::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int JumpXExporter::Version()
{				
	#pragma message(TODO("Return Version number * 100 (i.e. v3.01 = 301)"))
	return 100;
}

void JumpXExporter::ShowAbout(HWND /*hWnd*/)
{			
	// Optional
}

BOOL JumpXExporter::SupportsOptions(int /*ext*/, DWORD /*options*/)
{
	#pragma message(TODO("Decide which options to support.  Simply return true for each option supported by each Extension the exporter supports."))
	return TRUE;
}

__declspec(dllexport) int ActualDoExport(const TCHAR* name, ExpInterface* ei, Interface* ip, BOOL suppressPrompts, DWORD options) {

#pragma message(TODO("Implement the actual file Export here and"))

	/*
	if (!suppressPrompts)
		DialogBoxParam(hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			GetActiveWindow(),
			JumpXExporterOptionsDlgProc, NULL);
			*/
	XScene *scene = new XScene();
	__try {
		scene->saveToFile(name, options);
	} __except (1) {
		return FALSE;
	}
	delete scene;

#pragma message(TODO("return TRUE If the file is exported properly"))
	return TRUE;
}

int	JumpXExporter::DoExport(const TCHAR* name, ExpInterface* ei, Interface* ip, BOOL suppressPrompts, DWORD options)
{
	return ActualDoExport(name, ei, ip, suppressPrompts, options);
}


