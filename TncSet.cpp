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

#include "TncSet.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//---------------------------------------------------------------------
__fastcall TTNCSetDlg::TTNCSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = "TNC emulation mode";
		CancelBtn->Caption = "Cancel";
		LMSG->Caption = "Note: Use a null modem cable for connecting to the other PC.";
	}
	SBHelp->Visible = !JanHelp.IsEmpty();
	m_MMList.QueryList("MMT");
	for( int i = 0; i < m_MMList.GetCount(); i++ ){
		PortName->Items->Insert(1, m_MMList.GetItemName(i));
	}
	PortName->DropDownCount = m_MMList.GetCount() + 10;
}
//---------------------------------------------------------------------
void __fastcall TTNCSetDlg::UpdateUI(void)
{
	m_DisEvent++;
	int cf = (m_MMList.IndexOf(AnsiString(PortName->Text).c_str()) < 0);
	int f;
	if( TNCType->ItemIndex == 2 ){
		f = FALSE;
	}
	else {
		f = cf;
	}
	for(int i = 0; i < GroupBox1->ControlCount; i++ ){
		if( (GroupBox1->Controls[i]->Name != "PortName") &&
        	(GroupBox1->Controls[i]->Name != "L1")
        ){
			GroupBox1->Controls[i]->Enabled = f;
		}
	}
	flwXON->Enabled = f;
	flwCTS->Enabled = f;
	Echo->Enabled = !(TNCType->ItemIndex == 2);
    LMSG->Enabled = cf;
    m_DisEvent--;
}
//---------------------------------------------------------------------
int __fastcall TTNCSetDlg::Execute(void)
{
	PortName->Text = TNC.StrPort;
	Baud->Text = TNC.BaudRate;
	BitLen->ItemIndex = TNC.BitLen;
	Parity->ItemIndex = TNC.Parity;
	Stop->ItemIndex = TNC.Stop;
	flwXON->Checked = TNC.flwXON;
	flwCTS->Checked = TNC.flwCTS;

	Echo->ItemIndex = TNC.Echo;
	TNCType->ItemIndex = TNC.Type;
	DisWindow->Checked = sys.m_DisWindow;
	StayOnTop->Checked = sys.m_StayOnTop;
    UpdateUI();
	m_DisEvent = 0;
	if( ShowModal() == IDOK ){
		StrCopy(TNC.StrPort, AnsiString(PortName->Text).c_str(), 31);
		int dd;
		if( sscanf(AnsiString(Baud->Text).c_str(), "%u", &dd) == 1 ){
			TNC.BaudRate = dd;
		}
		TNC.BitLen = BitLen->ItemIndex;
		TNC.Parity = Parity->ItemIndex;
		TNC.Stop = Stop->ItemIndex;
		TNC.flwXON = flwXON->Checked;
		TNC.flwCTS = flwCTS->Checked;

		TNC.Echo = Echo->ItemIndex;
		TNC.Type = TNCType->ItemIndex;
		sys.m_DisWindow = DisWindow->Checked;
		sys.m_StayOnTop = StayOnTop->Checked;
		TNC.change = 1;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void __fastcall TTNCSetDlg::TNCTypeClick(TObject *Sender)
{
	if( m_DisEvent ) return;

    UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TTNCSetDlg::SBHelpClick(TObject *Sender)
{
	ShowHtmlHelp("usemmttyasamodem.htm");
}
//---------------------------------------------------------------------------

