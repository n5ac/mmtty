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
#ifndef ConvDefH
#define ConvDefH
//----------------------------------------------------------------------------
/* JA7UDE 0428
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
#include <vcl\Grids.hpp>
*/
//----------------------------------------------------------------------------
#include "LogConv.h"
//----------------------------------------------------------------------------
#define	PREREADMAX	100
class TConvDefDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TStringGrid *Grid;
	TComboBox *ConvText;
	TLabel *LConv;
	TGroupBox *GrpRef;
	TSpeedButton *SBBack;
	TSpeedButton *SBNext;
	TRadioGroup *ConvSel;
	TButton *InitBtn;
	TButton *UpBtn;
	TButton *DownBtn;
	TButton *InitMax;
	TCheckBox *CheckUTC;
	TCheckBox *CheckDBL;
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row, TRect &Rect,
	TGridDrawState State);
	void __fastcall ConvTextChange(TObject *Sender);
	void __fastcall GridSelectCell(TObject *Sender, int Col, int Row,
	bool &CanSelect);
	void __fastcall SBBackClick(TObject *Sender);
	void __fastcall SBNextClick(TObject *Sender);


	void __fastcall GridGetEditText(TObject *Sender, int ACol, int ARow,
	AnsiString &Value);
	void __fastcall GridSetEditText(TObject *Sender, int ACol, int ARow,
	const AnsiString Value);
	void __fastcall InitBtnClick(TObject *Sender);
	void __fastcall UpBtnClick(TObject *Sender);
	void __fastcall DownBtnClick(TObject *Sender);
	void __fastcall InitMaxClick(TObject *Sender);
	void __fastcall ConvSelClick(TObject *Sender);
private:
	TCONV	Conv[TEXTCONVMAX];

	int		m_Mode;
	int		m_DisEvent;

	int		m_Bgn;
	int		m_End;
	int		m_Cur;

	void __fastcall UpdateUI(int n);
	void __fastcall SetupComBox(void);

	AnsiString	m_text[PREREADMAX];
	AnsiString	m_line[TEXTCONVMAX];

	void __fastcall LoadText(LPCSTR pName);
	void __fastcall UpdateLine(void);

public:
	virtual __fastcall TConvDefDlg(TComponent* AOwner);

	int __fastcall Execute(TCONV *tp, int &delm, int &utc, int &dbl, int b, int e);
	int __fastcall Execute(TCONV *tp, int &delm, int &utc, int &dbl, LPCSTR pName);
};
//----------------------------------------------------------------------------
//extern TConvDefDlg *ConvDefDlg;
//----------------------------------------------------------------------------
#endif
