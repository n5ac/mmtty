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

#include "InputWin.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TInputWinDlg::TInputWinDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	if( AOwner == NULL ) return;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		CancelBtn->Caption = "Cancel";
	}
}

//---------------------------------------------------------------------
int InputMB(LPCSTR pTitle, LPCSTR pMsg, AnsiString &as)
{
	TForm *fp;
	if( Screen->ActiveForm != NULL ){
		fp = Screen->ActiveForm;
	}
	else if( Application->MainForm != NULL ){
		fp = Application->MainForm;
	}
	else {
		fp = NULL;
	}

	TInputWinDlg *pBox = new TInputWinDlg(fp);
	pBox->LMsg->Caption = pMsg;
	pBox->Caption = pTitle;
	pBox->Edit->Text = as.c_str();
	int r = FALSE;
	if( pBox->ShowModal() == IDOK ){
		as = pBox->Edit->Text.c_str();
		r = TRUE;
	}
	delete pBox;
	return r;
}
//---------------------------------------------------------------------




