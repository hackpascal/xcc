// XCC Audio PlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XCC Audio PlayerDlg.h"
#include "XCCAudioPlayerInfoDlg.h"
#include "XCCSetDirectoriesDlg.h"

#include "ima_adpcm_wav_decode.h"
#include "riff_structures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CXCCAudioPlayerDlg dialog

CXCCAudioPlayerDlg::CXCCAudioPlayerDlg(CWnd* pParent /*=NULL*/)
	: ETSLayoutDialog(CXCCAudioPlayerDlg::IDD, pParent, "AudioPlayerDlg")
{
	//{{AFX_DATA_INIT(CXCCAudioPlayerDlg)
	m_statusbar = "";
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


CXCCAudioPlayerDlg::~CXCCAudioPlayerDlg()
{
	release_memory();
}

void CXCCAudioPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	ETSLayoutDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXCCAudioPlayerDlg)
	DDX_Control(pDX, IDEXTRACT_RAW, m_extract_raw_button);
	DDX_Control(pDX, IDEXTRACT, m_extractbutton);
	DDX_Control(pDX, IDPLAY, m_playbutton);
	DDX_Control(pDX, IDSHUFFLE, m_shufflebutton);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_STATUSBAR, m_statusbar);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CXCCAudioPlayerDlg, ETSLayoutDialog)
	//{{AFX_MSG_MAP(CXCCAudioPlayerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDEXTRACT, OnExtract)
	ON_BN_CLICKED(IDOPENMIX, OnOpenmix)
	ON_BN_CLICKED(IDSETDIRECTORIES, OnSetdirectories)
	ON_BN_CLICKED(IDPLAY, OnPlay)
	ON_BN_CLICKED(IDSTOP, OnStop)
	ON_BN_CLICKED(IDINFO, OnInfo)
	ON_BN_CLICKED(IDC_OpenMovies, OnOpenMovies)
	ON_BN_CLICKED(IDC_OpenSounds, OnOpenSounds)
	ON_BN_CLICKED(IDC_OpenScores, OnOpenScores)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_BN_CLICKED(IDSHUFFLE, OnShuffle)
	ON_BN_CLICKED(IDEXTRACT_RAW, OnExctractRaw)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOPENAUD, OnOpenaud)
	ON_BN_CLICKED(IDOPENVQA, OnOpenvqa)
	ON_BN_CLICKED(IDC_OpenTheme, OnOpenTheme)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, OnColumnclickList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXCCAudioPlayerDlg message handlers

string time2str(long v)
{
	string s;
	s = nwzl(3, v % 1000);
	v /= 1000;
	if (!v)
		return s;
	s = nwzl(2, v % 60) + "." + s;
	v /= 60;
	if (!v)
		return s;
	s = n(v) + ':' + s;
	return s;
}

