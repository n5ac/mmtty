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

#include "ComLib.h"
#include "TextEdit.h"
//---------------------------------------------------------------------------
enum TFontPitch { fpDefault, fpVariable, fpFixed };
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TTextEditDlg *TextEditDlg;
//---------------------------------------------------------------------
__fastcall TTextEditDlg::TTextEditDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	ap = NULL;
	Memo->Font->Pitch = fpFixed;
	EntryAlignControl();
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
//	Memo->Font->Name = sys.m_BtnFontName;
//	Memo->Font->Charset = sys.m_BtnFontCharset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		CancelBtn->Caption = "Cancel";
		FontBtn->Caption = "Font";
	}
}
//---------------------------------------------------------------------
int __fastcall TTextEditDlg::Execute(AnsiString &as, int flag, LPCSTR pTitle /* = NULL*/)
{
	CWaitCursor tw;
	ap = as.c_str();
	if( flag != TRUE ){
		OKBtn->Visible = FALSE;
		CancelBtn->Caption = ( Font->Charset != SHIFTJIS_CHARSET ) ? "Close":"閉じる";
		Memo->ReadOnly = TRUE;
	}
	if( pTitle != NULL ){
		Caption = pTitle;
	}
	tw.Delete();
	int r = ShowModal();
	if( r == IDOK ){
		if( Memo->Modified == FALSE ) return FALSE;
		if( flag == TRUE ){
			as = Memo->Text;
		}
		return TRUE;
	}
	return FALSE;
}

void __fastcall TTextEditDlg::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Assign(Memo->Font);
	NormalWindow(this);
	if( FontDialog->Execute() == TRUE ){
		CWaitCursor tw;
		Memo->Font->Assign(FontDialog->Font);
		Memo->Update();
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
// サイズ変更コントロールの登録
void __fastcall TTextEditDlg::EntryAlignControl(void)
{
	RECT	rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = 792-1;
	rc.bottom = 572-1;

	AlignList.EntryControl(OKBtn, &rc, OKBtn->Font);
	AlignList.EntryControl(CancelBtn, &rc, CancelBtn->Font);
	AlignList.EntryControl(FontBtn, &rc, FontBtn->Font);
	AlignList.EntryControl(Memo, &rc, Memo->Font);

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
//	int CX = ::GetSystemMetrics(SM_CXSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	FormCenter(this, CX, CY);
	if( Owner != NULL ){
		WindowState = ((TForm *)Owner)->WindowState;
	}
}
//---------------------------------------------------------------------------
void __fastcall TTextEditDlg::FormResize(TObject *Sender)
{
	CWaitCursor tw;
	AlignList.NewAlign(this);
}
//---------------------------------------------------------------------------
void __fastcall TTextEditDlg::FormPaint(TObject *Sender)
{
	if( ap == NULL ) return;
	CWaitCursor tw;
	Memo->Text = ap;
	ap = NULL;
	Memo->Update();
}
//---------------------------------------------------------------------------
// テキストヘルプの表示
void __fastcall ShowHelp(TForm *pForm, LPCSTR pName)
{
	char	bf[2048];
	char	Name[256];
	AnsiString	in;
	FILE	*fp;

	sprintf(Name, "%s%s", BgnDir, pName);
	CWaitCursor w;
	if( (fp = fopen(Name, "rt"))!=NULL ){
		while(!feof(fp)){
			if( fgets(bf, 2048, fp) != NULL ){
				ClipLF(bf);
				in += bf;
				in += "\r\n";
			}
		}
		fclose(fp);
	}
	else {
		if((sys.m_WinFontCharset != SHIFTJIS_CHARSET )){
			ErrorMB( "'%s' was not found.\r\n\r\nPlease search in the MMTTY English Web Site.", Name);
		}
		else {
			ErrorMB( "'%s'が見つかりません.\r\n\r\nMMTTY English Web Site からダウンロードして下さい.", Name);
		}
		return;
	}
	if( sys.m_HelpNotePad ){
		sprintf( bf, "NOTEPAD.EXE %s", Name);
		WinExec(bf, SW_SHOWDEFAULT);
	}
	else {
		TTextEditDlg *Box = new TTextEditDlg(pForm);
		Box->Memo->Font->Name = sys.m_HelpFontName;
		Box->Memo->Font->Charset = sys.m_HelpFontCharset;
		Box->Memo->Font->Size = sys.m_HelpFontSize;
		Box->Execute(in, FALSE, pName);
		delete Box;
	}
}
//---------------------------------------------------------------------------
