// edit_teamtypes_dlg.cpp : implementation file
//

#include "stdafx.h"
#include "xcc editor.h"
#include "edit_teamtypes_dlg.h"
#include "edit_teamtype_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cedit_teamtypes_dlg dialog


Cedit_teamtypes_dlg::Cedit_teamtypes_dlg(Cxcc_level& level, bool selection_wanted)
	: CDialog(Cedit_teamtypes_dlg::IDD, 0),
	m_level(level),
	m_teamtype_data(level.teamtype_data),
	m_selection_wanted(selection_wanted)
{
	//{{AFX_DATA_INIT(Cedit_teamtypes_dlg)
	//}}AFX_DATA_INIT
}


void Cedit_teamtypes_dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cedit_teamtypes_dlg)
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDOK, m_ok_button);
	DDX_Control(pDX, IDC_EDIT, m_edit_button);
	DDX_Control(pDX, IDC_DELETE, m_delete_button);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cedit_teamtypes_dlg, CDialog)
	//{{AFX_MSG_MAP(Cedit_teamtypes_dlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnItemchangedList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cedit_teamtypes_dlg message handlers

BOOL Cedit_teamtypes_dlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_list.set_full_row_selection();

	m_list.add_column("Name", 0);
	m_list.add_column("Side", 1);	
	m_list.add_column("Objects", 2);	
	m_list.add_column("Actions", 3);	

	for (t_teamtype_data::const_iterator i = m_teamtype_data.begin(); i != m_teamtype_data.end(); i++)
	{
		add_teamtype(i->first);
	};

	m_list.autosize_columns();
	
	return true;
}

const string& Cedit_teamtypes_dlg::get_selected_name() const
{
	return m_selected_name;
}

void Cedit_teamtypes_dlg::OnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->uNewState & LVIS_FOCUSED)
	{
		m_selected_name = m_list.GetItemText(pNMListView->iItem, 0);
	}
	update_controls();
	*pResult = 0;
}

void Cedit_teamtypes_dlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_selection_wanted)
		OnOK();
	*pResult = 0;
}

void Cedit_teamtypes_dlg::OnNew() 
{
	Cedit_teamtype_dlg dlg(m_level);
	if (dlg.DoModal() == IDOK)
	{
		string new_name;
		dlg.save(new_name);
		add_teamtype(new_name);
		m_list.autosize_columns();
	}
}

void Cedit_teamtypes_dlg::OnEdit() 
{
	Cedit_teamtype_dlg dlg(m_level);
	dlg.load(m_selected_name);
	if (dlg.DoModal() == IDOK)
	{
		string new_name;
		dlg.save(new_name);
		if (m_selected_name != new_name)
		{
			m_teamtype_data.erase(m_selected_name);
			m_list.set_item(new_name, get_index(m_selected_name), 0);
			m_selected_name = new_name;
		}
		update_teamtype(m_selected_name);
		m_list.autosize_columns();
	}
}

void Cedit_teamtypes_dlg::OnDelete() 
{
	m_teamtype_data.erase(m_selected_name);
	m_list.DeleteItem(get_index(m_selected_name));
	update_controls();
}

void Cedit_teamtypes_dlg::add_teamtype(const string& name)
{
	m_list.add_item(name, m_list.GetItemCount());
	update_teamtype(name);
}

void Cedit_teamtypes_dlg::update_teamtype(const string& name)
{
	long i = get_index(name);
	m_list.set_item(name, i, 0);
	const t_teamtype_data_entry& d = m_level.teamtype_data[name];
	m_list.set_item(d.side != -1 ? side_code[d.side] : "none", i, 1);
	string s;
	for (long j = 0; j < d.c_objects; j++)
	{
		int v = d.object_list[j];
		if (j)
			s += ", ";
		s += static_cast<string>(v & 0x8000 ? xcc_units::unit_data[v >> 8 & 0x7f].short_name : xcc_infantry::infantry_data[v >> 8].short_name) + 
			':' + n(v & 0xff);
	}
	m_list.set_item(s, i, 2);
	s.erase();
	for (j = 0; j < d.c_actions; j++)
	{
		int v = d.action_list[j];
		if (j)
			s += ", ";
		s += static_cast<string>(action_code[v >> 8]) + ':' + n(v & 0xff);
	}
	m_list.set_item(s, i, 3);
}

long Cedit_teamtypes_dlg::get_index(const string& name) const
{
	for (long i = 0; i < m_list.GetItemCount(); i++)
	{
		if (m_list.GetItemText(i, 0) == name.c_str())
			return i;
	}
	return -1;
}

void Cedit_teamtypes_dlg::update_controls()
{
	long selected_i = m_list.get_focused_item();
	if (m_selection_wanted)
		m_ok_button.EnableWindow(selected_i != -1);
	m_edit_button.EnableWindow(selected_i != -1);
	m_delete_button.EnableWindow(selected_i != -1);
}
