// config_dialog.cpp : インプリメンテーション ファイル
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdafx.h"
#include "dsp_superequ.h"
#include "config_dialog.h"

#include "paramlist.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern float last_srate;
extern void equ_makeTable(float *lbc,float *rbc,paramlist *,float fs);

extern float lbands[];
extern float rbands[];
extern paramlist paramroot;
paramlist lastparamroot;

int lslpos[19],rslpos[19],last_lslpos[19],last_rslpos[19];
int locklr = 1, not_applied = 0;
int last_param_index = 0;
extern char autoloadfile[257];

int freqs[] = {55,77,110,156,220,311,440,622,880,1244,1760,2489,3520,4978,7040,9956,14080,19912};

/////////////////////////////////////////////////////////////////////////////
// config_dialog ダイアログ

CSliderCtrl *lsl[19],*rsl[19];

config_dialog::config_dialog(CWnd* pParent /*=NULL*/)
	: CDialog(config_dialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(config_dialog)
	m_ch_dither = FALSE;
	m_ch_pleft = FALSE;
	m_ch_pright = FALSE;
	//}}AFX_DATA_INIT
	//m_ch_autopre = FALSE;
	m_ch_enable = FALSE;
	m_ch_lock = FALSE;

}


void config_dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(config_dialog)
	DDX_Control(pDX, IDC_ED_AUTOLOAD, m_ed_autoload);
	DDX_Control(pDX, IDC_BUT_PNEW, m_but_pnew);
	DDX_Control(pDX, IDC_CH_PRIGHT, m_ch_pright2);
	DDX_Control(pDX, IDC_CH_PLEFT, m_ch_pleft2);
	DDX_Control(pDX, IDC_CH_DITHER, m_ch_dither2);
	DDX_Control(pDX, IDC_LIST_PARAM, m_list_param);
	DDX_Control(pDX, IDC_ED_PUP, m_ed_pup);
	DDX_Control(pDX, IDC_ED_PLOW, m_ed_plow);
	DDX_Control(pDX, IDC_ED_GAIN, m_ed_gain);
	DDX_Control(pDX, IDC_BUT_APPLY, m_but_apply);
	DDX_Control(pDX, IDC_CH_LOCK, m_ch_lock2);
	DDX_Control(pDX, IDC_CH_ENABLE, m_ch_enable2);
	DDX_Control(pDX, IDC_ED_TARGET, m_ed_target);
	DDX_Control(pDX, IDC_SL_RPRE, m_rpre);
	DDX_Control(pDX, IDC_SL_R9, m_r9);
	DDX_Control(pDX, IDC_SL_R8, m_r8);
	DDX_Control(pDX, IDC_SL_R7, m_r7);
	DDX_Control(pDX, IDC_SL_R6, m_r6);
	DDX_Control(pDX, IDC_SL_R5, m_r5);
	DDX_Control(pDX, IDC_SL_R4, m_r4);
	DDX_Control(pDX, IDC_SL_R3, m_r3);
	DDX_Control(pDX, IDC_SL_R2, m_r2);
	DDX_Control(pDX, IDC_SL_R18, m_r18);
	DDX_Control(pDX, IDC_SL_R17, m_r17);
	DDX_Control(pDX, IDC_SL_R16, m_r16);
	DDX_Control(pDX, IDC_SL_R15, m_r15);
	DDX_Control(pDX, IDC_SL_R14, m_r14);
	DDX_Control(pDX, IDC_SL_R13, m_r13);
	DDX_Control(pDX, IDC_SL_R12, m_r12);
	DDX_Control(pDX, IDC_SL_R11, m_r11);
	DDX_Control(pDX, IDC_SL_R10, m_r10);
	DDX_Control(pDX, IDC_SL_R1, m_r1);
	DDX_Control(pDX, IDC_SL_L9, m_l9);
	DDX_Control(pDX, IDC_SL_L8, m_l8);
	DDX_Control(pDX, IDC_SL_L7, m_l7);
	DDX_Control(pDX, IDC_SL_L6, m_l6);
	DDX_Control(pDX, IDC_SL_L5, m_l5);
	DDX_Control(pDX, IDC_SL_L4, m_l4);
	DDX_Control(pDX, IDC_SL_L3, m_l3);
	DDX_Control(pDX, IDC_SL_L2, m_l2);
	DDX_Control(pDX, IDC_SL_L18, m_l18);
	DDX_Control(pDX, IDC_SL_L17, m_l17);
	DDX_Control(pDX, IDC_SL_L16, m_l16);
	DDX_Control(pDX, IDC_SL_L15, m_l15);
	DDX_Control(pDX, IDC_SL_L14, m_l14);
	DDX_Control(pDX, IDC_SL_L13, m_l13);
	DDX_Control(pDX, IDC_SL_L12, m_l12);
	DDX_Control(pDX, IDC_SL_L11, m_l11);
	DDX_Control(pDX, IDC_SL_L10, m_l10);
	DDX_Control(pDX, IDC_SL_L1, m_l1);
	DDX_Control(pDX, IDC_SL_LPRE, m_lpre);
	DDX_Check(pDX, IDC_CH_LOCK, m_ch_lock);
	DDX_Check(pDX, IDC_CH_ENABLE, m_ch_enable);
	DDX_Check(pDX, IDC_CH_DITHER, m_ch_dither);
	DDX_Check(pDX, IDC_CH_PLEFT, m_ch_pleft);
	DDX_Check(pDX, IDC_CH_PRIGHT, m_ch_pright);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(config_dialog, CDialog)
	//{{AFX_MSG_MAP(config_dialog)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUT_CANCEL, OnButCancel)
	ON_BN_CLICKED(IDC_BUT_LOAD, OnButLoad)
	ON_BN_CLICKED(IDC_BUT_OK, OnButOk)
	ON_BN_CLICKED(IDC_BUT_SAVE, OnButSave)
	ON_BN_CLICKED(IDC_CH_ENABLE, OnChEnable)
	ON_BN_CLICKED(IDC_CH_LOCK, OnChLock)
	ON_BN_CLICKED(IDC_BUT_UP, OnButUp)
	ON_BN_CLICKED(IDC_BUT_SDEF, OnButSdef)
	ON_BN_CLICKED(IDC_BUT_DOWN, OnButDown)
	ON_BN_CLICKED(IDC_BUT_APPLY, OnButApply)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUT_PNEW, OnButPnew)
	ON_BN_CLICKED(IDC_BUT_PDEL, OnButPdel)
	ON_EN_CHANGE(IDC_ED_GAIN, OnChangeEdGain)
	ON_EN_CHANGE(IDC_ED_PLOW, OnChangeEdPlow)
	ON_EN_CHANGE(IDC_ED_PUP, OnChangeEdPup)
	ON_BN_CLICKED(IDC_CH_DITHER, OnChDither)
	ON_LBN_SELCHANGE(IDC_LIST_PARAM, OnSelchangeListParam)
	ON_EN_KILLFOCUS(IDC_ED_PLOW, OnKillfocusEdPlow)
	ON_EN_KILLFOCUS(IDC_ED_PUP, OnKillfocusEdPup)
	ON_EN_KILLFOCUS(IDC_ED_GAIN, OnKillfocusEdGain)
	ON_BN_CLICKED(IDC_CH_PLEFT, OnChPleft)
	ON_BN_CLICKED(IDC_CH_PRIGHT, OnChPright)
	ON_EN_CHANGE(IDC_ED_AUTOLOAD, OnChangeEdAutoload)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// config_dialog メッセージ ハンドラ

