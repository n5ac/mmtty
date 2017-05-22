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

#include "SetHelp.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TSetHelpBox *SetHelpBox;
//---------------------------------------------------------------------
__fastcall TSetHelpBox::TSetHelpBox(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		CancelBtn->Caption = "Cancel";
		NotePad->Caption = "Use NotePad";
		FontBtn->Caption = "Font";
	}
}
//---------------------------------------------------------------------
int __fastcall TSetHelpBox::Execute(void)
{
	m_HelpFontName = sys.m_HelpFontName;
	m_HelpFontCharset = sys.m_HelpFontCharset;
	m_HelpFontSize = sys.m_HelpFontSize;
	EditHTML->Text = sys.m_HTMLHelp;
	EditMMTTY->Text = sys.m_Help;
	EditLog->Text = sys.m_HelpLog;
	EditDigital->Text = sys.m_HelpDigital;
	NotePad->Checked = sys.m_HelpNotePad;
	if( ShowModal() == IDOK ){
		sys.m_HelpFontName = m_HelpFontName;
		sys.m_HelpFontCharset = m_HelpFontCharset;
		sys.m_HelpFontSize = m_HelpFontSize;

		sys.m_HTMLHelp = EditHTML->Text;
		sys.m_Help = EditMMTTY->Text;
		sys.m_HelpLog = EditLog->Text;
		sys.m_HelpDigital = EditDigital->Text;

		sys.m_HelpNotePad = NotePad->Checked;
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//---------------------------------------------------------------------
void __fastcall TSetHelpBox::JABtnClick(TObject *Sender)
{
	EditHTML->Text = "MMTTY.chm";   //1.70K
	EditMMTTY->Text = "Mmtty.txt";
	EditLog->Text = "Mmttylog.txt";
	EditDigital->Text = "Digital.txt";
	m_HelpFontName = "ＭＳ ゴシック";
	m_HelpFontCharset = SHIFTJIS_CHARSET;
	m_HelpFontSize = 10;

	//1.70K hide English Help panels
	GroupBox1->Visible=True;
	GroupBox2->Visible=True;
	GroupBox3->Visible=True;
	GroupBox4->Visible=False;
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::EngBtnClick(TObject *Sender)
{
	EditHTML->Text = "MMTTY.chm"; //1.70J
	EditMMTTY->Text = "MmttyE.txt";
	EditLog->Text = "LogE.txt";
	EditDigital->Text = "DigitalE.txt";
	m_HelpFontName = "Courier";//"Courier New";
	m_HelpFontCharset = ANSI_CHARSET;
	m_HelpFontSize = 10;

	//1.70K hide Japanese Help panels
	GroupBox1->Visible=False;
	GroupBox2->Visible=False;
	GroupBox3->Visible=False;
	GroupBox4->Visible=True;       //1.70K
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Name = m_HelpFontName;
	FontDialog->Font->Charset = m_HelpFontCharset;
	FontDialog->Font->Size = m_HelpFontSize;
	NormalWindow(this);
	if( FontDialog->Execute() == TRUE ){
		m_HelpFontName = FontDialog->Font->Name;
		m_HelpFontCharset = FontDialog->Font->Charset;
		m_HelpFontSize = FontDialog->Font->Size;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
int TSetHelpBox::SetFileName(AnsiString &as)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if(sys.m_WinFontCharset != SHIFTJIS_CHARSET){
		OpenDialog->Title = "Choose help file";
		OpenDialog->Filter = "Text files(*.txt)|*.txt|All files(*.*)|*.*|";
	}
	else {
		OpenDialog->Title = "ヘルプファイルの選択";
		OpenDialog->Filter = "テキストファイル(*.txt)|*.txt|すべてのファイル(*.*)|*.*|";
	}
	OpenDialog->FileName = as.c_str();
	OpenDialog->DefaultExt = "txt";
	OpenDialog->InitialDir = BgnDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char name[_MAX_FNAME];
		char ext[_MAX_EXT];

		::_splitpath(AnsiString(OpenDialog->FileName).c_str(), drive, dir, name, ext );	//JA7UDE 0428
		as = name;
		as += ext;
		TopWindow(this);
		return TRUE;
	}
	else {
		TopWindow(this);
		return FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::RefMMTTYClick(TObject *Sender)
{
	AnsiString as = EditMMTTY->Text.c_str();
	if( SetFileName(as) == TRUE ){
		EditMMTTY->Text = as.c_str();
	}
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::RefLogClick(TObject *Sender)
{
	AnsiString as = EditLog->Text.c_str();
	if( SetFileName(as) == TRUE ){
		EditLog->Text = as.c_str();
	}
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::RefDigitalClick(TObject *Sender)
{
	AnsiString as = EditDigital->Text.c_str();
	if( SetFileName(as) == TRUE ){
		EditDigital->Text = as.c_str();
	}
}
//---------------------------------------------------------------------------
void __fastcall TSetHelpBox::RefHTMLClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if(sys.m_WinFontCharset != SHIFTJIS_CHARSET){
		OpenDialog->Title = "Choose HTML/Windows help file";
	}
	else {
		OpenDialog->Title = "HTML/Windowsヘルプファイルの選択";
	}
	OpenDialog->Filter = "HTML/Windows help files(*.chm;*.htm;*.html;*.hlp)|*.chm;*.htm;*.html;*.hlp";
	OpenDialog->FileName = EditHTML->Text.c_str();
	OpenDialog->DefaultExt = "chm";
	OpenDialog->InitialDir = BgnDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char name[_MAX_FNAME];
		char ext[_MAX_EXT];

		::_splitpath(AnsiString(OpenDialog->FileName).c_str(), drive, dir, name, ext );	//JA7UDE 0428
		AnsiString as;
		as = name;
		as += ext;
		EditHTML->Text = as.c_str();
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
