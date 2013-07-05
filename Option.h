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



//----------------------------------------------------------------------------
#ifndef OptionH
#define OptionH
//----------------------------------------------------------------------------
//JA7UDE 0427
#include <System.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Graphics.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>

#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include "MMLink.h"
#include "Rtty.h"
//----------------------------------------------------------------------------
class TOptionDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TPageControl *Page;
	TTabSheet *TabDem;
	TTabSheet *TabPara;
	TRadioGroup *BitLen;
	TRadioGroup *StopLen;
	TRadioGroup *ParitySel;
	TComboBox *BaudRate;
	TLabel *LBaudRate;
	TGroupBox *GrpDisc;
	TLabel *L6;
	TLabel *L7;
	TLabel *LTAPFW;
	TComboBox *MarkFreq;
	TComboBox *ShiftFreq;
	TComboBox *FilterTap;
	TLabel *Label9;
	TLabel *Label10;
	TGroupBox *GrpLPF;
	TLabel *LLPF;
	TComboBox *SmoozFreq;
	TLabel *Label12;
	TButton *HamBtn;
	TCheckBox *CheckInv;
	TTabSheet *TabTx;
	TGroupBox *GPTT;
	TComboBox *PortName;
	TLabel *L13;
	TCheckBox *PttInv;
	TGroupBox *GB5;
	TLabel *L14;
	TEdit *EditCall;
	TGroupBox *GB6;
	TCheckBox *CheckTXUOS;
	TTabSheet *TabFont;
	TEdit *FontName;
	TLabel *LFont;
	TGroupBox *GrpDispAdj;
	TLabel *LW;
	TEdit *FontAdjX;
	TEdit *FontAdjY;
	TLabel *LH;
	TFontDialog *FontDialog;
	TButton *FontBtn;
	TCheckBox *CheckZero;
	TCheckBox *CheckDblSft;
	TGroupBox *GB9;
	TGroupBox *GB10;
	TCheckBox *CheckTXBPF;
	TLabel *L25;
	TComboBox *TxBpfTap;
	TTrackBar *TBOutLvl;
	TLabel *LOutLvl;
	TSpeedButton *SBM1;
	TSpeedButton *SBM2;
	TSpeedButton *SBM3;
	TSpeedButton *SBM4;
	TSpeedButton *SBM8;
	TSpeedButton *SBM7;
	TSpeedButton *SBM6;
	TSpeedButton *SBM5;
	TSpeedButton *SBM9;
	TSpeedButton *SBM10;
	TSpeedButton *SBM11;
	TSpeedButton *SBM12;
	TSpeedButton *SBM13;
	TSpeedButton *SBM14;
	TSpeedButton *SBM15;
	TSpeedButton *SBM16;
	TGroupBox *GrpInBtn;
	TSpeedButton *SBIN1;
	TSpeedButton *SBIN2;
	TSpeedButton *SBIN3;
	TSpeedButton *SBIN4;
	TTabSheet *TabMisc;
	TCheckBox *CheckTXLPF;
	TComboBox *TxLpfFreq;
	TLabel *Label28;
	TLabel *L29;
	TGroupBox *GrpFilter;
	TButton *DemBpfBtn;
	TButton *PreBpfBtn;
	TSpeedButton *DispTxBpf;
	TCheckBox *CheckMajority;
	TCheckBox *CheckIgnoreFream;
	TGroupBox *GrpRx;
	TLabel *LRXB;
	TLabel *LRXC;
	TPanel *PCRXBack;
	TPanel *PCRXChar;
	TColorDialog *ColorDialog;
	TGroupBox *GrpIn;
	TLabel *LINB;
	TLabel *LINC;
	TPanel *PCINBack;
	TPanel *PCINChar;
	TLabel *LINS;
	TPanel *PCINTx;
	TCheckBox *CheckPalette;
	TPanel *PCLow;
	TPanel *PCHigh;
	TLabel *Label37;
	TLabel *Label38;
	TRadioGroup *TxPort;
	TPageControl *PageBPF;
	TTabSheet *TabBPF;
	TTabSheet *TabLMS;
	TLabel *LDelay;
	TComboBox *lmsDelay;
	TLabel *LMU2;
	TLabel *LGM;
	TComboBox *lmsMU2;
	TComboBox *lmsGM;
	TCheckBox *lmsAGC;
	TCheckBox *CheckRXBPFAFC;
	TLabel *LFW;
	TComboBox *RxBpfFW;
	TCheckBox *SelLMS;
	TCheckBox *SelBPF;
	TLabel *L31;
	TComboBox *lmsTap;
	TLabel *L30;
	TComboBox *RxBpfTap;
	TLabel *LCharW;
	TTrackBar *TBCharWait;
	TLabel *Label40;
	TTrackBar *TBDiddleWait;
	TCheckBox *lmsBPF;
	TComboBox *IIRFW;
	TLabel *LHZ;
	TTabSheet *TabDem2;
	TGroupBox *GB7;
	TLabel *Label16;
	TLabel *L17;
	TLabel *L18;
	TCheckBox *CheckAFC;
	TGroupBox *GrpLimit;
	TLabel *L8;
	TComboBox *LimitGain;
	TComboBox *AFCTime;
	TComboBox *AFCSQ;
	TComboBox *AFCSweep;
	TRadioGroup *DemType;
	TCheckBox *lmsInv;
	TGroupBox *GB1;
	TLabel *Label1;
	TGroupBox *GroupBox17;
	TLabel *LLoopOrder;
	TComboBox *pllVCOGain;
	TComboBox *pllLoopOrder;
	TLabel *L3;
	TComboBox *pllLoopFC;
	TLabel *Label4;
	TGroupBox *GroupBox18;
	TLabel *LOutOrder;
	TLabel *L44;
	TLabel *Label45;
	TComboBox *pllOutOrder;
	TComboBox *pllOutFC;
	TRadioGroup *DefStopBit;
	TGroupBox *GSFont;
	TEdit *WinFont;
	TButton *WinFontBtn;
	TLabel *L46;
	TLabel *L47;
	TEdit *BtnFont;
	TButton *BtnFontBtn;
	TButton *JapaneseBtn;
	TButton *EnglishBtn;
	TComboBox *FontAdj;
	TComboBox *BtnFontAdj;
	TGroupBox *GB2;
	TPanel *PCXY;
	TCheckBox *XYInv;
	TCheckBox *WaitDiddle;
	TRadioGroup *FixShift;
	TRadioGroup *DemLpf;
	TLabel *LSmooz;
	TComboBox *DemLpfOrder;
	TSpeedButton *DispDemLpf;
	TComboBox *SmoozIIR;
	TSpeedButton *DispLoopLPF;
	TSpeedButton *DispOutLPF;
	TCheckBox *LimitOver;
	TGroupBox *GrpCard;
	TLabel *Label27;
	TComboBox *FifoRX;
	TRadioGroup *SoundPriority;
	TGroupBox *GB3;
	TSpeedButton *SBClockAdj;
	TLabel *Label5;
	TGroupBox *GB8;
	TRadioGroup *DiddleSel;
	TCheckBox *RandomDiddle;
	TCheckBox *WaitTimer;
	TCheckBox *MacroImm;
	TCheckBox *TxDisRev;
	TButton *DefBtn;
	TGroupBox *GrpATC;
	TCheckBox *ATC;
	TComboBox *ATCTime;
	TLabel *L2;
	TCheckBox *MemWin;
	TButton *RadioBtn;
	TComboBox *EditSamp;
	TCheckBox *lmsType;
	TCheckBox *lmsTwo;
	TLabel *LRXS;
	TPanel *PCRXTXChar;
	TButton *TxdJob;
	TRadioGroup *Source;
	TCheckBox *TxFixShift;
	TSpeedButton *SBHelp;
	TCheckBox *LimitAGC;
	TComboBox *FifoTX;
	TLabel *L15;
	TRadioGroup *RGLoop;
	TLabel *L16;
	TEdit *TxOff;
	TLabel *L1;
	TGroupBox *GB4;
	TRadioGroup *RGC;
    TGroupBox *GroupBox1;
    TComboBox *DevNo;
    TComboBox *DevOutNo;
    TLabel *Label2;
    TLabel *Label3;
    TTabSheet *TabSound;
    TRadioGroup *InputSoundcards;
    TRadioGroup *OutputSoundcards;
	TComboBox *Tones;
	TGroupBox *GB20;
	TEdit *DefMarkFreq;
	TEdit *DefShift;
	TCheckBox *CBFix45;
	TCheckBox *CBAA6YQ;
	TSpeedButton *SBAA6YQ;
	void __fastcall HamBtnClick(TObject *Sender);
	
	
	
	
	
	
	
	
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall SBMClick(TObject *Sender);
	void __fastcall SBIN1Click(TObject *Sender);
	void __fastcall SBIN2Click(TObject *Sender);
	void __fastcall SBIN3Click(TObject *Sender);
	void __fastcall SBIN4Click(TObject *Sender);

	void __fastcall DemBpfBtnClick(TObject *Sender);
	void __fastcall PreBpfBtnClick(TObject *Sender);
	void __fastcall DispTxBpfClick(TObject *Sender);
	void __fastcall PCRXBackClick(TObject *Sender);
	void __fastcall PCRXCharClick(TObject *Sender);
	void __fastcall PCINBackClick(TObject *Sender);
	void __fastcall PCINCharClick(TObject *Sender);
	void __fastcall PCINTxClick(TObject *Sender);
	void __fastcall PCLowClick(TObject *Sender);
	void __fastcall PCHighClick(TObject *Sender);
	void __fastcall PortNameChange(TObject *Sender);
	
	void __fastcall PCXYClick(TObject *Sender);

	
	void __fastcall WinFontBtnClick(TObject *Sender);
	void __fastcall BtnFontBtnClick(TObject *Sender);
	void __fastcall JapaneseBtnClick(TObject *Sender);
	void __fastcall EnglishBtnClick(TObject *Sender);
	
	
	
	
	void __fastcall DispDemLpfClick(TObject *Sender);
	void __fastcall DispLoopLPFClick(TObject *Sender);
	void __fastcall DispOutLPFClick(TObject *Sender);
	void __fastcall SBClockAdjClick(TObject *Sender);
	void __fastcall DefBtnClick(TObject *Sender);
	void __fastcall RadioBtnClick(TObject *Sender);
	
	
	void __fastcall BaudRateChange(TObject *Sender);
	
	void __fastcall BitLenClick(TObject *Sender);
	void __fastcall PCRXTXCharClick(TObject *Sender);
	
	void __fastcall TxdJobClick(TObject *Sender);
	void __fastcall lmsTapChange(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
	
	void __fastcall PortNameDropDown(TObject *Sender);
	void __fastcall DevNoDropDown(TObject *Sender);
	
	void __fastcall DemBpfBtnMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    

    void __fastcall DevOutNoDropDown(TObject *Sender);

    void __fastcall InputSoundcardsClick(TObject *Sender);
    void __fastcall OutputSoundcardsClick(TObject *Sender);

    void __fastcall DevNoClick(TObject *Sender);
    void __fastcall DevOutNoClick(TObject *Sender);

	void __fastcall SBAA6YQClick(TObject *Sender);
private:
	int m_FontCharset;
	int	m_FontStyle;
	int	m_FontSize;

	int	m_DisEvent;

	CFSKDEM	*pDem;

	TSpeedButton *__fastcall GetSB(int n);
	int __fastcall GetMacroIndex(TObject *Sender);
	void __fastcall UpdateUI(void);
	void __fastcall UpdateMacro(void);
	void __fastcall SBINClick(int n);
	void __fastcall SetCustomColor(void);
	void __fastcall AddCustomColor(TColor col);

    int __fastcall IsSoundcard(LPCSTR t);

	BYTE	m_WinFontCharset;
	BYTE	m_BtnFontCharset;
	int		m_WinFontStyle;
	int		m_BtnFontStyle;

	int		m_TxdJob;
	double	m_lmsTap;
	double	m_NotchTap;
	CMMList	m_MMList;
    CMMList	m_MMListW;
public:
	virtual __fastcall TOptionDlg(TComponent* AOwner);

	int __fastcall Execute(CFSKDEM *fp, CFSKMOD *mp);
	int m_ColorChange;
};
//----------------------------------------------------------------------------
//extern TAgcSetDlg *AgcSetDlg;
extern int PageIndex;
//----------------------------------------------------------------------------
#endif    