void setBandsFromSlpos(void)
{
	int i;
	float lpreamp = lslpos[0] == 96 ? 0 : pow(10,lslpos[0]/-20.0);
	float rpreamp = rslpos[0] == 96 ? 0 : pow(10,rslpos[0]/-20.0);

	for(i=0;i<18;i++)
	{
		lbands[i] = lslpos[i+1] == 96 ? 0 : lpreamp*pow(10,lslpos[i+1]/-20.0);
		rbands[i] = rslpos[i+1] == 96 ? 0 : rpreamp*pow(10,rslpos[i+1]/-20.0);
	}

	equ_makeTable(lbands,rbands,&paramroot,last_srate);
}

void config_dialog::sort_param(void)
{
	paramlistelm *selelm = NULL,*lastelm = NULL;
	int sel = m_list_param.GetCurSel();
	if (sel != LB_ERR)
		selelm = (paramlistelm *)m_list_param.GetItemDataPtr(sel);
	if (last_param_index != -1)
		lastelm = (paramlistelm *)m_list_param.GetItemDataPtr(last_param_index);

	m_list_param.ResetContent();
	paramroot.sortelm();
	
	for(paramlistelm *e = paramroot.elm;e != NULL;e = e->next)
	{
		int indx = m_list_param.AddString(e->getString());
		m_list_param.SetItemDataPtr(indx,e);
		if (selelm == e) sel = indx;
		if (lastelm == e) last_param_index = indx;
	}

	if (sel != LB_ERR) {
		m_list_param.SetCurSel(sel);
	}
}

