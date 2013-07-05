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
#ifndef SetHelpH
#define SetHelpH
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
#include <Dialogs.hpp>
//----------------------------------------------------------------------------
class TSetHelpBox : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TGroupBox *GroupBox1;
	TEdit *EditMMTTY;
	TButton *RefMMTTY;
	TGroupBox *GroupBox2;
	TEdit *EditLog;
	TButton *RefLog;
	TGroupBox *GroupBox3;
	TEdit *EditDigital;
	TButton *RefDigital;
	TButton *JABtn;
	TButton *EngBtn;
	TButton *FontBtn;
	TFontDialog *FontDialog;
	TOpenDialog *OpenDialog;
	TCheckBox *NotePad;
	TGroupBox *GroupBox4;
	TEdit *EditHTML;
	TButton *RefHTML;
	void __fastcall JABtnClick(TObject *Sender);
	void __fastcall EngBtnClick(TObject *Sender);
	void __fastcall FontBtnClick(TObject *Sender);
	void __fastcall RefMMTTYClick(TObject *Sender);
	void __fastcall RefLogClick(TObject *Sender);
	void __fastcall RefDigitalClick(TObject *Sender);
	void __fastcall RefHTMLClick(TObject *Sender);
private:
	int SetFileName(AnsiString &as);

	AnsiString	m_HelpFontName;
	BYTE		m_HelpFontCharset;
	int			m_HelpFontSize;
public:
	virtual __fastcall TSetHelpBox(TComponent* AOwner);

	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//extern TSetHelpBox *SetHelpBox;
//----------------------------------------------------------------------------
#endif    
