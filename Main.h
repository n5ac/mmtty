//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba
//-----------------------------------------------------------------------------------------------------------------------------------------------
// This file is part of MMTTY.

// MMTTY is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// MMTTY is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License along with MMTTY.  If not, see 
// <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
//JA7UDE 0427
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include "ComLib.h"
#include "Sound.h"
#include "Comm.h"
#include "Ctnc.h"
#include "Cradio.h"
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#include "SendFile.h"
#define	USEPAL	1
#define	SQSCALE		1024	// SQ の表示スケール
#define	HELPNAME_A	"MMTTY.CHM"
#define	HELPNAME_B	"MMTTY.HLP"
//---------------------------------------------------------------------------
enum {
	RXM_HANDLE=0x0000,	// APP -> MMTTY
	RXM_REQHANDLE,
	RXM_EXIT,
	RXM_PTT,
	RXM_CHAR,

	RXM_WINPOS,		// 5
	RXM_WIDTH,
	RXM_REQPARA,
	RXM_SETBAUD,
	RXM_SETMARK,

	RXM_SETSPACE,	// 10
	RXM_SETSWITCH,
	RXM_SETHAM,
	RXM_SHOWSETUP,
	RXM_SETVIEW,

	RXM_SETSQLVL,	// 15
	RXM_SHOW,
	RXM_SETFIG,
	RXM_SETRESO,
	RXM_SETLPF,

	RXM_SETTXDELAY,	// 20
	RXM_UPDATECOM,
	RXM_SUSPEND,
	RXM_NOTCH,
	RXM_PROFILE,

	RXM_TIMER,		// 25
    RXM_ENBFOCUS,
    RXM_SETDEFFREQ,
    RXM_SETLENGTH,
    RXM_ENBSHARED,

	RXM_PTTFSK,		// 30	Added by JE3HHT on Sep.2010
	RXM_SOUNDSOURCE,// 31	Added by AA6YQ in 1.70F
//--------------------------------------
	TXM_HANDLE=0x8000,	// MMTTY -> APP
	TXM_REQHANDLE,
	TXM_START,
	TXM_CHAR,
	TXM_PTTEVENT,

	TXM_HEIGHT,
	TXM_BAUD,
	TXM_MARK,
	TXM_SPACE,
	TXM_SWITCH,

	TXM_VIEW,
	TXM_LEVEL,
	TXM_FIGEVENT,
	TXM_RESO,
	TXM_LPF,

	TXM_THREAD,
	TXM_PROFILE,
	TXM_NOTCH,
	TXM_DEFSHIFT,
	TXM_RADIOFREQ,

    TXM_SHOWSETUP,
    TXM_SHOWPROFILE,
};
#pragma option -a-	// Pack of structer
typedef struct {
	LONG	smpFreq;	// MMTTY->APP
	CHAR	title[128];	// APP->MMTTY
	CHAR	comName[16];// APP->MMTTY
	LONG	smpFFT;		// APP->MMTTY 0-11025, 1-8000

	LONG	flagFFT;
	LONG	arrayFFT[2048];
	LONG	flagXY;
	LONG	arrayX[XYCOLLECT];
	LONG	arrayY[XYCOLLECT];

	CHAR	verMMTTY[16];
	CHAR	comRadio[16];
	LONG	flagLostSound;	// サウンドを一部失った(->APP)
	LONG	flagOverflow;	// 入力が大き過ぎる(->APP)
	LONG	errorClock;		// ppm
	LONG	smpDemFreq;		// デモジュレータのサンプリング周波数
	LONG	txBuffCount;	// 送信バッファの未送信文字数
	CHAR	ProfileName[16][64];
}COMARRAY;
#pragma option -a.

