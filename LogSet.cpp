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

#include "LogSet.h"
#include "EditDlg.h"
#include "country.h"
#include "Loglink.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TLogSetDlg *LogSetDlg;
static int PageIndex = 0;
//---------------------------------------------------------------------
__fastcall TLogSetDlg::TLogSetDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	m_DisEvent = TRUE;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		TabConv->Caption = "Conversion";
		TabMisc->Caption = "Misc";
		CancelBtn->Caption = "Cancel";
		Caption = "Setup Logging";
		TabFlag->Caption = "Input";
		TabMacro->Caption = "QSO Button";
		GrpConv->Caption = "Convert UpperCase";
		GrpCopy->Caption = "Copy Before Data";
		CopyFreq->Caption = "Copy Band or Freq.";
		CopyFreq->Items->Strings[0] = "Copy Band";
		CopyFreq->Items->Strings[1] = "Copy Freq.";
		DefMyRST->Caption = "599 Default";
        CBBackup->Caption = "Make backup";

		MacroFlag->Caption = "Auto Macro";
		CheckBand->Caption = "Check SameBand";
		AutoZone->Caption = "Ignore daylight saving";
		CBRemC->Visible = FALSE;
        GB9->Caption = "Mode";
	}
	if( lcid == LANG_KOREAN ){
		TimeZone->Items->Strings[0] = "Korea";
	}
	else {
		TimeZone->Items->Strings[0] = "Japan";
	}
	SBHelp->Visible = !JanHelp.IsEmpty();

	m_MMList.QueryList("MML");
	for( int i = 0; i < m_MMList.GetCount(); i++ ){
		CBMMLink->Items->Add(m_MMList.GetItemName(i));
	}
}
//---------------------------------------------------------------------
void __fastcall TLogSetDlg::UpdateUI(void)
{
	int f = !AutoZone->Checked;
	TimeOff->Enabled = f;
	MinOff->Enabled = f;
	UDOffset->Enabled = f;
	UDMin->Enabled = f;
	ClearOff->Enabled = f;
	CBPoll->Enabled = RGLink->ItemIndex;
	CBMMLink->Enabled = (RGLink->ItemIndex == 2) ? TRUE : FALSE;
	LT->Enabled = CBMMLink->Enabled;
}
//---------------------------------------------------------------------
int __fastcall TLogSetDlg::Execute(void)
{
	int i;
	for( i = 0; i < 5; i++ ){
		m_MacroStr[i] = Log.m_LogSet.m_QSOMacroStr[i];
		m_MacroKey[i] = Log.m_LogSet.m_QSOMacroKey[i];
	}
	UpperName->Checked = Log.m_LogSet.m_UpperName;
	UpperQTH->Checked = Log.m_LogSet.m_UpperQTH;
	UpperREM->Checked = Log.m_LogSet.m_UpperREM;
	UpperQSL->Checked = Log.m_LogSet.m_UpperQSL;

	DefMyRST->Checked = Log.m_LogSet.m_DefMyRST;
	Contest->ItemIndex = Log.m_LogSet.m_Contest;

	CopyFreq->ItemIndex = Log.m_LogSet.m_CopyFreq;
	CopyHis->ItemIndex = Log.m_LogSet.m_CopyHis;
	CopyName->Checked = Log.m_LogSet.m_CopyName;
	CopyQTH->Checked = Log.m_LogSet.m_CopyQTH;
	CopyREM->Checked = Log.m_LogSet.m_CopyREM;
	CopyQSL->Checked = Log.m_LogSet.m_CopyQSL;
	CBRemC->Checked = Log.m_LogSet.m_CopyREMB4;
	CBBackup->Checked = Log.m_LogSet.m_Backup;

	if( Log.m_LogSet.m_TimeZone != 'I' ){
		TimeZone->ItemIndex = 1;
	}
	else {
		TimeZone->ItemIndex = 0;
	}
	AutoSave->Checked = Log.m_LogSet.m_AutoSave;
	MacroFlag->Checked = Log.m_LogSet.m_QSOMacroFlag;
	CheckBand->Checked = Log.m_LogSet.m_CheckBand;
	Macro1->Checked = Log.m_LogSet.m_QSOMacro[0];
	Macro2->Checked = Log.m_LogSet.m_QSOMacro[1];
	Macro3->Checked = Log.m_LogSet.m_QSOMacro[2];
	Macro4->Checked = Log.m_LogSet.m_QSOMacro[3];
	Macro5->Checked = Log.m_LogSet.m_QSOMacro[4];

	THRTTY->Text = Log.m_LogSet.m_THRTTY;
	THSSTV->Text = Log.m_LogSet.m_THSSTV;
	THTZ->ItemIndex = Log.m_LogSet.m_THTZ;
	ClipRSTADIF->Checked = Log.m_LogSet.m_ClipRSTADIF;
	DateType->ItemIndex = Log.m_LogSet.m_DateType;
	AutoZone->Checked = sys.m_AutoTimeOffset;
	UDOffset->Position = short(sys.m_TimeOffset);
	UDMin->Position = short(sys.m_TimeOffsetMin);
	RGLink->ItemIndex = sys.m_LogLink;
	CBPoll->Checked = LogLink.IsPolling();
    CBPTT->Checked = LogLink.GetPTTEnabled();
	CBMMLink->ItemIndex = CBMMLink->Items->IndexOf(LogLink.GetItemName());
	if( m_MMList.GetCount() ){
		RGLink->Controls[2]->Enabled = TRUE;
		if( CBMMLink->ItemIndex < 0 ){
			CBMMLink->ItemIndex = 0;
		}
	}
	else {
		RGLink->Controls[2]->Enabled = FALSE;
	}

	if( (PageIndex >= 0) && (PageIndex < Page->PageCount) ){
		if( Page->Pages[PageIndex]->TabVisible == FALSE ){
			PageIndex = 0;
		}
		Page->ActivePage = Page->Pages[PageIndex];
	}
	int r = FALSE;
	UpdateUI();
	m_DisEvent = FALSE;
	if( ShowModal() == IDOK ){
		Log.m_LogSet.m_UpperName = UpperName->Checked;
		Log.m_LogSet.m_UpperQTH = UpperQTH->Checked;
		Log.m_LogSet.m_UpperREM = UpperREM->Checked;
		Log.m_LogSet.m_UpperQSL = UpperQSL->Checked;

		Log.m_LogSet.m_DefMyRST = DefMyRST->Checked;
		Log.m_LogSet.m_Contest = Contest->ItemIndex;

		Log.m_LogSet.m_CopyFreq = CopyFreq->ItemIndex;
		Log.m_LogSet.m_CopyHis = CopyHis->ItemIndex;
		Log.m_LogSet.m_CopyName = CopyName->Checked;
		Log.m_LogSet.m_CopyQTH = CopyQTH->Checked;
		Log.m_LogSet.m_CopyREM = CopyREM->Checked;
		Log.m_LogSet.m_CopyQSL = CopyQSL->Checked;
		Log.m_LogSet.m_CopyREMB4 = ( Font->Charset != SHIFTJIS_CHARSET ) ? 0 : CBRemC->Checked;
		Log.m_LogSet.m_Backup = CBBackup->Checked;

		if( TimeZone->ItemIndex ){
			Log.m_LogSet.m_TimeZone = 'Z';
		}
		else {
			Log.m_LogSet.m_TimeZone = 'I';
		}

		Log.m_LogSet.m_AutoSave = AutoSave->Checked;
		Log.m_LogSet.m_QSOMacroFlag = MacroFlag->Checked;
		Log.m_LogSet.m_CheckBand = CheckBand->Checked;
		Log.m_LogSet.m_QSOMacro[0] = Macro1->Checked;
		Log.m_LogSet.m_QSOMacro[1] = Macro2->Checked;
		Log.m_LogSet.m_QSOMacro[2] = Macro3->Checked;
		Log.m_LogSet.m_QSOMacro[3] = Macro4->Checked;
		Log.m_LogSet.m_QSOMacro[4] = Macro5->Checked;

		for( i = 0; i < 5; i++ ){
			Log.m_LogSet.m_QSOMacroStr[i] = m_MacroStr[i];
			Log.m_LogSet.m_QSOMacroKey[i] = m_MacroKey[i];
		}
		Log.m_LogSet.m_THRTTY = THRTTY->Text;
		Log.m_LogSet.m_THSSTV = THSSTV->Text;
		Log.m_LogSet.m_THTZ = THTZ->ItemIndex;
		Log.m_LogSet.m_ClipRSTADIF = ClipRSTADIF->Checked;
		Log.m_LogSet.m_DateType = DateType->ItemIndex;

		sys.m_AutoTimeOffset = AutoZone->Checked;
		sys.m_TimeOffset = UDOffset->Position;
		sys.m_TimeOffsetMin = UDMin->Position;
		if( sys.m_AutoTimeOffset ){
			SetTimeOffsetInfo(sys.m_TimeOffset, sys.m_TimeOffsetMin);
		}
		sys.m_LogLink = RGLink->ItemIndex;
		LogLink.SetPolling(CBPoll->Checked);
        LogLink.SetPTTEnabled(CBPTT->Checked);
		if( CBMMLink->ItemIndex >= 0 ){
			LogLink.SetItemName(AnsiString(CBMMLink->Items->Strings[CBMMLink->ItemIndex]).c_str());	//JA7UDE 0428
		}
		r = TRUE;
	}
	for( PageIndex = 0; PageIndex < Page->PageCount; PageIndex++ ){
		if( Page->ActivePage == Page->Pages[PageIndex] ) break;
	}
	return r;
}
//---------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtnClick(int n)
{
	if( n >= 0 ){
		TEditDlgBox *pBox = new TEditDlgBox(this);
		pBox->ButtonName->Text = "";
		pBox->Execute(m_MacroStr[n], m_MacroKey[n], NULL, NULL, 2);
		delete pBox;
	}
}
//---------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtn1Click(TObject *Sender)
{
	MacroBtnClick(0);
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtn2Click(TObject *Sender)
{
	MacroBtnClick(1);
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtn3Click(TObject *Sender)
{
	MacroBtnClick(2);
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtn4Click(TObject *Sender)
{
	MacroBtnClick(3);
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::MacroBtn5Click(TObject *Sender)
{
	MacroBtnClick(4);
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::ClearOffClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UDOffset->Position = 0;
	UDMin->Position = 0;
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::AutoZoneClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( AutoZone->Checked ){
		int hour, min;
		SetTimeOffsetInfo(hour, min);
		UDOffset->Position = short(hour);
		UDMin->Position = short(min);
	}
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::SBHelpClick(TObject *Sender)
{
#if 1
	ShowHtmlHelp("logging.htm");
#else
	int n;

	if( Page->ActivePage == TabMacro ){
		n = 20;
	}
	else {
		n = 18;
	}
	ShowHelp(n);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::ContestClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	switch(Contest->ItemIndex){
		case testOFF:
			CopyHis->ItemIndex = 0;
			DefMyRST->Checked = FALSE;
			MacroFlag->Checked = FALSE;
			break;
		case testON:
			CopyHis->ItemIndex = 1;
			DefMyRST->Checked = TRUE;
			MacroFlag->Checked = TRUE;
			break;
		case testCQRJ:
			CopyHis->ItemIndex = 0;
			DefMyRST->Checked = TRUE;
			MacroFlag->Checked = TRUE;
			break;
		case testPED:
			CopyHis->ItemIndex = 0;
			DefMyRST->Checked = TRUE;
			MacroFlag->Checked = TRUE;
			break;
		case testBARTG:
			CopyHis->ItemIndex = 2;
			DefMyRST->Checked = TRUE;
			MacroFlag->Checked = TRUE;
			break;
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TLogSetDlg::RGLinkClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------