BOOL CXCCAudioPlayerDlg::OnInitDialog()
{
	ETSLayoutDialog::OnInitDialog();

	CreateRoot(VERTICAL)
		<< (pane(HORIZONTAL, GREEDY)
			<< item(IDC_LIST1, GREEDY)
			<< (pane(VERTICAL, ABSOLUTE_HORZ)
				<< item(IDOK, NORESIZE)
				<< item(IDEXTRACT, NORESIZE)
				<< item(IDEXTRACT_RAW, NORESIZE)
				<< item(IDOPENMIX, NORESIZE)
				<< item(IDSETDIRECTORIES, NORESIZE)
				<< item(IDPLAY, NORESIZE)
				<< item(IDSHUFFLE, NORESIZE)
				<< item(IDSTOP, NORESIZE)
				<< item(IDINFO, NORESIZE)
				<< item(IDC_OpenMovies, NORESIZE)
				<< item(IDC_OpenScores, NORESIZE)
				<< item(IDC_OpenSounds, NORESIZE)
				<< item(IDC_OpenTheme, NORESIZE)
				<< item(IDOPENAUD, NORESIZE)
				<< item(IDOPENVQA, NORESIZE)
				)
			)
		<< item(IDC_STATUSBAR, ABSOLUTE_VERT);
	UpdateLayout();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (m_listfont.CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New"))
		m_list.SetFont(&m_listfont);

	mixdata = NULL;
	m_statusbar = "Ready";
	audio_output = video_output = true;
	if (dd.create(m_hWnd))
	{
		video_output = false;
		m_statusbar = "DirectDraw unavailable";
	}
	if (ds.create(m_hWnd))
	{
		audio_output = false;
		m_statusbar = "DirectSound unavailable";
	}
	UpdateData(false);
	ListView_SetExtendedListViewStyle(m_list.m_hWnd, ListView_GetExtendedListViewStyle(m_list.m_hWnd) | LVS_EX_FULLROWSELECT);
	if (add_column("ID", 0, 80) ||
		add_column("Index", 1, 40, LVCFMT_RIGHT) ||
		add_column("Size", 2, 80, LVCFMT_RIGHT) ||
		add_column("Length", 3, 80, LVCFMT_RIGHT) ||
		add_column("Name", 4, 100) ||
		add_column("Description", 5, 240))
		throw;
	mixf.enable_mix_expansion();
	OpenMix("scores.mix");
	SetTimer(0, 5 * 1000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXCCAudioPlayerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for Painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		dword cxIcon = GetSystemMetrics(SM_CXICON);
		dword cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		dword x = (rect.Width() - cxIcon + 1) / 2;
		dword y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		ETSLayoutDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXCCAudioPlayerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CXCCAudioPlayerDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	OnPlay();
}

const char* all_filter = "All files (*.*)|*.*|";
const char* aud_filter = "AUD files (*.aud)|*.aud|";
const char* mix_filter = "MIX files (*.mix)|*.mix|";
const char* vqa_filter = "VQA files (*.vqa)|*.vqa|";
const char* wav_filter = "WAV files (*.wav)|*.wav|";

void CXCCAudioPlayerDlg::OnExtract() 
{
	if (!valid_index() || mixdata[current_index].type != ft_aud)
		return;
	CFileDialog dlg(false, "wav", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, wav_filter, this);

	char s[MAX_PATH];
	strcpy(s, mix_database::get_description(game, mixdata[current_index].id).c_str()); 
	dlg.m_ofn.lpstrFile = s;
	if (IDOK == dlg.DoModal())
	{
		string fname = dlg.GetPathName();
		m_statusbar = ("Extracting to " + fname).c_str();
		UpdateData(false);
		
		Caud_file audf;
		audf.open(mixdata[current_index].id, mixf);
		m_statusbar = audf.extract_as_wav(fname) ? "Extraction failed" : "Extraction succeeded";
		UpdateData(false);
		audf.close();		
	}
}

void CXCCAudioPlayerDlg::OnExctractRaw() 
{
	if (!valid_index())
		return;
	CFileDialog dlg(false, "", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST, all_filter, this);

	char s[MAX_PATH];
	strcpy(s, mix_database::get_name(game, mixdata[current_index].id).c_str()); 
	dlg.m_ofn.lpstrFile = s;
	if (IDOK == dlg.DoModal())
	{
		string fname = dlg.GetPathName();
		m_statusbar = ("Extracting to " + fname).c_str();
		UpdateData(false);
		
		Ccc_file f(false);
		f.open(mixdata[current_index].id, mixf);
		m_statusbar = f.extract(fname) ? "Extraction failed" : "Extraction succeeded";
		UpdateData(false);
		f.close();		
	}
}

void CXCCAudioPlayerDlg::OnOpenmix() 
{
	CFileDialog dlg(true, "mix", 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, mix_filter, this);
	dlg.m_ofn.lpstrInitialDir = xcc_dirs::get_td_primary_dir().c_str();
	if (IDOK == dlg.DoModal())
		OpenMix(static_cast<string>(dlg.GetPathName()));
}

void CXCCAudioPlayerDlg::OnOpenaud() 
{
	CFileDialog dlg(true, "aud", 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, aud_filter, this);
	if (IDOK == dlg.DoModal())
	{
		Caud_file f;
		if (!f.open(static_cast<string>(dlg.GetPathName())))
		{
			play_aud(f);
			f.close();
		}
	}	
}

void CXCCAudioPlayerDlg::OnOpenvqa() 
{
	CFileDialog dlg(true, "vqa", 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, vqa_filter, this);
	dlg.m_ofn.lpstrInitialDir = xcc_dirs::get_td_primary_dir().c_str();
	if (IDOK == dlg.DoModal())
	{
		Cvqa_file f;
		if (!f.open(static_cast<string>(dlg.GetPathName())))
		{
			play_vqa(f);
			f.close();
		}
	}	
}

void CXCCAudioPlayerDlg::OnSetdirectories() 
{
	/*
	CXCCSetDirectoriesDlg dlg;
	dlg.DoModal();
	*/
}

long CXCCAudioPlayerDlg::OpenMix(const string &fname)
{
	m_shufflebutton.EnableWindow(false);
	m_c_files = 0;
	m_shuffle = false;
	if (mixf.is_open())
		mixf.close();
	release_memory();
	m_list.SetRedraw(false);
	m_list.DeleteAllItems();
	valid_index();
	mix_database::load();
	if (mixf.open(fname))
	{
		m_statusbar = "Open failed";
		UpdateData(false);
		return 1;
	}
	m_c_files = mixf.get_c_files();
	game = mixf.get_game();
	mixdata = new mixdata_entry[m_c_files];
	int j = 0;
	for (int i = 0; i < m_c_files; i++)
	{
		dword id = mixf.get_id(i);
		t_file_type ft = mixf.get_type(id);
		if (ft != ft_aud && ft != ft_vqa && ft != ft_wav && ft != ft_wsa)
			continue;
		if (add_item(nh(8, id), j, 0) ||		
			add_item(n(j), j, 1) ||
			add_item(n(mixf.get_size(id)), j, 2) ||
			add_item(mix_database::get_name(game, id), j, 4) ||
			add_item(mix_database::get_description(game, id), j, 5))
			throw;
		m_list.SetItemData(j, id);
		mixdata[j].id = id;
		mixdata[j].type = ft;
		string s = ft_name[ft];
		switch (ft)
		{
		case ft_aud:
			{
				Caud_file f;
				f.open(id, mixf);
				if(!f.can_be_decoded())
					s = "aud: compression unsupported";
				else
					s = time2str(f.get_c_samples() / (f.get_samplerate() / 1000));
				f.close();
				break;
			}
		case ft_vqa:
			{
				Cvqa_file f;
				f.open(id, mixf);
				if(!f.can_be_decoded())
					s = "vqa: compression unsupported";
				else
					s = time2str(f.get_c_frames() * 1000 / 15);
				f.close();
				break;
			}
		case ft_wav:
			{
				Cwav_file f;
				f.open(id, mixf);
				f.process();
				const t_riff_wave_format_chunk& format_chunk = f.get_format_chunk();
				if (format_chunk.tag != 0x11 || format_chunk.cbits_sample != 4)
					s = "wav: compression unspported";
				else
					s = time2str(f.get_fact_chunk().c_samples / (format_chunk.samplerate / 1000));
				f.close();
				break;
			}
		}
		add_item(s, j, 3);
		j++;
	}	
	for (i = 0; i < 10; i++)
		m_list.SetColumnWidth(i, LVSCW_AUTOSIZE);
	sort_list(4, false);
	m_list.SetRedraw(true);
	m_list.Invalidate();
	if (has_scores())
		m_shufflebutton.EnableWindow(true);
	UpdateData(false);
	return 0;
}

long CXCCAudioPlayerDlg::add_column(const string &text, dword index, dword size, dword format, dword subindex)
{
	LV_COLUMN column_data;
	column_data.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	column_data.fmt = format;
	column_data.cx = size;
	char s[256];
	strcpy(s, text.c_str()); 
	column_data.pszText = s;
	column_data.iSubItem = subindex == -1 ? index : subindex;
	return m_list.InsertColumn(index, &column_data) == -1 ? 1 : 0;
}

long CXCCAudioPlayerDlg::add_item(const string &text, dword index, dword subindex, long param)
{
	LV_ITEM item_data;
	item_data.mask = LVIF_TEXT | (param == -1 ? 0 : LVIF_PARAM);
	item_data.iItem = index;
	item_data.iSubItem = subindex;
	char s[256];
	strcpy(s, text.c_str()); 
	item_data.pszText = s;
	item_data.lParam = param;
	return (subindex ? m_list.SetItem(&item_data) : m_list.InsertItem(&item_data)) == -1 ? 1 : 0;
}

void CXCCAudioPlayerDlg::OnPlay() 
{
	if (!valid_index())
		return;
	int id = mixdata[current_index].id;
	switch (mixdata[current_index].type)
	{
	case ft_aud:
		if (play_aud(id))
			m_statusbar = "Play AUD failed";
		else
			m_statusbar = "Ready";
		break;
	case ft_vqa:
		if (play_vqa(id))
			m_statusbar = "Play VQA failed";
		else
			m_statusbar = "Ready";
		break;
	case ft_wav:
		if (play_wav(id))
			m_statusbar = "Play wav failed";
		else
			m_statusbar = "Ready";
		break;
	}
	UpdateData(false);
}


void CXCCAudioPlayerDlg::OnShuffle() 
{
	m_shuffle = true;
}

void CXCCAudioPlayerDlg::OnStop() 
{
	if (!dsb.is_available())
		return;
	dsb.stop();	
	m_shuffle = false;
}

void CXCCAudioPlayerDlg::OnInfo() 
{
	CXCCAudioPlayerInfoDlg dlg;
	dlg.DoModal();	
}

int CXCCAudioPlayerDlg::play_aud(dword id)
{
	Caud_file f;
	if (f.open(id, mixf))
		return 1;
	int error = play_aud(f);
	f.close();
	return error;
}

int CXCCAudioPlayerDlg::play_aud(Caud_file& audf)
{
	int error = 0;
	if (!audio_output)
		return 1;
	int cb_sample = audf.get_cb_sample();
	if (dsb.is_available())
		dsb.destroy();
	if (!audf.can_be_decoded())
		error = 1;
	else if (dsb.create(ds, cb_sample * audf.get_c_samples(), 1, audf.get_samplerate(), cb_sample << 3, DSBCAPS_GLOBALFOCUS))
		error = 1;
	else
	{			
		int cs_remaining = audf.get_c_samples();
		int writeofs = 0;			
		decode.init();
		int chunk_i = 0;
		bool playing = false;			
		while (cs_remaining)
		{	
			m_statusbar = n(cs_remaining / audf.get_samplerate()).c_str();
			UpdateData(false);
			const t_aud_chunk_header& header = *audf.get_chunk_header(chunk_i);
			int cs_audio = header.size_out / cb_sample;
			const byte* audio_in = audf.get_chunk_data(chunk_i++);
			cs_remaining -= cs_audio;
			void* p1;
			dword s1;
			if (dsb.lock(writeofs, cb_sample * cs_audio, &p1, &s1, 0, 0))
				error = 1;
			else
			{
				if (cb_sample == 2)
					decode.decode_chunk(audio_in, reinterpret_cast<short*>(p1), cs_audio);
				else if (header.size_in < header.size_out)
					aud_decode_ws_chunk(audio_in, reinterpret_cast<char*>(p1), cs_audio);
				else
					memcpy(p1, audio_in, cs_audio);
				writeofs += cb_sample * cs_audio;
				if (dsb.unlock(p1, s1, 0, 0))
					error = 1;
				else if (!playing)
				{
					if (dsb.play(0))
						error = 1;
					else
						playing = true;
				}
			}
		}
	}
	return error;
}

int CXCCAudioPlayerDlg::play_vqa(dword id)
{
	Cvqa_file f;
	if (f.open(id, mixf))
		return 1;
	int error = play_vqa(f);
	f.close();
	return error;
}

int CXCCAudioPlayerDlg::play_vqa(Cvqa_file& f)
{
	int error = 0;
	if (!(audio_output && video_output))
		return 1;
	if (!f.can_be_decoded())
		error = 1;
	else
	{
		Cvqa_play vqa_play(dd.get_p(), ds.get_p());
		long result = vqa_play.create(f);
		if (result)
			AfxMessageBox(("Error initializing DD or DS, error code is " + n(result)).c_str(), MB_ICONEXCLAMATION);
		else
		{
			while (vqa_play.run())
			{
				MSG msg;		
				while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE))
					AfxGetApp()->PumpMessage();
				
				long count = 0;
				while (AfxGetApp()->OnIdle(count++));
			}
		}
		vqa_play.destroy();
	}
	return error;
}