class TMmttyWd : public TForm
{
__published:	// IDE 管理のコンポーネント
	TTimer *Timer;
	TPanel *PanelRx;
	TPaintBox *PBoxRx;
	TScrollBar *ScrollBarRx;
	TPanel *PanelTop;
	TMainMenu *MainMenu;
	TMenuItem *KMFile;
	TGroupBox *GroupCtr;
	TSpeedButton *SBFIG;
	TSpeedButton *SBUOS;
	TGroupBox *GroupDem;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *LTap;
	TSpeedButton *SBRev;
	TSpeedButton *SBHAM;
	TSpeedButton *SBATC;
	TSpeedButton *SBSQ;
	TSpeedButton *SBAFC;
	TComboBox *MarkFreq;
	TComboBox *ShiftFreq;
	TPanel *PanelFFT;
	TPaintBox *PBoxFFTIN;
	TPanel *PanelWater;
	TPaintBox *PBoxWater;
	TPanel *PanelQSO;
	TPanel *PanelIn;
	TPanel *PanelStat;
	TLabel *LCall;
	TEdit *HisCall;
	TLabel *LName;
	TEdit *HisName;
	TLabel *LHIS;
	TComboBox *HisRST;
	TComboBox *MyRST;
	TSpeedButton *SBQSO;
	TPaintBox *PBoxIn;
	TScrollBar *ScrollBarIn;
	TLabel *LMY;
	TSpeedButton *SBINClear;
	TSpeedButton *SBIN1;
	TSpeedButton *SBIN2;
	TSpeedButton *SBIN3;
	TSpeedButton *SBIN4;
	TComboBox *MsgList;
	TSpeedButton *SBINEdit;
	TSpeedButton *SBTXOFF;
	TSpeedButton *SBTX;
	TMenuItem *KMEdit;
	TMenuItem *KView;
	TMenuItem *KMOpt;
	TMenuItem *KMHelp;
	TMenuItem *KOption;
	TGroupBox *GroupM;
	TSpeedButton *SBM1;
	TSpeedButton *SBM2;
	TSpeedButton *SBM3;
	TSpeedButton *SBM4;
	TSpeedButton *SBM5;
	TSpeedButton *SBM6;
	TSpeedButton *SBM7;
	TSpeedButton *SBM8;
	TSpeedButton *SBM9;
	TSpeedButton *SBM10;
	TSpeedButton *SBM11;
	TSpeedButton *SBM12;
	TSpeedButton *SBM13;
	TSpeedButton *SBM14;
	TSpeedButton *SBM15;
	TSpeedButton *SBM16;
	TMenuItem *KOS;
	TMenuItem *KFFT;
	TMenuItem *KMacro;
	TMenuItem *KMsg;
	TMenuItem *KWebHHT;
	TMenuItem *N1;
	TMenuItem *KHlpTxt;
	TMenuItem *KVol;
	TMenuItem *KRcvLog;
	TMenuItem *KFileOut;
	TOpenDialog *OpenDialog;
	TMenuItem *N2;
	TMenuItem *KExit;
	TMenuItem *N3;
	TMenuItem *KLog;
	TMenuItem *KQSO;
	TSpeedButton *SBFind;
	TSpeedButton *SBData;
	TEdit *HisQTH;
	TSpeedButton *SBInit;
	TMenuItem *N5;
	TMenuItem *KOpenLog;
	TMenuItem *KFlush;
	TMenuItem *KTest;
	TMenuItem *N4;
	TMenuItem *KHlpLog;
	TSaveDialog *SaveDialog;
	TMenuItem *KSaveRx;
	TMenuItem *KFFTFW;
	TMenuItem *N6;
	TMenuItem *KFFTWA;
	TMenuItem *KFFTW1;
	TMenuItem *KFFTW2;
	TMenuItem *KFFTW3;
	TMenuItem *KFFTW05;
	TMenuItem *KFFTW15;
	TSpeedButton *SBBPF;
	TLabel *LLPF;
	TLabel *Label7;
	TMenuItem *N7;
	TMenuItem *KDispVer;
	TSpeedButton *SBLMS;
	TLabel *LWait;
	TTrackBar *TBCharWait;
	TMenuItem *N8;
	TMenuItem *KClrRx;
	TPanel *PanelXY;
	TPaintBox *PBoxXY;
	TMenuItem *KXYScope;
	TLabel *LHZ;
	TSpeedButton *SBNET;
	TMenuItem *N9;
	TMenuItem *KRxRec;
	TMenuItem *KRxPlay;
	TMenuItem *KRxStop;
	TMenuItem *KXYScopeSize;
	TMenuItem *KXYSmall;
	TMenuItem *KXYBig;
	TMenuItem *KRxTime;
	TMenuItem *KRxPause;
	TSpeedButton *SBDem;
	TMenuItem *KRxTop;
	TMenuItem *KRxPos;
	TMenuItem *KHlpDig;
	TMenuItem *KShortCut;
	TMenuItem *N10;
	TMenuItem *KHint;
	TMenuItem *KOptLog;
	TMenuItem *KPanel;
	TMenuItem *N11;
	TMenuItem *KPanelSize;
	TMenuItem *KPanelSmall;
	TMenuItem *KPanelBig;
	TMenuItem *KPanelMedium;
	TMenuItem *KExtCmd;
	TMenuItem *KExtCmd1;
	TMenuItem *KExtCmd2;
	TMenuItem *KExtCmd3;
	TMenuItem *KExtCmd4;
	TMenuItem *N12;
	TMenuItem *KExtEnt;
	TMenuItem *KExtEnt1;
	TMenuItem *KExtEnt2;
	TMenuItem *KExtEnt3;
	TMenuItem *KExtEnt4;
	TMenuItem *N13;
	TMenuItem *KExtReset;
	TMenuItem *KExtSusp;
	TMenuItem *KExtDel;
	TMenuItem *KExtDel1;
	TMenuItem *KExtDel2;
	TMenuItem *KExtDel3;
	TMenuItem *KExtDel4;
	TPanel *PanelSQ;
	TPaintBox *PBoxSQ;
	TMenuItem *KPanelSuper;
	TMenuItem *KXYScopeQuality;
	TMenuItem *KXYQuality1;
	TMenuItem *KXYQuality2;
	TMenuItem *KXYQuality3;
	TMenuItem *KXYQuality4;
	TMenuItem *KFFTGain;
	TMenuItem *KFFTGL;
	TMenuItem *KFFTGM;
	TMenuItem *KFFTGH;
	TMenuItem *KFFTGT;
	TMenuItem *KWebJARTS;
	TMenuItem *KWebGRP;
	TMenuItem *KWebENG;
	TComboBox *Freq;
	TMenuItem *N14;
	TMenuItem *KSetHelp;
	TMenuItem *N15;
	TMenuItem *KSend;
	TMenuItem *KRun;
	TMenuItem *KSChar;
	TMenuItem *KSWord;
	TMenuItem *KSLine;
	TMenuItem *KFFTR;
	TMenuItem *KFFTRS;
	TMenuItem *KFFTRM;
	TMenuItem *KFFTRF;
	TMenuItem *KPaste;
	TMenuItem *N16;
	TMenuItem *KW7TI;
	TMenuItem *KWebWSK;
	TMenuItem *N17;
	TMenuItem *KTNC;
	TMenuItem *KRM;
	TMenuItem *KMac;
	TMenuItem *N18;
	TMenuItem *KENT;
	TMenuItem *N19;
	TMenuItem *N20;
	TMenuItem *KFFTGS1;
	TMenuItem *KFFTGS2;
	TMenuItem *KFFTGS3;
	TMenuItem *KFFTGS4;
	TPanel *PanelMac;
	TMenuItem *KWP;
	TMenuItem *KS;
	TMenuItem *KS1;
	TMenuItem *KS2;
	TMenuItem *KS3;
	TMenuItem *KS4;
	TMenuItem *KS5;
	TMenuItem *KS6;
	TMenuItem *KS7;
	TMenuItem *KS8;
	TMenuItem *N21;
	TMenuItem *KSDel;
	TMenuItem *KSEnt;
	TMenuItem *KDS1;
	TMenuItem *KDS2;
	TMenuItem *KDS3;
	TMenuItem *KDS4;
	TMenuItem *KDS5;
	TMenuItem *KDS6;
	TMenuItem *KDS7;
	TMenuItem *KDS8;
	TMenuItem *KES1;
	TMenuItem *KES2;
	TMenuItem *KES3;
	TMenuItem *KES4;
	TMenuItem *KES5;
	TMenuItem *KES6;
	TMenuItem *KES7;
	TMenuItem *KES8;
	TMenuItem *N22;
	TMenuItem *KSSV;
	TMenuItem *KSLD;
	TMenuItem *N23;
	TMenuItem *KSDEF;
	TMenuItem *KSRET;
	TMenuItem *KPttTim;
	TMenuItem *KVolIn;
	TMenuItem *KRO;
	TMenuItem *KROT;
	TMenuItem *KROF;
	TMenuItem *KROTU;
	TMenuItem *KROTD;
	TMenuItem *N24;
	TPopupMenu *PopupC;
	TComboBox *DemoBox2;
	TComboBox *DemoBox1;
	TMenuItem *KTX;
	TMenuItem *N25;
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	
	void __fastcall PBoxFFTINPaint(TObject *Sender);
	void __fastcall PBoxWaterPaint(TObject *Sender);

