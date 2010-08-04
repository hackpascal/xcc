#pragma once

#include <cc_structures.h>
#include "xcc_dirs.h"

using namespace std;

class id_log  
{
public:
	static int open_binary(const string& fname);
	static void add_name(t_game game, const string& name, const string& description);
	static string get_name(t_game game, int id);
	static string get_description(t_game game, int id);
};

namespace mix_database
{
	inline void add_name(t_game game, const string& name, const string& description)
	{
		id_log::add_name(game, name, description);
	}

	inline string get_name(t_game game, int id)
	{
		return id_log::get_name(game, id);
	}

	inline string get_description(t_game game, int id)
	{
		return id_log::get_description(game, id);
	}

	inline int load()
	{
		return id_log::open_binary(xcc_dirs::get_data_dir() + "global mix database.dat");
	}
};