int CXCCAudioPlayerDlg::play_wav(dword id)
{
	Cwav_file f;
	if (f.open(id, mixf))
		return 1;
	int error = play_wav(f);
	f.close();
	return error;
}

int CXCCAudioPlayerDlg::play_wav(Cwav_file& f)
{
	int error = 0;
	if (!audio_output)
		return 1;
	if (f.process())
		return 1;
	const t_riff_wave_format_chunk& format_chunk = f.get_format_chunk();
	const t_riff_wave_fact_chunk& fact_chunk = f.get_fact_chunk();
	int c_channels = format_chunk.c_channels;
	int cb_sample = format_chunk.cbits_sample >> (format_chunk.tag == 1 ? 3 : 1);
	if (dsb.is_available())
		dsb.destroy();
	if (dsb.create(ds, c_channels * cb_sample * fact_chunk.c_samples, c_channels, format_chunk.samplerate, cb_sample << 3, DSBCAPS_GLOBALFOCUS))
		error = 1;
	else
	{
		int cb_s = f.get_data_header().size;
		byte* s = new byte[cb_s];
		const byte* r = s;
		f.seek(f.get_data_ofs());
		f.read(s, cb_s);
		int cs_remaining = fact_chunk.c_samples;
		int writeofs = 0;			
		int chunk_i = 0;
		bool playing = false;
		void* p1;
		dword s1;
		if (dsb.lock(writeofs, cb_sample * cs_remaining * c_channels, &p1, &s1, 0, 0))
			error = 1;
		else
		{
			Cima_adpcm_wav_decode decode;
			decode.load(s, cb_s, c_channels, 512 * c_channels);
			memcpy(reinterpret_cast<short*>(p1), decode.data(), c_channels * cs_remaining << 1);
			if (dsb.unlock(p1, s1, 0, 0))
				error = 1;
			else if (dsb.play(0))
				error = 1;
		}
		/*
		while (cs_remaining)
		{	
			m_statusbar = n(cs_remaining / format_chunk.samplerate).c_str();
			UpdateData(false);
			for (int channel = 0; channel < c_channels; channel++)
			{
				const t_ima_adpcm_chunk_header& chunk_header = *reinterpret_cast<const t_ima_adpcm_chunk_header*>(r);
				r += sizeof(t_ima_adpcm_chunk_header);
				int cs_chunk = min(cs_remaining, format_chunk.block_align / c_channels - sizeof(t_ima_adpcm_chunk_header) << 1);
				decode.init(chunk_header.index, chunk_header.sample);
				cs_remaining -= cs_chunk;
				void* p1;
				dword s1;
				if (dsb.lock(writeofs, cb_sample * cs_chunk, &p1, &s1, 0, 0))
					error = 1;
				else
				{
					short* w = reinterpret_cast<short*>(p1);
					*w++ = chunk_header.sample;
					decode.decode_chunk(r, w, cs_chunk);
					r += cs_chunk >> 1;
					writeofs += cb_sample * cs_chunk;
					if (dsb.unlock(p1, s1, 0, 0))
						error = 1;
					else if (!playing)
					{
						if (dsb.play(0))
							error = 1;
						else
							playing = true;
					}
				}
			}
		}
		*/
		delete[] s;
	}
	return error;
}