	void __fastcall SBFIGClick(TObject *Sender);
	void __fastcall MarkFreqChange(TObject *Sender);
	void __fastcall ShiftFreqChange(TObject *Sender);
	
	
	void __fastcall SBUOSClick(TObject *Sender);
	

	
	
	
	
	
	
	void __fastcall PBoxRxPaint(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	
	void __fastcall SBRevClick(TObject *Sender);
	
	
	
	
	void __fastcall SBATCClick(TObject *Sender);
	
	
	
	void __fastcall SBSQClick(TObject *Sender);
	
	void __fastcall SBTXClick(TObject *Sender);


	
	
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	
	
	
	
	void __fastcall PBoxRxMouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall FormKeyPress(TObject *Sender, char &Key);

	
	void __fastcall ScrollBarRxChange(TObject *Sender);
	void __fastcall SBAFCClick(TObject *Sender);
	
	
	void __fastcall PBoxInPaint(TObject *Sender);
	void __fastcall ScrollBarInChange(TObject *Sender);
	void __fastcall SBTXOFFClick(TObject *Sender);
	void __fastcall SBINClearClick(TObject *Sender);
	void __fastcall SBIN1Click(TObject *Sender);
	void __fastcall SBIN2Click(TObject *Sender);
	void __fastcall SBIN4Click(TObject *Sender);
	void __fastcall SBIN3Click(TObject *Sender);
	void __fastcall MsgListChange(TObject *Sender);
	void __fastcall SBINEditClick(TObject *Sender);
	void __fastcall KOptionClick(TObject *Sender);
	void __fastcall HisCallKeyPress(TObject *Sender, char &Key);
	void __fastcall SBMClick(TObject *Sender);
	void __fastcall KOSClick(TObject *Sender);
	void __fastcall KFFTClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	
	
	void __fastcall TimerTimer(TObject *Sender);

	
	void __fastcall KWebHHTClick(TObject *Sender);
	void __fastcall KHlpTxtClick(TObject *Sender);
	void __fastcall KVolClick(TObject *Sender);
	
	
	void __fastcall KRcvLogClick(TObject *Sender);
	void __fastcall KFileOutClick(TObject *Sender);
	void __fastcall KExitClick(TObject *Sender);
	
	void __fastcall KLogClick(TObject *Sender);
	void __fastcall KQSOClick(TObject *Sender);
	void __fastcall SBQSOClick(TObject *Sender);
	void __fastcall SBFindClick(TObject *Sender);
	void __fastcall HisCallChange(TObject *Sender);

	void __fastcall LNameClick(TObject *Sender);
	void __fastcall SBInitClick(TObject *Sender);
	void __fastcall PBoxInClick(TObject *Sender);
	void __fastcall KTestClick(TObject *Sender);
	void __fastcall SBHAMClick(TObject *Sender);
	void __fastcall KOpenLogClick(TObject *Sender);
	void __fastcall KFlushClick(TObject *Sender);
	void __fastcall KHlpLogClick(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall KSaveRxClick(TObject *Sender);
	void __fastcall KFFTWAClick(TObject *Sender);
	void __fastcall KFFTW1Click(TObject *Sender);
	void __fastcall KFFTW2Click(TObject *Sender);
	void __fastcall KFFTW3Click(TObject *Sender);
	void __fastcall KFFTW05Click(TObject *Sender);
	void __fastcall KFFTW15Click(TObject *Sender);
	void __fastcall SBBPFClick(TObject *Sender);
	
	void __fastcall FormShow(TObject *Sender);
	
	
	void __fastcall KDispVerClick(TObject *Sender);
	void __fastcall SBLMSClick(TObject *Sender);
	void __fastcall TBCharWaitChange(TObject *Sender);
	void __fastcall KClrRxClick(TObject *Sender);
	void __fastcall KXYScopeClick(TObject *Sender);
	void __fastcall PBoxXYPaint(TObject *Sender);
	
	void __fastcall SBNETClick(TObject *Sender);
	void __fastcall KRxRecClick(TObject *Sender);
	void __fastcall KRxPlayClick(TObject *Sender);
	void __fastcall KRxStopClick(TObject *Sender);
	void __fastcall KXYSmallClick(TObject *Sender);
	void __fastcall KXYBigClick(TObject *Sender);
	void __fastcall KRxTimeClick(TObject *Sender);
	void __fastcall KRxPauseClick(TObject *Sender);
	void __fastcall SBDemClick(TObject *Sender);
	void __fastcall KRxTopClick(TObject *Sender);
	
	
	void __fastcall KRxPosClick(TObject *Sender);
	void __fastcall KHlpDigClick(TObject *Sender);
	void __fastcall KShortCutClick(TObject *Sender);
	void __fastcall KHintClick(TObject *Sender);

	void __fastcall KOptLogClick(TObject *Sender);
	void __fastcall KPanelClick(TObject *Sender);
	void __fastcall KPanelSmallClick(TObject *Sender);
	void __fastcall KPanelBigClick(TObject *Sender);
	void __fastcall KPanelMediumClick(TObject *Sender);
	void __fastcall KExtEnt1Click(TObject *Sender);
	void __fastcall KExtEnt2Click(TObject *Sender);
	void __fastcall KExtEnt3Click(TObject *Sender);
	void __fastcall KExtEnt4Click(TObject *Sender);
	void __fastcall KExtCmd1Click(TObject *Sender);
	void __fastcall KExtCmd2Click(TObject *Sender);
	void __fastcall KExtCmd3Click(TObject *Sender);
	void __fastcall KExtCmd4Click(TObject *Sender);
	void __fastcall KExtResetClick(TObject *Sender);
	void __fastcall KExtSuspClick(TObject *Sender);
	
	
	
	
	void __fastcall KExtDel1Click(TObject *Sender);
	void __fastcall KExtDel2Click(TObject *Sender);
	void __fastcall KExtDel3Click(TObject *Sender);
	void __fastcall KExtDel4Click(TObject *Sender);
	void __fastcall SBMMouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall SBIN1MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall SBIN2MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall SBIN3MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall SBIN4MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);

	void __fastcall PBoxSQMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall PBoxSQPaint(TObject *Sender);
	void __fastcall KPanelSuperClick(TObject *Sender);
	void __fastcall LLPFClick(TObject *Sender);
	void __fastcall KXYQuality3Click(TObject *Sender);
	void __fastcall KXYQuality2Click(TObject *Sender);
	void __fastcall KXYQuality1Click(TObject *Sender);
	void __fastcall KXYQuality4Click(TObject *Sender);
	
	
	void __fastcall Label3Click(TObject *Sender);
	void __fastcall KFFTGLClick(TObject *Sender);
	void __fastcall KFFTGMClick(TObject *Sender);
	void __fastcall KFFTGHClick(TObject *Sender);
	void __fastcall KFFTGTClick(TObject *Sender);
	void __fastcall KWebJARTSClick(TObject *Sender);
	void __fastcall KWebGRPClick(TObject *Sender);
	void __fastcall KWebENGClick(TObject *Sender);
	void __fastcall LCallClick(TObject *Sender);
	void __fastcall KSetHelpClick(TObject *Sender);


	void __fastcall KSCharClick(TObject *Sender);
	void __fastcall KSWordClick(TObject *Sender);
	void __fastcall KSLineClick(TObject *Sender);
	void __fastcall FreqChange(TObject *Sender);
	void __fastcall KFFTRSClick(TObject *Sender);
	void __fastcall KFFTRMClick(TObject *Sender);
	void __fastcall KFFTRFClick(TObject *Sender);
	void __fastcall KPasteClick(TObject *Sender);
	void __fastcall KW7TIClick(TObject *Sender);
	void __fastcall KWebWSKClick(TObject *Sender);
	void __fastcall KTNCClick(TObject *Sender);
	void __fastcall KRMClick(TObject *Sender);
	
	void __fastcall LTapClick(TObject *Sender);
	
	void __fastcall KMacClick(TObject *Sender);
	void __fastcall KENTClick(TObject *Sender);

	void __fastcall KFFTGS1Click(TObject *Sender);
	void __fastcall KFFTGS2Click(TObject *Sender);
	void __fastcall KFFTGS4Click(TObject *Sender);
	void __fastcall KFFTGS3Click(TObject *Sender);
	void __fastcall PBoxFFTINMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall HisCallClick(TObject *Sender);
	void __fastcall SBLMSMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall SBFindMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall LWaitClick(TObject *Sender);
	void __fastcall KWPClick(TObject *Sender);
	void __fastcall SBDataMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall KESClick(TObject *Sender);
	void __fastcall KDSClick(TObject *Sender);
	void __fastcall KSClick(TObject *Sender);
	void __fastcall KSSVClick(TObject *Sender);
	void __fastcall KSLDClick(TObject *Sender);
	void __fastcall KSDEFClick(TObject *Sender);
	void __fastcall KSRETClick(TObject *Sender);
	void __fastcall KPttTimClick(TObject *Sender);
	
	
	void __fastcall KROFClick(TObject *Sender);
	void __fastcall KROTClick(TObject *Sender);
	void __fastcall KROTUClick(TObject *Sender);
	void __fastcall KROTDClick(TObject *Sender);
	void __fastcall KMFileClick(TObject *Sender);
	void __fastcall HisCallDblClick(TObject *Sender);
	void __fastcall LCallMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y);
	void __fastcall DemoBox2Change(TObject *Sender);
	void __fastcall DemoBox1Change(TObject *Sender);
	
	
	void __fastcall SBTXMouseDown(TObject *Sender, TMouseButton Button,
		  TShiftState Shift, int X, int Y);
	void __fastcall KTXClick(TObject *Sender);
	void __fastcall KMOptClick(TObject *Sender);
	void __fastcall KViewClick(TObject *Sender);
	void __fastcall KExtCmdClick(TObject *Sender);


private:	// ユーザー宣言
	WAVEFORMAT	wfm;

