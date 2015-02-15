#if !defined(AFX_CONFIG_DIALOG_H__B30E796E_C2D1_11D3_9ECA_00A024503B95__INCLUDED_)
#define AFX_CONFIG_DIALOG_H__B30E796E_C2D1_11D3_9ECA_00A024503B95__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// config_dialog.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// config_dialog ダイアログ

class config_dialog : public CDialog
{
// コンストラクション
public:
	void display(char *);
	config_dialog(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void set_paramlistelm_from_gui(int);
	void set_gui_from_paramlistelm(int);
	void sort_param(void);
	// ダイアログ データ
	//{{AFX_DATA(config_dialog)
	enum { IDD = IDD_DIALOG1 };
	CEdit	m_ed_autoload;
	CButton	m_but_pnew;
	CButton	m_ch_pright2;
	CButton	m_ch_pleft2;
	CButton	m_ch_dither2;
	CListBox	m_list_param;
	CEdit	m_ed_pup;
	CEdit	m_ed_plow;
	CEdit	m_ed_gain;
	CButton	m_but_apply;
	CButton	m_ch_lock2;
	CButton	m_ch_enable2;
	CEdit	m_ed_target;
	CSliderCtrl	m_rpre;
	CSliderCtrl	m_r9;
	CSliderCtrl	m_r8;
	CSliderCtrl	m_r7;
	CSliderCtrl	m_r6;
	CSliderCtrl	m_r5;
	CSliderCtrl	m_r4;
	CSliderCtrl	m_r3;
	CSliderCtrl	m_r2;
	CSliderCtrl	m_r18;
	CSliderCtrl	m_r17;
	CSliderCtrl	m_r16;
	CSliderCtrl	m_r15;
	CSliderCtrl	m_r14;
	CSliderCtrl	m_r13;
	CSliderCtrl	m_r12;
	CSliderCtrl	m_r11;
	CSliderCtrl	m_r10;
	CSliderCtrl	m_r1;
	CSliderCtrl	m_l9;
	CSliderCtrl	m_l8;
	CSliderCtrl	m_l7;
	CSliderCtrl	m_l6;
	CSliderCtrl	m_l5;
	CSliderCtrl	m_l4;
	CSliderCtrl	m_l3;
	CSliderCtrl	m_l2;
	CSliderCtrl	m_l18;
	CSliderCtrl	m_l17;
	CSliderCtrl	m_l16;
	CSliderCtrl	m_l15;
	CSliderCtrl	m_l14;
	CSliderCtrl	m_l13;
	CSliderCtrl	m_l12;
	CSliderCtrl	m_l11;
	CSliderCtrl	m_l10;
	CSliderCtrl	m_l1;
	CSliderCtrl	m_lpre;
	BOOL	m_ch_lock;
	BOOL	m_ch_enable;
	BOOL	m_ch_dither;
	BOOL	m_ch_pleft;
	BOOL	m_ch_pright;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(config_dialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(config_dialog)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButCancel();
	afx_msg void OnButLoad();
	afx_msg void OnButOk();
	afx_msg void OnButSave();
	virtual BOOL OnInitDialog();
	afx_msg void OnChEnable();
	afx_msg void OnChLock();
	afx_msg void OnButUp();
	afx_msg void OnButSdef();
	afx_msg void OnButDown();
	afx_msg void OnButApply();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnButPnew();
	afx_msg void OnButPdel();
	afx_msg void OnChangeEdGain();
	afx_msg void OnChangeEdPlow();
	afx_msg void OnChangeEdPup();
	afx_msg void OnChDither();
	afx_msg void OnSelchangeListParam();
	afx_msg void OnKillfocusEdPlow();
	afx_msg void OnKillfocusEdPup();
	afx_msg void OnKillfocusEdGain();
	afx_msg void OnChPleft();
	afx_msg void OnChPright();
	afx_msg void OnChangeEdAutoload();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdrawSlRpre(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlL1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSlL18(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CONFIG_DIALOG_H__B30E796E_C2D1_11D3_9ECA_00A024503B95__INCLUDED_)
#pragma once


// config_dialog dialog

class config_dialog : public CDialog
{
	DECLARE_DYNAMIC(config_dialog)

public:
	config_dialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~config_dialog();

// Dialog Data
	enum { IDD = IDD_EQUALIZER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
