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
#ifndef ShortCutH
#define ShortCutH
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
#include <Grids.hpp>

//----------------------------------------------------------------------------
#include "ComLib.h"
//----------------------------------------------------------------------------
class TShortCutDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TStringGrid *Grid;
	TComboBox *ShortCutName;
	TButton *CheckBtn;
	TButton *DelBtn;
	TSpeedButton *SBHelp;
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row,
	TRect &Rect, TGridDrawState State);
	void __fastcall GridSelectCell(TObject *Sender, int Col, int Row,
	bool &CanSelect);
	void __fastcall ShortCutNameChange(TObject *Sender);
	void __fastcall CheckBtnClick(TObject *Sender);
	void __fastcall DelBtnClick(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
private:
	int		m_DisEvent;
	void __fastcall UpdateUI(int n);
	int __fastcall IsDupe(int n);

public:
	virtual __fastcall TShortCutDlg(TComponent* AOwner);

	void __fastcall Execute(void);
};

void SetMenuName(LPCSTR pkey, LPCSTR p);
void SetExtMenuName(int n, LPCSTR p);
void SetProMenuName(int n, LPCSTR p);
//----------------------------------------------------------------------------
//extern TShortCutDlg *ShortCutDlg;
//----------------------------------------------------------------------------
#endif    