	Graphics::TBitmap *pBitmapFFTIN;
	Graphics::TBitmap *pBitmapWater;
	Graphics::TBitmap *pBitmapXY;
	Graphics::TBitmap *pBitmapSQ;

	Graphics::TBitmap *pBitmapRx;
	Graphics::TBitmap *pBitmapIn;

	CAlignList	AlignMain;
	CAlignList	AlignTop;
	CAlignList	AlignQSO;
	CAlignList	AlignStat;

	CWebRef		WebRef;

	CRTTY		rtty;
	CRTTY		rttysub;
	CFifoEdit	FifoEdit;
	CComm		*pComm;
	CCtnc		*pTnc;
	CCradio		*pRadio;

	int			m_ReqRecv;
	int			m_DemType;
	int			m_LpfType;
	int			m_SendWay;

	double		m_RxMarkFreq;
	double		m_RxSpaceFreq;
	void __fastcall UpdateNet(void);
	void __fastcall UpdateLogLink(void);

	void __fastcall UpdateColor(void);

	void __fastcall WndProc(TMessage &Message);
	void __fastcall WndCopyData(TMessage &Message);
	void __fastcall EntryAlignControl(void);
	void __fastcall OnIdle(TObject *Sender, bool &Done);
	void __fastcall OnMSG(tagMSG &Msg, bool &Handled);
	void __fastcall OnActivate(TObject *Sender);
	void __fastcall OnRestore(TObject *Sender);