bool CXCCAudioPlayerDlg::valid_index()
{
	current_index = m_list.GetNextItem(-1, LVNI_ALL | LVNI_FOCUSED);
	bool can_extract = false, can_extract_raw = false, can_play = false;
	if (current_index != -1)
	{
		can_extract_raw = true;
		switch (mixdata[current_index].type)
		{
		case ft_aud:
			can_extract = can_play = true;
			break;
		case ft_vqa:
		case ft_wav:
			can_play = true;
			break;
		}
	}
	m_extractbutton.EnableWindow(can_extract);
	m_extract_raw_button.EnableWindow(can_extract_raw);
	m_playbutton.EnableWindow(can_play);
	return current_index != -1;
}

void CXCCAudioPlayerDlg::OnOpenMovies() 
{
	OpenMix("movies.mix");
}

void CXCCAudioPlayerDlg::OnOpenScores() 
{
	OpenMix("scores.mix");
}

void CXCCAudioPlayerDlg::OnOpenSounds() 
{
	OpenMix("sounds.mix");	
}

void CXCCAudioPlayerDlg::OnOpenTheme() 
{
	OpenMix("theme.mix");	
}

void CXCCAudioPlayerDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	valid_index();
	*pResult = 0;
}

void CXCCAudioPlayerDlg::release_memory()
{
	delete[] mixdata;
	mixdata = 0;
}

