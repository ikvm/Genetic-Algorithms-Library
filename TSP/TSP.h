// TSP.h : main header file for the TSP application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CTSPApp:
// See TSP.cpp for the implementation of this class
//

class CTSPApp : public CWinApp
{
public:
	CTSPApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTSPApp theApp;