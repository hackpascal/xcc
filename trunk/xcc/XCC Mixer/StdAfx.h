// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__94521418_D8CC_11D3_9F8E_0000F81AF9BE__INCLUDED_)
#define AFX_STDAFX_H__94521418_D8CC_11D3_9F8E_0000F81AF9BE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma warning(disable: 4503 4554 4786 4800)

#include <cassert>
#include <climits>
#include <memory>
#include <map>
#include <set>
#include <string>
#include "ETSLayout.h"

using namespace ETSLayout;
using namespace std;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__94521418_D8CC_11D3_9F8E_0000F81AF9BE__INCLUDED_)