void config_dialog::set_paramlistelm_from_gui(int index)
{
	int sel = m_list_param.GetCurSel();
	paramlistelm *e = (paramlistelm *)m_list_param.GetItemDataPtr(index);

	e->left  = m_ch_pleft2.GetCheck();	
	e->right = m_ch_pright2.GetCheck();	

	char str[64];
	m_ed_pup.GetWindowText(str,63);
	e->upper = atof(str);
	m_ed_plow.GetWindowText(str,63);
	e->lower = atof(str);
	m_ed_gain.GetWindowText(str,63);
	e->gain = atof(str);

	m_list_param.DeleteString(index);
	m_list_param.InsertString(index,e->getString());
	m_list_param.SetItemDataPtr(index,e);

	m_list_param.SetCurSel(sel);

	not_applied = 1;
	m_but_apply.EnableWindow(TRUE);

	sort_param();
}

void config_dialog::set_gui_from_paramlistelm(int index)
{
	if (index == -1) {
		m_ed_plow.SetWindowText("");
		m_ed_plow.EnableWindow(FALSE);
		m_ed_pup.SetWindowText("");
		m_ed_pup.EnableWindow(FALSE);
		m_ed_gain.SetWindowText("");
		m_ed_gain.EnableWindow(FALSE);
		m_ch_pleft2.SetCheck(FALSE);
		m_ch_pleft2.EnableWindow(FALSE);
		m_ch_pright2.SetCheck(FALSE);
		m_ch_pright2.EnableWindow(FALSE);
		return;
	}

	m_ed_plow.EnableWindow(TRUE);
	m_ed_pup.EnableWindow(TRUE);
	m_ed_gain.EnableWindow(TRUE);
	m_ch_pleft2.EnableWindow(TRUE);
	m_ch_pright2.EnableWindow(TRUE);

	int sel = m_list_param.GetCurSel();
	paramlistelm *e = (paramlistelm *)m_list_param.GetItemDataPtr(index);

	m_ch_pleft2.SetCheck(e->left);
	m_ch_pright2.SetCheck(e->right);

	char str[64];
	sprintf(str,"%.10g",e->upper);
	m_ed_pup.SetWindowText(str);

	sprintf(str,"%.10g",e->lower);
	m_ed_plow.SetWindowText(str);

	sprintf(str,"%.10g",e->gain);
	m_ed_gain.SetWindowText(str);

	m_list_param.SetCurSel(sel);
}