	TSpeedButton *__fastcall GetSB(int n);
	int __fastcall GetMacroIndex(TObject *Sender);
	void __fastcall UpdateControlPanel(void);
	void __fastcall UpdateXY2(void);
	void __fastcall AlignLogPanel(void);
	void __fastcall UpdatePanel(void);
	void __fastcall UpdateItem(void);
	void __fastcall UpdateMacro(void);
	void __fastcall UpdateLMS(void);
	void __fastcall UpdateUI(void);
	void __fastcall ReadSampFreq(void);
	void __fastcall ReadCombList(TMemIniFile *tp, AnsiString &as, LPCSTR pKey, LPCSTR pDef);
	void __fastcall ReadRegister(void);
	void __fastcall WriteRegister(void);
	void __fastcall StoreCWID(LPSTR &tt, char c, int &nc, int size);
	void __fastcall ConvString(LPSTR t, LPCSTR p, int size, int sw);
	void __fastcall SetGreetingString(LPSTR t, LPCSTR pCall, int type);
	void __fastcall UpdateFSK(void);
	void __fastcall OpenCloseCom(void);

	void __fastcall DoOption(TObject *Sender, int sw);
	void __fastcall SetFSKPara(COMMPARA *cp);
	void __fastcall OpenClosePTT(void);
	void __fastcall WriteFifoEdit(LPCSTR p);
	void __fastcall PushKey(char Key);