bool CXCCAudioPlayerDlg::has_scores()
{
	for (int i = 0; i < m_c_files; i++)
	{
		if (is_score(i))
			return true;
	}
	return false;
}

bool CXCCAudioPlayerDlg::is_score(int i)
{
	switch (mixdata[i].type)
	{
	case ft_aud:
	case ft_wav:
		return mixf.get_size(mixdata[i].id) > 128 << 10;
	default:
		return false;
	}
	
}

void CXCCAudioPlayerDlg::shuffle_aud()
{
	if (!has_scores())
		return;
	int i;
	do
	{
		i = rand() % m_c_files;
	}
	while (!is_score(i));
	switch (mixdata[i].type)
	{
	case ft_aud:
		play_aud(mixdata[i].id);
		break;
	case ft_wav:
		play_wav(mixdata[i].id);
		break;
	}
}

void CXCCAudioPlayerDlg::OnTimer(UINT nIDEvent) 
{
	if (dsb.is_available())
	{
		dword status;
		dsb.get_p()->GetStatus(&status);
		if (~status & DSBSTATUS_PLAYING)
			dsb.destroy();				
	}
	else if (m_shuffle)
		shuffle_aud();
	ETSLayoutDialog::OnTimer(nIDEvent);
}

void CXCCAudioPlayerDlg::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int column = reinterpret_cast<NM_LISTVIEW*>(pNMHDR)->iSubItem;
	sort_list(column, column == m_sort_column ? !m_sort_reverse : false);
	*pResult = 0;
}

