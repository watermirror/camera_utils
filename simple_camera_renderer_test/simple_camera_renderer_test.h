
// simple_camera_renderer_test.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// SimpleCameraRendererTestApp:
// See simple_camera_renderer_test.cpp for the implementation of this class
//

class SimpleCameraRendererTestApp : public CWinApp
{
public:
	SimpleCameraRendererTestApp();

// Overrides
public:
	virtual BOOL InitInstance();
  virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern SimpleCameraRendererTestApp theApp;