	void __fastcall TimerFocus(void);
	void __fastcall XMIT(int sw);
	void __fastcall ToTX(LPCSTR p);
	void __fastcall ToRX(int sw);
	void __fastcall FindMsgList(AnsiString fs);
	void __fastcall FindMsgLst(WORD nKey);
	void __fastcall UpdateMsgList(void);
	int __fastcall OutputFile(LPCSTR pName);

	void __fastcall SetShift(double sf);
	void __fastcall UpdateLogData(void);
	void __fastcall UpdateTextData(void);
	int __fastcall DrawXY(void);
	void __fastcall UpdateXYScope(void);
	void __fastcall UpdateXYQuality(void);
	void __fastcall UpdateCallsign(void);

	void __fastcall RecvJob(void);

	void __fastcall CallCapture(void);
	void __fastcall QSOIN(void);
	void __fastcall QSOOUT(int sw);

	CINTPXY2	i2m;
	CINTPXY2	i2s;
	CINTPXY4FIR	i4m;
	CINTPXY4FIR	i4s;
	CINTPXY8FIR	i8m;
	CINTPXY8FIR	i8s;

	int		m_XYSize;
	int		m_XYQuality;
	int		m_PanelSize;

	int		m_InitFirst;
	int		m_DisAlign;
	int		m_BefHeight;
	int		m_DisEvent;
	int		m_MacroStat;
	int		m_Macro;
	int		m_PaletteQuery;
	int		m_NameQTH;
	int		m_Ctrl;
	int		m_ReqPaletteChange;
	int		m_PaletteTimer;
	int		m_Dupe;
	int		m_Running;
	int		m_Baud;
	int		m_RemoteTimer;
	int		m_RemoteDelay;

