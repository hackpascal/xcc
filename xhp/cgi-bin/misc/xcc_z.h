// xcc_z.h: interface for the xcc_z class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XCC_Z_H__63B3CD06_15B5_11D6_B606_C89000A7846E__INCLUDED_)
#define AFX_XCC_Z_H__63B3CD06_15B5_11D6_B606_C89000A7846E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "virtual_binary.h"

namespace xcc_z
{
	Cvirtual_binary gzip(const byte* s, int cb_s);
	Cvirtual_binary gzip(string v);
	void gzip_out(const void* s, int cb_s);
	void gzip_out(string v);
}

#endif // !defined(AFX_XCC_Z_H__63B3CD06_15B5_11D6_B606_C89000A7846E__INCLUDED_)