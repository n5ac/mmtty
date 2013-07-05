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
#ifndef LogSetH
#define LogSetH
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
//----------------------------------------------------------------------------
#include "LogFile.h"
#include "MMLink.h"
//----------------------------------------------------------------------------
class TLogSetDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TPageControl *Page;
	TTabSheet *TabFlag;
	TGroupBox *GrpConv;
	TCheckBox *UpperName;
	TCheckBox *UpperQTH;
	TCheckBox *UpperREM;
	TCheckBox *UpperQSL;
	TGroupBox *GrpCopy;
	TCheckBox *CopyName;
	TCheckBox *CopyQTH;
	TCheckBox *CopyREM;
	TCheckBox *CopyQSL;
	TTabSheet *TabMacro;
	TCheckBox *MacroFlag;
	TGroupBox *GroupBox3;
	TGroupBox *GroupBox4;
	TCheckBox *Macro1;
	TCheckBox *Macro2;
	TCheckBox *Macro3;
	TCheckBox *Macro4;
	TCheckBox *Macro5;
	TButton *MacroBtn1;
	TButton *MacroBtn2;
	TButton *MacroBtn3;
	TButton *MacroBtn4;
	TButton *MacroBtn5;
	TCheckBox *CheckBand;
	TRadioGroup *CopyHis;
	TRadioGroup *CopyFreq;
	TGroupBox *GroupBox5;
	TCheckBox *DefMyRST;
	TRadioGroup *TimeZone;
	TRadioGroup *Contest;
	TLabel *Label1;
	TTabSheet *TabConv;
	TGroupBox *GroupBox1;
	TGroupBox *GroupBox2;
	TCheckBox *ClipRSTADIF;
	TTabSheet *TabMisc;
	TGroupBox *GTime;
	TEdit *TimeOff;
	TUpDown *UDOffset;
	TRadioGroup *DateType;
	TEdit *MinOff;
	TUpDown *UDMin;
	TLabel *Label4;
	TLabel *Label5;
	TButton *ClearOff;
	TCheckBox *AutoSave;
	TCheckBox *AutoZone;
	TSpeedButton *SBHelp;
	TCheckBox *CBRemC;
	TGroupBox *GB6;
	TRadioGroup *RGLink;
	TCheckBox *CBPoll;
	TRadioGroup *THTZ;
	TComboBox *CBMMLink;
	TLabel *LT;
	TCheckBox *CBBackup;
	TCheckBox *CBPTT;
	TGroupBox *GB9;
	TLabel *Label2;
	TLabel *Label3;
	TEdit *THRTTY;
	TEdit *THSSTV;
	void __fastcall MacroBtn1Click(TObject *Sender);
	void __fastcall MacroBtn2Click(TObject *Sender);
	void __fastcall MacroBtn3Click(TObject *Sender);
	void __fastcall MacroBtn4Click(TObject *Sender);
	void __fastcall MacroBtn5Click(TObject *Sender);
	void __fastcall ClearOffClick(TObject *Sender);
	void __fastcall AutoZoneClick(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
	void __fastcall ContestClick(TObject *Sender);
	void __fastcall RGLinkClick(TObject *Sender);
private:
	int			m_DisEvent;
	AnsiString	m_MacroStr[5];
	WORD		m_MacroKey[5];
	CMMList		m_MMList;

	void __fastcall MacroBtnClick(int n);
	void __fastcall UpdateUI(void);

public:
	virtual __fastcall TLogSetDlg(TComponent* AOwner);

	int __fastcall Execute(void);

};
//----------------------------------------------------------------------------
//extern TLogSetDlg *LogSetDlg;
//----------------------------------------------------------------------------
#endif    
