// shp_dune2_file.cpp: implementation of the Cshp_dune2_file class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "shp_dune2_file.h"

#include "image_file.h"
#include "shp_decode.h"
#include "string_conversion.h"

int Cshp_dune2_file::extract_as_pcx(const Cfname& name, t_file_type ft, const t_palet _palet) const
{
	t_palet palet;
	convert_palet_18_to_24(_palet, palet);
	for (int i = 0; i < get_c_images(); i++)
	{
		const int cx = get_cx(i);
		const int cy = get_cy(i);
		Cvirtual_binary image;
		if (is_compressed(i))
		{
			byte* d = new byte[get_image_header(i)->size_out];
			decode2(d, image.write_start(cx * cy), decode80(get_image(i), d), get_reference_palet(i));
			delete[] d;
		}
		else
			decode2(get_image(i), image.write_start(cx * cy), get_image_header(i)->size_out, get_reference_palet(i));
		Cfname t = name;
		t.set_title(name.get_ftitle() + " " + nwzl(4, i));
		int error = image_file_write(ft, image, palet, cx, cy).export(t);
		if (error)
			return error;
	}
	return 0;
}