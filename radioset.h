//Copyright+LGPL

//-----------------------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000-2013 Makoto Mori, Nobuyuki Oba, Dave Bernstein
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
#ifndef RadioSetH
#define RadioSetH
//----------------------------------------------------------------------------
/*
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
#include "Cradio.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
/*
#include <vcl\Dialogs.hpp>
#include <vcl\ComCtrls.hpp>
*/
//----------------------------------------------------------------------------
class TRADIOSetDlg : public TForm
{
__published:        
	TButton *OKBtn;
	TButton *CancelBtn;
	TGroupBox *GB1;
	TComboBox *PortName;
	TLabel *Label1;
	TLabel *Label2;
	TComboBox *Baud;
	TRadioGroup *BitLen;
	TRadioGroup *Stop;
	TRadioGroup *Parity;
	TGroupBox *GB3;
	TCheckBox *flwXON;
	TCheckBox *flwCTS;
	TLabel *Label3;
	TComboBox *ByteWait;
	TLabel *Label8;
	TButton *LoadBtn;
	TButton *SaveBtn;
	TComboBox *Maker;
	TGroupBox *GGNR;
	TLabel *LGNR;
	TEdit *CmdGNR;
	TButton *RefBtn;
	TCheckBox *OpenGNR;
	TLabel *LPTT;
	TGroupBox *GCmd;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Lxx;
	TLabel *LVFO;
	TEdit *CmdInit;
	TEdit *CmdRx;
	TEdit *CmdTx;
	TEdit *Cmdxx;
	TComboBox *VFOType;
	TRadioGroup *FreqOffset;
	TComboBox *PollInt;
	TLabel *LInt;
	TLabel *LInts;
	TCheckBox *AddrScan;
	TSpeedButton *SBHelp;
	TGroupBox *GB2;
	TCheckBox *CBPTT;
    TLabel *Label7;
	void __fastcall LoadBtnClick(TObject *Sender);
	void __fastcall SaveBtnClick(TObject *Sender);
	void __fastcall MakerChange(TObject *Sender);
	void __fastcall PortNameChange(TObject *Sender);
	void __fastcall RefBtnClick(TObject *Sender);
	
	void __fastcall VFOTypeChange(TObject *Sender);
	void __fastcall SBHelpClick(TObject *Sender);
	
    
    
private:
	int				m_DisEvent;
	int __fastcall IsXX(void);
	int __fastcall IsSame(LPCSTR v, LPCSTR t);
    int __fastcall IsCompatible(int PollType, int MakerIndex); //AA6YQ 1.66
	void __fastcall SetMaker(void);
	void __fastcall UpdateUI(void);
	void __fastcall SetVFOList(void);
	int __fastcall GetVFOType(LPCSTR pKey);
	LPCSTR __fastcall GetVFOName(int r);
	int				m_Maker;
	int				m_PollType;
	int				m_InitWidth;
	CMMList			m_MMList;
public:
	virtual __fastcall TRADIOSetDlg(TComponent* AOwner);
	int __fastcall Execute(void);
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif    