void config_dialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	int i,l,r;

	l = lsl[0]->GetPos();
	r = rsl[0]->GetPos();

	if (lslpos[0] != l) {
		char str[32];
		if (l == 96) wsprintf(str,"L preamp cut"); else wsprintf(str,"L preamp %ddB",-l);
		m_ed_target.SetWindowText(str);
	}

	if (rslpos[0] != r) {
		char str[32];
		if (r == 96) wsprintf(str,"R preamp cut"); else wsprintf(str,"R preamp %ddB",-r);
		m_ed_target.SetWindowText(str);
	}

	if (locklr) {
		if (lslpos[0] != l) {r = l; rsl[0]->SetPos(l);}
		if (rslpos[0] != r) {l = r; lsl[0]->SetPos(r);}
	}
	
	lslpos[0] = l;
	rslpos[0] = r;

	for(i=0;i<18;i++)
	{
		l = lsl[i+1]->GetPos();
		r = rsl[i+1]->GetPos();

		if (lslpos[i+1] != l) {
			char str[32];
			if (l == 96) wsprintf(str,"L %dHz cut",freqs[i]); else wsprintf(str,"L %dHz %ddB",freqs[i],-l);
			m_ed_target.SetWindowText(str);
		}

		if (rslpos[i+1] != r) {
			char str[32];
			if (r == 96) wsprintf(str,"R %dHz cut",freqs[i]); else wsprintf(str,"R %dHz %ddB",freqs[i],-r);
			m_ed_target.SetWindowText(str);
		}

		if (locklr) {
			if (lslpos[i+1] != l) {r = l; rsl[i+1]->SetPos(l);}
			if (rslpos[i+1] != r) {l = r; lsl[i+1]->SetPos(r);}
		}

		lslpos[i+1] = l;
		rslpos[i+1] = r;
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);

	//setBandsFromSlpos();
	not_applied = 1;
	m_but_apply.EnableWindow(TRUE);
}

void config_dialog::OnButCancel() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
#if 0
	int i;

	for(i=0;i<19;i++)
	{
		lslpos[i] = last_lslpos[i];
		rslpos[i] = last_rslpos[i];
	}

	setBandsFromSlpos();
#endif

	CDialog::OnCancel();
}

void my_fgets(char *p,int size,CFile &cf)
{
	while(size > 1)
	{
		if (cf.Read(p,1) == 0) break;
		p++;
		size--;
		if (p[-1] == '\n') break;
	}
	*p = '\0';
}

void load_from(char *filename,int igerr)
{
	CFile f;
	CFileException e;

	if( !f.Open( filename, CFile::modeRead, &e ) )
    {
		if (!igerr)
			::MessageBox(NULL,"Couldn't open file","Error",MB_OK | MB_ICONEXCLAMATION);
		return;
    }

	char buf[256];

	my_fgets(buf,256,f);

	if (sscanf(buf,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			&lslpos[0],&lslpos[1],&lslpos[2],&lslpos[3],&lslpos[4],&lslpos[5],
			&lslpos[6],&lslpos[7],&lslpos[8],&lslpos[9],&lslpos[10],&lslpos[11],
			&lslpos[12],&lslpos[13],&lslpos[14],&lslpos[15],&lslpos[16],&lslpos[17],
			&lslpos[18],
			&rslpos[0],&rslpos[1],&rslpos[2],&rslpos[3],&rslpos[4],&rslpos[5],
			&rslpos[6],&rslpos[7],&rslpos[8],&rslpos[9],&rslpos[10],&rslpos[11],
			&rslpos[12],&rslpos[13],&rslpos[14],&rslpos[15],&rslpos[16],&rslpos[17],
			&rslpos[18]) != 38) {
		::MessageBox(NULL,"Format error","Error",MB_OK | MB_ICONEXCLAMATION);
		// error
	}

	delete paramroot.elm;
	paramroot.elm = NULL;
	paramlistelm **p;

	for(p = &paramroot.elm;;p = &(*p)->next)
	{
		float lower,upper,gain;
		int left,right,r;

		my_fgets(buf,256,f);
		r = sscanf(buf,"%g %g %g %d %d",&upper,&lower,&gain,&left,&right);
		if (r == 0 || r == EOF) break;
		if (r != 5) {
			sprintf(buf,"Format error 2 (%d)",r);
			::MessageBox(NULL,buf,"Error",MB_OK | MB_ICONEXCLAMATION);
			break;
		}

		*p = new paramlistelm;
		(*p)->lower = lower;
		(*p)->upper = upper;
		(*p)->gain  = gain;
		(*p)->left  = left;
		(*p)->right = right;
	}

	f.Close();
}