	int		m_OptionDialog;
    HWND	m_hOptionWnd;

	AnsiString	m_NowStr;

	int		m_filemode;
	int		m_FifoOutTimer;

	int		m_FIGTimer;
	void __fastcall UpdateLWait(void);
	int __fastcall IsTxIdle(void);
	int __fastcall GetTxBufCount(void);

	TMenuItem	*pAndyMenu;
	TMenuItem	*pAndyMenu2;
	void __fastcall KAndyHelp(TObject *Sender);
	void __fastcall AddHelpMenu(void);

	void __fastcall UpdateTitle(void);
	void __fastcall AutoLogSave(void);

// Profileのインプリメント
	TMenuItem *__fastcall GetKS(int n);
	TMenuItem *__fastcall GetKDS(int n);
	TMenuItem *__fastcall GetKES(int n);
	void __fastcall UpdateProfile(void);
	void __fastcall ReadProfileList(void);
	void __fastcall ReadProfile(int n, LPCSTR pName);
	void __fastcall WriteProfile(int n);
	void __fastcall WriteProfile(int n, LPCSTR pName, int Flag);
	void __fastcall RemoteProfile(DWORD flag);
	LPCSTR			m_pRemOpenName;

#if USEPAL
public:
	HPALETTE UsrPal;
private:
	HPALETTE OldPal;
	void __fastcall ClosePalette(void);
	void __fastcall SetupPalette(RGBQUAD *pTbl, int max);
	int __fastcall 	EntryColor(RGBQUAD *pTbl, TColor col, int n);
	void __fastcall SetColorIndex(void);

	DYNAMIC HPALETTE __fastcall GetPalette(void);
#endif

	CRecentMenu	RecentMenu;
	void __fastcall UpdateRecentMenu(void);
	void __fastcall RecentAdd(LPCSTR pNew, int f);
	void __fastcall OnRecentClick(TObject *Sender);

	void __fastcall SetHint(TControl *tp, WORD Key);
	void __fastcall SetShortCut(TMenuItem *mp, WORD Key);

	void __fastcall UpdateExtProgram(void);
	void __fastcall KExtEntClick(int n, TMenuItem *mp);
	void __fastcall KExtCmdClick(int n);

	void __fastcall SBINMouseUp(int n);

	void __fastcall DrawSQ(void);

	void __fastcall EditMB(int n);
	void __fastcall FindCall(void);
// PttTimerのインプリメント
	int			m_PttTimer;
	int			m_PttTimerCount;
	void __fastcall UpdatePttTimer(void);

// ファイル送信のインプリメント
	TSendFileDlg *p_SendFileDlg;

// BARTG
	int			m_BARTGTimer;
	void __fastcall UpdateBARTG(void);

public:		// ユーザー宣言
	__fastcall TMmttyWd(TComponent* Owner);
	__fastcall ~TMmttyWd();
	void __fastcall StartOption(void);

	CPrintText	PrintText;
	TSound		*pSound;

