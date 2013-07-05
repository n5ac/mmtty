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
#ifndef EditDlgH
#define EditDlgH
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

#include "ComLib.h"
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
//----------------------------------------------------------------------------
class TEditDlgBox : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TMemo *Memo;
	TLabel *LEntryName;
	TComboBox *EntryName;
	TButton *DelBtn;
	TButton *NewNameBtn;
	TEdit *ButtonName;
	TLabel *LShort;
	TComboBox *ShortCutName;
	TButton *TxBtn;
	TButton *ClearBtn;
	TPanel *PanelCol;
	TColorDialog *ColorDialog;
	TLabel *LCol;
	TLabel *LMInt;
	TUpDown *UDInt;
	TLabel *LMUN;
	TComboBox *EMInt;
	TButton *ComBtn;
	TSpeedButton *SBHelp;
	void __fastcall EntryNameChange(TObject *Sender);
	void __fastcall NewNameBtnClick(TObject *Sender);
	void __fastcall DelBtnClick(TObject *Sender);
	void __fastcall MemoChange(TObject *Sender);
	void __fastcall OKBtnClick(TObject *Sender);
	
	
	void __fastcall ShortCutNameChange(TObject *Sender);
	void __fastcall TxBtnClick(TObject *Sender);
	
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall ClearBtnClick(TObject *Sender);
	void __fastcall PanelColClick(TObject *Sender);
	void __fastcall ComBtnClick(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
private:
	int	m_DisEvent;
	int	m_CurIndex;
	int	m_MacroSet;

	void __fastcall UpdateList(void);
	void __fastcall UpdateUI(void);

	int __fastcall CheckLast(LPCSTR s, char c);
	int __fastcall CheckTop(LPCSTR s, char c);
	void __fastcall DeleteSpace(void);

public:
	virtual __fastcall TEditDlgBox(TComponent* AOwner);
	int __fastcall Execute(AnsiString &as, WORD &nKey, TColor *pcol, int *Intval, int set);
	int __fastcall EditMsg(void);

	int	m_JobSel;
};
//----------------------------------------------------------------------------
//extern TEditDlgBox *EditDlgBox;
//----------------------------------------------------------------------------
#endif    