void config_dialog::OnButLoad() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	static char filter[] = "equalizer file (*.eq)\0*.eq\0All files (*.*)\0*.*\0\0";
	CFileDialog *filedialog;
	char filename[257];

	filedialog = new CFileDialog(TRUE,".eq");

	filedialog->m_ofn.lpstrFilter = filter;
	filedialog->m_ofn.lpstrFile = filename;
	filename[0] = '\0';
	filedialog->m_ofn.nMaxFile = 256;

	if (filedialog->DoModal() == IDCANCEL) {
		delete filedialog;
		return;
	}

	delete filedialog;

	//

	load_from(filename,0);

	for(int i=0;i<19;i++)
	{
		lsl[i]->SetPos(lslpos[i]);
		rsl[i]->SetPos(rslpos[i]);
	}

    m_list_param.ResetContent();

	for(paramlistelm *e = paramroot.elm;e != NULL;e = e->next)
	{
		int indx = m_list_param.AddString(e->getString());
		m_list_param.SetItemDataPtr(indx,e);
	}

	setBandsFromSlpos();

	not_applied = 0;
	m_but_apply.EnableWindow(FALSE);
}

void config_dialog::OnButOk() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	CDialog::OnOK();

	for(int i=0;i<19;i++)
	{
		last_lslpos[i] = lslpos[i];
		last_rslpos[i] = rslpos[i];
	}
	//if (not_applied) setBandsFromSlpos();

	lastparamroot.copy(paramroot);
}

void config_dialog::OnButSave() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	static char filter[] = "equalizer file (*.eq)\0*.eq\0All files (*.*)\0*.*\0\0";
	CFileDialog *filedialog;
	char filename[257];

	if (not_applied) {
		not_applied = 0;
		m_but_apply.EnableWindow(FALSE);
		setBandsFromSlpos();
	}

	filedialog = new CFileDialog(FALSE,".eq");

	filedialog->m_ofn.lpstrFilter = filter;
	filedialog->m_ofn.lpstrFile = filename;
	filename[0] = '\0';
	filedialog->m_ofn.nMaxFile = 256;

	if (filedialog->DoModal() == IDCANCEL) {
		delete filedialog;
		return;
	}

	delete filedialog;	

	//

	CFile f;
	CFileException e;

	if( !f.Open( filename, CFile::modeCreate | CFile::modeWrite, &e ) )
    {
		::MessageBox(NULL,"Couldn't open file","Error",MB_OK | MB_ICONEXCLAMATION);
		// error
		return;
    }

	char buf[1024];

	sprintf(buf,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			lslpos[0],lslpos[1],lslpos[2],lslpos[3],lslpos[4],lslpos[5],
			lslpos[6],lslpos[7],lslpos[8],lslpos[9],lslpos[10],lslpos[11],
			lslpos[12],lslpos[13],lslpos[14],lslpos[15],lslpos[16],lslpos[17],
			lslpos[18],
			rslpos[0],rslpos[1],rslpos[2],rslpos[3],rslpos[4],rslpos[5],
			rslpos[6],rslpos[7],rslpos[8],rslpos[9],rslpos[10],rslpos[11],
			rslpos[12],rslpos[13],rslpos[14],rslpos[15],rslpos[16],rslpos[17],
			rslpos[18]);

	f.Write(buf,strlen(buf));

	for(paramlistelm *p = paramroot.elm;p != NULL;p = p->next)
	{
		sprintf(buf,"%.20g %.20g %.20g %d %d\n",
			(double)p->upper,(double)p->lower,(double)p->gain,(int)p->left,(int)p->right);
		f.Write(buf,strlen(buf));
	}
	
	f.Close();
}

