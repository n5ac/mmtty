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
#ifndef TncSetH
#define TncSetH
//----------------------------------------------------------------------------
/* JA7UDE 0427
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
*/
//----------------------------------------------------------------------------
#include "Ctnc.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
//----------------------------------------------------------------------------
class TTNCSetDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TGroupBox *GroupBox1;
	TComboBox *PortName;
	TLabel *L1;
	TLabel *L2;
	TComboBox *Baud;
	TRadioGroup *BitLen;
	TRadioGroup *Stop;
	TRadioGroup *Parity;
	TRadioGroup *TNCType;
	TLabel *LMSG;
	TGroupBox *GroupBox3;
	TCheckBox *flwXON;
	TCheckBox *flwCTS;
	TRadioGroup *Echo;
	TCheckBox *DisWindow;
	TCheckBox *StayOnTop;
	TSpeedButton *SBHelp;
	void __fastcall TNCTypeClick(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
	
private:
	int				m_DisEvent;
	CMMList			m_MMList;

	void __fastcall UpdateUI(void);

public:
	virtual __fastcall TTNCSetDlg(TComponent* AOwner);
	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif    
