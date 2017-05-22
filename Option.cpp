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



//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Option.h"
#include "EditDlg.h"
#include "LogFile.h"
#include "Main.h"
#include "FreqDisp.h"
#include "ClockAdj.h"
#include "radioset.h"
#include "TxdDlg.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
//TAgcSetDlg *AgcSetDlg;
int PageIndex = 0;
static int PageIndexBPF = 0;

// Static array to map selected audio devices from radio group index
// to unit number - one each for input & output devices
// K6TU 1.70A 3/17/2015
static int InputDeviceMap[16];
static int OutputDeviceMap[16];
//---------------------------------------------------------------------
__fastcall TOptionDlg::TOptionDlg(TComponent* AOwner)
	: TForm(AOwner)
{

	m_DisEvent = 1;
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	pDem = NULL;
	m_FontCharset = SHIFTJIS_CHARSET;
	m_FontStyle = 0;
	m_ColorChange = 0;
	UpdateMacro();
	MarkFreq->Items->Assign(MmttyWd->MarkFreq->Items);
	ShiftFreq->Items->Assign(MmttyWd->ShiftFreq->Items);
	SetComboBox(IIRFW, MmttyWd->m_asIIRBW.c_str());
	SetComboBox(FilterTap, MmttyWd->m_asFilterTap.c_str());
	SetComboBox(SmoozFreq, MmttyWd->m_asSmooth.c_str());
	SmoozIIR->Items->Assign(SmoozFreq->Items);
	SetComboBox(BaudRate, MmttyWd->m_asBaud.c_str());
	SetComboBox(pllVCOGain, MmttyWd->m_asVCOGain.c_str());
	SetComboBox(pllLoopFC, MmttyWd->m_asLoopFC.c_str());

	//K6TU 1.70A
	// Amended enumeration of audio units to review the
	// first 32 units for each of input & output.
	//
	// To address the introduction of DAX by FlexRadio systems
	// for their Flex-6000 series radios, only devices that do NOT
	// include the substrings "IQ" or "RESERVED" are provided
	// as choices to the user.
	int CountUnits = 0;
	int CurrentUnit = 0;
	LPCSTR devName;
	char *cString;

	//AA6YQ 1.66
	InputSoundcards->Items->BeginUpdate();
	InputSoundcards->Items->Clear();

	while (CountUnits < 16 && CurrentUnit < 32) {
		devName = MmttyWd->pSound->GetInputSoundcard(CurrentUnit);
		cString = AnsiString(devName).c_str();

		if (sys.m_HideFlexAudio) { //AA6YQ 1.70E
			if (strstr(cString, "IQ") || strstr(cString, "RESERVED")) {
				// This is one of the FlexRadio audio devices we don't want
				CurrentUnit++;
				continue;
			}
		}

		// This is a device we want...
		if (devName) {
			InputSoundcards->Items->Add(devName);
			InputDeviceMap[CountUnits++] = CurrentUnit++;
		} else {
			CurrentUnit++;
		}
	}
	// for( int i = 0; i < 16; i++ ){
	// 		InputSoundcards->Items->Add(MmttyWd->pSound->GetInputSoundcard(i));
	// }
	InputSoundcards->Items->EndUpdate();

	OutputSoundcards->Items->BeginUpdate();
	OutputSoundcards->Items->Clear();

	CountUnits = 0;
	CurrentUnit = 0;
	while (CountUnits < 16 && CurrentUnit < 32) {
		devName = MmttyWd->pSound->GetOutputSoundcard(CurrentUnit);
		cString = AnsiString(devName).c_str();

		if (sys.m_HideFlexAudio) { //AA6YQ 1.70E
			if (strstr(cString, "IQ") || strstr(cString, "RESERVED")) {
				// This is one of the FlexRadio audio devices we don't want
				CurrentUnit++;
				continue;
			}
		}

		// This is a device we want...
		if (devName) {
			OutputSoundcards->Items->Add(devName);
			OutputDeviceMap[CountUnits++] = CurrentUnit++;
		} else {
			CurrentUnit++;
		}
	}
	//for( int i = 0; i < 16; i++ ){
	//	OutputSoundcards->Items->Add(MmttyWd->pSound->GetOutputSoundcard(i));
	//}
	OutputSoundcards->Items->EndUpdate();


	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = Remote ? SETUPTITLEREMOTE:SETUPTITLE;   //AA6YQ 1.66C
		DefBtn->Caption = "Set Default(Demodulator)";
		TabPara->Caption = "Decode";
		TabTx->Caption = "TX";
		TabFont->Caption = "Font/Window";
		TabMisc->Caption = "Misc";
		GrpDisc->Caption = "Discriminator";
		GrpLimit->Caption = "Limit Amp.";
		GrpFilter->Caption = "Pre-Filter";
		DemType->Caption = "Type";
		DemType->Items->Strings[0] = "IIR resonator";
		DemType->Items->Strings[1] = "FIR BPF";
		DemType->Items->Strings[2] = "PLL";
		DemBpfBtn->Caption = "Show";
		GrpLPF->Caption = "Smooth LPF";
		LSmooz->Caption = "Order";
		PreBpfBtn->Caption = "Show";
		CheckRXBPFAFC->Caption = "AFC Connection";
		LMU2->Caption = "2µ";
		LGM->Caption = "Gm.";
		CancelBtn->Caption = "Cancel";
		LLoopOrder->Caption = "Order";
		LOutOrder->Caption = "Order";
		LBaudRate->Caption = "BaudRate";
		CheckMajority->Caption = "Majority Logic";
		CheckIgnoreFream->Caption = "Ignore framing error";
		BitLen->Caption = "BitLength";
		StopLen->Caption = "StopLength";
		ParitySel->Caption = "Parity";
		DefStopBit->Caption = "Default RxStop bit";
		CheckDblSft->Caption = "Double shift";
		LOutLvl->Caption = "Digital Output";
		LCharW->Caption = "Char. Wait";
		PttInv->Caption = "Invert Logic";
		GrpInBtn->Caption = "Input Button";
		MacroImm->Caption = "Convert Immediately";
		LFont->Caption = "Font";
		FontBtn->Caption = "Ref";
		CheckPalette->Caption = "Use Palette";
		CheckZero->Caption = "slash zero";
		GrpDispAdj->Caption = "Adjust";
		LW->Caption = "Width";
		LH->Caption = "Height";
		XYInv->Caption = "Reverse rotation";
		GrpRx->Caption = "RxWindow";
		GrpIn->Caption = "TxWindow";
		LRXB->Caption = "Back";
		LRXC->Caption = "Char.";
		LRXS->Caption = "Char. sent";
		LINB->Caption = "Back";
		LINC->Caption = "Char.";
		LINS->Caption = "Char. sent";
		GrpCard->Caption = "Sound Card";
		RGLoop->Caption = "Sound loopback";
		RGLoop->Items->Strings[1] = "Int.";
		RGLoop->Items->Strings[2] = "Ext.(SAT)";
		TxPort->Caption = "Tx Port";
		TxPort->Items->Strings[0] = "Sound";
		TxPort->Items->Strings[1] = "Sound + COM-TxD (FSK)";

		CBFix45->Caption = "Fixes 45.45 baud";
		SBIN1->Font->Name = sys.m_BtnFontName;
		SBIN1->Font->Charset = sys.m_BtnFontCharset;
		SBIN2->Font->Name = sys.m_BtnFontName;
		SBIN2->Font->Charset = sys.m_BtnFontCharset;
		SBIN3->Font->Name = sys.m_BtnFontName;
		SBIN3->Font->Charset = sys.m_BtnFontCharset;
		SBIN4->Font->Name = sys.m_BtnFontName;
		SBIN4->Font->Charset = sys.m_BtnFontCharset;
		for( int i = 0; i < 16; i++ ){
			GetSB(i)->Font->Name = sys.m_BtnFontName;
			GetSB(i)->Font->Charset = sys.m_BtnFontCharset;
		}
	}
	else if( Remote ){
		Caption = "Ý’è‰æ–Ê";
	}
	if( Remote ){
		GB5->Visible = FALSE;
		GrpInBtn->Visible = FALSE;
		LFont->Visible = FALSE;
		FontName->Visible = FALSE;
		FontBtn->Visible = FALSE;
		CheckZero->Visible = FALSE;
		GrpDispAdj->Visible = FALSE;
		GrpRx->Visible = FALSE;
		GrpIn->Visible = FALSE;
		MemWin->Visible = FALSE;
		if( Remote & REMNOPTT ){
			GPTT->Visible = FALSE;
		}
		if( Remote & REMSHOWOFF ){
			TabFont->TabVisible = FALSE;
			GSFont->Visible = FALSE;
		}
		if( !(Remote & REMCTRL) ){
			GSFont->Visible = FALSE;
		}
	}
	SBHelp->Visible = !JanHelp.IsEmpty();