BOOL config_dialog::OnInitDialog() 
{
	int i;
	extern int enable,dither;

	m_ch_enable = enable;
	m_ch_dither = dither;
	m_ch_lock = locklr;

	CDialog::OnInitDialog();

	rsl[0] = &m_rpre;
	rsl[1] = &m_r1;
	rsl[2] = &m_r2;
	rsl[3] = &m_r3;
	rsl[4] = &m_r4;
	rsl[5] = &m_r5;
	rsl[6] = &m_r6;
	rsl[7] = &m_r7;
	rsl[8] = &m_r8;
	rsl[9] = &m_r9;
	rsl[10] = &m_r10;
	rsl[11] = &m_r11;
	rsl[12] = &m_r12;
	rsl[13] = &m_r13;
	rsl[14] = &m_r14;
	rsl[15] = &m_r15;
	rsl[16] = &m_r16;
	rsl[17] = &m_r17;
	rsl[18] = &m_r18;

	lsl[0] = &m_lpre;
	lsl[1] = &m_l1;
	lsl[2] = &m_l2;
	lsl[3] = &m_l3;
	lsl[4] = &m_l4;
	lsl[5] = &m_l5;
	lsl[6] = &m_l6;
	lsl[7] = &m_l7;
	lsl[8] = &m_l8;
	lsl[9] = &m_l9;
	lsl[10] = &m_l10;
	lsl[11] = &m_l11;
	lsl[12] = &m_l12;
	lsl[13] = &m_l13;
	lsl[14] = &m_l14;
	lsl[15] = &m_l15;
	lsl[16] = &m_l16;
	lsl[17] = &m_l17;
	lsl[18] = &m_l18;

	for(i=0;i<19;i++)
	{
		lsl[i]->SetRangeMin( 0);
		lsl[i]->SetRangeMax(96);
		//lsl[i+1]->SetPos(lslpos[i+1] = (lbands[i] == 0 ? 96 : (int)log10(lbands[i]/lpreamp)*-20));
		lsl[i]->SetPos(lslpos[i]);
		last_lslpos[i] = lslpos[i];

		rsl[i]->SetRangeMin( 0);
		rsl[i]->SetRangeMax(96);
		//rsl[i+1]->SetPos(rslpos[i+1] = (rbands[i] == 0 ? 96 : (int)log10(rbands[i]/rpreamp)*-20));
		rsl[i]->SetPos(rslpos[i]);
		last_rslpos[i] = rslpos[i];
	}
		
	m_but_apply.EnableWindow(FALSE);

	for(paramlistelm *e = paramroot.elm;e != NULL;e = e->next)
	{
		int indx = m_list_param.AddString(e->getString());
		m_list_param.SetItemDataPtr(indx,e);
	}
	
	m_ed_autoload.SetWindowText(autoloadfile);
	
	last_param_index = -1;

	set_gui_from_paramlistelm(-1);
	
	lastparamroot.copy(paramroot);
	
	m_ed_target.SetWindowText("2002/03/08  http://shibatch.sourceforge.net/");
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void config_dialog::OnChEnable() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	extern int enable;
	enable = m_ch_enable2.GetCheck();
}

void config_dialog::OnChLock() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	locklr = m_ch_lock2.GetCheck();
}

void config_dialog::OnButUp() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	int i;

	for(i=1;i<19;i++)
		if (lslpos[i] == 0 || rslpos[i] == 0) return;
	
	for(i=1;i<19;i++)
	{
		lslpos[i]--;
		rslpos[i]--;
		lsl[i]->SetPos(lslpos[i]);
		rsl[i]->SetPos(rslpos[i]);
	}

	not_applied = 1;
	m_but_apply.EnableWindow(TRUE);

#if 0
	for(paramlistelm *e = paramroot.elm;e != NULL;e = e->next)
		e->gain = e->gain + 1;

	sort_param();
#endif
}

void config_dialog::OnButDown() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	int i;

	for(i=1;i<19;i++)
		if (lslpos[i] == 96 || rslpos[i] == 96) return;
	
	for(i=1;i<19;i++)
	{
		lslpos[i]++;
		rslpos[i]++;
		lsl[i]->SetPos(lslpos[i]);
		rsl[i]->SetPos(rslpos[i]);
	}

	not_applied = 1;
	m_but_apply.EnableWindow(TRUE);

#if 0
	for(paramlistelm *e = paramroot.elm;e != NULL;e = e->next)
		e->gain = e->gain - 1;

	sort_param();
#endif
}

void config_dialog::OnButSdef() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	static char filter[] = "equalizer file (*.eq)\0*.eq\0All files (*.*)\0*.*\0\0";
	CFileDialog *filedialog;

	filedialog = new CFileDialog(TRUE,".eq");

	filedialog->m_ofn.lpstrFilter = filter;
	filedialog->m_ofn.lpstrFile = autoloadfile;
	autoloadfile[0] = '\0';
	filedialog->m_ofn.nMaxFile = 256;

	if (filedialog->DoModal() == IDCANCEL) {
		delete filedialog;
		return;
	}

	delete filedialog;

	m_ed_autoload.SetWindowText(autoloadfile);
}