	void __fastcall FirstFileMapping(void);
	void __fastcall UpdateComarray(void);
	void __fastcall UpdateRemort(void);
	void __fastcall OutputStr(LPCSTR pOut);

	void __fastcall SetYourCallsign(LPCSTR pCall);
	void __fastcall SetYourName(LPCSTR pName);
	void __fastcall SetYourQTH(LPCSTR pQTH);
	void __fastcall SetYourRST(LPCSTR pRST);
	void __fastcall UpdateLimit(void);

	void __fastcall UpdateShortCut(BOOL bMenu);

	void __fastcall UpdateSystemFont(void);
	void __fastcall ReqPaletteChange(void);
	void __fastcall UpdateRev(void);
	void __fastcall PostApp(WPARAM wParam, LPARAM lParam);

//Macro Intvalのインプリメント
	void __fastcall ClearMacroTimer(void);
	void __fastcall MacBtnExec(int n);
	int		m_MacRetry;
	int		m_MacTimer;

	void __fastcall AdjustFocus(void);

//コールリストのインプリメント
#define	CALLLISTMAX	32
	void __fastcall AddCall(LPCSTR p);
	void __fastcall KCClick(TObject *Sender);

//復調器コンンボボックスのインプリメント
	AnsiString		m_asFilterTap;
	AnsiString		m_asVCOGain;
	AnsiString		m_asIIRBW;
	AnsiString		m_asBaud;
	AnsiString		m_asSmooth;
	AnsiString		m_asLoopFC;
	int				m_ShowBaud;
	void __fastcall SelectCombo(int sw);

//TNC模擬機能のインプリメント
	void __fastcall OnTncEvent(void);
	void __fastcall OpenCloseTNC(void);
	void __fastcall UpdateTNCPTT(void);
	void __fastcall SetNMMT(void);
	void __fastcall TncConvers(void);
	BOOL __fastcall RxCommon(BYTE c);
	void __fastcall RxTnc241(char c);
	void __fastcall RxKAM(char c);
	void __fastcall CmdTnc241(LPSTR p);
	void __fastcall CmdKAM(LPSTR p);
	void __fastcall PutTNC(char c);
	void __fastcall UpdateStayOnTop(void);
	void __fastcall UpdateMacroShow(int sw);

//リモート機能のインプリメント
	int			m_cRemoteTimeout;
	AnsiString	m_strCommonMemory;

	HANDLE	hMap;
	COMARRAY	*pMap;
	int		m_RemoteHeight;
	int		m_RemoteBaud;
	int		m_RemoteSwitch;
	int		m_RemoteView;
	int		m_RemoteIIR;
	int		m_RemoteLPF;
	int		m_RemoteMark;
	int		m_RemoteSpace;
	int		m_RemoteNotch;
	int		m_RemoteDefShift;

	void __fastcall RemoteFreq(void);
	void __fastcall InitRemoteStat(void);
	DWORD __fastcall GetRemoteSwitch(void);
	DWORD __fastcall GetRemoteView(void);
	void __fastcall RemoteStat(void);
	void __fastcall RemoteSwitch(DWORD d);
	void __fastcall RemoteView(DWORD d);
	void __fastcall RemoteSigLevel(DWORD d);
	void __fastcall RemoteMMTTY(tagMSG &Msg);
	void __fastcall SetRemoteFFT(void);
//Radio command機能のインプリメント
	void __fastcall OpenCloseRadio(void);

//	void __fastcall TopWindow(void);

	CTextFifo		m_cmdTNC;
	AnsiString		m_TNCStg;	// 送信保留データ
#define	CM_CTNC	    (WM_USER+400)		// Hook on WndProc
	void __fastcall OnTncEvent(TMessage &Message);

#define	CM_CRADIO   (WM_USER+401)
#define	CM_CMML		(WM_USER+402)
#define	CM_CMMR		(WM_USER+403)
protected:
	void __fastcall CMWheel(TMessage Message);
	void __fastcall CMMML(TMessage Message);
	void __fastcall CMMMR(TMessage Message);

BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_MOUSEWHEEL, TMessage, CMWheel)
	MESSAGE_HANDLER(CM_CMML, TMessage, CMMML)
	MESSAGE_HANDLER(CM_CMMR, TMessage, CMMMR)
END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern TMmttyWd *MmttyWd;
//---------------------------------------------------------------------------
#endif

