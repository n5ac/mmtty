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
#ifndef SendFileH
#define SendFileH
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
#include "ComLib.h"
//----------------------------------------------------------------------------
class TSendFileDlg : public TForm
{
__published:        
	TButton *AbortBtn;
	TLabel *LName;
	TComboBox *CharLen;
	TLabel *Lline;
	void __fastcall AbortBtnClick(TObject *Sender);
	void __fastcall CharLenChange(TObject *Sender);
private:
	int			m_DisEvent;
	AnsiString	m_FileName;
	FILE		*m_fp;
public:
	virtual __fastcall TSendFileDlg(TComponent* AOwner);
	__fastcall ~TSendFileDlg();

	int __fastcall Start(LPCSTR pName, int Y);
	int __fastcall TSendFileDlg::Timer(int buffcnt);
};
//----------------------------------------------------------------------------
//extern TSendFileDlg *SendFileDlg;
//----------------------------------------------------------------------------
#endif    