void config_dialog::OnButApply() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	if (not_applied) {
		not_applied = 0;
		m_but_apply.EnableWindow(FALSE);
		setBandsFromSlpos();
	}
}

int config_dialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//BringWindowToTop();

	// TODO: この位置に固有の作成用コードを追加してください
	
	return 0;
}

void config_dialog::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	int i;

	for(i=0;i<19;i++)
	{
		lslpos[i] = last_lslpos[i];
		rslpos[i] = last_rslpos[i];
	}

	paramroot.copy(lastparamroot);
	
	setBandsFromSlpos();	
}

void config_dialog::OnButPnew() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	paramlistelm *e = paramroot.newelm();
	int indx = m_list_param.AddString(e->getString());
	m_list_param.SetItemDataPtr(indx,e);

	last_param_index = indx;

	set_gui_from_paramlistelm(indx);
	m_list_param.SetCurSel(indx);
	m_but_apply.EnableWindow(TRUE);
	not_applied = 1;
}

void config_dialog::OnButPdel() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	int indx = m_list_param.GetCurSel();
	if (indx == LB_ERR) return;
	paramlistelm *e = (paramlistelm *)m_list_param.GetItemDataPtr(indx);
	m_list_param.DeleteString(indx);
	paramroot.delelm(e);
	
	last_param_index = -1;
	set_gui_from_paramlistelm(-1);
	m_but_apply.EnableWindow(TRUE);
	not_applied = 1;
}

void config_dialog::OnChangeEdGain() 
{
	// TODO: これが RICHEDIT コントロールの場合、コントロールは、 lParam マスク
	// 内での論理和の ENM_CHANGE フラグ付きで EM_SETEVENTMASK
	// メッセージをコントロールへ送るために CDialog::OnInitDialog() 関数をオーバー
	// ライドしない限りこの通知を送りません。
	
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
}

void config_dialog::OnChangeEdPlow() 
{
	// TODO: これが RICHEDIT コントロールの場合、コントロールは、 lParam マスク
	// 内での論理和の ENM_CHANGE フラグ付きで EM_SETEVENTMASK
	// メッセージをコントロールへ送るために CDialog::OnInitDialog() 関数をオーバー
	// ライドしない限りこの通知を送りません。
	
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
}

void config_dialog::OnChangeEdPup() 
{
	// TODO: これが RICHEDIT コントロールの場合、コントロールは、 lParam マスク
	// 内での論理和の ENM_CHANGE フラグ付きで EM_SETEVENTMASK
	// メッセージをコントロールへ送るために CDialog::OnInitDialog() 関数をオーバー
	// ライドしない限りこの通知を送りません。
	
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
}

void config_dialog::display(char *mes)
{
	m_ed_target.SetWindowText(mes);
}

void config_dialog::OnChDither() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	extern int dither;
	dither = m_ch_dither2.GetCheck();	
}

void config_dialog::OnSelchangeListParam() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	if (last_param_index != -1) set_paramlistelm_from_gui(last_param_index);

	last_param_index = m_list_param.GetCurSel();
	if (last_param_index == LB_ERR) last_param_index = -1;

	set_gui_from_paramlistelm(last_param_index);
}

void config_dialog::OnKillfocusEdPlow() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	set_paramlistelm_from_gui(last_param_index);
}

void config_dialog::OnKillfocusEdPup() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	set_paramlistelm_from_gui(last_param_index);
}

void config_dialog::OnKillfocusEdGain() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	set_paramlistelm_from_gui(last_param_index);
}

void config_dialog::OnChPleft() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	set_paramlistelm_from_gui(last_param_index);
}

void config_dialog::OnChPright() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	set_paramlistelm_from_gui(last_param_index);
}

void config_dialog::OnChangeEdAutoload() 
{
	// TODO: これが RICHEDIT コントロールの場合、コントロールは、 lParam マスク
	// 内での論理和の ENM_CHANGE フラグ付きで EM_SETEVENTMASK
	// メッセージをコントロールへ送るために CDialog::OnInitDialog() 関数をオーバー
	// ライドしない限りこの通知を送りません。
	
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

	m_ed_autoload.GetWindowText(autoloadfile,256);
}