//AA6YQ 1.66B moved here from TOptionDlg::DevNoDropDown

	m_MMListW.QueryList("MMW");
	for( int i = 0; i < m_MMListW.GetCount(); i++ ){
		DevNo->Items->Add(m_MMListW.GetItemName(i));
		DevOutNo->Items->Add(m_MMListW.GetItemName(i));
		}

}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::UpdateUI(void)
{
	if( PortName->Text == "NONE" ){
		TxPort->Controls[1]->Enabled = FALSE;
		TxPort->Controls[2]->Enabled = FALSE;
		TxdJob->Visible = FALSE;
	}
	else {
		TxPort->Controls[1]->Enabled = TRUE;
		TxPort->Controls[2]->Enabled = TRUE;
		TxdJob->Visible = TxPort->ItemIndex ? TRUE : FALSE;
	}
	switch(DemType->ItemIndex){
		case 0:		// IIR
			IIRFW->Visible = TRUE;
			FilterTap->Visible = FALSE;
			Tones->Visible = FALSE;
			LTAPFW->Caption = "BW";
			LHZ->Visible = TRUE;
			LTAPFW->Visible = TRUE;
			DemBpfBtn->Visible = TRUE;
			GrpLimit->Enabled = TRUE;
			break;
		case 1:		// FIR
			IIRFW->Visible = FALSE;
			FilterTap->Visible = TRUE;
			Tones->Visible = FALSE;
			LTAPFW->Caption = "Tap";
			LHZ->Visible = FALSE;
			LTAPFW->Visible = TRUE;
			DemBpfBtn->Visible = TRUE;
			GrpLimit->Enabled = TRUE;
			break;
		case 2:
			IIRFW->Visible = FALSE;
			Tones->Visible = FALSE;
			LHZ->Visible = FALSE;
			FilterTap->Visible = FALSE;
			LTAPFW->Visible = FALSE;
			DemBpfBtn->Visible = FALSE;
			GrpLimit->Enabled = TRUE;
			break;
		case 3:		// FFT
			IIRFW->Visible = FALSE;
			FilterTap->Visible = FALSE;
			Tones->Visible = TRUE;
			LTAPFW->Caption = "Tones";
			LHZ->Visible = FALSE;
			LTAPFW->Visible = TRUE;
			DemBpfBtn->Visible = FALSE;
			GrpLimit->Enabled = FALSE;
			break;
	}
	SetGroupEnabled(GrpLimit);

	if( DemLpf->ItemIndex ){	// IIR
		LSmooz->Visible = TRUE;
		DemLpfOrder->Visible = TRUE;
		SmoozFreq->Visible = FALSE;
		SmoozIIR->Visible = TRUE;
		LLPF->Caption = "LPF";
//        DispDemLpf->Visible = TRUE;
	}
	else {						// FIR-Avg
		LSmooz->Visible = FALSE;
		DemLpfOrder->Visible = FALSE;
		SmoozFreq->Visible = TRUE;
		SmoozIIR->Visible = FALSE;
		LLPF->Caption = "Freq";
//        DispDemLpf->Visible = FALSE;
	}
	int f;
	m_DisEvent++;
	if( lmsType->Checked ){
		lmsTap->Text = m_NotchTap;
		f = FALSE;
	}
	else {
		lmsTap->Text = m_lmsTap;
		f = TRUE;
	}
	m_DisEvent--;
	LDelay->Visible = f;
	LMU2->Visible = f;
	LGM->Visible = f;
	lmsDelay->Visible = f;
	lmsMU2->Visible = f;
	lmsGM->Visible = f;
	lmsAGC->Visible = f;
	lmsBPF->Visible = f;
	lmsInv->Visible = f;
	lmsTwo->Visible = !f;

	int dd;
	f = (sscanf(AnsiString(DevNo->Text).c_str(), "%d", &dd) == 1 );	//JA7UDE 0428
	GB4->Enabled = f;
	SetGroupEnabled(GB4);
	Source->Enabled = f;

}
//---------------------------------------------------------------------------
TSpeedButton *__fastcall TOptionDlg::GetSB(int n)
{
	TSpeedButton *_sb[]={SBM1, SBM2, SBM3, SBM4, SBM5, SBM6, SBM7,
						 SBM8, SBM9, SBM10, SBM11, SBM12, SBM13, SBM14, SBM15, SBM16};
	if( (n >= 0) && (n < AN(_sb)) ){
		return _sb[n];
	}
	else {
		return NULL;
	}
}
//---------------------------------------------------------------------------
int __fastcall TOptionDlg::GetMacroIndex(TObject *Sender)
{
	for( int i = 0; i < 16; i++ ){
		TSpeedButton *_sb[]={SBM1, SBM2, SBM3, SBM4, SBM5, SBM6, SBM7,
							 SBM8, SBM9, SBM10, SBM11, SBM12, SBM13, SBM14, SBM15, SBM16};

		if( ((TObject *)_sb[i]) == Sender ) return i;
	}
	return -1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::UpdateMacro(void)
{
	SetButtonCaption(SBIN1, sys.m_InBtnName[0], 6, sys.m_SBINFontSize, sys.m_InBtnCol[0]);
	SetButtonCaption(SBIN2, sys.m_InBtnName[1], 6, sys.m_SBINFontSize, sys.m_InBtnCol[1]);
	SetButtonCaption(SBIN3, sys.m_InBtnName[2], 6, sys.m_SBINFontSize, sys.m_InBtnCol[2]);
	SetButtonCaption(SBIN4, sys.m_InBtnName[3], 6, sys.m_SBINFontSize, sys.m_InBtnCol[3]);
	for( int i = 0; i < 16; i++ ){
		SetButtonCaption(GetSB(i), sys.m_UserName[i], 4, sys.m_SBFontSize, sys.m_UserCol[i]);
	}
}

//AA6YQ 1.66
int __fastcall TOptionDlg::IsSoundcard(LPCSTR t)
{
	for( ; ; t++ ){
		if (*t=='\0') {
			return 1;
		}
		else if ((*t<'0')||(*t>'9')){
			return 0;
		}
	}
}

int __fastcall TOptionDlg::Execute(CFSKDEM *fp, CFSKMOD *mp)
{

	pDem = fp;
	m_ColorChange = 0;

	m_TxdJob = sys.m_TxdJob;
	EditSamp->Text = sys.m_SampFreq;
	char bf[32];
	sprintf(bf, "%.2lf", sys.m_TxOffset);
	TxOff->Text = bf;
	WinFont->Text = sys.m_WinFontName;
	m_WinFontCharset = sys.m_WinFontCharset;
	m_WinFontStyle = sys.m_WinFontStyle;
	MemWin->Checked = sys.m_MemWindow;

	BtnFont->Text = sys.m_BtnFontName;
	m_BtnFontCharset = sys.m_BtnFontCharset;
	m_BtnFontStyle = sys.m_BtnFontStyle;

	FontAdj->Text = sys.m_FontAdjSize;
	BtnFontAdj->Text = sys.m_BtnFontAdjSize;

	DemType->ItemIndex = fp->m_type;
	RGLoop->ItemIndex = sys.m_echo;
	MacroImm->Checked = sys.m_MacroImm;

	pllVCOGain->Text = fp->m_pll.m_vcogain;
	pllLoopOrder->Text = fp->m_pll.m_loopOrder;
	pllLoopFC->Text = fp->m_pll.m_loopFC;
	pllOutOrder->Text = fp->m_pll.m_outOrder;
	pllOutFC->Text = fp->m_pll.m_outFC;

	WaitDiddle->Checked = mp->m_CharWaitDiddle;
	TBDiddleWait->Position = SetTBValue(mp->m_DiddleWait, 50, 50);
	TBCharWait->Position = SetTBValue(mp->m_CharWait, 50, 50);

	PCRXBack->Color = sys.m_ColorRXBack;
	PCRXChar->Color = sys.m_ColorRX;
	PCRXTXChar->Color = sys.m_ColorRXTX;
	PCINBack->Color = sys.m_ColorINBack;
	PCINChar->Color = sys.m_ColorIN;
	PCINTx->Color = sys.m_ColorINTX;
	PCLow->Color = sys.m_ColorLow;
	PCHigh->Color = sys.m_ColorHigh;
	PCXY->Color = sys.m_ColorXY;

	TxPort->ItemIndex = sys.m_TxPort;
	CBFix45->Checked = sys.m_DefFix45;
	DefMarkFreq->Text = sys.m_DefMarkFreq;
	DefShift->Text = sys.m_DefShift;
	DefStopBit->ItemIndex = sys.m_DefStopLen;

	MarkFreq->Text = fp->GetMarkFreq();
	ShiftFreq->Text = fp->GetSpaceFreq() - fp->GetMarkFreq();
	FilterTap->Text = fp->GetFilterTap();
	IIRFW->Text = fp->m_iirfw;
    Tones->Text = fp->m_Phase.m_TONES;
	CBAA6YQ->Checked = fp->m_AA6YQ.m_fEnabled;

	SmoozFreq->Text = fp->GetSmoozFreq();
	SmoozIIR->Text = fp->m_lpffreq;
	DemLpf->ItemIndex = fp->m_lpf;
	DemLpfOrder->Text = fp->m_lpfOrder;
	LimitGain->Text = sys.m_LimitGain;
	LimitOver->Checked = fp->m_LimitOverSampling;
	LimitAGC->Checked = fp->m_LimitAGC;

	CheckInv->Checked = sys.m_Rev;

	BaudRate->Text = fp->GetBaudRate();
	BitLen->ItemIndex = fp->m_BitLen - 5;
	StopLen->ItemIndex = fp->m_StopLen;
	ParitySel->ItemIndex = fp->m_Parity;
	RGC->ItemIndex = sys.m_CodeSet;

	PortName->Text = sys.m_TxRxName;
	PttInv->Checked = sys.m_TxRxInv;

	DiddleSel->ItemIndex = mp->m_diddle;
	RandomDiddle->Checked = mp->m_RandomDiddle;
	WaitTimer->Checked = mp->m_WaitTimer;

	EditCall->Text = sys.m_Call;

	CheckTXUOS->Checked = sys.m_txuos;
	CheckTXBPF->Checked = mp->m_bpf;
	CheckDblSft->Checked = sys.m_dblsft;
	CheckTXLPF->Checked = mp->m_lpf;
	TxLpfFreq->Text = mp->GetLPFFreq();

	TxBpfTap->Text = mp->m_bpftap;
	TxDisRev->Checked = sys.m_TxDisRev;
	TxFixShift->Checked = sys.m_TxFixShift;

	CheckAFC->Checked = sys.m_AFC;
	FixShift->ItemIndex = sys.m_FixShift;
	AFCTime->Text = sys.m_AFCTime;
	AFCSQ->Text = sys.m_AFCSQ;
	AFCSweep->Text = sys.m_AFCSweep;

	ATC->Checked = fp->m_atc;
	ATCTime->Text = fp->m_atcMark.m_Max;

	TBOutLvl->Position = int(mp->GetOutputGain()*64.0/32768.0);

	FontName->Text = sys.m_FontName;
	FontAdjX->Text = sys.m_FontAdjX;
	FontAdjY->Text = sys.m_FontAdjY;
	m_FontCharset = sys.m_FontCharset;
	m_FontStyle = sys.m_FontStyle;
	m_FontSize = sys.m_FontSize;
	CheckZero->Checked = sys.m_FontZero;

	CheckPalette->Checked = sys.m_Palette;

	XYInv->Checked = sys.m_XYInv;

	FifoRX->Text = sys.m_SoundFifoRX;
	FifoTX->Text = sys.m_SoundFifoTX;
	SoundPriority->ItemIndex = sys.m_SoundPriority;

	HideFlexAudio->Checked = sys.m_HideFlexAudio; //AA6YQ 1.70E

	if( sys.m_SoundDevice == -2 ){
		DevNo->Text = sys.m_SoundMMW;
	}
    else {
		DevNo->Text = sys.m_SoundDevice;
    }

    //AA6YQ 1.66
	if( sys.m_SoundOutDevice == -2 ){
        DevOutNo->Text = sys.m_SoundMMW;
    }
	else {
		DevOutNo->Text = sys.m_SoundOutDevice;
	}

	//AA6YQ 1.66
	//K6TU 1.70A
	if (IsSoundcard (AnsiString(DevNo->Text).c_str())) {	//JA7UDE 0428

		// Assuming that devices haven't been re-enumerated by Windows,
		// we need to find the corresponding unit number in the map in order
		// to select the right unit
		int unitNum = atoi(AnsiString(DevNo->Text).c_str());
		int i;
		for (i=0; i < 16; i++) {
			if (InputDeviceMap[i] == unitNum) {
				break;
			}
		}

		InputSoundcards->ItemIndex = i != 16 ? i : -1;
		// InputSoundcards->ItemIndex = atoi(AnsiString(DevNo->Text).c_str());  //AA6YQ 1.66	//JA7UDE 0428
	}
	else {
		InputSoundcards->ItemIndex =-1;
	}

	//AA6YQ 1.66
	//K6TU 1.70A
	if (IsSoundcard (AnsiString(DevOutNo->Text).c_str())) {	//JA7UDE 0428

		// Assuming that devices haven't been re-enumerated by Windows,
		// we need to find the corresponding unit number in the map in order
		// to select the right unit
		int unitNum = atoi(AnsiString(DevOutNo->Text).c_str());
		int i;
		for (i=0; i < 16; i++) {
			if (OutputDeviceMap[i] == unitNum) {
				break;
			}
		}

		OutputSoundcards->ItemIndex = i != 16 ? i : -1;
		// OutputSoundcards->ItemIndex = atoi(AnsiString(DevOutNo->Text).c_str());  //AA6YQ 1.66	//JA7UDE 0428

	}
	else {
		OutputSoundcards->ItemIndex =-1;
	}

	Source->ItemIndex = sys.m_SoundStereo;

	SelBPF->Checked = MmttyWd->pSound->m_bpf;
	SelLMS->Checked = MmttyWd->pSound->m_lmsbpf;
	CheckRXBPFAFC->Checked = MmttyWd->pSound->m_bpfafc;
	RxBpfTap->Text = MmttyWd->pSound->m_bpftap;
	RxBpfFW->Text = MmttyWd->pSound->m_bpffw;
	m_lmsTap = MmttyWd->pSound->m_lms.m_Tap;
	m_NotchTap = MmttyWd->pSound->m_lms.m_NotchTap;
	//	lmsTap->Text = MmttyWd->pSound->m_lms.m_Tap;
	lmsType->Checked = MmttyWd->pSound->m_lms.m_Type;
	lmsDelay->Text = MmttyWd->pSound->m_lms.m_lmsDelay;
	lmsMU2->Text = MmttyWd->pSound->m_lms.m_lmsMU2;
	lmsGM->Text = MmttyWd->pSound->m_lms.m_lmsGM;
	lmsAGC->Checked = MmttyWd->pSound->m_lms.m_lmsAGC;
	lmsInv->Checked = MmttyWd->pSound->m_lms.m_lmsInv;
	lmsBPF->Checked = MmttyWd->pSound->m_lms.m_bpf;
	lmsTwo->Checked = MmttyWd->pSound->m_lms.m_twoNotch;

	CheckMajority->Checked = fp->m_majority;
	CheckIgnoreFream->Checked = fp->m_ignoreFream;

	if( (PageIndex >= 0) && (PageIndex < Page->PageCount) ){
		if( Page->Pages[PageIndex]->TabVisible == FALSE ){
			PageIndex = 0;
		}
		Page->ActivePage = Page->Pages[PageIndex];
	}
	if( (PageIndexBPF >= 0) && (PageIndexBPF < PageBPF->PageCount) ){
		PageBPF->ActivePage = PageBPF->Pages[PageIndexBPF];
	}
	UpdateUI();
	m_DisEvent = 0;

	SetupTimer->Enabled= True; //1.70K

	int r = ShowModal();
	if( r == IDOK ){
		m_DisEvent++;
		double d;
		int dd;

		sys.m_TxdJob = m_TxdJob;
		sscanf(AnsiString(EditSamp->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 5000.0) && (d <= 12500.0) ){
			sys.m_SampFreq = d;
		}
		sscanf(AnsiString(TxOff->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		sys.m_TxOffset = d;

		sys.m_WinFontName = WinFont->Text;
		sys.m_WinFontCharset = m_WinFontCharset;
		sys.m_WinFontStyle = m_WinFontStyle;

		sys.m_BtnFontName = BtnFont->Text;
		sys.m_BtnFontCharset = m_BtnFontCharset;
		sys.m_BtnFontStyle = m_BtnFontStyle;

		if( sscanf(AnsiString(FontAdj->Text).c_str(), "%ld", &dd) == 1 ){	//JA7UDE 0428
			sys.m_FontAdjSize = dd;
		}
		if( sscanf(AnsiString(BtnFontAdj->Text).c_str(), "%ld", &dd) == 1 ){	//JA7UDE 0428
			sys.m_BtnFontAdjSize = dd;
		}

		fp->m_type = DemType->ItemIndex;
		sys.m_echo = RGLoop->ItemIndex;
		sys.m_MacroImm = MacroImm->Checked;

		sys.m_DefFix45 = CBFix45->Checked;
		sscanf(AnsiString(DefMarkFreq->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= MARKL) && (d <= SPACEH) ){
			sys.m_DefMarkFreq = d;
		}
		sscanf(AnsiString(DefShift->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= SHIFTL) && (d <= 1500.0) ){
			sys.m_DefShift = d;
		}
		sys.m_DefStopLen = DefStopBit->ItemIndex;

		sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= MARKL) && (d <= SPACEH) ){
			fp->SetMarkFreq(d);
			mp->SetMarkFreq(d);
		}
		sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= SHIFTL) && (d <= 1500.0) ){
			d += fp->GetMarkFreq();
			fp->SetSpaceFreq(d);
			mp->SetSpaceFreq(d);
		}
		sscanf(AnsiString(FilterTap->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( dd >= 2 ){
			fp->SetFilterTap(dd);
		}
		sscanf(AnsiString(IIRFW->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( d > 0 ){
			fp->SetIIR(d);
		}
		sscanf(AnsiString(Tones->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( (dd >= 2) && (dd <= 6) ){
			fp->m_Phase.SetTones(dd);
		}

		if( fp->m_AA6YQ.m_fEnabled != CBAA6YQ->Checked ){
			fp->m_AA6YQ.m_fEnabled = CBAA6YQ->Checked;
			if( CBAA6YQ->Checked ) fp->m_AA6YQ.Create();
		}

		fp->m_lpf = DemLpf->ItemIndex;
		sscanf(AnsiString(DemLpfOrder->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( (dd >= 1) && (dd <= 32) ){
			fp->m_lpfOrder = dd;
		}

		sscanf(AnsiString(SmoozFreq->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 20.0) && (d <= 1500.0) ){
			fp->SetSmoozFreq(d);
		}
		sscanf(AnsiString(SmoozIIR->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 20.0) && (d <= 1500.0) ){
			fp->SetLPFFreq(d);
		}

		sscanf(AnsiString(LimitGain->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d > 0.0) && (d <= 32768.0) ){
			sys.m_LimitGain = d;
		}
		fp->m_LimitOverSampling = LimitOver->Checked;
		fp->m_LimitAGC = LimitAGC->Checked;
		MmttyWd->UpdateLimit();


		sscanf(AnsiString(BaudRate->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( d > 0.0 ){
			fp->SetBaudRate(d);
			mp->SetBaudRate(d);
		}

		sys.m_TxFixShift = TxFixShift->Checked;
		sys.m_TxDisRev = TxDisRev->Checked;
		sys.m_Rev = CheckInv->Checked;
		MmttyWd->UpdateRev();

		fp->m_BitLen = BitLen->ItemIndex + 5;
		fp->m_StopLen = StopLen->ItemIndex;
		fp->m_Parity = ParitySel->ItemIndex;
		sys.m_CodeSet = RGC->ItemIndex;

		sys.m_TxRxName = PortName->Text;
		if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//JA7UDE 0428
			strcpy(RADIO.StrPort, "NONE");
		}
		sys.m_TxRxInv = PttInv->Checked;

		mp->m_diddle = DiddleSel->ItemIndex;
		mp->m_RandomDiddle = RandomDiddle->Checked;
		mp->m_WaitTimer = WaitTimer->Checked;

		char bf[MLCALL+1];
		StrCopy(bf, AnsiString(EditCall->Text).c_str(), MLCALL);	//JA7UDE 0428
		jstrupr(bf);
		sys.m_Call = bf;

		sys.m_txuos = CheckTXUOS->Checked;
		mp->m_bpf = CheckTXBPF->Checked;
		sys.m_dblsft = CheckDblSft->Checked;

		mp->m_lpf = CheckTXLPF->Checked;
		sscanf(AnsiString(TxLpfFreq->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 20.0) && (d <= 2000.0) ){
			mp->SetLPFFreq(d);
		}

		sscanf(AnsiString(TxBpfTap->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( dd >= 2 ){
			mp->m_bpftap = dd;
			mp->CalcBPF();
		}

		sys.m_AFC = CheckAFC->Checked;
		sys.m_FixShift = FixShift->ItemIndex;

		sscanf(AnsiString(AFCTime->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 1.0) && (d <= 128.0) ){
			sys.m_AFCTime = d;
		}
		sscanf(AnsiString(AFCSweep->Text).c_str(), "%lf", &d);	//JA7UDE 0428
		if( (d >= 0.01) && (d <= 1.0) ){
			sys.m_AFCSweep = d;
		}
		sscanf(AnsiString(AFCSQ->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( dd >= 2 ){
			sys.m_AFCSQ = dd;
		}

		fp->m_atc = ATC->Checked;
		sscanf(AnsiString(ATCTime->Text).c_str(), "%u", &dd);	//JA7UDE 0428
		if( (dd >= 0) && (dd <= 16) ){
			fp->m_atcMark.m_Max = dd;
			fp->m_atcSpace.m_Max = dd;
		}

		mp->SetOutputGain(TBOutLvl->Position*32768.0/64.0);

		mp->m_BitLen = fp->m_BitLen;
		mp->m_StopLen = fp->m_StopLen;
		mp->m_Parity = fp->m_Parity;

		r = 1;
		if( sys.m_FontName != FontName->Text ){
			sys.m_FontName = FontName->Text;
			r = 2;
		}
		if( sscanf(AnsiString(FontAdjX->Text).c_str(), "%d", &dd) == 1 ){	//JA7UDE 0428
			if( dd != sys.m_FontAdjX ){
				r = 2;
				sys.m_FontAdjX = dd;
			}
		}
		if( sscanf(AnsiString(FontAdjY->Text).c_str(), "%d", &dd) == 1 ){	//JA7UDE 0428
			if( dd != sys.m_FontAdjY ){
				r = 2;
				sys.m_FontAdjY = dd;
			}
		}
		if( (sys.m_FontCharset != m_FontCharset) ||
			(sys.m_FontStyle != m_FontStyle) ||
			(sys.m_FontSize != m_FontSize)
		){
			sys.m_FontCharset = m_FontCharset;
			sys.m_FontStyle = m_FontStyle;
			sys.m_FontSize = m_FontSize;
			r = 2;
		}
		sys.m_FontZero = CheckZero->Checked;

		sys.m_Palette = CheckPalette->Checked;

		sys.m_XYInv = XYInv->Checked;

		if( sscanf(AnsiString(FifoRX->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd >= 2) && (dd <= 16) ){
				sys.m_SoundFifoRX = dd;
			}
		}
		if( sscanf(AnsiString(FifoTX->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd >= 2) && (dd <= 16) ){
				sys.m_SoundFifoTX = dd;
			}
		}

		sys.m_SoundPriority = SoundPriority->ItemIndex;

		sys.m_HideFlexAudio=HideFlexAudio->Checked; //AA6YQ 1.70E

		if( sscanf(AnsiString(DevNo->Text).c_str(), "%d", &dd) == 1 ){	//JA7UDE 0428
			//K6TU 1.70A Find the unit number in the input map and update
			sys.m_SoundDevice = InputDeviceMap[dd];
			// sys.m_SoundDevice = dd;
		}
		else {
			sys.m_SoundDevice = -2;
			sys.m_SoundMMW = DevNo->Text.c_str();
		}

		//AA6YQ 1.66
		if( sscanf(AnsiString(DevOutNo->Text).c_str(), "%d", &dd) == 1 ){	//JA7UDE 0428
			//K6TU 1.70A Find the unit in the output map and update
			sys.m_SoundOutDevice = OutputDeviceMap[dd];
			// sys.m_SoundOutDevice = dd;
		}
		else {
			sys.m_SoundOutDevice = -2;
			sys.m_SoundMMW = DevOutNo->Text.c_str();
		}

		sys.m_SoundStereo = Source->ItemIndex;

		MmttyWd->pSound->m_bpf = SelBPF->Checked;
		MmttyWd->pSound->m_lmsbpf = SelLMS->Checked;
		MmttyWd->pSound->m_bpfafc = CheckRXBPFAFC->Checked;
		if( sscanf(AnsiString(RxBpfTap->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd >= 2) && (dd <= 512) ){
				MmttyWd->pSound->m_bpftap = dd;
			}
		}
		if( sscanf(AnsiString(RxBpfFW->Text).c_str(), "%lf", &d) == 1 ){	//JA7UDE 0428
			if( (d >= 0.0) && (d <= 500.0) ){
				MmttyWd->pSound->m_bpffw = d;
			}
		}
		if( sscanf(AnsiString(lmsDelay->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd >= 0) && (dd <= DELAYMAX) ){
				MmttyWd->pSound->m_lms.m_lmsDelay = dd;
			}
		}
		MmttyWd->pSound->m_lms.m_Tap = m_lmsTap;
		MmttyWd->pSound->m_lms.m_NotchTap = m_NotchTap;
		if( sscanf(AnsiString(lmsMU2->Text).c_str(), "%lf", &d) == 1 ){	//JA7UDE 0428
			MmttyWd->pSound->m_lms.m_lmsMU2 = d;
		}
		if( sscanf(AnsiString(lmsGM->Text).c_str(), "%lf", &d) == 1 ){	//JA7UDE 0428
			MmttyWd->pSound->m_lms.m_lmsGM = d;
		}
		MmttyWd->pSound->m_lms.m_lmsInv = lmsInv->Checked;
		MmttyWd->pSound->m_lms.m_lmsAGC = lmsAGC->Checked;
		MmttyWd->pSound->m_lms.m_bpf = lmsBPF->Checked;
		MmttyWd->pSound->m_lms.m_Type = lmsType->Checked;
		MmttyWd->pSound->m_lms.m_twoNotch = lmsTwo->Checked;

		MmttyWd->pSound->CalcBPF();

		if( sscanf(AnsiString(pllVCOGain->Text).c_str(), "%lf", &d) == 1 ){	//JA7UDE 0428
			if( d > 0.0 ) fp->m_pll.SetVcoGain(d);
		}
		if( sscanf(AnsiString(pllLoopOrder->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd > 0) && (dd < 32) ) fp->m_pll.m_loopOrder = dd;
		}
		if( sscanf(AnsiString(pllLoopFC->Text).c_str(), "%lf", &d) == 1 ){	//JA7UDE 0428
			if( d > 0.0 ) fp->m_pll.m_loopFC = d;
		}
		if( sscanf(AnsiString(pllOutOrder->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
			if( (dd > 0) && (dd < 32) ) fp->m_pll.m_outOrder = dd;
		}
		if( sscanf(AnsiString(pllOutFC->Text).c_str(), "%lf", &d) == 1 ){
			if( d > 0.0 ) fp->m_pll.m_outFC = d;
		}
		fp->m_pll.MakeLoopLPF();
		fp->m_pll.MakeOutLPF();

		fp->m_majority = CheckMajority->Checked;
		fp->m_ignoreFream = CheckIgnoreFream->Checked;

		sys.m_ColorRXBack = PCRXBack->Color;
		sys.m_ColorRX = PCRXChar->Color;
		sys.m_ColorRXTX = PCRXTXChar->Color;
		sys.m_ColorINBack = PCINBack->Color;
		sys.m_ColorIN = PCINChar->Color;
		sys.m_ColorINTX = PCINTx->Color;
		sys.m_ColorLow = PCLow->Color;
		sys.m_ColorHigh = PCHigh->Color;
		sys.m_ColorXY = PCXY->Color;

		mp->m_DiddleWait = GetTBValue(TBDiddleWait->Position, 50, 50) + 0.5;
		mp->m_CharWait = GetTBValue(TBCharWait->Position, 50, 50) + 0.5;
		mp->m_CharWaitDiddle = WaitDiddle->Checked;

		if( PortName->Text == "NONE" ) TxPort->ItemIndex = 0;
		if( sys.m_TxPort != TxPort->ItemIndex ) COMM.change = 1;
		sys.m_TxPort = TxPort->ItemIndex;
		sys.m_MemWindow = MemWin->Checked;

		sys.m_SetupOnTop = SetupOnTop->Checked; //1.70K
	}
	else {
		r = FALSE;
	}
	for( PageIndex = 0; PageIndex < Page->PageCount; PageIndex++ ){
		if( Page->ActivePage == Page->Pages[PageIndex] ) break;
	}
	for( PageIndexBPF = 0; PageIndexBPF < PageBPF->PageCount; PageIndexBPF++ ){
		if( PageBPF->ActivePage == PageBPF->Pages[PageIndexBPF] ) break;
	}
	return r;
}
//---------------------------------------------------------------------
void __fastcall TOptionDlg::HamBtnClick(TObject *Sender)
{
	MarkFreq->Text = DefMarkFreq->Text.c_str();
	ShiftFreq->Text = DefShift->Text.c_str();
	BaudRate->Text = 45.45;
	BitLen->ItemIndex = 0;
	StopLen->ItemIndex = DefStopBit->ItemIndex + 3;
	ParitySel->ItemIndex = 0;
	if( sys.m_TxPort ) COMM.change = 1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::FontBtnClick(TObject *Sender)
{
	FontDialog->Font->Name = FontName->Text;
	FontDialog->Font->Size = m_FontSize;
	FontDialog->Font->Charset = BYTE(m_FontCharset);
	TFontStyles fs = Code2FontStyle(m_FontStyle);
	FontDialog->Font->Style = fs;
	NormalWindow(this);
	if( FontDialog->Execute() == TRUE ){
		m_FontSize = FontDialog->Font->Size;
		FontName->Text = FontDialog->Font->Name;
		m_FontCharset = FontDialog->Font->Charset;
		fs = FontDialog->Font->Style;
		m_FontStyle = FontStyle2Code(fs);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBMClick(TObject *Sender)
{
	int n  = GetMacroIndex(Sender);
	if( n >= 0 ){
		TEditDlgBox *pBox = new TEditDlgBox(this);
		pBox->ButtonName->Text = sys.m_UserName[n];
		if( pBox->Execute(sys.m_User[n], sys.m_UserKey[n], &sys.m_UserCol[n], &sys.m_UserTimer[n], 1) == TRUE ){
			if( !pBox->ButtonName->Text.IsEmpty() ){
				sys.m_UserName[n] = pBox->ButtonName->Text;
			}
		}
		delete pBox;
		UpdateMacro();
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBINClick(int n)
{
	TEditDlgBox *pBox = new TEditDlgBox(this);
    pBox->Caption = (Font->Charset != SHIFTJIS_CHARSET)? "Edit Button":"“ü—Íƒ{ƒ^ƒ“•ÒW";  //AA6YQ 1.66
	pBox->ButtonName->Text = sys.m_InBtnName[n];
	if( pBox->Execute(sys.m_InBtn[n], sys.m_InBtnKey[n], &sys.m_InBtnCol[n], NULL, 0) == TRUE ){
		if( !pBox->ButtonName->Text.IsEmpty() ){
			sys.m_InBtnName[n] = pBox->ButtonName->Text;
		}
	}
	delete pBox;
	UpdateMacro();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBIN1Click(TObject *Sender)
{
	SBINClick(0);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBIN2Click(TObject *Sender)
{
	SBINClick(1);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBIN3Click(TObject *Sender)
{
	SBINClick(2);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBIN4Click(TObject *Sender)
{
	SBINClick(3);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DemBpfBtnClick(TObject *Sender)
{
	if( DemType->ItemIndex ){
		double HBPF1[TAPMAX+1];
		double HBPF2[TAPMAX+1];

		double mfq;
		double sft;
		int	   tap;
		sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &mfq);	//JA7UDE 0428
		sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sft);	//JA7UDE 0428
		double sfq = mfq + sft;
		sscanf(AnsiString(FilterTap->Text).c_str(), "%u", &tap);	//JA7UDE 0428
		if( !tap ) tap = 2;

		MakeFilter(HBPF1, tap, ffBPF, DemSamp, mfq-pDem->GetFilWidth(tap), mfq+pDem->GetFilWidth(tap), 60, 1.0);
		MakeFilter(HBPF2, tap, ffBPF, DemSamp, sfq-pDem->GetFilWidth(tap), sfq+pDem->GetFilWidth(tap), 60, 1.0);

		TFreqDispDlg *pBox = new TFreqDispDlg(this);
		pBox->m_Max = 3000;
		pBox->Execute(HBPF1, HBPF2, tap, DemOver+1);
		delete pBox;
	}
	else {
		double mfq;
		double sft;
		double sfq;
		double fw;
		CIIRTANK	iirm;
		CIIRTANK	iirs;

		sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &mfq);	//JA7UDE 0428
		sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sft);	//JA7UDE 0428
		sscanf(AnsiString(IIRFW->Text).c_str(), "%lf", &fw);	//JA7UDE 0428
		sfq = mfq + sft;
		iirm.SetFreq(mfq, DemSamp, fw);
		iirs.SetFreq(sfq, DemSamp, fw);

		TFreqDispDlg *pBox = new TFreqDispDlg(this);
		pBox->m_Max = 3000;
		pBox->Execute(iirm.a0, iirm.b1, iirm.b2, iirs.a0, iirs.b1, iirs.b2, DemOver+1);
		delete pBox;
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PreBpfBtnClick(TObject *Sender)
{
	CLMS	lms;
	CLMS	lmsbak;
	int nowbpf = MmttyWd->pSound->m_bpf;
	int nowlms = MmttyWd->pSound->m_lmsbpf;
	int nowtap = MmttyWd->pSound->m_bpftap;
	int bpfafc = MmttyWd->pSound->m_bpfafc;

	lmsbak.Copy(MmttyWd->pSound->m_lms);
	lms.Copy(MmttyWd->pSound->m_lms);

	int tap;
	sscanf(AnsiString(RxBpfTap->Text).c_str(), "%u", &tap);	//JA7UDE 0428
	if( !tap ) tap = 2;

	MmttyWd->pSound->m_bpftap = tap;
	double mfq;
	double sft;
	double fw;
	sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &mfq);	//JA7UDE 0428
	sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sft);	//JA7UDE 0428
	double sfq = mfq + sft;
	sscanf(AnsiString(RxBpfFW->Text).c_str(), "%lf", &fw);	//JA7UDE 0428

	lms.m_Tap = m_lmsTap;
	lms.m_NotchTap = m_NotchTap;
	sscanf(AnsiString(lmsDelay->Text).c_str(), "%u", &lms.m_lmsDelay);	//JA7UDE 0428
	sscanf(AnsiString(lmsMU2->Text).c_str(), "%lf", &lms.m_lmsMU2);	//JA7UDE 0428
	sscanf(AnsiString(lmsGM->Text).c_str(), "%lf", &lms.m_lmsGM);	//JA7UDE 0428
	lms.m_lmsAGC = lmsAGC->Checked;
	lms.m_Type = lmsType->Checked;
	lms.m_bpf = lmsBPF->Checked;
	lms.m_twoNotch = lmsTwo->Checked;
	lms.m_lmsNotch = (mfq + sfq) / 2;
	lms.m_lmsNotch2 = mfq - 80;

	MmttyWd->pSound->m_lms.Copy(lms);
	MmttyWd->pSound->m_bpf = 1;
	MmttyWd->pSound->m_bpfafc = CheckRXBPFAFC->Checked;
	MmttyWd->pSound->CalcBPF(mfq, sfq, fw);

	TFreqDispDlg *pBox = new TFreqDispDlg(this);
	pBox->Timer->Enabled = TRUE;
	if(PageBPF->ActivePage == TabBPF){
		MmttyWd->pSound->m_bpf = 1;
		pBox->Execute(MmttyWd->pSound->HBPF, tap, 1);
	}
	else {
		MmttyWd->pSound->m_bpf = SelBPF->Checked;
		MmttyWd->pSound->m_lmsbpf = 1;
		if( lms.m_Type ){
			if( lms.m_twoNotch ){
				pBox->Timer->Enabled = FALSE;
				pBox->Execute(MmttyWd->pSound->m_lms.H, MmttyWd->pSound->m_lms.GetHBPF(), lms.m_NotchTap, 1);
			}
			else {
				pBox->Execute(MmttyWd->pSound->m_lms.H, lms.m_NotchTap, 1);
			}
		}
		else {
			pBox->Execute(MmttyWd->pSound->m_lms.H, lms.m_Tap, 1);
		}
	}
	delete pBox;

	MmttyWd->pSound->m_bpf = nowbpf;
	MmttyWd->pSound->m_lmsbpf = nowlms;
	MmttyWd->pSound->m_bpftap = nowtap;
	MmttyWd->pSound->m_bpfafc = bpfafc;
	MmttyWd->pSound->m_lms.Copy(lmsbak);
	MmttyWd->pSound->CalcBPF();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispTxBpfClick(TObject *Sender)
{
	double HBPF[TAPMAX+1];

	double mfq;
	double sft;
	int	   tap;
	sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &mfq);	//JA7UDE 0428
	sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sft);	//JA7UDE 0428
	double sfq = mfq + sft;
	sscanf(AnsiString(TxBpfTap->Text).c_str(), "%u", &tap);	//JA7UDE 0428
	if( !tap ) tap = 2;

	MakeFilter(HBPF, tap, ffBPF, SampFreq, mfq - 150, sfq + 150, 60, 1.0);

	TFreqDispDlg *pBox = new TFreqDispDlg(this);
	pBox->Execute(HBPF, tap, 1);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispDemLpfClick(TObject *Sender)
{
	int order;
	double fc;
	if( DemLpf->ItemIndex ){		// IIR
		sscanf(AnsiString(SmoozIIR->Text).c_str(), "%lf", &fc);	//JA7UDE 0428
		if( (fc >= 20.0) && (fc <= 1500.0) ){
			sscanf(AnsiString(DemLpfOrder->Text).c_str(), "%u", &order);	//JA7UDE 0428
			if( (order >= 1) && (order <= 32) ){
				CIIR	iir;
				iir.MakeIIR(fc, DemSamp, order, 0, 1.0);
				TFreqDispDlg *pBox = new TFreqDispDlg(this);
				pBox->Execute(&iir, DemOver+1, 200.0);
				delete pBox;
			}
		}
	}
	else {							// FIR
		sscanf(AnsiString(SmoozFreq->Text).c_str(), "%lf", &fc);	//JA7UDE 0428
		if( (fc >= 20.0) && (fc <= 1500.0) ){
			double HLPF[TAPMAX+1];

			int n = int((DemSamp) / fc + 0.5);
			double d = 1 / double(n);
			for( int i = 0; i < n; i++ ){
				HLPF[i] = d;
			}
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(HLPF, n-1, DemOver+1, 200.0);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispLoopLPFClick(TObject *Sender)
{
	int order;
	double fc;

	sscanf(AnsiString(pllLoopFC->Text).c_str(), "%lf", &fc);	//JA7UDE 0428
	if( fc > 0.0 ){
		sscanf(AnsiString(pllLoopOrder->Text).c_str(), "%u", &order);	//JA7UDE 0428
		if( (order >= 1) && (order <= 32) ){
			CIIR	iir;
			iir.MakeIIR(fc, DemSamp, order, 0, 1.0);
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(&iir, DemOver+1, 800);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DispOutLPFClick(TObject *Sender)
{
	int order;
	double fc;

	sscanf(AnsiString(pllOutFC->Text).c_str(), "%lf", &fc);	//JA7UDE 0428
	if( fc > 0.0 ){
		sscanf(AnsiString(pllOutOrder->Text).c_str(), "%u", &order);	//JA7UDE 0428
		if( (order >= 1) && (order <= 32) ){
			CIIR	iir;
			iir.MakeIIR(fc, DemSamp, order, 0, 1.0);
			TFreqDispDlg *pBox = new TFreqDispDlg(this);
			pBox->Execute(&iir, DemOver+1, 800);
			delete pBox;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCRXBackClick(TObject *Sender)
{
	ColorDialog->Color = PCRXBack->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCRXBack->Color = ColorDialog->Color;
		m_ColorChange = 1;
		if( PCRXBack->Color != PCINBack->Color ){
			if( YesNoMB( (Font->Charset != SHIFTJIS_CHARSET) ? "Copy the color to InputWindow ?":"‚±‚ÌF‚ð“ü—Í‰æ–Ê‚Ì”wŒiF‚É‚à“K‰ž‚µ‚Ü‚·‚©H") == IDYES ){
				PCINBack->Color = ColorDialog->Color;
			}
		}
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCRXCharClick(TObject *Sender)
{
	ColorDialog->Color = PCRXChar->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCRXChar->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCRXTXCharClick(TObject *Sender)
{
	ColorDialog->Color = PCRXTXChar->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCRXTXChar->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCINBackClick(TObject *Sender)
{
	ColorDialog->Color = PCINBack->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCINBack->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCINCharClick(TObject *Sender)
{
	ColorDialog->Color = PCINChar->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCINChar->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCINTxClick(TObject *Sender)
{
	ColorDialog->Color = PCINTx->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCINTx->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------
void __fastcall TOptionDlg::PCLowClick(TObject *Sender)
{
	ColorDialog->Color = PCLow->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCLow->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCHighClick(TObject *Sender)
{
	ColorDialog->Color = PCHigh->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCHigh->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SetCustomColor(void)
{
	ColorDialog->CustomColors->Text = "";
	AddCustomColor(PCRXBack->Color);
	AddCustomColor(PCRXChar->Color);
	AddCustomColor(PCINBack->Color);
	AddCustomColor(PCINChar->Color);
	AddCustomColor(PCINTx->Color);
	AddCustomColor(PCLow->Color);
	AddCustomColor(PCHigh->Color);
	AddCustomColor(PCXY->Color);
}
//---------------------------------------------------------------------
void __fastcall TOptionDlg::AddCustomColor(TColor col)
{
	char bf[256];

	sprintf(bf, "Color%c=%06lX", ColorDialog->CustomColors->Count + 'A', DWORD(col) & 0x00ffffff);
	ColorDialog->CustomColors->Add(bf);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PortNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PCXYClick(TObject *Sender)
{
	ColorDialog->Color = PCXY->Color;
	SetCustomColor();
	NormalWindow(this);
	if( ColorDialog->Execute() == TRUE ){
		PCXY->Color = ColorDialog->Color;
		m_ColorChange = 1;
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::WinFontBtnClick(TObject *Sender)
{
	FontDialog->Font->Name = WinFont->Text;
	FontDialog->Font->Charset = m_WinFontCharset;
	TFontStyles fs = Code2FontStyle(m_WinFontStyle);
	FontDialog->Font->Style = fs;
	NormalWindow(this);
	if( FontDialog->Execute() == TRUE ){
		WinFont->Text = FontDialog->Font->Name;
		m_WinFontCharset = FontDialog->Font->Charset;
		fs = FontDialog->Font->Style;
		m_WinFontStyle = FontStyle2Code(fs);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::BtnFontBtnClick(TObject *Sender)
{
	FontDialog->Font->Name = BtnFont->Text;
	FontDialog->Font->Charset = m_BtnFontCharset;
	TFontStyles fs = Code2FontStyle(m_BtnFontStyle);
	FontDialog->Font->Style = fs;
	NormalWindow(this);
	if( FontDialog->Execute() == TRUE ){
		BtnFont->Text = FontDialog->Font->Name;
		m_BtnFontCharset = FontDialog->Font->Charset;
		fs = FontDialog->Font->Style;
		m_BtnFontStyle = FontStyle2Code(fs);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::JapaneseBtnClick(TObject *Sender)
{
	WinFont->Text = "‚l‚r ‚oƒSƒVƒbƒN";
	m_WinFontCharset = SHIFTJIS_CHARSET;
	BtnFont->Text = "‚l‚r ƒSƒVƒbƒN";
	m_BtnFontCharset = SHIFTJIS_CHARSET;
	m_WinFontStyle = 0;
	m_BtnFontStyle = 0;
	FontName->Text = "‚l‚r ƒSƒVƒbƒN";
	m_FontCharset = SHIFTJIS_CHARSET;
	m_FontStyle = 0;
	if( sys.m_WinFontCharset != SHIFTJIS_CHARSET ){
		Application->MessageBox(L"MMTTY‚ðÄ‹N“®‚·‚é‚Æ“ú–{Œêƒ‚[ƒh‚É‚È‚è‚Ü‚·.\r\n(Please restart MMTTY for Japanese mode)", L"MMTTY", MB_ICONINFORMATION|MB_OK);	//JA7UDE 0428
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::EnglishBtnClick(TObject *Sender)
{
	WinFont->Text = "Times New Roman";
	m_WinFontCharset = ANSI_CHARSET;
	BtnFont->Text = "Courier New";
	m_BtnFontCharset = ANSI_CHARSET;
	m_WinFontStyle = 0;
	m_BtnFontStyle = 0;
	FontName->Text = "Courier New";
	m_FontCharset = ANSI_CHARSET;
	m_FontStyle = 0;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBClockAdjClick(TObject *Sender)
{
	double d;
	sscanf(AnsiString(EditSamp->Text).c_str(), "%lf", &d);	//JA7UDE 0428
	if( (d < 5000.0) || (d > 12500.0) ){
		d = sys.m_SampFreq;
	}

	TClockAdjDlg *pBox = new TClockAdjDlg(this);
	if( pBox->Execute(MmttyWd->pSound, d) == TRUE ){
		EditSamp->Text = d;
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::DefBtnClick(TObject *Sender)
{
	MarkFreq->Text = 2125;
	ShiftFreq->Text = 170;
	DefMarkFreq->Text = 2125;
	DefShift->Text = 170;
	DemType->ItemIndex = 0;
	IIRFW->Text = 60;
	FilterTap->Text = 72;
    Tones->Text = 4;
	LimitOver->Checked = 0;
	LimitGain->Text = 200;
	DemLpf->ItemIndex = 0;
	SmoozIIR->Text = 45;
	DemLpfOrder->Text = 5;
	SmoozFreq->Text = 70;
	ATC->Checked = FALSE;
	ATCTime->Text = 4;
	pllVCOGain->Text = 3;
	pllLoopOrder->Text = 2;
	pllLoopFC->Text = 250;
	pllOutOrder->Text = 4;
	pllOutFC->Text = 200;
	HamBtnClick(NULL);
}

//---------------------------------------------------------------------------
void __fastcall TOptionDlg::RadioBtnClick(TObject *Sender)
{
	// ƒ‰ƒWƒIƒRƒ“ƒgƒ[ƒ‹
	TRADIOSetDlg *pBox = new TRADIOSetDlg(this);

	if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//JA7UDE 0428
		strcpy(RADIO.StrPort, "NONE");
		RADIO.change = 1;
	}
	if( pBox->Execute() == TRUE ){
		if( (PortName->Text != "NONE") && !strcmp(AnsiString(PortName->Text).c_str(), RADIO.StrPort) ){	//JA7UDE 0428
			PortName->Text = "NONE";
		}
	}
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::BaudRateChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( sys.m_TxPort ) COMM.change = 1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::BitLenClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( sys.m_TxPort ) COMM.change = 1;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::TxdJobClick(TObject *Sender)
{
	TTxdDlgBox *pBox = new TTxdDlgBox(this);
	m_TxdJob = pBox->Execute(m_TxdJob);
	delete pBox;
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::lmsTapChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	int dd;
	if( sscanf(AnsiString(lmsTap->Text).c_str(), "%u", &dd) == 1 ){	//JA7UDE 0428
		if( (dd >= 0) && (dd <= 512) ){
			if( lmsType->Checked ){
				m_NotchTap = dd;
			}
			else {
				m_lmsTap = dd;
			}
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBHelpClick(TObject *Sender)
{
/*
Demodulator:     demodulatortab.htm
AFC/ATC/PLL:     afcatcplltab.htm
Decode:          decodetab.htm
TX:              txtab.htm
Font/Window:     fontwindowtab.htm
Misc:            misctab.htm
SoundCard:       soundtab.htm
*/
	LPCSTR pContext;
	if( Page->ActivePage == TabTx ){
		pContext = "txtab.htm";
	}
	else if( Page->ActivePage == TabDem2 ){
		pContext = "afcatcplltab.htm";
	}
	else if( Page->ActivePage == TabPara ){
		pContext = "decodetab.htm";
	}
	else if( Page->ActivePage == TabFont ){
		pContext = "fontwindowtab.htm";
	}
	else if( Page->ActivePage == TabMisc ){
		pContext = "misctab.htm";
	}
	else if( Page->ActivePage == TabSound ){
		pContext = "soundtab.htm";
	}
	else {
		pContext = "demodulatortab.htm";
	}
	ShowHtmlHelp(pContext);
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::PortNameDropDown(TObject *Sender)
{
	if( m_DisEvent ) return;

	if( !m_MMList.IsQuery() ){
		m_MMList.QueryList("FSK");
		for( int i = 0; i < m_MMList.GetCount(); i++ ){
			PortName->Items->Add(m_MMList.GetItemName(i));
		}
		//AA6YQ 1.70E
		//PortName->DropDownCount = PortName->Items->Count;
		PortName->DropDownCount = 8;
	}
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::DevNoDropDown(TObject *Sender)
{

//AA6YQ 1.66B - handled in TOptionDlg::TOptionDlg
/*
	if( m_DisEvent ) return;

    if( !m_MMListW.IsQuery() ){
		m_MMListW.QueryList("MMW");
		for( int i = 0; i < m_MMListW.GetCount(); i++ ){
			DevNo->Items->Add(m_MMListW.GetItemName(i));
		}
		DevNo->DropDownCount = m_MMListW.GetCount() + 5;
    }

*/
}
//---------------------------------------------------------------------------

// Value = 5148  Celeron 1.06GHz 2003/10/22
void __fastcall TOptionDlg::DemBpfBtnMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		CWaitCursor w;
		TSound *pSound = MmttyWd->pSound;
        if( pSound->m_ReqSpeedTest ) return;
		pSound->m_ReqSpeedTest = 1;
        while(pSound->m_ReqSpeedTest) ::Sleep(100);
        InfoMB("%u", pSound->m_SpeedValue);
    }
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::DevOutNoDropDown(TObject *Sender)
{

//AA6YQ 1.66B - handled in TOptionDlg::TOptionDlg
/*
    if( m_DisEvent ) return;

    if( !m_MMListW.IsQuery() ){
		m_MMListW.QueryList("MMW");
		for( int i = 0; i < m_MMListW.GetCount(); i++ ){
			DevOutNo->Items->Add(m_MMListW.GetItemName(i));
		}
		DevOutNo->DropDownCount = m_MMListW.GetCount() + 5;
    }

    */
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::InputSoundcardsClick(TObject *Sender)
{
	//AA6YQ 1.66
	DevNo->ItemIndex = InputSoundcards->ItemIndex+1;
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::DevNoClick(TObject *Sender)
{
    //AA6YQ 1.66
	if (IsSoundcard (AnsiString(DevNo->Text).c_str())) {	//JA7UDE 0428
		InputSoundcards->ItemIndex = atoi(AnsiString(DevNo->Text).c_str());  //AA6YQ 1.66	//JA7UDE 0428
    }
    else {
		InputSoundcards->ItemIndex =-1;
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TOptionDlg::OutputSoundcardsClick(TObject *Sender)
{
    //AA6YQ 1.66
    DevOutNo->ItemIndex = OutputSoundcards->ItemIndex+1;
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::DevOutNoClick(TObject *Sender)
{
	//AA6YQ 1.66
	if (IsSoundcard (AnsiString(DevOutNo->Text).c_str())) {	//JA7UDE 0428
		OutputSoundcards->ItemIndex = atoi(AnsiString(DevOutNo->Text).c_str());  //AA6YQ 1.66	//JA7UDE 0428
    }
    else {
		OutputSoundcards->ItemIndex =-1;
    }
}
//---------------------------------------------------------------------------
void __fastcall TOptionDlg::SBAA6YQClick(TObject *Sender)
{
	TFreqDispDlg *pBox = new TFreqDispDlg(this);
	pBox->Timer->Enabled = TRUE;
	pBox->m_Max = 3000;
	pDem->m_AA6YQ.Create();
	CFIR2 *pBPF = &pDem->m_AA6YQ.m_BPF;
    CFIR2 *pBEF = &pDem->m_AA6YQ.m_BEF;
	pBox->Execute(pBPF->GetHP(), pBPF->GetTap(), pBEF->GetHP(), pBEF->GetTap(), 1);
	delete pBox;
}
//---------------------------------------------------------------------------

void __fastcall TOptionDlg::HideFlexAudioClick(TObject *Sender)
{
	int CountUnits = 0;
	int CurrentUnit = 0;

	int unitnum;
	int i;
	int NewDeviceNumber;

	int InputDeviceNumber;
	int MappedInputDeviceNumber;
	int OutputDeviceNumber;
	int MappedOutputDeviceNumber;

	LPCSTR devName;
	char *cString;

	InputDeviceNumber = atoi(AnsiString(DevNo->Text).c_str());

	if (InputDeviceNumber < 16) {
		MappedInputDeviceNumber = InputDeviceMap[InputDeviceNumber];
	} else {
		MappedInputDeviceNumber=-1;
	}

	OutputDeviceNumber = atoi(AnsiString(DevOutNo->Text).c_str());

	if (OutputDeviceNumber < 16) {
		MappedOutputDeviceNumber = OutputDeviceMap[OutputDeviceNumber];
	} else {
		MappedOutputDeviceNumber=-1;
	}

	InputSoundcards->Items->BeginUpdate();
	InputSoundcards->Items->Clear();

	while (CountUnits < 16 && CurrentUnit < 32) {
		devName = MmttyWd->pSound->GetInputSoundcard(CurrentUnit);
		cString = AnsiString(devName).c_str();

		if (HideFlexAudio->Checked) { //AA6YQ 1.70E
			if (strstr(cString, "IQ") || strstr(cString, "RESERVED")) {
				// This is one of the FlexRadio audio devices we don't want
				CurrentUnit++;
				continue;
			}
		}

		// This is a device we want...
		if (devName) {
			InputSoundcards->Items->Add(devName);
			InputDeviceMap[CountUnits++] = CurrentUnit++;
		} else {
			CurrentUnit++;
		}
	}
	// for( int i = 0; i < 16; i++ ){
	// 		InputSoundcards->Items->Add(MmttyWd->pSound->GetInputSoundcard(i));
	// }
	InputSoundcards->Items->EndUpdate();

	if (HideFlexAudio->Checked != sys.m_HideFlexAudio) {

		NewDeviceNumber=-1;

		if (IsSoundcard (AnsiString(DevNo->Text).c_str())) {

			if (HideFlexAudio->Checked) {

				for (i=0; i < 16; i++) {
					if (InputDeviceMap[i] == InputDeviceNumber) {
						break;
					}
				}

				NewDeviceNumber = i != 16 ? i : -1;

			} else {

				NewDeviceNumber = MappedInputDeviceNumber;

			}
		}

		InputSoundcards->ItemIndex = NewDeviceNumber;
		DevNo->ItemIndex = NewDeviceNumber+1;
		sys.m_SoundDevice = NewDeviceNumber;
	}


	OutputSoundcards->Items->BeginUpdate();
	OutputSoundcards->Items->Clear();

	CountUnits = 0;
	CurrentUnit = 0;
	while (CountUnits < 16 && CurrentUnit < 32) {
		devName = MmttyWd->pSound->GetOutputSoundcard(CurrentUnit);
		cString = AnsiString(devName).c_str();

		if (HideFlexAudio->Checked) { //AA6YQ 1.70E
			if (strstr(cString, "IQ") || strstr(cString, "RESERVED")) {
				// This is one of the FlexRadio audio devices we don't want
				CurrentUnit++;
				continue;
			}
		}

		// This is a device we want...
		if (devName) {
			OutputSoundcards->Items->Add(devName);
			OutputDeviceMap[CountUnits++] = CurrentUnit++;
		} else {
			CurrentUnit++;
		}
	}
	//for( int i = 0; i < 16; i++ ){
	//	OutputSoundcards->Items->Add(MmttyWd->pSound->GetOutputSoundcard(i));
	//}
	OutputSoundcards->Items->EndUpdate();

	if (HideFlexAudio->Checked != sys.m_HideFlexAudio) {

		NewDeviceNumber=-1;

		if (IsSoundcard (AnsiString(DevOutNo->Text).c_str())) {

			if (HideFlexAudio->Checked) {

				for (i=0; i < 16; i++) {
					if (OutputDeviceMap[i] == OutputDeviceNumber) {
						break;
					}
				}

				NewDeviceNumber = i != 16 ? i : -1;

			} else {
				NewDeviceNumber = MappedOutputDeviceNumber;
			}
		}

		OutputSoundcards->ItemIndex = NewDeviceNumber;
		DevOutNo->ItemIndex = NewDeviceNumber+1;
		sys.m_SoundOutDevice = NewDeviceNumber;

	}

	sys.m_HideFlexAudio = HideFlexAudio->Checked;
}
//---------------------------------------------------------------------------


void __fastcall TOptionDlg::SetupOnTopClick(TObject *Sender)
{
	sys.m_SetupOnTop = SetupOnTop->Checked; //1.70K
	if( sys.m_SetupOnTop ){
		FormStyle = fsStayOnTop;
		}
	else {
		FormStyle = fsNormal;
	}
}


void __fastcall TOptionDlg::SetupTimerTimer(TObject *Sender)
{

	SetupTimer->Enabled=False;

	SetupOnTop->Checked = sys.m_SetupOnTop;   //1.70K
}
//---------------------------------------------------------------------------

