// pcx_file.h: interface for the Cpcx_file class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCX_FILE_H__27392220_D5BC_11D3_B604_0000B4936994__INCLUDED_)
#define AFX_PCX_FILE_H__27392220_D5BC_11D3_B604_0000B4936994__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>
#include <cc_file_sh.h>
#include <cc_structures.h>
#include <palet.h>

class Cpcx_file: public Ccc_file_sh<t_pcx_header>  
{
public:
	int extract_as_png(const string& name) const;

	bool is_valid() const
	{
		const t_pcx_header& header = *get_header();
		int size = get_size();
		return !(sizeof(t_pcx_header) > size ||
			header.manufacturer != 10 ||
			header.version != 5 ||
			header.encoding != 1 ||
			header.cbits_pixel != 8 ||
			header.c_planes != 1 && header.c_planes != 3);
	}

	int get_cx() const
	{
		return get_header()->xmax - get_header()->xmin + 1;
	}

	int get_cy() const
	{
		return get_header()->ymax - get_header()->ymin + 1;
	}

	int get_c_planes() const
	{
		return get_header()->c_planes;
	}

	const byte* get_image() const
	{
		return get_data() + sizeof(t_pcx_header);
	}

    const t_palet* get_palet() const
    {
        return reinterpret_cast<const t_palet*>(get_data() + get_size() - sizeof(t_palet));
    }
};

#endif // !defined(AFX_PCX_FILE_H__27392220_D5BC_11D3_B604_0000B4936994__INCLUDED_)
