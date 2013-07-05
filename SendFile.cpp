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

#include "SendFile.h"
#include "Main.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TSendFileDlg::TSendFileDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = "Sending file";
		AbortBtn->Caption = "Abort";
		Lline->Caption = "Letters per line";
	}
	m_fp = NULL;
	if( sys.m_CharLenFile < 20 ) sys.m_CharLenFile = 20;
	if( sys.m_CharLenFile > 1024 ) sys.m_CharLenFile = 1024;
	m_DisEvent = 1;
	CharLen->Text = sys.m_CharLenFile;
	m_DisEvent = 0;
}
__fastcall TSendFileDlg::~TSendFileDlg()
{
	if( m_fp != NULL ){
		fclose(m_fp);
		m_fp = NULL;
	}
}
//---------------------------------------------------------------------
int __fastcall TSendFileDlg::Start(LPCSTR pName, int Y)
{
	m_FileName = pName;
	LName->Caption = pName;
	m_fp = fopen(pName, "rt");
	if( m_fp != NULL ){
		Show();
		Top = Y;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
int __fastcall TSendFileDlg::Timer(int buffcnt)
{
	if( m_fp == NULL ){
		return 2;
	}
	if( feof(m_fp) ){
		fclose(m_fp);
		m_fp = NULL;
		return 1;
	}
	if( buffcnt < 10 ){
		char bf[1025];
		if( fgets(bf, sys.m_CharLenFile, m_fp) != NULL ){
			ClipLF(bf);
			MmttyWd->OutputStr(bf);
			MmttyWd->OutputStr("\r\n");
		}
	}
	return 0;
}
//---------------------------------------------------------------------
void __fastcall TSendFileDlg::AbortBtnClick(TObject *Sender)
{
	if( m_fp != NULL ){
		fclose(m_fp);
		m_fp = NULL;
	}
}
//---------------------------------------------------------------------------
void __fastcall TSendFileDlg::CharLenChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	int dd;
	sscanf(AnsiString(CharLen->Text).c_str(), "%u", &dd);	//JA7UDE 0428
	if( (dd >= 20) && (dd <= 1024) ){
		sys.m_CharLenFile = dd;
	}
}
//---------------------------------------------------------------------------