int compare_int(unsigned int a, unsigned int b)
{
	if (a < b)
		return -1;
	else if (a == b)
		return 0;
	else
		return 1;
}

int compare_string(string a, string b)
{
	if (a < b)
		return -1;
	else if (a == b)
		return 0;
	else
		return 1;
}

int CXCCAudioPlayerDlg::compare(int id_a, int id_b) const
{
	if (m_sort_reverse)
		swap(id_a, id_b);
	switch (m_sort_column)
	{
	case 0:
		return compare_int(id_a, id_b);
		break;
	case 1:
		return compare_int(mixf.get_index(id_a), mixf.get_index(id_b));
		break;
	case 2:
		return compare_int(mixf.get_size(id_a), mixf.get_size(id_b));
		break;
	case 4:
		return compare_string(mix_database::get_name(game, id_a), mix_database::get_name(game, id_b));
		break;
	case 5:
		return compare_string(mix_database::get_description(game, id_a), mix_database::get_description(game, id_b));
		break;
	default:
		return 0;
	}
}

static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return reinterpret_cast<CXCCAudioPlayerDlg*>(lParamSort)->compare(lParam1, lParam2);
}

void CXCCAudioPlayerDlg::sort_list(int i, bool reverse)
{
	m_sort_column = i;
	m_sort_reverse = reverse;
	m_list.SortItems(Compare, reinterpret_cast<dword>(this));
}