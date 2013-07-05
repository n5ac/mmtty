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
#ifndef TextEditH
#define TextEditH
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
#include <Dialogs.hpp>
//----------------------------------------------------------------------------
class TTextEditDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TMemo *Memo;
	TButton *FontBtn;
	TFontDialog *FontDialog;
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	
	
	
	
	void __fastcall FormPaint(TObject *Sender);
	
private:
	int		InitFlag;
	LPCSTR	ap;

	CAlignList	AlignList;
	void __fastcall EntryAlignControl(void);
public:
	virtual __fastcall TTextEditDlg(TComponent* AOwner);

	int __fastcall Execute(AnsiString &as, int flag, LPCSTR pTitle = NULL);
};
//----------------------------------------------------------------------------
//extern TTextEditDlg *TextEditDlg;

extern void __fastcall ShowHelp(TForm *pForm, LPCSTR pName);
//----------------------------------------------------------------------------
#endif    
