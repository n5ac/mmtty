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



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TxdDlg.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TTxdDlgBox *TxdDlgBox;
//---------------------------------------------------------------------
__fastcall TTxdDlgBox::TTxdDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		CancelBtn->Caption = "Cancel";
		LMsg->Caption = "Please try to test B, C, D, if you have a trouble in the USB-COM adaptor. (C)Limiting speed seems to be well.";
		TxdJob->Caption = "Processing method";
		TxdJob->Items->Strings[0] = "A: Normal";
		TxdJob->Items->Strings[1] = "B: Polling";
		TxdJob->Items->Strings[2] = "C: Limiting speed";
		TxdJob->Items->Strings[3] = "D: Polling and Limiting speed";

	}
	else {
		LMsg->Caption = "FSKがUSBシリアルアダプターで正しく動作しない場合はB, C, Dを試して下さい。(C)送信速度の制限が良いようです.";
	}
}
//---------------------------------------------------------------------
int __fastcall TTxdDlgBox::Execute(int sel)
{
	TxdJob->ItemIndex = sel;
	if( ShowModal() == IDOK ){
		sel = TxdJob->ItemIndex;
	}
	return sel;
}

