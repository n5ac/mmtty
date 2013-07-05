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

#include "EditDlg.h"
#include "MacroKey.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TEditDlgBox *EditDlgBox;
//---------------------------------------------------------------------
__fastcall TEditDlgBox::TEditDlgBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	Memo->Font->Name = sys.m_BtnFontName;
	Memo->Font->Charset = sys.m_BtnFontCharset;
	m_JobSel = 0;
	m_DisEvent = 0;
	m_CurIndex = -1;
	m_MacroSet = 0;

	ShortCutName->Clear();
	int i;
	ShortCutName->Items->Add("NULL");
	for( i = 0; KEYTBL[i].Key; i++ ){
		ShortCutName->Items->Add(ToDXKey(KEYTBL[i].pName));
	}
	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = "Assign Macro";
		CancelBtn->Caption = "Cancel";
		LShort->Caption = "ShortCut";
		LEntryName->Caption = "Name";
		DelBtn->Caption = "Del";
		NewNameBtn->Caption = "New Name";
		TxBtn->Caption = "TX(F9)";
	}
//	SBHelp->Visible = !JanHelp.IsEmpty();
	SBHelp->Visible = FALSE;
    TxBtn->Enabled = !(sys.m_DisTX&1);
}
//---------------------------------------------------------------------
int __fastcall TEditDlgBox::Execute(AnsiString &as, WORD &nKey, TColor *pcol, int *Intval, int set)
{
	m_MacroSet = set;
	m_DisEvent++;
	if( ButtonName->Text.IsEmpty() ){
		ButtonName->Visible = FALSE;
		LEntryName->Visible = FALSE;
	}
	else {
		ButtonName->Visible = TRUE;
	}
	EntryName->Visible = FALSE;
	DelBtn->Visible = FALSE;
	NewNameBtn->Visible = FALSE;
	Memo->Text = as;
	if( pcol != NULL ){
		PanelCol->Visible = TRUE;
		LCol->Visible = TRUE;
		PanelCol->Color = *pcol;
	}
	if( Intval != NULL ){
		LMInt->Visible = TRUE;
		EMInt->Visible = TRUE;
		UDInt->Visible = TRUE;
		LMUN->Visible = TRUE;
		UDInt->Position = SHORT(*Intval);
	}
	ShortCutName->Text = GetKeyName(nKey);
	if( ShowModal() == IDOK ){
		as = Memo->Text;
		nKey = GetKeyCode(AnsiString(ShortCutName->Text).c_str());	//JA7UDE 0428
		if( pcol != NULL ){
			*pcol = PanelCol->Color;
		}
		if( Intval != NULL ){
			*Intval = UDInt->Position;
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TEditDlgBox::UpdateUI(void)
{
	if( EntryName->Visible != FALSE ){
		NewNameBtn->Enabled = (m_CurIndex >= 0) ? TRUE : FALSE;
		DelBtn->Enabled = (m_CurIndex >= 0) ? TRUE : FALSE;
		OKBtn->Enabled = ((!EntryName->Text.IsEmpty()) && (!Memo->Text.IsEmpty()) && Memo->Modified) ? TRUE : FALSE;
	}
}
//---------------------------------------------------------------------
int __fastcall TEditDlgBox::EditMsg(void)
{
	Caption = (Font->Charset != SHIFTJIS_CHARSET) ? "Edit Message":"定型メッセージの編集";

	ButtonName->Visible = FALSE;
	EntryName->Visible = TRUE;
	DelBtn->Visible = TRUE;
	NewNameBtn->Visible = TRUE;

	if(Font->Charset != SHIFTJIS_CHARSET){
		CancelBtn->Caption = "Close";
		OKBtn->Caption = "Assign";
	}
	else {
		CancelBtn->Caption = "閉じる";
		OKBtn->Caption = "登録";
	}

	UpdateList();
//    EntryName->SetFocus();
	Memo->Clear();
	if( !sys.m_MsgName[0].IsEmpty() ){
		Memo->Text = sys.m_MsgList[0];
		m_CurIndex = 0;
	}
	UpdateUI();
	m_MacroSet = 0;
	if( ShowModal() == IDOK ){
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::UpdateList(void)
{
	m_DisEvent++;
	EntryName->Clear();
	int i;
	for( i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		EntryName->Items->Add(sys.m_MsgName[i]);
	}
	if( !sys.m_MsgName[0].IsEmpty() ){
		EntryName->Text = sys.m_MsgName[0];
	}
	ShortCutName->Text = GetKeyName(sys.m_MsgKey[0]);
	m_DisEvent--;
}

void __fastcall TEditDlgBox::EntryNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	AnsiString	ttl = EntryName->Text;
	AnsiString	as;
	m_CurIndex = -1;
	ShortCutName->Text = "NULL";
	int i;
	for( i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		if( sys.m_MsgName[i] == ttl ){
			as = sys.m_MsgList[i];
			Memo->Clear();
			Memo->Text = as;
			m_CurIndex = i;
			ShortCutName->Text = GetKeyName(sys.m_MsgKey[i]);
			break;
		}
	}
	if( (m_CurIndex == -1) && (!EntryName->Text.IsEmpty()) && (!Memo->Text.IsEmpty()) ) Memo->Modified = TRUE;
	UpdateUI();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::NewNameBtnClick(TObject *Sender)
{
	//AnsiString ttl = EntryName->Text;	//JA7UDE 0428
	UnicodeString ttl = EntryName->Text;	//JA7UDE 0428
	if( InputQuery("MMTTY", (Font->Charset != SHIFTJIS_CHARSET) ? "Input Name":"登録名（メッセージの別名）を入力", ttl) == TRUE ){
		if( ttl.IsEmpty() ){
			ttl = "無題";
		}
		sys.m_MsgName[m_CurIndex] = ttl;
		EntryName->Text = ttl;
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::DelBtnClick(TObject *Sender)
{
	if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Delete [%s]. Are you sure?":"[%s]をリストから削除します.\r\n\r\nよろしおまっか？", EntryName->Text.c_str()) == IDYES ){
		int i;
		for( i = m_CurIndex; i < (MSGLISTMAX - 1); i++ ){
			if( sys.m_MsgName[i].IsEmpty() ) break;
			sys.m_MsgList[i] = sys.m_MsgList[i+1];
			sys.m_MsgName[i] = sys.m_MsgName[i+1];
			sys.m_MsgKey[i] = sys.m_MsgKey[i+1];
		}
		sys.m_MsgName[i] = "";
		sys.m_MsgList[i] = "";
		sys.m_MsgKey[i] = 0;
		if( m_CurIndex ) m_CurIndex--;
		if( m_CurIndex >= 0 ){
			EntryName->Text = sys.m_MsgName[m_CurIndex];
			Memo->Text = sys.m_MsgList[m_CurIndex];
			ShortCutName->Text = GetKeyName(sys.m_MsgKey[m_CurIndex]);
		}
		else {
			EntryName->Text = "";
			Memo->Clear();
			ShortCutName->Text = "";
		}
		UpdateList();
	}
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::MemoChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::OKBtnClick(TObject *Sender)
{
	if( EntryName->Visible == FALSE ){
		ModalResult = mrOk;
		return;
	}
	if( EntryName->Text.IsEmpty() ) return;
	if( Memo->Text.IsEmpty() ) return;
	if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Assign text to [%s]. Are you sure?":"[%s]に現在のテキストを定義します.\r\n\r\nよろしおまっか？", EntryName->Text.c_str()) == IDYES ){
		EntryMsg(AnsiString(EntryName->Text), AnsiString(Memo->Text), GetKeyCode(AnsiString(ShortCutName->Text).c_str()));	//JA7UDE 0428
		Memo->Modified = FALSE;
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::ShortCutNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	Memo->Modified = TRUE;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::TxBtnClick(TObject *Sender)
{
	if( Memo->Text.IsEmpty() ) return;
	if( sys.m_DisTX&1 ) return;

	m_JobSel = 1;
	ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::FormKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	switch(Key){
		case VK_F9:
			Key = 0;
			TxBtnClick(NULL);
			break;
		case VK_ESCAPE:
			Key = 0;
			ModalResult = mrCancel;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::FormKeyUp(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	switch(Key){
		case VK_F9:
			Key = 0;
			break;
		case VK_ESCAPE:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::ClearBtnClick(TObject *Sender)
{
	if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Clear text and shortcut. Are you sure?":"テキストとショートカットを初期化します.\r\n\r\nよろしおまっか？") == IDYES ){
		Memo->Text = "";
		ShortCutName->Text = "NULL";
		UDInt->Position = 0;
	}
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::PanelColClick(TObject *Sender)
{
	ColorDialog->Color = PanelCol->Color;
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PanelCol->Color = ColorDialog->Color;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int __fastcall TEditDlgBox::CheckLast(LPCSTR s, char c)
{
	LPCSTR p = s + strlen(s) - 1;
	for( ; p >= s; p-- ){
		if( *p == c ) return 1;
		if( isalpha(*p) || (*p == '%') ) return 0;
	}
	return 0;
}
//---------------------------------------------------------------------------
int __fastcall TEditDlgBox::CheckTop(LPCSTR s, char c)
{
	LPCSTR p = s;
	for( ; *p; p++ ){
		if( *p == c ) return 1;
		if( isalpha(*p) || (*p == '%') ) return 0;
	}
	return 0;
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::DeleteSpace(void)
{
	AnsiString	ws = Memo->Text.c_str();
	LPSTR	p = ws.c_str();
	AnsiString	as;
	LPSTR	bp = new char[strlen(p)+1];

	LPSTR	t = bp;
	for( ; *p; p++ ){
		if( *p != CR ){
			if( *p == LF ){
				*t = 0;
				clipsp(bp);
				if( *bp ) as += bp;
				as += "\r\n";
				t = bp;
			}
			else {
				*t++ = *p;
			}
		}
	}
	*t = 0;
	if( *bp ){
		clipsp(bp);
		if( *bp ) as += bp;
	}
	p = as.c_str();
	t = lastp(p);
	if( (*t == LF)||(*t == CR) ){
		for( ; t >= p; t-- ){
			if( (*t == LF)||(*t == CR) ){
				*t = 0;
			}
			else {
				break;
			}
		}
		t++;
		*t++ = CR;
		*t = LF;
	}
	Memo->Text = p;
	Memo->Modified = TRUE;
	delete bp;
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::ComBtnClick(TObject *Sender)
{
	TMacroKeyDlg *pBox = new TMacroKeyDlg(this);
	AnsiString as, bs;
	LPCSTR p;
	int r = pBox->Execute(as, m_MacroSet);
	switch(r){
		case 1:
			Memo->SetFocus();
			for( p = as.c_str(); *p; p++ ){
				::PostMessage(Memo->Handle, WM_CHAR, *p, 0);
			}
			break;
		case 2:		// 最後に追加
			bs = Memo->Text;
			if( !CheckLast(bs.c_str(), *as.c_str()) ){
				bs += as.c_str();
				Memo->Text = bs;
				Memo->Modified = TRUE;
			}
			break;
		case 3:		// 先頭に追加
			if( !CheckTop(AnsiString(Memo->Text).c_str(), *as.c_str()) ){	//JA7UDE 0428
				as += Memo->Text.c_str();
				Memo->Text = as.c_str();
				Memo->Modified = TRUE;
			}
			break;
		case -1:	// 行末のスペースを削除
			DeleteSpace();
			break;
	}
	delete pBox;
	UpdateUI();
	Memo->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TEditDlgBox::SBHelpClick(TObject *Sender)
{
	ShowHtmlHelp();
}
//---------------------------------------------------------------------------
