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
#ifndef MmcgDlgH
#define MmcgDlgH
//----------------------------------------------------------------------------
/* JA7UDE 0428
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
#include <vcl\Grids.hpp>
*/
//----------------------------------------------------------------------------
#include "Mmcg.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <Grids.hpp>
#include <StdCtrls.hpp>
//----------------------------------------------------------------------------
class TMmcgDlgBox : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
	TStringGrid *Grid;
	TEdit *EditYomi;
	TSpeedButton *SBMask;
	TSpeedButton *SBInc;
	void __fastcall GridDrawCell(TObject *Sender, int Col, int Row, TRect &Rect,
	TGridDrawState State);
	void __fastcall EditYomiChange(TObject *Sender);




	void __fastcall EditYomiKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall EditYomiKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall SBMaskClick(TObject *Sender);
	void __fastcall SBIncClick(TObject *Sender);
	void __fastcall EditYomiKeyPress(TObject *Sender, char &Key);
private:
	int		m_DisEvent;

	void __fastcall UpdateCaption(void);
public:
	virtual __fastcall TMmcgDlgBox(TComponent* AOwner);

	int __fastcall Execute(AnsiString &call, AnsiString &qth, AnsiString &op);
};
//----------------------------------------------------------------------------
//extern TMmcgDlgBox *MmcgDlgBox;
//----------------------------------------------------------------------------
#endif
