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



//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "clipbrd.hpp"
#include <locale.h>

#include "MMSYSTEM.H"
#include "Main.h"
#include "Scope.h"
#include "Option.h"
#include "EditDlg.h"
#include "TextEdit.h"
#include "LogList.h"
#include "LogLink.h"
#include "QsoDlg.h"
#include "Country.h"
#include "Mmcg.h"
#include "VerDsp.h"
#include "LogConv.h"
#include "PlayDlg.h"
#include "ShortCut.h"
#include "LogSet.h"
#include "SetHelp.h"
#include "TncSet.h"
#include "InputWin.h"

//---------------------------------------------------------------------------
#pragma resource "*.dfm"
TMmttyWd *MmttyWd;
UINT MSG_MMTTY;
HWND APP_HANDLE;
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OnMSG(tagMSG &Msg, bool &Handled)
{
/*
Msg.hwnd := Handle;
Msg.message := WM_KEYDOWN;
Msg.wParam := Message.WParam;
Msg.lParam := Message.LParam;*/
	if( Msg.message != MSG_MMTTY ) return;

	RemoteMMTTY(Msg);
	Handled = true;
}
//ÉEÉCÉìÉhÉEÉÅÉbÉZÅ[ÉWÇÃÉnÉìÉhÉâ---------------------------------------------
void __fastcall TMmttyWd::WndProc(TMessage &Message)
{
	switch(Message.Msg){
		case WM_COPYDATA:
			WndCopyData(Message);
			break;
		case CM_CTNC:
			OnTncEvent(Message);
        	break;
		default:
			TForm::WndProc(Message);
			break;
	}
}
//---------------------------------------------------------------------------
// WM_COPYDATAÇÃèàóù
void __fastcall TMmttyWd::WndCopyData(TMessage &Message)
{
	COPYDATASTRUCT *cp = (COPYDATASTRUCT *)Message.LParam;
	if( LogLink.IsCopyData() ){
		Message.Result = LogLink.m_pLink->OnCopyData(HWND(Message.WParam), cp);
		return;
	}
	switch(cp->dwData){
		case 0:
		case 1:             // HamlogÇ©ÇÁÇÃï‘êM
			if( sys.m_LogLink != 1 ) return;
		   switch(LogLink.AnaData(&Log.m_sd, cp)){
				case 115:
					UpdateTextData();
					break;
				case 106:
					Freq->Text = Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq);
					break;
			}
			Message.Result = TRUE;
			break;
		case 0x80001212:    // é¸îgêîÉfÅ[É^ÇÃéwíË
			if( cp->cbData && (cp->lpData != NULL) ){
				char bf[16];
				int len = cp->cbData;
				if( len > 15 ) len = 15;
				memcpy(bf, cp->lpData, len);
				bf[len] = 0;
				Freq->Text = bf;
			}
			Message.Result = TRUE;
			break;
		default:
			Message.Result = FALSE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OnActivate(TObject *Sender)
{
	if( m_OptionDialog ) return;
    if( Remote & REMDISFOCUS ) return;
	if( APP_HANDLE != HWND_BROADCAST ) ::SetForegroundWindow(APP_HANDLE);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OnRestore(TObject *Sender)
{
	if( m_OptionDialog ) return;
	if( Remote & REMSHOWOFF ){
		if( APP_HANDLE != HWND_BROADCAST ){
	    if( !(Remote & REMDISFOCUS) ) ::SetForegroundWindow(APP_HANDLE);
			Hide();
		}
	}
}
//---------------------------------------------------------------------------
__fastcall TMmttyWd::TMmttyWd(TComponent* Owner)
	: TForm(Owner)
{
	::VirtualLock(this, sizeof(TMmttyWd));
	::VirtualLock(&sys, sizeof(sys));

	pSound = NULL;
	pAndyMenu = NULL;
	pTnc = NULL;
	pRadio = NULL;
	pComm = NULL;
	p_SendFileDlg = NULL;
	hMap = NULL;
	pMap = NULL;
	MSG_MMTTY=0;
	APP_HANDLE = HWND_BROADCAST;

	lcid = GetThreadLocale() & 0x00ff;
	if( lcid != LANG_JAPANESE ){
		sys.m_WinFontName = "Times New Roman";
		sys.m_WinFontCharset = ANSI_CHARSET;
		sys.m_BtnFontName = "Courier New";
		sys.m_BtnFontCharset = ANSI_CHARSET;
		sys.m_FontName = "Courier New";
		sys.m_FontCharset = ANSI_CHARSET;
		Log.m_LogSet.m_TimeZone = 'Z';
		sys.m_TimeStampUTC = 1;
		sys.m_HTMLHelp = HELPNAME_A; //AA6YQ 1.70J was KB2EOQ.htm
		KENT->Checked = FALSE;
		sys.m_LWait = 2;
		sys.m_LogLink = 0;
	}
	else {
		sys.m_WinFontName = "ÇlÇr ÇoÉSÉVÉbÉN";
		sys.m_WinFontCharset = SHIFTJIS_CHARSET;
		sys.m_BtnFontName = "ÇlÇr ÉSÉVÉbÉN";
		sys.m_BtnFontCharset = SHIFTJIS_CHARSET;
		sys.m_FontName = "ÇlÇr ÉSÉVÉbÉN";
		sys.m_FontCharset = SHIFTJIS_CHARSET;
		sys.m_TimeStampUTC = 0;
		sys.m_HTMLHelp = "";
		KENT->Checked = TRUE;
		sys.m_LWait = 0;
		sys.m_LogLink = 1;
	}

	sys.m_WinFontStyle = 0;
	sys.m_BtnFontStyle = 0;
	sys.m_Help = "Mmtty.txt";
	sys.m_HelpLog = "Mmttylog.txt";
	sys.m_HelpDigital = "Digital.txt";
	sys.m_HelpFontName = "ÇlÇr ÉSÉVÉbÉN";
	sys.m_HelpFontCharset = SHIFTJIS_CHARSET;
	sys.m_HelpFontSize = 10;
	sys.m_HelpNotePad = 0;

	DWORD dwVersion = ::GetVersion();
	if( dwVersion < 0x80000000 ){

        //AA6YQ 1.66G distinguish Vista from XP

        OSVERSIONINFO osvi;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&osvi);
        if (osvi.dwMajorVersion == 5) {
            WinNT = TRUE;
            WinVista=FALSE;
        }
        else {
            WinNT = FALSE;
            WinVista=TRUE;
        }
	}
	else {
		WinNT = FALSE;
        WinVista=FALSE;
	}

	EntryAlignControl();
	InitCOMMPara();
	InitTNCPara();
	InitRADIOPara();
	sys.m_DefFix45 = TRUE;
	m_BARTGTimer = 0;
	m_XYSize = 0;
	m_XYQuality = 0;
	m_PanelSize = 2;
	m_DemType = -1;
	m_LpfType = -1;
	m_InitFirst = TRUE;
	DisPaint = TRUE;
	m_MacroStat = 0;
	m_Macro = 0;
	m_filemode = -1;
	m_PaletteQuery = 0;
	m_ReqRecv = 0;
	m_NameQTH = 0;
	m_Ctrl = 0;
	m_ReqPaletteChange = 0;
	m_FifoOutTimer = 0;
	m_Dupe = 0;
	m_Running = 0;
	m_SendWay = 0;
	m_DisAlign = 0;
	m_BefHeight = ClientHeight;
	m_Baud = m_ShowBaud = 0;
	m_RemoteTimer = 0;
	m_RemoteDelay = 0;
	m_OptionDialog = 0;
    m_hOptionWnd = NULL;
	m_MacTimer = 0;
	m_FIGTimer = 0;
	Remote = 0;
	m_pRemOpenName = NULL;
	m_PttTimerCount = 0;
	m_PttTimer = 60;

	InitRemoteStat();

	m_RxMarkFreq = 2125.0;
	m_RxSpaceFreq = 2125.0+170.0;

	StartOption();

	pBitmapFFTIN = new Graphics::TBitmap();
	pBitmapFFTIN->Width = PBoxFFTIN->Width;
	pBitmapFFTIN->Height = PBoxFFTIN->Height;

	pBitmapWater = new Graphics::TBitmap();
	pBitmapWater->Width = PBoxWater->Width;
	pBitmapWater->Height = PBoxWater->Height;

	pBitmapXY = new Graphics::TBitmap();
	pBitmapXY->Width = PBoxXY->Width;
	pBitmapXY->Height = PBoxXY->Height;

	pBitmapSQ = new Graphics::TBitmap();
	pBitmapSQ->Width = PBoxSQ->Width;
	pBitmapSQ->Height = PBoxSQ->Height;

	pBitmapRx = NULL;
	pBitmapIn = NULL;

	sys.m_DisTX = 0;
	sys.m_DisWindow = 0;
	sys.m_StayOnTop = 0;
	sys.m_WindowState = wsNormal;
	int i;
	char bf[256];
	for( i = 0; i < 16; i++ ){
		sprintf(bf, "M%u", i+1);
		sys.m_UserName[i] = bf;
		sys.m_User[i] = "";
		sys.m_UserKey[i] = 0;
		sys.m_UserEditKey[i] = 0;
		sys.m_UserCol[i] = clBlack;
		sys.m_UserTimer[i] = 0;
	}
	sys.m_User[0] = "__\r\n%c DE %m %m K\r\n_\\";
	sys.m_UserName[0] = "1X2";
	sys.m_User[1] = "__\r\n%c %c DE %m %m %m K\r\n_\\";
	sys.m_UserName[1] = "2X3";
	sys.m_User[2] = "__\r\nDE %m %m %m K\r\n_\\";
	sys.m_UserName[2] = "DE3";
	sys.m_User[3] = "__\r\nQSL UR 599-599 TU\r\n_\\";
	sys.m_UserName[3] = "UR599";
	sys.m_User[4] = "__\r\nOK TNX %n\r\nUR %r %r NAME MAKO %c DE %m TU SK\r\n_\\";
	sys.m_UserName[4] = "QANS";
	sys.m_User[8] = "\\\r\n73 TU SK SK\r\n%{TU}\\";
	sys.m_UserName[8] = "SK";
	sys.m_User[9] = "[__~~__~\\";
	sys.m_UserName[9] = "EE";
	sys.m_User[12] = "\r\nRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRY#";
	sys.m_UserName[12] = "RY";
	sys.m_User[14] = "__\r\nRYRY CQ CQ CQ DE %m %m %m\r\nRYRY CQ CQ CQ DE %m %m %m PSE K\r\n_\\";
	sys.m_UserName[14] = "CQ2";
	sys.m_User[15] = "__\r\nRYRY CQ CQ CQ DE %m %m %m PSE K\r\n_\\";
	sys.m_UserName[15] = "CQ1";

	sys.m_UserKey[0] = '1'+0x100;
	sys.m_UserKey[1] = '2'+0x100;
	sys.m_UserKey[2] = '3'+0x100;
	sys.m_UserKey[3] = '4'+0x100;
	sys.m_UserKey[4] = '5'+0x100;
	sys.m_UserKey[5] = '6'+0x100;
	sys.m_UserKey[6] = '7'+0x100;
	sys.m_UserKey[7] = '8'+0x100;
	sys.m_UserKey[8] = '9'+0x100;
	sys.m_UserKey[9] = VK_F10;
	sys.m_UserKey[10] = VK_F11;
	sys.m_UserKey[11] = VK_F12;
	sys.m_UserKey[14] = VK_F7;
	sys.m_UserKey[15] = VK_F6;

	sys.m_MsgList[0] = "\\__\r\nRGR %c DE %m  %g DEAR %n\r\nTHANK YOU FOR THE NICE REPORT.\r\nYOU ARE %r %r %r IN OSAKA OSAKA OSAKA\r\nMY NAME IS MAKO MAKO MAKO\r\nHOW COPY? BTU %c DE %m KN\r\n_\\";
	sys.m_MsgList[1] = "\\__\\";
	sys.m_MsgList[2] = "\\__\r\nTNX AGAIN DEAR %n CU SK\r\n__\\";
	sys.m_MsgList[3] = "\\__\r\nOK DEAR %n\r\nMANY THANK YOU FOR THE NICE QSO.\r\nI AM LOOKING FORWARD TO SEE YOU AGAIN ON CW AND PSK31.\r\nCUL! AND BEST 73\r\n%c DE %m TU SK SK..\r\n__";
	sys.m_MsgList[4] = "\\__\r\nRGR %c DE %m  ALL OK DEAR %n\r\nTHANK YOU FOR THE NICE REPORT.\r\n\r\nI AM RUNNING 50W 50W INTO A SMALL LOOP ANTENNA(ONLY 90CM DIA!)\r\n\r\nRTTY MODEM IS HOMEMADE SOFTWARE ON WINDOWS-95 SOUNDBLASTER.\r\n\r\nTHANK YOU FOR THE NICE QSO DEAR %n\r\nMY QSL IS OK VIA THE BURO.\r\n\r\nHOW? BTU %c DE %m KN\r\n_\\";
	sys.m_MsgList[5] = "\\__\r\n%c %c DE %m  %g\r\nTHANK YOU FOR YOUR CALL. I AM GLAD TO SEE YOU ON RTTY.\r\nYOU ARE %r %r %r IN OSAKA OSAKA OSAKA\r\nMY NAME IS MAKO MAKO MAKO\r\nHOW COPY? BTU %c %c DE %m KN\r\n_\\";
	sys.m_MsgList[6] = "\\__\r\nRGR %c DE %m  ALL OK DEAR %n\r\nTHANK YOU FOR THE NICE INFO.\r\n\r\nI AM RUNNING 50W 50W INTO A SMALL LOOP ANTENNA(ONLY 90CM DIA!)\r\n\r\nRTTY MODEM IS HOMEMADE SOFTWARE ON WINDOWS-95 SOUNDBLASTER.\r\n\r\nTHANK YOU FOR THE NICE QSO DEAR %n\r\nMY QSL IS OK VIA THE BURO.\r\n\r\nHOW? BTU %c DE %m KN\r\n_\\";
	sys.m_MsgList[7] = "\\__\r\nRGR TNX %n\r\nUR %r %r %r NAME IS MAKO MAKO MAKO\r\nHOW? BTU %c DE %m KN\r\n_\\";
	sys.m_MsgList[8] = "\\__\r\nOK DEAR %n\r\nTNX FB QSO, CUL BEST 73 %c DE %m TU SK..\r\n__\\";

	sys.m_MsgName[0] = "AF CALL1";
	sys.m_MsgName[1] = "FINAL3";
	sys.m_MsgName[2] = "FINAL2";
	sys.m_MsgName[3] = "FINAL";
	sys.m_MsgName[4] = "AF CQ2 (GEAR)";
	sys.m_MsgName[5] = "AF CQ1";
	sys.m_MsgName[6] = "AF CALL2 (GEAR)";
	sys.m_MsgName[7] = "AF CALL1 (QUICK)";
	sys.m_MsgName[8] = "FINAL (QUICK)";

	sys.m_MsgKey[0] = VK_F1 + 0x100;
	sys.m_MsgKey[1] = VK_F2 + 0x100;
	sys.m_MsgKey[2] = VK_F3 + 0x100;
	sys.m_MsgKey[3] = VK_F4 + 0x100;
	sys.m_MsgKey[4] = VK_F5 + 0x100;
	sys.m_MsgKey[5] = VK_F6 + 0x100;
	sys.m_MsgKey[6] = VK_F7 + 0x100;
	sys.m_MsgKey[7] = VK_F8 + 0x100;
	sys.m_MsgKey[8] = VK_F9 + 0x100;

	sys.m_InBtn[0] = "%c DE %m";
	sys.m_InBtnName[0] = "1X1";
	sys.m_InBtn[1] = "DEAR %n";
	sys.m_InBtnName[1] = "DEAR";
	sys.m_InBtn[2] = "\__\r\nRGR %c DE %m ALL OK DEAR %n\r\n";
	sys.m_InBtnName[2] = "ANS";
	sys.m_InBtn[3] = "BTU %c DE %m KN\r\n_\\";
	sys.m_InBtnName[3] = "BTU";
	sys.m_InBtnKey[0] = VK_F2;
	sys.m_InBtnKey[1] = VK_F3;
	sys.m_InBtnKey[2] = VK_F4;
	sys.m_InBtnKey[3] = VK_F5;
	sys.m_InBtnCol[0] = clBlack;
	sys.m_InBtnCol[1] = clBlack;
	sys.m_InBtnCol[2] = clBlack;
	sys.m_InBtnCol[3] = clBlack;

	sys.m_MacroImm = 0;

	sys.m_ExtCmd[0] = "";
	sys.m_ExtCmd[1] = "";
	sys.m_ExtCmd[2] = "";
	sys.m_ExtCmd[3] = "";

	KExtCmd1->Caption = "G3PLX PSK31SBW";
	KExtCmd2->Caption = "IZ8BLY Hellschreiber";

	sys.m_FontAdjSize = 0;
	sys.m_BtnFontAdjSize = 0;

	memset(sys.m_SysKey, 0, sizeof(sys.m_SysKey));
	sys.m_SysKey[kkPlayPos] = 'A' + 0x100;
	sys.m_SysKey[kkScope] = 'O' + 0x100;
	sys.m_SysKey[kkLogList] = 'L' + 0x100;
	sys.m_SysKey[kkQSOData] = 'D' + 0x100;

	sys.m_SysKey[kkTX] = VK_F9;
	sys.m_SysKey[kkTXOFF] = VK_F8;
	sys.m_SysKey[kkCall] = 'C' + 0x100;
	sys.m_SysKey[kkName] = 'N' + 0x100;
	sys.m_SysKey[kkQTH] = 'Q' + 0x100;
	sys.m_SysKey[kkRST] = 'R' + 0x100;
	sys.m_SysKey[kkFreq] = 'B' + 0x100;
	sys.m_SysKey[kkFind] = 'F' + 0x100;
	sys.m_SysKey[kkClear] = VK_F1;

	sys.m_SysKey[kkInHome] = VK_HOME;
	sys.m_SysKey[kkInEnd] = VK_END;

	sys.m_SysKey[kkRxPUp] = VK_PRIOR;
	sys.m_SysKey[kkRxPDown] = VK_NEXT;

	sys.m_SysKey[kkInHeightUp]= VK_UP | 0x100;
	sys.m_SysKey[kkInHeightDown] = VK_DOWN | 0x100;

	sys.m_SysKey[kkPanel] = 'C' + 0x200;
	sys.m_SysKey[kkTxLTR] = 'L' + 0x200;

	sys.m_SysKey[kkDecShift] = VK_LEFT + 0x200;
	sys.m_SysKey[kkIncShift] = VK_RIGHT + 0x200;
	sys.m_SysKey[kkToggleShift] = 'T' + 0x200;

	sys.m_SysKey[kkCList] = 'F' + 0x200;

	sys.m_Call = "NOCALL";
	sys.m_TxRxName = "NONE";
	sys.m_TxRxInv = 0;

	sys.m_TxNet = 1;
	sys.m_Rev = 0;
	sys.m_TxDisRev = 0;
	sys.m_TxPort = txSound;
	sys.m_TxdJob = 0;
	sys.m_TxFixShift = 0;
	sys.m_echo = 1;

	sys.m_FFTGain = 1;
	sys.m_FFTResp = 2;

	sys.m_ColorRXBack = clWhite;
	sys.m_ColorRX = clBlack;
	sys.m_ColorRXTX = clRed;
	sys.m_ColorINBack = clWhite;
	sys.m_ColorIN = clBlack;
	sys.m_ColorINTX = clRed;

	sys.m_ColorLow = clBlack;
	sys.m_ColorHigh = clWhite;

	sys.m_ColorXY = clWhite;

	sys.m_SoundFifoRX = 12;
	sys.m_SoundFifoTX = 4;
	sys.m_SoundDevice = -1;
    sys.m_SoundOutDevice=-1;    //AA6YQ 1.66
	sys.m_SoundPriority = 1;
	sys.m_SoundStereo = 0;

	sys.m_HideFlexAudio=0;     //AA6YQ 1.70E

	sys.m_txuos = 1;
	sys.m_dblsft = 0;

	sys.m_AFC = 1;
	sys.m_FixShift = 1;
	sys.m_AFCTime = 8.0;
	sys.m_AFCSweep = 1.0;
	sys.m_AFCSQ = 32;

	sys.m_LimitGain = 200.0;

	sys.m_DefMarkFreq = 2125.0;
	sys.m_DefShift = 170.0;
	sys.m_DefStopLen = 0;

	sys.m_log = 0;
	sys.m_LogName = "";
	sys.m_logTimeStamp = 1;
	sys.m_ShowTimeStamp = 1;

//	sys.m_FontName = "ÇlÇr ÉSÉVÉbÉN";
//	sys.m_FontCharset = SHIFTJIS_CHARSET;
	sys.m_FontAdjX = 0;
	sys.m_FontAdjY = 0;
	sys.m_FontSize = 10;
	sys.m_FontZero = 1;
	sys.m_FontStyle = 0;

	sys.m_FontAdjSize = 0;

	sys.d_PaletteMask = 0x00000000;
	sys.m_Palette = 0;

	sys.m_XYInv = 0;

	sys.m_SBFontSize = SBM1->Font->Size;
	sys.m_SBINFontSize = SBIN1->Font->Size;
	KHint->Checked = TRUE;

	sys.m_CharLenFile = 1024;

	sys.m_AutoTimeOffset = 0;
	sys.m_TimeOffset = 0;
	sys.m_TimeOffsetMin = 0;
//	Panel2->Top = GroupBox1->Height + 1;	// éÛêMâÊñ ÉTÉCÉYÇÃí≤êÆ
	UpdatePanel();
	LogLink.SetHandle(Handle, CM_CMML);

	ReadSampFreq();
	pSound = new TSound(TRUE);
	WriteProfile(1025, "Default", TRUE);				// ÉfÉtÉHÉãÉgÉpÉâÉÅÅ[É^ÇÃãLâØ
	ReadRegister();
	WriteProfile(1026, "Return to the startup", TRUE);	// ÉfÉtÉHÉãÉgÉpÉâÉÅÅ[É^ÇÃãLâØ

	//AA6YQ 1.70J

	if( lcid != LANG_JAPANESE ){
		sprintf(bf, "%s"HELPNAME_A, BgnDir);
	}
	if( IsFile(bf) ){
			JanHelp = bf;
	}

	//sprintf(bf, "%s"HELPNAME_A, BgnDir);
	//if( (sys.m_HTMLHelp == HELPNAME_A)||(sys.m_HTMLHelp == HELPNAME_B) ){
	//	sys.m_HTMLHelp = "";
	//}
	//if( IsFile(bf) ){
	//	sys.m_HTMLHelp = HELPNAME_A;
	//	JanHelp = bf;
	//}
	//else {
	//	sprintf(bf, "%s"HELPNAME_B, BgnDir);
	//	if( IsFile(bf) ){
	//		sys.m_HTMLHelp = HELPNAME_B;
	//		JanHelp = bf;
	//    }
	//}

#if 0
	if( !strcmpi(GetEXT(sys.m_HTMLHelp.c_str()), "HLP") || !strcmpi(GetEXT(sys.m_HTMLHelp.c_str()), "CHM") ){
		sprintf(bf, "%s%s", BgnDir, sys.m_HTMLHelp.c_str());
		if( (fp = fopen(bf, "rb"))!=NULL ){
			fclose(fp);
			JanHelp = bf;
		}
	}
#endif
#if 0
	sprintf(bf, "%sReadme.txt", BgnDir);
	fp = fopen(bf, "rb");
	if( fp != NULL ){
		fclose(fp);
	}
	else {
		KMHelp->Remove(KRM);
	}
#endif

	if( sys.m_AutoTimeOffset ) SetTimeOffsetInfo(sys.m_TimeOffset, sys.m_TimeOffsetMin);
	AddHelpMenu();
	UpdateSystemFont();

	InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
	PrintText.SetPaintBox(PBoxRx, ScrollBarRx);
	FifoEdit.SetPaintBox(PBoxIn, ScrollBarIn);
	PrintText.SetCursor(1);
	rtty.SetCodeSet(); rttysub.SetCodeSet();

	if( sys.m_log ){
		PrintText.MakeLogName();
		PrintText.OpenLogFile();
		if( sys.m_logTimeStamp ) PrintText.TrigTimeStamp("MMTTY Startup");
	}
	m_DisEvent = 1;
	if( !Remote ){
		UpdateMsgList();
		UpdateItem();
		UpdateMacro();
		UpdateMacroShow(0);
		UpdateUI();
		UpdateLMS();
		UpdateRecentMenu();
		UpdateRev();
		UpdateShortCut(TRUE);
		UpdateStayOnTop();
		UpdateLWait();
	}
	if( !Remote ){
		if( int(WindowState) != sys.m_WindowState ) WindowState = TWindowState(sys.m_WindowState);
	}
	Application->OnIdle = OnIdle;
	if( Remote ){
		sys.m_DisTX = 0;
		UpdateRemort();
		UpdateItem();
		UpdateUI();
		UpdateLMS();
		UpdateStayOnTop();
		if( Remote & REMSHOWOFF ){
			Application->ShowMainForm = FALSE;
			FormPaint(NULL);
		}
		else if( Remote & REMNOMOVE ){
			Application->ShowMainForm = FALSE;
			FormPaint(NULL);
		}
	}
	ReadProfileList();
}

__fastcall TMmttyWd::~TMmttyWd()
{
	if( hMap != NULL ){
		if( pMap != NULL ){
			::UnmapViewOfFile(pMap);
		}
		::CloseHandle(hMap);
		hMap = NULL;
	}
	pMap = NULL;
}

void __fastcall TMmttyWd::StartOption(void)
{
	AnsiString as = ParamStr(0).c_str();
	SetDirName(BgnDir, as.c_str());
	m_strCommonMemory = "MMTTY";
	m_cRemoteTimeout = 100;

	for( int i = 0; i <= ParamCount(); i++ ){
		as = ParamStr(i).c_str();
		LPCSTR p = as.c_str();
		if( as == "-r" ){
			Remote = 0x80000000|REMMENU|REMCTRL;
			ClientWidth = 640;
			ClientHeight = 180;
		}
		else if( as == "-u" ){
			Remote = 0x80000000|REMCTRL;
			ClientWidth = 640;
			ClientHeight = 150;
		}
		else if( as == "-s" ){
			Remote = 0x80000000|REMMENU;
			ClientWidth = 300;
			ClientHeight = 180;
		}
		else if( as == "-t" ){
			Remote = 0x80000000;
			ClientWidth = 300;
			ClientHeight = 150;
		}
		else if( as == "-f" ){
			BorderStyle = bsSingle;
		}
		else if( as == "-m" ){
			Remote = 0x80000000 | REMSHOWOFF;
		}
		else if( as == "-d" ){
			if( Remote ){
				BorderStyle = bsNone;
				Remote |= REMNOMOVE;
			}
		}
		else if( as == "-p" ){
			if( Remote ){
				Remote |= REMNOPTT;
			}
		}
		else if( as == "-n" ){
			if( Remote ){
				Remote |= REMNOCNV;
			}
		}
        else if( as == "-a" ){
			if( Remote ) Remote |= REMDISSTAYONTOP;
        }
		else if( !strncmp(p, "-h", 2) ){
			p += 2;
			APP_HANDLE = (HWND)htoin(p, -1);
			if( !APP_HANDLE ){
				APP_HANDLE = HWND_BROADCAST;
			}
		}
		else if( !strncmp(p, "-C", 2) ){
			p += 2;
			if( *p ){
				m_strCommonMemory = p;
            }
        }
		else if( !strncmp(p, "-T", 2) ){
			p += 2;
			if( *p ){
				m_cRemoteTimeout = atoin(p, -1) * 10;
            }
        }
	}
	strcpy(LogDir, BgnDir);
	strcpy(OutFileDir, BgnDir);
	strcpy(MMLogDir, BgnDir);
	strcpy(ExtLogDir, BgnDir);
	strcpy(RecDir, BgnDir);
}

void __fastcall TMmttyWd::UpdateComarray(void)
{
	if( pMap == NULL ) return;

	if( pMap->title[0] ){
		Caption = pMap->title;
	}
	if( pMap->comName[0] ){
		sys.m_TxRxName = pMap->comName;
		COMM.change = 1;
	}
	else {
		StrCopy(pMap->comName, sys.m_TxRxName.c_str(), sizeof(pMap->comName));
	}
	if( pMap->comRadio[0] ){
		StrCopy(RADIO.StrPort, pMap->comRadio, sizeof(RADIO.StrPort)-1);
		RADIO.change = 1;
	}
	else {
		StrCopy(pMap->comRadio, RADIO.StrPort, sizeof(pMap->comRadio)-1);
	}
	OpenCloseCom();
}

void __fastcall TMmttyWd::UpdateRemort(void)
{
	if( Remote ){
		MSG_MMTTY=::RegisterWindowMessage("MMTTY");

		if( KPanel->Checked == FALSE ){
			KPanel->Checked = TRUE;
			UpdateControlPanel();
		}
		if( KFFT->Checked == FALSE ){
			KFFT->Checked = TRUE;
			KFFTClick(NULL);
		}
		BorderIcons >> biSystemMenu >> biMinimize >> biMaximize	>> biHelp;
		KMFile->Visible = FALSE;
		KMEdit->Visible = FALSE;
		KExtCmd->Visible = FALSE;
		KMHelp->Visible = FALSE;

		KPanel->Visible = FALSE;
		KPanelSize->Visible = FALSE;
		N11->Visible = FALSE;
		KMac->Visible = FALSE;
		N18->Visible = FALSE;
		KFFT->Visible = FALSE;
		KClrRx->Visible = FALSE;
		N6->Visible = FALSE;
		KOS->Visible = FALSE;
		KHint->Visible = FALSE;
		KLog->Visible = FALSE;
		KQSO->Visible = FALSE;
		N8->Visible = FALSE;
		N3->Visible = FALSE;

		KVol->Visible = FALSE;
		KVolIn->Visible = FALSE;
		KTest->Visible = FALSE;
		N15->Visible = FALSE;
		KSend->Visible = FALSE;
		KENT->Visible = FALSE;
		KWP->Visible = FALSE;
		N19->Visible = FALSE;
		KRun->Visible = FALSE;
		KPttTim->Visible = FALSE;
		N4->Visible = FALSE;
		KTX->Visible = FALSE;
        N25->Visible = FALSE;
		KTNC->Visible = FALSE;
		KOptLog->Visible = FALSE;
		GroupM->Visible = FALSE;
		if( !(Remote & REMCTRL) ){
			GroupDem->Visible = FALSE;
			GroupCtr->Visible = FALSE;
		}
		if( !(Remote & REMMENU) ){
			KView->Visible = FALSE;
			KMOpt->Visible = FALSE;
			KS->Visible = FALSE;
			if( !(Remote & REMCTRL) ){
				Application->OnActivate = OnActivate;
			}
		}
		if( Remote & REMSHOWOFF ){
			Application->OnActivate = OnRestore;
		}

		Application->OnMessage = OnMSG;
		Caption = (Remote >= 2) ? "RTTY Indicator":"RTTY Control panel";
		m_RemoteTimer = m_cRemoteTimeout;		// Changed 50 to 100 by JE3HHT on Sep.2010
		FirstFileMapping();
		OnKeyDown = NULL;
		OnKeyPress = NULL;
		OnKeyUp = NULL;
	}
}

void __fastcall TMmttyWd::FirstFileMapping(void)
{
	if( hMap != NULL ) return;

	hMap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, m_strCommonMemory.c_str());
	if( hMap != NULL ){
		pMap = (COMARRAY *)::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		pMap->smpFreq = pMap->smpFFT ? 8000 : SampFreq;
		UpdateComarray();
		pMap->flagFFT = 0;
		pMap->flagXY = 0;
		strcpy(pMap->verMMTTY, VERID);
		double err;
		switch(SampType){
			case 1:
				err = (SampFreq - 8000.0) * 1.0e6/8000.0;
				break;
			case 2:
				err = (SampFreq - 6000.0) * 1.0e6/6000.0;
				break;
			case 3:
				err = (SampFreq - 12000.0) * 1.0e6/12000.0;
				break;
			default:
				err = (SampFreq - 11025.0) * 1.0e6/11025.0;
				break;
		}
		pMap->errorClock = err + 0.5;
		pMap->smpDemFreq = DemSamp + 0.5;
	}
	Application->Title = Caption.c_str();
}

void __fastcall TMmttyWd::UpdateSystemFont(void)
{
	TFontStyles fsw = Code2FontStyle(sys.m_WinFontStyle);
	TFontStyles fsb = Code2FontStyle(sys.m_BtnFontStyle);
	if( (sys.m_WinFontName != Font->Name)||
		(sys.m_WinFontCharset != Font->Charset)||
		(fsw != Font->Style)||
		(sys.m_BtnFontName != SBM1->Font->Name)||
		(sys.m_BtnFontCharset != SBM1->Font->Charset)||
		(fsb != SBM1->Font->Style)
	){

		Font->Name = sys.m_WinFontName;
		Font->Charset = sys.m_WinFontCharset;
		Font->Style = fsw;

		AlignMain.NewFont(sys.m_WinFontName, sys.m_WinFontCharset, fsw);
		AlignTop.NewFont(sys.m_WinFontName, sys.m_WinFontCharset, fsw);
		AlignQSO.NewFont(sys.m_WinFontName, sys.m_WinFontCharset, fsw);
		AlignStat.NewFont(sys.m_WinFontName, sys.m_WinFontCharset, fsw);

		for( int i = 0; i < 16; i++ ){
			GetSB(i)->Font->Name = sys.m_BtnFontName;
			GetSB(i)->Font->Charset = sys.m_BtnFontCharset;
			GetSB(i)->Font->Style = fsb;
		}
		SBIN1->Font->Name = sys.m_BtnFontName;
		SBIN1->Font->Charset = sys.m_BtnFontCharset;
		SBIN1->Font->Style = fsb;
		SBIN2->Font->Name = sys.m_BtnFontName;
		SBIN2->Font->Charset = sys.m_BtnFontCharset;
		SBIN2->Font->Style = fsb;
		SBIN3->Font->Name = sys.m_BtnFontName;
		SBIN3->Font->Charset = sys.m_BtnFontCharset;
		SBIN3->Font->Style = fsb;
		SBIN4->Font->Name = sys.m_BtnFontName;
		SBIN4->Font->Charset = sys.m_BtnFontCharset;
		SBIN4->Font->Style = fsb;
		HisCall->Font->Name = sys.m_BtnFontName;
		HisCall->Font->Charset = sys.m_BtnFontCharset;
		HisCall->Font->Style = fsb;
		HisName->Font->Name = sys.m_BtnFontName;
		HisName->Font->Charset = sys.m_BtnFontCharset;
		HisName->Font->Style = fsb;
		HisQTH->Font->Name = sys.m_BtnFontName;
		HisQTH->Font->Charset = sys.m_BtnFontCharset;
		HisQTH->Font->Style = fsb;
		HisRST->Font->Name = sys.m_BtnFontName;
		HisRST->Font->Charset = sys.m_BtnFontCharset;
		HisRST->Font->Style = fsb;
		MyRST->Font->Name = sys.m_BtnFontName;
		MyRST->Font->Charset = sys.m_BtnFontCharset;
		MyRST->Font->Style = fsb;
		Freq->Font->Name = sys.m_BtnFontName;
		Freq->Font->Charset = sys.m_BtnFontCharset;
		Freq->Font->Style = fsb;
	}
	if( Font->Charset != SHIFTJIS_CHARSET ){
		KMFile->Caption = "File(&F)";
		KMEdit->Caption = "Edit(&E)";
		KView->Caption = "View(&V)";
		KMOpt->Caption = "Option(&O)";
		KExtCmd->Caption = "Program(&P)";
		KS->Caption = "Profiles(&S)";
		KMHelp->Caption = "Help(&H)";
// File Nenu for english
		KRcvLog->Caption = "Log Rx file(&L)";
		KFileOut->Caption = "Send Text(&S)...";
		KSaveRx->Caption = "RxWindow to file(&T)...";
		KOpenLog->Caption = "Open LogData file(&O)...";
		KFlush->Caption = "Save data now(&M)";
		KRxTime->Caption = "Record WAVE(mmv) immediately (&A)";
		KRxRec->Caption = "Record WAVE(mmv) (&W) as...";
		KRxPlay->Caption = "Play WAVE(mmv) (&R)...";
		KRxPos->Caption = "Seek Play position...";
		KRxTop->Caption = "Rewind Play/Record(&B)";
		KRxPause->Caption = "Pause Play/Record(&P)";
		KRxStop->Caption = "Close Play/Record(&Z)";
		KExit->Caption = "Exit MMTTY(&X)";

		KPaste->Caption = "Paste to TxWindow(&P)";
		KMacro->Caption = "Edit Macro Buttons(&M)...";
		KMsg->Caption = "Edit Messages(&C)...";

		KShortCut->Caption = "Assign ShortCut Keys(&K)...";

		KPanelSize->Caption = "Control Panel Size";
		KMac->Caption = "Macro Buttons";
		KFFT->Caption = "FFT Display";
		KFFTFW->Caption = "FFT Width";
		KFFTGain->Caption = "FFT Sensitivity";
		KFFTR->Caption = "FFT Response";
		KXYScopeSize->Caption = "XYScope Size";
		KXYScopeQuality->Caption = "XYScope Quality";
		KOS->Caption = "Scope";
		KClrRx->Caption = "Clear Rx Window";
		KHint->Caption = "Show Button Hint";
		KLog->Caption = "LogData List...";
		KQSO->Caption = "Current QSO Data...";

		KVol->Caption = "Soundcard output level(&V)...";
		KVolIn->Caption = "Soundcard input level(&I)...";
		KTest->Caption = "Test";
		KSend->Caption = "Way to send(&S)";
		KSChar->Caption = "CHARACTER OUT";
		KSWord->Caption = "WORD OUT";
		KSLine->Caption = "LINE OUT";
		KENT->Caption = "Auto send CR/LF with TX button";
		KTX->Caption = "Disable transmission";
		KWP->Caption = "Word wrap on keyboard";
		KRun->Caption = "Running mode";
		KPttTim->Caption = "PTT Timer(&P)...";
		KTNC->Caption = "Setup TNC emulation(&T)...";
		KOptLog->Caption = "Setup Logging(&L)...";
		KOption->Caption = Remote ? "Setup(&O)...":"Setup MMTTY(&O)...";

		KSDel->Caption = "Delete Menu";
		KSEnt->Caption = "Assign Menu";
		KSSV->Caption = "Save to file(&S)";
		KSLD->Caption = "Load from file(&L)";
		KExtEnt->Caption = "Assign Menu";
		KExtDel->Caption = "Delete Menu";

		KExtReset->Caption = "Resume(&R)";
		KExtSusp->Caption = "Suspend(&S)";

		KSetHelp->Caption = "Setup Help(&S)...";
		KWebHHT->Caption = "MM Open Source WebSite";
		KWebJARTS->Caption = "JARTS WebSite";
		KDispVer->Caption = "About MMTTY(&A)";

		KRM->Caption = "Version-up information";

		KRO->Caption = "Options of Received-log";
		KROF->Caption = "Folder...";
		KROT->Caption = "Log time stamp";
		KROTU->Caption = "Time stamp is UTC";
		KROTD->Caption = "Show time stamp in the RX window";
	}
	KHlpTxt->Caption = sys.m_Help.c_str();
	KHlpLog->Caption = sys.m_HelpLog.c_str();
	KHlpDig->Caption = sys.m_HelpDigital.c_str();
    SBTX->Font->Color = clRed;
}

void __fastcall TMmttyWd::ReqPaletteChange(void)
{
	if( UsrPal != NULL ){		// 256êFÉAÉ_ÉvÉ^Å[ÇÃéû
		ClosePalette();
		m_ReqPaletteChange = 1;
	}
}

void __fastcall TMmttyWd::UpdateColor(void)
{
	PanelRx->Color = sys.m_ColorRXBack;
	PanelIn->Color = sys.m_ColorINBack;
	if( UsrPal != NULL ){		// 256êFÉAÉ_ÉvÉ^Å[ÇÃéû
		ClosePalette();
		m_ReqPaletteChange = 1;
	}
	else {
		PBoxWater->Invalidate();
		PBoxXY->Invalidate();
		PBoxRx->Invalidate();
		PBoxIn->Invalidate();
	}
}
//---------------------------------------------------------------------------
// ÉAÉCÉhÉãèàóù
void __fastcall TMmttyWd::OnIdle(TObject *Sender, bool &Done)
{
	if( Log.IsOpen() ){
		if( Log.m_CurChg ){
			Log.SetLastPos();
			UpdateTextData();
			Log.m_CurChg = 0;
			SBQSO->Down = Log.m_sd.btime ? 1 : 0;
		}
	}
	UpdateUI();
	TimerFocus();
	if( m_ReqPaletteChange ){
		m_ReqPaletteChange = 0;
		if( sys.m_Palette ){
			if( UsrPal == NULL ){
				SetColorIndex();
			}
		}
		else {
			if( UsrPal != NULL ){
				ClosePalette();
			}
		}
		if( UsrPal != NULL ){
			m_PaletteTimer = 3;
		}
	}
}
#if 0
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::TopWindow(void)
{
	if( FormStyle == fsStayOnTop ){
		::SetWindowPos(Handle, HWND_TOPMOST, Left, Top, Width, Height, SWP_NOMOVE|SWP_NOSIZE);
	}
}
#endif
//---------------------------------------------------------------------------
// ÉRÉìÉgÉçÅ[ÉãÉpÉlÉãÇÃí≤êÆ
void __fastcall TMmttyWd::UpdateXY2(void)
{
	int w;
	if( m_XYSize ){
		w = PanelWater->Top - PanelFFT->Top + PanelWater->Height;
	}
	else {
		w = PanelFFT->Height - 1;
	}
	if( PanelTop->Visible ){
		if( PanelXY->Parent != PanelTop ){
			PanelXY->Visible = FALSE;
			PanelRx->RemoveControl(PanelXY);
			PanelTop->InsertControl(PanelXY);
			PanelXY->Parent = PanelTop;
			PanelXY->Visible = KXYScope->Checked;
		}
		PanelXY->SetBounds(PanelFFT->Left + PanelFFT->Width - w, PanelFFT->Top, w, w);
	}
	else {
		if( PanelXY->Parent != PanelRx ){
			PanelXY->Visible = FALSE;
			PanelTop->RemoveControl(PanelXY);
			PanelRx->InsertControl(PanelXY);
			PanelXY->Parent = PanelRx;
			PanelXY->Visible = KXYScope->Checked;
		}
		PanelXY->SetBounds(PBoxRx->Width - PanelXY->Width, PBoxRx->Top, w, w);
	}
}
//---------------------------------------------------------------------------
// ÉRÉìÉgÉçÅ[ÉãÉpÉlÉãÇÃí≤êÆ
void __fastcall TMmttyWd::UpdateControlPanel(void)
{
	if( KPanel->Checked != PanelTop->Visible ){
		PanelTop->Visible = KPanel->Checked;
		PanelRx->Top = (PanelQSO->Top + PanelQSO->Height);
		PanelRx->Height = PanelStat->Top - (PanelQSO->Top + PanelQSO->Height);
		PrintText.SetPaintBox(PBoxRx, ScrollBarRx);
		UpdateXY2();
		if( Remote ) PostApp(TXM_HEIGHT, Height);
	}
}
//---------------------------------------------------------------------------
// éÛêMâÊñ ÇÃÉTÉCÉYí≤êÆ
void __fastcall TMmttyWd::UpdatePanel(void)
{
	PanelRx->Top = (PanelQSO->Top + PanelQSO->Height);
	PanelRx->Height = PanelStat->Top - (PanelQSO->Top + PanelQSO->Height);
	PrintText.SetPaintBox(PBoxRx, ScrollBarRx);
	FifoEdit.SetPaintBox(PBoxIn, ScrollBarIn);
}
//---------------------------------------------------------------------------
// ÉçÉOÉpÉlÉãÇÃí≤êÆ
void __fastcall TMmttyWd::AlignLogPanel(void)
{
	AlignQSO.NewAlign(PanelQSO);
	if( Log.m_LogSet.m_Contest && (Log.m_LogSet.m_Contest != testPED) ){
		int wname = HisName->Width;
		int wrst = HisRST->Width;
		int nname = wname * 7 / 16;
		HisName->Width = nname;
		HisQTH->Width = nname;
		int off = (wname - nname)/2;
		int nrst = wrst + off;
		LMY->Left -= off * 2;
		MyRST->Left -= off * 2;
		LHIS->Left -= off;
		HisRST->Left -= off;
		HisRST->Width = nrst;
		MyRST->Width = nrst;
	}
}
//---------------------------------------------------------------------------
// ÉTÉCÉYïœçXÉCÉxÉìÉg
void __fastcall TMmttyWd::FormResize(TObject *Sender)
{
	if( Timer->Enabled == FALSE ) return;
	if( pSound == NULL ) return;
	if( m_DisAlign ) return;
	if( Remote & REMSHOWOFF ) return;

	if( Remote ){		// ÉäÉÇÅ[Égéû
		PanelRx->Visible = FALSE;
		PanelIn->Visible = FALSE;
		PanelStat->Visible = FALSE;
		PanelQSO->Visible = FALSE;
		PanelTop->Height = ClientHeight;
		AlignTop.NewAlign(PanelTop);
		if( !(Remote & REMCTRL) ){
			if( PanelFFT->Top != 0 ){
				PanelFFT->Height += PanelFFT->Top;
				PanelFFT->Top = 0;
				PanelXY->Top = 0;
				PanelWater->Height = PanelTop->ClientHeight - PanelWater->Top;
			}
		}
		UpdatePanel();
		if( GroupCtr->Visible == FALSE ){
			PanelFFT->Left = GroupCtr->Left;
			PanelWater->Left = GroupCtr->Left;
			PanelFFT->Width = ClientWidth - GroupCtr->Left - 2;
			PanelWater->Width = ClientWidth - GroupCtr->Left - 2;
		}
		else if( GroupM->Visible == FALSE ){
			PanelFFT->Left = GroupM->Left;
			PanelWater->Left = GroupM->Left;
			PanelFFT->Width = ClientWidth - GroupM->Left - 2;
			PanelWater->Width = ClientWidth - GroupM->Left - 2;
		}
		else {
			PanelFFT->Left = GroupM->Left + GroupM->Width + 2;
			PanelWater->Left = GroupM->Left + GroupM->Width + 2;
			PanelFFT->Width = ClientWidth - PanelFFT->Left - 2;
			PanelWater->Width = ClientWidth - PanelWater->Left - 2;
		}
	}
	else {		// ÉXÉ^ÉìÉhÉAÉçÅ[Éìéû
		if( PanelIn->Align == alNone ){
			m_DisAlign++;
			PanelMac->Align = alTop;
			PanelIn->Align = alBottom;
			PanelStat->Align = alBottom;
			ClientHeight = m_BefHeight;
			m_DisAlign--;
		}
		else {
			m_BefHeight = ClientHeight;
		}
		PanelTop->Visible = TRUE;
		AlignMain.NewAlign(this);
		switch(m_PanelSize){
			case 0:
				AlignMain.NewAlign(this, PanelTop, 0.75);
				break;
			case 1:
				AlignMain.NewAlign(this, PanelTop, 0.9);
				break;
			case 3:
				AlignMain.NewAlign(this, PanelTop, 1.25);
				break;
			default:
				break;
		}
		UpdatePanel();
		AlignTop.NewAlign(PanelTop);
#if 1
		AlignLogPanel();
#else
		AlignQSO.NewAlign(PanelQSO);
#endif
		AlignStat.NewAlign(PanelQSO);
		if( GroupCtr->Visible == FALSE ){
			PanelFFT->Left = GroupCtr->Left;
			PanelWater->Left = GroupCtr->Left;
			PanelFFT->Width = ClientWidth - GroupCtr->Left - 2;
			PanelWater->Width = ClientWidth - GroupCtr->Left - 2;
		}
		else if( GroupM->Visible == FALSE ){
			PanelFFT->Left = GroupM->Left;
			PanelWater->Left = GroupM->Left;
			PanelFFT->Width = ClientWidth - GroupM->Left - 2;
			PanelWater->Width = ClientWidth - GroupM->Left - 2;
		}
		else {
			PanelFFT->Left = GroupM->Left + GroupM->Width + 2;
			PanelWater->Left = GroupM->Left + GroupM->Width + 2;
			PanelFFT->Width = ClientWidth - PanelFFT->Left - 2;
			PanelWater->Width = ClientWidth - PanelWater->Left - 2;
		}
		if( PanelMac->Visible == TRUE ){
			TSpeedButton *tp;
			double w = double(PanelMac->ClientWidth - 12)/16;
			double a = 0;
			for( int i = 0; i < 16; i++, a += w ){
				if( i && (!(i % 4)) ) a += 4.0;
				tp = GetSB(i);
				tp->Top = 0;
				tp->Height = PanelMac->ClientHeight;
				tp->Left = int(a);
				tp->Width = int(w);
			}
		}
	}
	if( UsrPal != NULL ){
		ClosePalette();
		m_ReqPaletteChange = 1;
	}
	delete pBitmapFFTIN;
	delete pBitmapWater;
	delete pBitmapSQ;
//    delete pBitmapXY;
	pBitmapFFTIN = new Graphics::TBitmap();
	pBitmapFFTIN->Width = PBoxFFTIN->Width;
	pBitmapFFTIN->Height = PBoxFFTIN->Height;

	pBitmapWater = new Graphics::TBitmap();
	pBitmapWater->Width = PBoxWater->Width;
	pBitmapWater->Height = PBoxWater->Height;

	pBitmapSQ = new Graphics::TBitmap();
	pBitmapSQ->Width = PBoxSQ->Width;
	pBitmapSQ->Height = PBoxSQ->Height;

	UpdateXYScope();
#if 0
	pBitmapXY = new Graphics::TBitmap();
	pBitmapXY->Width = PBoxXY->Width;
	pBitmapXY->Height = PBoxXY->Height;
#endif

	if( pSound != NULL ){
		pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
		pSound->DrawFFTWater(pBitmapWater, 1, KXYScope->Checked ? PBoxXY->Width : 0);
		DrawSQ();
	}
	UpdateMacro();
	UpdateControlPanel();

	// ÉXÉ^Å[ÉìÉhÉAÉçÅ[ÉìéûÇÃÉEÉCÉìÉhÉEã÷é~èàóù
	if( sys.m_DisWindow && (!Remote) ){
		m_DisAlign++;
		PanelMac->Align = alNone;
		PanelIn->Align = alNone;
		PanelStat->Align = alNone;
		ClientHeight = PanelTop->Height;
		m_DisAlign--;
	}
}
//---------------------------------------------------------------------------
// ÉTÉCÉYïœçXÉRÉìÉgÉçÅ[ÉãÇÃìoò^
void __fastcall TMmttyWd::EntryAlignControl(void)
{
	AlignMain.EntryControl(PanelTop, this, NULL);
	AlignMain.EntryControl(PanelMac, this, NULL);
	AlignMain.EntryControl(PanelQSO, this, NULL);
	AlignMain.EntryControl(PanelStat, this, NULL);

// ÉgÉbÉvÉpÉlÉãÇÃìoò^
	AlignTop.EntryControl(GroupCtr, PanelTop, GroupCtr->Font);
	AlignTop.EntryControl(SBTXOFF, PanelTop, SBTXOFF->Font);
	AlignTop.EntryControl(SBTX, PanelTop, SBTX->Font);
	AlignTop.EntryControl(SBFIG, PanelTop, SBFIG->Font);
	AlignTop.EntryControl(SBUOS, PanelTop, SBUOS->Font);
	AlignTop.EntryControl(GroupDem, PanelTop, GroupDem->Font);
	AlignTop.EntryControl(GroupM, PanelTop, GroupM->Font);
	AlignTop.EntryControl(Label1, PanelTop, Label1->Font);
	AlignTop.EntryControl(Label2, PanelTop, Label2->Font);
	AlignTop.EntryControl(Label3, PanelTop, Label3->Font);
	AlignTop.EntryControl(Label4, PanelTop, Label4->Font);
	AlignTop.EntryControl(LLPF, PanelTop, LLPF->Font);
	AlignTop.EntryControl(Label7, PanelTop, Label7->Font);
	AlignTop.EntryControl(LTap, PanelTop, LTap->Font);
	AlignTop.EntryControl(LHZ, PanelTop, LHZ->Font);
	AlignTop.EntryControl(SBRev, PanelTop, SBRev->Font);
	AlignTop.EntryControl(SBDem, PanelTop, SBDem->Font);
	AlignTop.EntryControl(SBHAM, PanelTop, SBHAM->Font);
	AlignTop.EntryControl(SBATC, PanelTop, SBATC->Font);
	AlignTop.EntryControl(SBNET, PanelTop, SBNET->Font);
	AlignTop.EntryControl(SBSQ, PanelTop, SBSQ->Font);
	AlignTop.EntryControl(SBAFC, PanelTop, SBAFC->Font);
	AlignTop.EntryControl(SBBPF, PanelTop, SBBPF->Font);
	AlignTop.EntryControl(SBLMS, PanelTop, SBLMS->Font);
	AlignTop.EntryControl(MarkFreq, PanelTop, MarkFreq->Font);
	AlignTop.EntryControl(ShiftFreq, PanelTop, ShiftFreq->Font);
	AlignTop.EntryControl(DemoBox1, PanelTop, DemoBox2->Font);
	AlignTop.EntryControl(DemoBox2, PanelTop, DemoBox2->Font);
	AlignTop.EntryControl(PanelSQ, PanelTop, NULL);
	AlignTop.EntryControl(PanelFFT, PanelTop, NULL);
	AlignTop.EntryControl(PanelWater, PanelTop, NULL);
	AlignTop.EntryControl(PanelXY, PanelTop, NULL);
	for( int i =0; i < 16; i++ ){
		AlignTop.EntryControl(GetSB(i), PanelTop, GetSB(i)->Font);
	}

// QSOÉpÉlÉãÇÃìoò^
	AlignQSO.EntryControl(LCall, PanelQSO, LCall->Font);
	AlignQSO.EntryControl(HisCall, PanelQSO, HisCall->Font);
	AlignQSO.EntryControl(LName, PanelQSO, LName->Font);
	AlignQSO.EntryControl(HisName, PanelQSO, HisName->Font);
	AlignQSO.EntryControl(HisQTH, PanelQSO, HisQTH->Font);
	AlignQSO.EntryControl(LHIS, PanelQSO, LHIS->Font);
	AlignQSO.EntryControl(HisRST, PanelQSO, HisRST->Font);
	AlignQSO.EntryControl(LMY, PanelQSO, LMY->Font);
	AlignQSO.EntryControl(MyRST, PanelQSO, MyRST->Font);
	AlignQSO.EntryControl(SBQSO, PanelQSO, SBQSO->Font);
	AlignQSO.EntryControl(SBData, PanelQSO, SBData->Font);
	AlignQSO.EntryControl(SBInit, PanelQSO, SBInit->Font);
	AlignQSO.EntryControl(SBFind, PanelQSO, SBFind->Font);
	AlignQSO.EntryControl(Freq, PanelQSO, Freq->Font);

// ì¸óÕÉ{É^ÉìÉpÉlÉãÇÃìoò^
	AlignStat.EntryControl(SBINClear, PanelStat, SBINClear->Font);
	AlignStat.EntryControl(SBIN1, PanelStat, SBIN1->Font);
	AlignStat.EntryControl(SBIN2, PanelStat, SBIN2->Font);
	AlignStat.EntryControl(SBIN3, PanelStat, SBIN3->Font);
	AlignStat.EntryControl(SBIN4, PanelStat, SBIN4->Font);
	AlignStat.EntryControl(MsgList, PanelStat, MsgList->Font);
	AlignStat.EntryControl(SBINEdit, PanelStat, SBINEdit->Font);
	AlignStat.EntryControl(LWait, PanelStat, LWait->Font);
	AlignStat.EntryControl(TBCharWait, PanelStat, NULL);

	int CX = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int CY = ::GetSystemMetrics(SM_CYFULLSCREEN);
//	int CX = ::GetSystemMetrics(SM_CXSCREEN);
//	int CY = ::GetSystemMetrics(SM_CYSCREEN);
	if( (CX < Width)||(CY < Height) ){
		Top = 0;
		Left = 0;
		Width = CX;
		Height = CY;
	}
	PanelTop->Align = alTop;
	PanelMac->Align = alTop;
	PanelQSO->Align = alTop;
	PanelRx->Align = alTop;
	PanelIn->Align = alBottom;
	PanelStat->Align = alBottom;
	FormCenter(this, CX, CY);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateStayOnTop(void)
{
	TFormStyle fs = (sys.m_StayOnTop || Remote) ? fsStayOnTop : fsNormal;
	if( FormStyle != fs ){
		AnsiString asMarkFreq;
		GetComboBox(asMarkFreq, MarkFreq);
		AnsiString asShiftFreq;
		GetComboBox(asShiftFreq, ShiftFreq);
		AnsiString asHisRST;
		GetComboBox(asHisRST, HisRST);

		FormStyle = fs;
        if( Remote && (Remote & REMDISSTAYONTOP) ){
			FormStyle = fsNormal;
        }
		if( pTnc != NULL ) pTnc->UpdateHandle(Handle);
		LogLink.SetHandle(Handle, CM_CMML);
		if( pRadio ) pRadio->UpdateHandle(Handle, CM_CMMR);

		SetComboBox(MarkFreq, asMarkFreq.c_str());
		SetComboBox(ShiftFreq, asShiftFreq.c_str());
		SetComboBox(HisRST, asHisRST.c_str());
		SetComboBox(MyRST, asHisRST.c_str());
		UpdateItem();
	}
//	if( pRadio != NULL ) pRadio->UpdateHandle(Handle);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateTitle(void)
{
	if( Remote ) return;
	char bf[512];
	if( IsCall(sys.m_Call.c_str()) ){
		sprintf(bf, "%s (%s) - "VERTTL2, sys.m_Call.c_str(), Log.GetName() );
	}
	else {
		strcpy(bf, VERTTL2);
	}
	if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
		strcat(bf, m_Running ? " (Running mode)":" (S&P mode)");
	}
	if( SampType ){
		char bbf[64];
		sprintf(bbf, " [based on %.lfHz]", SampBase);
		strcat(bf, bbf);
	}
	if( LogLink.IsLink() ){
		char bbf[64];
		sprintf(bbf, " [Link to %s]", LogLink.GetSessionName());
		strcat(bf, bbf);
	}
	Caption = bf;
	UpdateCallsign();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateItem(void)
{
	m_DisEvent++;
	UpdateTitle();
	SBUOS->Down = rtty.m_uos;
	SBFIG->Down = rtty.m_fig;

	SBSQ->Down = pSound->FSKDEM.GetSQ();

	SBRev->Down = sys.m_Rev;
	KTest->Checked = pSound->m_Test;
	KXYScope->Checked = pSound->FSKDEM.m_XYScope;

	SBATC->Down = pSound->FSKDEM.m_atc;
	SBNET->Down = sys.m_TxNet;
	KFFT->Checked = pSound->m_FFTSW;
	SBAFC->Down = sys.m_AFC;
	SBBPF->Down = pSound->m_bpf;
	SBLMS->Down = pSound->m_lmsbpf;
	KPttTim->Checked = m_PttTimer ? 1 : 0;
	UpdateLWait();

	if( pRadio != NULL ) pRadio->SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
	MarkFreq->Text = pSound->FSKDEM.GetMarkFreq();
	ShiftFreq->Text = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
	SelectCombo(TRUE);
	if( pComm != NULL ) pComm->SetInv(sys.m_TxRxInv);
	UpdateUI();
//    UpdateXYScope();
	UpdateLimit();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateFSK(void)
{
	if( sys.m_TxPort ) OpenClosePTT();
	if( (TNC.Type == 2) && (pTnc!=NULL) && !pTnc->IsMMT() ) OpenCloseTNC();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OpenCloseCom(void)
{
	if( COMM.change || TNC.change || RADIO.change ){
		if( COMM.change && strcmp(sys.m_TxRxName.c_str(), "NONE") ){
			if( pTnc != NULL ){
				TNC.change = 1;
				pTnc->Close();
				delete pTnc;
				pTnc = NULL;
			}
			if( pRadio != NULL ){
				RADIO.change = 1;
				pRadio->Close();
				delete pRadio;
				pRadio = NULL;
			}
		}
		if( RADIO.change && strcmp(RADIO.StrPort, "NONE") ){
			if( pTnc != NULL ){
				TNC.change = 1;
				pTnc->Close();
				delete pTnc;
				pTnc = NULL;
			}
		}
		if( COMM.change ){
			if( pComm != NULL ){
				delete pComm;
				pComm = NULL;
			}
			OpenClosePTT();
		}
		if( RADIO.change || ((pRadio == NULL)&&strcmp(RADIO.StrPort, "NONE")) ){
			OpenCloseRadio();
		}
		if( TNC.change || ((pTnc == NULL)&&strcmp(TNC.StrPort, "NONE")) ){
			OpenCloseTNC();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetFSKPara(COMMPARA *cp)
{
	cp->Baud = int(pSound->FSKDEM.GetBaudRate() + 0.5);
	cp->BitLen = pSound->FSKDEM.m_BitLen;
		// 0-1bit, 1-1.5bit, 2-2bit, 3-1.5bit, 4-1.5bit
//ÉfÅ[É^í∑5Ç∆ÉXÉgÉbÉv ÉrÉbÉg2ÇÃëgÇ›çáÇÌÇπÇÕñ≥å¯Ç≈Ç∑ÅB
//ÉfÅ[É^í∑6§ 7§ 8Ç∆ÉXÉgÉbÉv ÉrÉbÉg1.5Ç∆ÇÃëgÇ›çáÇÌÇπÇ‡ñ≥å¯Ç≈Ç∑ÅB
	switch(pSound->FSKMOD.m_StopLen){
		case 2:
			cp->Stop = (cp->BitLen >= 6) ? TWOSTOPBITS : ONE5STOPBITS;
			break;
		case 1:
		case 3:
		case 4:
			cp->Stop = (cp->BitLen >= 6) ? TWOSTOPBITS : ONE5STOPBITS;
			break;
		default:
			cp->Stop = ONESTOPBIT;
			break;
	}
	switch(pSound->FSKDEM.m_Parity){
		case 1:			// Even
			cp->Parity = EVENPARITY;
			break;
		case 2:			// Odd
			cp->Parity = ODDPARITY;
			break;
		case 3:			// 1
			cp->Parity = MARKPARITY;
			break;
		case 4:			// 0
		default:
			cp->Parity = NOPARITY;
			break;
	}
	if(cp->BitLen == 5){
		if( cp->Stop == ONE5STOPBITS ) cp->Stop = TWOSTOPBITS;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OpenClosePTT(void)
{
	if( !COMM.change ) return;
	COMM.change = 0;

	if( pComm != NULL ){
		delete pComm;
		pComm = NULL;
	}
	if( !strcmp(sys.m_TxRxName.c_str(), "NONE") ){
		if( (pMap != NULL) && !(Remote & REMDISSHARED) ) strcpy(pMap->comName, "NONE");
	}
	else {
		COMMPARA	cm;
		SetFSKPara(&cm);
		pComm = new CComm(TRUE);

        //AA6YQ 1.66 enable use of 2-digit com ports per http://support.microsoft.com/kb/115831/EN-US/
        char *ComPort = new char[33];

		if(!strncmp(sys.m_TxRxName.c_str(), "COM", 3) ){  //strncmp returns 0 if match
            //AA6YQ 1.66 but only if we're opening a COM port
            StrCopy(ComPort,"\\\\.\\");
            StrCat(ComPort,sys.m_TxRxName.c_str());
        }
        else {
            //AA6YQ 1.66 as opposed to, say, EXTFSK
            StrCopy(ComPort,sys.m_TxRxName.c_str());
        }

		if( sys.m_TxPort && !strncmp(sys.m_TxRxName.c_str(), "COM", 3) ){
                if( pComm->Open(ComPort, sys.m_TxRxInv, NULL) ){
				pComm->SetTXRX(0);
            	pComm->Close();
            }
        }

		if( pComm->Open(ComPort, sys.m_TxRxInv, sys.m_TxPort ? &cm : NULL) ){
			pComm->pMod = &pSound->FSKMOD;
			pComm->EnbTX(SBTX->Down);
			if( sys.m_TxPort ) pComm->Resume();		// TxdëóêMÉ^ÉXÉNÇìÆçÏÇ≥ÇπÇÈ
		}
		else {
			if( sys.m_TxPort && (cm.BitLen == 5) && (cm.Stop == TWOSTOPBITS) ){
				cm.Stop = ONE5STOPBITS;		// set 1.5bits
				if( pComm->Open(ComPort, sys.m_TxRxInv, &cm) == TRUE ){
					pComm->pMod = &pSound->FSKMOD;
					pComm->EnbTX(SBTX->Down);
					pComm->Resume();				// TxdëóêMÉ^ÉXÉNÇìÆçÏÇ≥ÇπÇÈ
					goto _noerr;
				}
			}
			delete pComm;
			pComm = NULL;
			if( !Remote || !m_RemoteTimer ){
				if (cm.Baud<110){ //1.70F notify user if attempting to open port at low speed, as many devices can't do this
					ErrorMB( (Font->Charset != SHIFTJIS_CHARSET)? "Cannot open '%s' at '%s' baud; if the selected serial port can't handle this baud rate, use EXTFSK.":"'%s'ÇÉ{Å[ÉåÅ[Ég'%s'Ç≈äJÇØÇ‹ÇπÇÒ.Ç‡ÇµÅAëIëÇµÇΩÉVÉäÉAÉãÉ|Å[ÉgÇ™Ç±ÇÃÉ{Å[ÉåÅ[ÉgÇÉTÉ|Å[ÉgÇµÇƒÇ¢Ç»Ç¢Ç∆Ç´ÇÕEXTFSKÇégÇ¡ÇƒÇ≠ÇæÇ≥Ç¢ÅB", sys.m_TxRxName.c_str(),AnsiString(cm.Baud).c_str());
				}
				else {
					ErrorMB( (Font->Charset != SHIFTJIS_CHARSET)? "Cannot open '%s'":"'%s'Ç™ÉIÅ[ÉvÉìÇ≈Ç´Ç‹ÇπÇÒ.", sys.m_TxRxName.c_str());
				}
			}
		}
_noerr:;
		if( (pMap != NULL) && !(Remote & REMDISSHARED) ) StrCopy(pMap->comName, sys.m_TxRxName.c_str(), sizeof(pMap->comName)-1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateMacro(void)
{
	int FontSize = sys.m_SBINFontSize * AlignStat.GetFontAdj(SBIN1);
	SetButtonCaption(SBIN1, sys.m_InBtnName[0], 6, FontSize, sys.m_InBtnCol[0]);
	SetButtonCaption(SBIN2, sys.m_InBtnName[1], 6, FontSize, sys.m_InBtnCol[1]);
	SetButtonCaption(SBIN3, sys.m_InBtnName[2], 6, FontSize, sys.m_InBtnCol[2]);
	SetButtonCaption(SBIN4, sys.m_InBtnName[3], 6, FontSize, sys.m_InBtnCol[3]);
	FontSize = sys.m_SBFontSize * AlignTop.GetFontAdj(SBM1);
	if( !KMac->Checked ) FontSize = sys.m_SBFontSize * (PanelMac->Width / 792.0);
	for( int i = 0; i < 16; i++ ){
		SetButtonCaption(GetSB(i), sys.m_UserName[i], KMac->Checked ? 4 : 5, FontSize, sys.m_UserCol[i]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateLimit(void)
{
	if( SBTX->Down && (sys.m_echo != 2) ){
		pSound->FSKDEM.m_Limit = 0;
	}
	else {
		pSound->FSKDEM.m_Limit = 1;
		pSound->FSKDEM.m_LimitGain = sys.m_LimitGain;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateLMS(void)
{
	SBLMS->Caption = pSound->m_lms.m_Type ? "Not." : "LMS";
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateUI(void)
{
	m_DisEvent++;
	SBAFC->Enabled = KFFT->Checked;
	SBQSO->Enabled = !HisCall->Text.IsEmpty();
	SBFind->Enabled = !HisCall->Text.IsEmpty();
	if( m_NameQTH ){
		if( HisName->Visible == TRUE ){
			LName->Caption = "QTH";
			HisName->Visible = FALSE;
			HisQTH->Visible = TRUE;
		}
	}
	else {
		if( HisName->Visible == FALSE ){
			LName->Caption = "Name";
			HisName->Visible = TRUE;
			HisQTH->Visible = FALSE;
		}
	}
	int i;
	for( i = 0; i < 16; i++ ){
		GetSB(i)->Enabled = !sys.m_User[i].IsEmpty();
	}
	if( pSound != NULL ){
		if( (m_DemType != pSound->FSKDEM.m_type)||
			(m_LpfType != pSound->FSKDEM.m_lpf)||
			(m_Baud != m_ShowBaud)
		){
			m_DemType = pSound->FSKDEM.m_type;
			m_LpfType = pSound->FSKDEM.m_lpf;
			switch(pSound->FSKDEM.m_type){
				case 0:
					GroupDem->Caption = "Demodulator (IIR)";
					LTap->Caption = m_Baud ? "Baud":"BW";
					LHZ->Visible = m_Baud ? FALSE : TRUE;
					LLPF->Caption = m_LpfType ? "LPF" : "AV.";
					break;
				case 1:
					GroupDem->Caption = "Demodulator (FIR)";
					LTap->Caption = m_Baud ? "Baud":"Tap";
					LHZ->Visible = FALSE;
					LLPF->Caption = m_LpfType ? "LPF" : "AV.";
					break;
				case 2:
					GroupDem->Caption = "Demodulator (PLL)";
					LTap->Caption = m_Baud ? "Baud":"VCO";
					LHZ->Visible = FALSE;
					LLPF->Caption = "Loop";
					break;
				case 3:
					GroupDem->Caption = "Demodulator (FFT)";
					LTap->Caption = m_Baud ? "Baud":"Tones";
					LHZ->Visible = FALSE;
					LLPF->Caption = m_LpfType ? "LPF" : "AV.";
					break;
			}
			SelectCombo(TRUE);
		}
		if( m_filemode != pSound->WaveFile.m_mode ){
			m_filemode = pSound->WaveFile.m_mode;
			switch(m_filemode){
				case 0:
					KRxRec->Checked = FALSE;
					KRxTime->Checked = FALSE;
					KRxRec->Enabled = TRUE;
					KRxTime->Enabled = TRUE;
					KRxPlay->Enabled = TRUE;
					KRxStop->Enabled = FALSE;
					KRxPause->Enabled = FALSE;
					KRxTop->Enabled = FALSE;
					KRxPos->Enabled = FALSE;
					break;
				case 1:
					KRxPlay->Enabled = FALSE;
					KRxTime->Enabled = FALSE;
					KRxRec->Enabled = FALSE;
					KRxStop->Enabled = TRUE;
					KRxPause->Enabled = TRUE;
					KRxTop->Enabled = TRUE;
					KRxPos->Enabled = TRUE;
					break;
				case 2:
					KRxPlay->Enabled = FALSE;
					KRxTime->Enabled = FALSE;
					KRxRec->Enabled = FALSE;
					KRxStop->Enabled = TRUE;
					KRxPause->Enabled = TRUE;
					KRxTop->Enabled = TRUE;
					KRxPos->Enabled = FALSE;
					break;
			}
		}
		KRxPause->Checked = pSound->WaveFile.m_pause;
	}
	if( Remote ){
    	RemoteStat();
    }
    else {
		SBTX->Enabled = !(sys.m_DisTX&1);
    }
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	CanClose = TRUE;
	Timer->Enabled = FALSE;
	CWaitCursor wait;
	if( Remote && m_OptionDialog ){
    	HWND hWnd;
		for( int i = Screen->FormCount - 1; i >= 0; i-- ){
			hWnd = Screen->Forms[i]->Handle;
			if( hWnd ){
            	if( hWnd != Handle ) ::SendMessage(hWnd, WM_CLOSE, 0, 0);
        	}
            else {
				break;
            }
        }
	}
	if( pComm != NULL ) pComm->ReqClose();
	if( pTnc != NULL ) pTnc->ReqClose();
	if( pRadio != NULL ) pRadio->ReqClose();
	if( pSound != NULL ){
		pSound->WaveFile.FileClose();
		ToRX(1);
		pSound->ReqStop();
		Log.Close();
        Log.DoBackup();
		if( sys.m_logTimeStamp ) PrintText.TrigTimeStampImm("MMTTY Close");
		PrintText.CloseLogFile();
		WriteRegister();
		pSound->WaitStop();
		delete pSound;
		pSound = NULL;
#if USEPAL
		ClosePalette();
#endif
		if( pBitmapRx != NULL ){
			delete pBitmapRx;
			pBitmapRx = NULL;
		}
		if( pBitmapIn != NULL ){
			delete pBitmapIn;
			pBitmapIn = NULL;
		}
		delete pBitmapFFTIN;
		delete pBitmapWater;
		delete pBitmapXY;
		delete pBitmapSQ;
		pBitmapFFTIN = NULL;
		pBitmapWater = NULL;
		pBitmapXY = NULL;
		pBitmapSQ = NULL;
	}
	if( p_SendFileDlg != NULL ){
		delete p_SendFileDlg;
		p_SendFileDlg = NULL;
	}
	if( pTnc != NULL ){
		pTnc->WaitClose();
		delete pTnc;
		pTnc = NULL;
	}
	if( pRadio != NULL ){
		pRadio->WaitClose();
		delete pRadio;
		pRadio = NULL;
	}
	if( pComm != NULL ){
		pComm->WaitClose();
		delete pComm;
		pComm = NULL;
	}
	::VirtualUnlock(&sys, sizeof(sys));
	::VirtualUnlock(this, sizeof(TMmttyWd));
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ©ÇÁÇÃì«Ç›èoÇµ
void __fastcall TMmttyWd::ReadSampFreq(void)
{
	char	bf[256];
	AnsiString	as, ws;

	sprintf(bf, "%sMmtty.ini", BgnDir);
	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	SampFreq = ReadDoubleIniFile(pIniFile, "SoundCard", "SampFreq", SampFreq);
	sys.m_TxOffset = ReadDoubleIniFile(pIniFile, "SoundCard", "TxOffset", 0);
#if 0
	if( (SampFreq > 12500.0) || (SampFreq < 10000.0) ) SampFreq = 11025.0;
#else
	if( (SampFreq > 12500.0) || (SampFreq < 5000.0) ) SampFreq = 11025.0;
	InitSampType();
#endif
	sys.m_SampFreq = SampFreq;
	delete pIniFile;
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ©ÇÁÇÃì«Ç›èoÇµ
void __fastcall TMmttyWd::ReadCombList(TMemIniFile *tp, AnsiString &as, LPCSTR pKey, LPCSTR pDef)
{
	as = tp->ReadString("ComboList", pKey, pDef);
	if( as.IsEmpty() ) as = pDef;
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ©ÇÁÇÃì«Ç›èoÇµ
void __fastcall TMmttyWd::ReadRegister(void)
{
	char	bf[256];
	AnsiString	as, ws;

	sprintf(bf, "%sMmtty.ini", BgnDir);
	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	ReadCombList(pIniFile, as, "Mark", "2125,2000,1700,1445,1275,1170,1000,915");
	SetComboBox(MarkFreq, as.c_str());
	ReadCombList(pIniFile, as, "Shift", "23,85,160,170,182,200,240,350,425,850");
	SetComboBox(ShiftFreq, as.c_str());
	ReadCombList(pIniFile, m_asBaud, "Baud", "22,45,45.45,50,56,75,100,110,150,200,300");
	ReadCombList(pIniFile, as, "RST", "599,579,569,559,449,339,448,337,225,111");
	SetComboBox(HisRST, as.c_str());
	SetComboBox(MyRST, as.c_str());
	ReadCombList(pIniFile, m_asIIRBW, "IIRBW", "15,20,25,30,35,40,45,50,60,70,80,90,100,150,200");
	ReadCombList(pIniFile, m_asVCOGain, "VCO", "1.0,1.2,1.5,2.0,2.5,3.0,3.25,3.5,3.75,4.0,4.5,5.0,8.0");
	ReadCombList(pIniFile, m_asLoopFC, "LoopFC", "30,40,50,80,100,150,200,250,300,350,400,450,500,600,800,1000");
	m_asSmooth = "20,22.5,25,30,35,40,45,50,60,70,80,100,150,200,300,600";
	m_asFilterTap = "24,48,56,64,72,80,96,128,144,192,256,386,512";

	sys.m_LogGridWidths = pIniFile->ReadString("Grid", "Log", "");
	sys.m_QSOGridWidths = pIniFile->ReadString("Grid", "QSO", "");

	KHint->Checked  = pIniFile->ReadInteger("Button", "Hint", KHint->Checked);

	sys.m_HTMLHelp = pIniFile->ReadString("Help", "HTML", sys.m_HTMLHelp);
	sys.m_Help = pIniFile->ReadString("Help", "MMTTY", sys.m_Help);
	sys.m_HelpLog = pIniFile->ReadString("Help", "MMTTYLOG", sys.m_HelpLog);
	sys.m_HelpDigital = pIniFile->ReadString("Help", "DIGITAL", sys.m_HelpDigital);
	sys.m_HelpFontName = pIniFile->ReadString("Help", "FontName", sys.m_HelpFontName);
	sys.m_HelpFontCharset = (BYTE)pIniFile->ReadInteger("Help", "FontCharset", sys.m_HelpFontCharset);
	sys.m_HelpFontSize = pIniFile->ReadInteger("Help", "FontSize", sys.m_HelpFontSize);
	sys.m_HelpNotePad = pIniFile->ReadInteger("Help", "UseNotePad", sys.m_HelpNotePad);

	sys.m_WindowState = pIniFile->ReadInteger("Window", "WindowState", sys.m_WindowState);
	sys.m_DisWindow = pIniFile->ReadInteger("Window", "Disable", sys.m_DisWindow);
	sys.m_StayOnTop = pIniFile->ReadInteger("Window", "StayOnTop", sys.m_StayOnTop);
	if( Remote ){
		if( !(Remote & REMSHOWOFF) ){
			Top = pIniFile->ReadInteger("Window", "WindowTop", Top);
			Left = pIniFile->ReadInteger("Window", "WindowLeft", Left);
			if( Remote & REMCTRL ){
				Width = pIniFile->ReadInteger("Window", "WindowWidth", Width);
			}
			else {
				Width = pIniFile->ReadInteger("Window", "WindowWidthS", Width);
			}
			if( Remote & REMMENU ){
				Height = pIniFile->ReadInteger("Window", "WindowHeightRM", Height);
			}
			else {
				Height = pIniFile->ReadInteger("Window", "WindowHeightR", Height);
			}
		}
	}
	else {
		KMac->Checked = pIniFile->ReadInteger("Window", "ShowMacro", KMac->Checked);
		GroupM->Visible = KMac->Checked;
		sys.m_MemWindow = pIniFile->ReadInteger("Window", "MemoryWindow", sys.m_MemWindow);
		if( sys.m_MemWindow ){
			Top = pIniFile->ReadInteger("Window", "WindowTopN", Top);
			Left = pIniFile->ReadInteger("Window", "WindowLeftN", Left);
			Width = pIniFile->ReadInteger("Window", "WindowWidthN", Width);
			if( !sys.m_DisWindow ){
				Height = pIniFile->ReadInteger("Window", "WindowHeightN", Height);
			}
		}

	}

	sys.m_SetupOnTop = pIniFile->ReadInteger("Window", "SetupOnTop", sys.m_SetupOnTop);   //1.70K

	sys.m_WinFontName = pIniFile->ReadString("WindowFont", "Name", sys.m_WinFontName);
	sys.m_WinFontCharset = (BYTE)pIniFile->ReadInteger("WindowFont", "Charset", sys.m_WinFontCharset);
	sys.m_WinFontStyle = pIniFile->ReadInteger("WindowFont", "Style", sys.m_WinFontStyle);

	//hack for forcing font to English (0) or Japanese (SHIFTJIS_CHARSET)
	//sys.m_WinFontCharset = 0; //SHIFTJIS_CHARSET;

	sys.m_FontAdjSize = pIniFile->ReadInteger("WindowFont", "Adjust", sys.m_FontAdjSize);
	sys.m_BtnFontName = pIniFile->ReadString("ButtonFont", "Name", sys.m_BtnFontName);
	sys.m_BtnFontCharset = (BYTE)pIniFile->ReadInteger("ButtonFont", "Charset", sys.m_BtnFontCharset);
	sys.m_BtnFontStyle = pIniFile->ReadInteger("ButtonFont", "Style", sys.m_BtnFontStyle);
	sys.m_BtnFontAdjSize = pIniFile->ReadInteger("ButtonFont", "Adjust", sys.m_BtnFontAdjSize);

	sys.m_FontName = pIniFile->ReadString("Font", "Name", sys.m_FontName);
	sys.m_FontSize = pIniFile->ReadInteger("Font", "Size", sys.m_FontSize);
	sys.m_FontAdjX = pIniFile->ReadInteger("Font", "AdjX", sys.m_FontAdjX);
	sys.m_FontAdjY = pIniFile->ReadInteger("Font", "AdjY", sys.m_FontAdjY);
	sys.m_FontCharset = pIniFile->ReadInteger("Font", "Charset", sys.m_FontCharset);
	sys.m_FontZero = pIniFile->ReadInteger("Font", "Zero", sys.m_FontZero);
	sys.m_FontStyle = pIniFile->ReadInteger("Font", "Style", sys.m_FontStyle);

	KPanel->Checked = pIniFile->ReadInteger("Define", "ControlPanel", 1);
	pSound->m_FFTSW = pIniFile->ReadInteger("Define", "FFT", pSound->m_FFTSW);
	pSound->m_FFTFW = pIniFile->ReadInteger("Define", "FFTFW", pSound->m_FFTFW);
	sys.m_FFTGain = pIniFile->ReadInteger("Define", "FFTGain", sys.m_FFTGain);
	sys.m_FFTResp = pIniFile->ReadInteger("Define", "FFTResp", sys.m_FFTResp);
	rtty.m_uos = pIniFile->ReadInteger("Define", "UOS", rtty.m_uos);
	sys.m_CodeSet = pIniFile->ReadInteger("Define", "CodeSet", sys.m_CodeSet);
	pSound->FSKDEM.m_XYScope = pIniFile->ReadInteger("Define", "XYScope", pSound->FSKDEM.m_XYScope);
	KXYScope->Checked = pSound->FSKDEM.m_XYScope;
	m_XYSize = pIniFile->ReadInteger("Define", "XYScopeSize", m_XYSize);
	m_XYQuality = pIniFile->ReadInteger("Define", "XYQuality", m_XYQuality);
	sys.m_XYInv = pIniFile->ReadInteger("Define", "XYScopeReverse", sys.m_XYInv);

	m_PanelSize = pIniFile->ReadInteger("Define", "PanelSize", m_PanelSize);

	sys.m_AFC = pIniFile->ReadInteger("Define", "AFC", sys.m_AFC);
	sys.m_FixShift = pIniFile->ReadInteger("Define", "AFCFixShift", sys.m_FixShift);
	sys.m_AFCSQ = pIniFile->ReadInteger("Define", "AFCSQ", sys.m_AFCSQ);
	sys.m_AFCTime = ReadDoubleIniFile(pIniFile, "Define", "AFCTime", sys.m_AFCTime);
	sys.m_AFCSweep = ReadDoubleIniFile(pIniFile, "Define", "AFCSweep", sys.m_AFCSweep);

	KENT->Checked = pIniFile->ReadInteger("Define", "AutoCR", KENT->Checked);
	KWP->Checked = pIniFile->ReadInteger("Define", "WardWarp", KWP->Checked);
	m_SendWay = pIniFile->ReadInteger("Define", "SendWay", m_SendWay);
	m_Running = pIniFile->ReadInteger("Define", "Running", m_Running);
	m_PttTimer = pIniFile->ReadInteger("Define", "PttTimer", m_PttTimer);
	sys.m_TxNet = pIniFile->ReadInteger("Define", "TxNet", sys.m_TxNet);
	sys.m_TxDisRev = pIniFile->ReadInteger("Define", "TxDisRev", sys.m_TxDisRev);
	sys.m_LimitGain = ReadDoubleIniFile(pIniFile, "Define", "LimitDxGain", sys.m_LimitGain);
	pSound->FSKDEM.m_LimitOverSampling = pIniFile->ReadInteger("Define", "LimitOverSampling", pSound->FSKDEM.m_LimitOverSampling);
	pSound->FSKDEM.m_LimitAGC = pIniFile->ReadInteger("Define", "LimitAGC", pSound->FSKDEM.m_LimitAGC);
	pSound->FSKDEM.m_atc = pIniFile->ReadInteger("Define", "ATC", pSound->FSKDEM.m_atc);
	pSound->FSKDEM.m_atcMark.m_Max = pIniFile->ReadInteger("Define", "ATCTime", pSound->FSKDEM.m_atcMark.m_Max);
	pSound->FSKDEM.m_atcSpace.m_Max = pSound->FSKDEM.m_atcMark.m_Max;

	pSound->FSKDEM.m_majority = pIniFile->ReadInteger("Define", "Majority", pSound->FSKDEM.m_majority);
	pSound->FSKDEM.m_ignoreFream = pIniFile->ReadInteger("Define", "IgnoreFreamError", pSound->FSKDEM.m_ignoreFream);

	pSound->FSKDEM.SetSQ(pIniFile->ReadInteger("Define", "SQ", pSound->FSKDEM.GetSQ()));
	pSound->FSKDEM.SetSQLevel(ReadDoubleIniFile(pIniFile, "Define", "SQLevel", pSound->FSKDEM.GetSQLevel()));

	sys.m_DefFix45 = pIniFile->ReadInteger("Define", "DefFix45", sys.m_DefFix45);
	sys.m_DefMarkFreq = ReadDoubleIniFile(pIniFile, "Define", "DefMarkFreq", sys.m_DefMarkFreq);
	sys.m_DefShift = ReadDoubleIniFile(pIniFile, "Define", "DefShift", sys.m_DefShift);
	sys.m_DefStopLen = pIniFile->ReadInteger("Define", "DefStopLen", sys.m_DefStopLen);
	pSound->FSKDEM.m_StopLen = sys.m_DefStopLen + 3;
	pSound->FSKMOD.m_StopLen = sys.m_DefStopLen + 3;

	pSound->FSKMOD.SetOutputGain(ReadDoubleIniFile(pIniFile, "Define", "OutputGain", pSound->FSKMOD.GetOutputGain()));

	sys.m_Rev = pIniFile->ReadInteger("Define", "Rev", sys.m_Rev);

	sys.m_echo = pIniFile->ReadInteger("Define", "TXLoop", sys.m_echo);
	pSound->FSKMOD.m_bpf = pIniFile->ReadInteger("Define", "TXBPF", pSound->FSKMOD.m_bpf);
	pSound->FSKMOD.m_bpftap = pIniFile->ReadInteger("Define", "TXBPFTAP", pSound->FSKMOD.m_bpftap);

	pSound->FSKMOD.m_lpf = pIniFile->ReadInteger("Define", "TXLPF", pSound->FSKMOD.m_lpf);
	pSound->FSKMOD.SetLPFFreq(ReadDoubleIniFile(pIniFile, "Define", "TXLPFFreq", pSound->FSKMOD.GetLPFFreq()));

	sys.m_LWait = pIniFile->ReadInteger("Define", "TXWaitType", sys.m_LWait);
	pSound->FSKMOD.m_CharWait = pIniFile->ReadInteger("Define", "TXCharWait", pSound->FSKMOD.m_CharWait);
	pSound->FSKMOD.m_DiddleWait = pIniFile->ReadInteger("Define", "TXDiddleWait", pSound->FSKMOD.m_DiddleWait);
	pSound->FSKMOD.m_CharWaitDiddle = pIniFile->ReadInteger("Define", "TXCharWaitDiddle", pSound->FSKMOD.m_CharWaitDiddle);
	pSound->FSKMOD.m_RandomDiddle = pIniFile->ReadInteger("Define", "TXRandomDiddle", pSound->FSKMOD.m_RandomDiddle);
	pSound->FSKMOD.m_WaitTimer = pIniFile->ReadInteger("Define", "TXWaitTimerDiddle", pSound->FSKMOD.m_WaitTimer);

	pSound->FSKDEM.SetSpaceFreq(ReadDoubleIniFile(pIniFile, "Define", "SpaceFreq", pSound->FSKDEM.GetSpaceFreq()));
	pSound->FSKDEM.SetMarkFreq(ReadDoubleIniFile(pIniFile, "Define", "MarkFreq", pSound->FSKDEM.GetMarkFreq()));
	pSound->FSKMOD.SetMarkFreq(pSound->FSKDEM.GetSetMarkFreq());
	pSound->FSKMOD.SetSpaceFreq(pSound->FSKDEM.GetSetSpaceFreq());
	m_RxMarkFreq = pSound->FSKDEM.GetMarkFreq();
	m_RxSpaceFreq = pSound->FSKDEM.GetSpaceFreq();
	pSound->m_bpf = pIniFile->ReadInteger("Define", "RXBPF", pSound->m_bpf);
	pSound->m_bpftap = pIniFile->ReadInteger("Define", "RXBPFTAP", pSound->m_bpftap);
	pSound->m_bpfafc = pIniFile->ReadInteger("Define", "RXBPFAFC", pSound->m_bpfafc);
	pSound->m_bpffw = ReadDoubleIniFile(pIniFile, "Define", "RXBPFFW", pSound->m_bpffw);

	pSound->m_lmsbpf = pIniFile->ReadInteger("Define", "RXlms", pSound->m_lmsbpf);
	pSound->m_lms.m_lmsDelay = pIniFile->ReadInteger("Define", "RXlmsDelay", pSound->m_lms.m_lmsDelay);
	pSound->m_lms.m_lmsMU2 = ReadDoubleIniFile(pIniFile, "Define", "RXlmsMU2", pSound->m_lms.m_lmsMU2);
	pSound->m_lms.m_lmsGM = ReadDoubleIniFile(pIniFile, "Define", "RXlmsGM", pSound->m_lms.m_lmsGM);
	pSound->m_lms.m_lmsAGC = pIniFile->ReadInteger("Define", "RXlmsAGC", pSound->m_lms.m_lmsAGC);
	pSound->m_lms.m_lmsInv = pIniFile->ReadInteger("Define", "RXlmsInv", pSound->m_lms.m_lmsInv);
	pSound->m_lms.m_Tap = pIniFile->ReadInteger("Define", "RXlmsTAP", pSound->m_lms.m_Tap);
	pSound->m_lms.m_NotchTap = pIniFile->ReadInteger("Define", "RXNotchTAP", pSound->m_lms.m_NotchTap);
	pSound->m_lms.m_bpf = pIniFile->ReadInteger("Define", "RXlmsBPF", pSound->m_lms.m_bpf);
	pSound->m_lms.m_Type = pIniFile->ReadInteger("Define", "RXlmsType", pSound->m_lms.m_Type);
	pSound->m_lms.m_lmsNotch = pIniFile->ReadInteger("Define", "RXlmsNotch", pSound->m_lms.m_lmsNotch);
	pSound->m_lms.m_lmsNotch2 = pIniFile->ReadInteger("Define", "RXlmsNotch2", pSound->m_lms.m_lmsNotch2);
	pSound->m_lms.m_twoNotch = pIniFile->ReadInteger("Define", "RXlmsTwoNotch", pSound->m_lms.m_twoNotch);
	if( !pSound->m_lmsbpf && pSound->m_lms.m_twoNotch ) pSound->m_lms.m_lmsNotch = pSound->m_lms.m_lmsNotch2 = 0;

	pSound->CalcBPF();

	int verfftdem = pIniFile->ReadInteger("Define", "VERFFTDEM", 0);
	pSound->FSKDEM.m_type = pIniFile->ReadInteger("Define", "DEMTYPE", pSound->FSKDEM.m_type);
	pSound->FSKDEM.SetBaudRate(ReadDoubleIniFile(pIniFile, "Define", "BaudRate", pSound->FSKDEM.GetBaudRate()));
	pSound->FSKDEM.m_lpf = pIniFile->ReadInteger("Define", "SmoozType", pSound->FSKDEM.m_lpf);
	pSound->FSKDEM.m_Phase.m_TONES = pIniFile->ReadInteger("Define", "FFTTones", pSound->FSKDEM.m_Phase.m_TONES);
	if( verfftdem < VERFFTDEM ){
		pSound->FSKDEM.m_Phase.m_TONES = 4;
    }
	pSound->FSKDEM.m_lpfOrder = pIniFile->ReadInteger("Define", "SmoozOrder", pSound->FSKDEM.m_lpfOrder);
	pSound->FSKDEM.SetLPFFreq(ReadDoubleIniFile(pIniFile, "Define", "SmoozIIR", pSound->FSKDEM.m_lpffreq));
	pSound->FSKDEM.SetSmoozFreq(ReadDoubleIniFile(pIniFile, "Define", "Smooz", pSound->FSKDEM.GetSmoozFreq()));
	sys.m_SoundFifoRX = pIniFile->ReadInteger("Define", "SoundRxFifo", sys.m_SoundFifoRX );
	sys.m_SoundFifoTX = pIniFile->ReadInteger("Define", "SoundTxFifo", sys.m_SoundFifoTX );
	if( sys.m_SoundFifoRX > WAVE_FIFO_MAX ) sys.m_SoundFifoRX = WAVE_FIFO_MAX;
	if( sys.m_SoundFifoTX > WAVE_FIFO_MAX ) sys.m_SoundFifoTX = WAVE_FIFO_MAX;
	sys.m_SoundPriority = pIniFile->ReadInteger("Define", "SoundPriority", sys.m_SoundPriority);
	sys.m_SoundDevice = pIniFile->ReadInteger("Define", "SoundDevice", sys.m_SoundDevice);
	sys.m_SoundOutDevice = pIniFile->ReadInteger("Define", "SoundOutDevice", sys.m_SoundDevice); //AA6YQ 1.66

	sys.m_HideFlexAudio = pIniFile->ReadInteger("Define", "HideFlexAudio", sys.m_HideFlexAudio); //AA6YQ 1.70E

	sys.m_SoundMMW = pIniFile->ReadString("Define", "SoundMMW", sys.m_SoundMMW);
	pSound->m_IDDevice = sys.m_SoundDevice;
	pSound->m_IDOutDevice=sys.m_SoundOutDevice;
	sys.m_SoundStereo = pIniFile->ReadInteger("Define", "SoundStereo", sys.m_SoundStereo);
	pSound->InitWFX();
	pSound->FSKDEM.SetFilterTap(pIniFile->ReadInteger("Define", "Tap", pSound->FSKDEM.GetFilterTap()));
	pSound->FSKDEM.m_iirfw = pIniFile->ReadInteger("Define", "IIRBW", pSound->FSKDEM.m_iirfw);
	pSound->FSKDEM.SetIIR(pSound->FSKDEM.m_iirfw);

	pSound->FSKDEM.m_pll.SetVcoGain(ReadDoubleIniFile(pIniFile, "Define", "pllVcoGain", pSound->FSKDEM.m_pll.m_vcogain));
	pSound->FSKDEM.m_pll.m_loopOrder = pIniFile->ReadInteger("Define", "pllLoopOrder", pSound->FSKDEM.m_pll.m_loopOrder);
	pSound->FSKDEM.m_pll.m_loopFC = ReadDoubleIniFile(pIniFile, "Define", "pllLoopFC", pSound->FSKDEM.m_pll.m_loopFC);
	pSound->FSKDEM.m_pll.m_outOrder = pIniFile->ReadInteger("Define", "pllOutOrder", pSound->FSKDEM.m_pll.m_outOrder);
	pSound->FSKDEM.m_pll.m_outFC = ReadDoubleIniFile(pIniFile, "Define", "pllOutFC", pSound->FSKDEM.m_pll.m_outFC);
	pSound->FSKDEM.m_pll.MakeLoopLPF();
	pSound->FSKDEM.m_pll.MakeOutLPF();

	pSound->FSKMOD.m_diddle = pIniFile->ReadInteger("Define", "Diddle", pSound->FSKMOD.m_diddle);

	sys.m_TxPort = pIniFile->ReadInteger("Define", "TxPort", sys.m_TxPort);
	sys.m_TxdJob = pIniFile->ReadInteger("Define", "TxdJob", sys.m_TxdJob);
	sys.m_TxFixShift = pIniFile->ReadInteger("Define", "TxFixShift", sys.m_TxFixShift);
	sys.m_TxRxName = pIniFile->ReadString("Define", "PTT", sys.m_TxRxName);
	sys.m_TxRxInv = pIniFile->ReadInteger("Define", "InvPTT", sys.m_TxRxInv);
	sys.m_txuos = pIniFile->ReadInteger("Define", "TXUOS", sys.m_txuos);
	sys.m_MacroImm = pIniFile->ReadInteger("Define", "MacroImm", sys.m_MacroImm);
	sys.m_CharLenFile = pIniFile->ReadInteger("Define", "CharLenFile", sys.m_CharLenFile);
	sys.m_DisTX = pIniFile->ReadInteger("Define", "DisTX", sys.m_DisTX);

	sys.m_Call = pIniFile->ReadString("Define", "Call", sys.m_Call);

	sys.m_log = pIniFile->ReadInteger("Log", "Switch", sys.m_log);
	sys.m_logTimeStamp = pIniFile->ReadInteger("Log", "TimeStamp", sys.m_logTimeStamp);
	sys.m_TimeStampUTC = pIniFile->ReadInteger("Log", "TimeStampUTC", sys.m_TimeStampUTC);
	sys.m_AutoTimeOffset = pIniFile->ReadInteger("Log", "AutoUTCOffset", sys.m_AutoTimeOffset);
	sys.m_TimeOffset = pIniFile->ReadInteger("Log", "UTCOffset", sys.m_TimeOffset);
	sys.m_TimeOffsetMin = pIniFile->ReadInteger("Log", "UTCOffsetMin", sys.m_TimeOffsetMin);
	sys.m_LogLink = pIniFile->ReadInteger("Log", "LogLink", sys.m_LogLink);
	LogLink.LoadMMLink(pIniFile);
	LogLink.SetPolling(pIniFile->ReadInteger("Log", "LinkPoll", 0));
	LogLink.SetPTTEnabled(pIniFile->ReadInteger("Log", "LinkPTT", FALSE));
	as = pIniFile->ReadString("Log", "PathName", LogDir);

	strcpy(LogDir, as.c_str());
	as = pIniFile->ReadString("Dir", "OutFile", OutFileDir);
	strcpy(OutFileDir, as.c_str());

	sys.m_ShowTimeStamp = pIniFile->ReadInteger("Log", "ShowTimeStamp", sys.m_ShowTimeStamp);

	Log.m_FileName = pIniFile->ReadString("LogFile", "Name", Log.m_FileName);
	Log.MakeName(Log.m_FileName.c_str());

	sys.m_Palette = pIniFile->ReadInteger("WaterFall", "Palette", sys.m_Palette);


	PanelIn->Height = pIniFile->ReadInteger("Input", "Height", PanelIn->Height);

	sys.m_ColorRXBack = (TColor)pIniFile->ReadInteger("Color", "RXBack", sys.m_ColorRXBack);
	sys.m_ColorRX = (TColor)pIniFile->ReadInteger("Color", "RXChar", sys.m_ColorRX);
	sys.m_ColorRXTX = (TColor)pIniFile->ReadInteger("Color", "RXTXChar", sys.m_ColorRX);
	sys.m_ColorINBack = (TColor)pIniFile->ReadInteger("Color", "INBack", sys.m_ColorINBack);
	sys.m_ColorIN = (TColor)pIniFile->ReadInteger("Color", "INChar", sys.m_ColorIN);
	sys.m_ColorINTX = (TColor)pIniFile->ReadInteger("Color", "INTXChar", sys.m_ColorINTX);

	sys.m_ColorLow = (TColor)pIniFile->ReadInteger("Color", "WaterLow", sys.m_ColorLow);
	sys.m_ColorHigh = (TColor)pIniFile->ReadInteger("Color", "WaterHigh", sys.m_ColorHigh);

	sys.m_ColorXY = (TColor)pIniFile->ReadInteger("Color", "XYScope", sys.m_ColorXY);


	int i;
	for(i = 0; i < kkEOF; i++ ){
		sprintf(bf, "S%d", i+1);
		sys.m_SysKey[i] = (WORD)pIniFile->ReadInteger("SysKey", bf, sys.m_SysKey[i]);
	}
	for(i = 0; i < 4; i++ ){
		sprintf(bf, "M%d", i+1);
		sys.m_InBtnCol[i] = (TColor)pIniFile->ReadInteger("InBtnCol", bf, sys.m_InBtnCol[i]);
		sys.m_InBtnKey[i] = (WORD)pIniFile->ReadInteger("InBtnKey", bf, sys.m_InBtnKey[i]);
		sys.m_InBtnName[i] = pIniFile->ReadString("InBtnName", bf, sys.m_InBtnName[i]);
		CrLf2Yen(ws, sys.m_InBtn[i]);
		as = pIniFile->ReadString("InBtn", bf, ws);
		Yen2CrLf(sys.m_InBtn[i], as);
	}

	for( i = 0; i < 16; i++ ){
		sprintf(bf, "M%d", i+1);
		sys.m_UserTimer[i] = pIniFile->ReadInteger("MacroTimer", bf, sys.m_UserTimer[i]);
		sys.m_UserCol[i] = (TColor)pIniFile->ReadInteger("MacroCol", bf, sys.m_UserCol[i]);
		sys.m_UserKey[i] = (WORD)pIniFile->ReadInteger("MacroKey", bf, sys.m_UserKey[i]);
		sys.m_UserEditKey[i] = (WORD)pIniFile->ReadInteger("MacroEditKey", bf, sys.m_UserEditKey[i]);
		sys.m_UserName[i] = pIniFile->ReadString("MacroName", bf, sys.m_UserName[i]);
		CrLf2Yen(ws, sys.m_User[i]);
		as = pIniFile->ReadString("Macro", bf, ws);
		Yen2CrLf(sys.m_User[i], as);
	}
	for( i = 0; i < MSGLISTMAX; i++ ){
		sprintf(bf, "M%d", i+1);
		sys.m_MsgKey[i] = (WORD)pIniFile->ReadInteger("MsgKey", bf, sys.m_MsgKey[i]);
		sys.m_MsgName[i] = pIniFile->ReadString("MsgName", bf, sys.m_MsgName[i]);
		if( !sys.m_MsgName[i].IsEmpty() ){
			CrLf2Yen(ws, sys.m_MsgList[i]);
			as = pIniFile->ReadString("MsgList", bf, ws);
			if( as.IsEmpty() ) break;
			Yen2CrLf(sys.m_MsgList[i], as);
		}
	}
	for( ; i < MSGLISTMAX; i++ ){
		sys.m_MsgKey[i] = 0;
		sys.m_MsgName[i] = "";
	}

	as = pIniFile->ReadString("Dir", "ExtLog", ExtLogDir);
	strcpy(ExtLogDir, as.c_str());
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "WDEF%d", i+1);
		LogText.m_tConv[i].Key = pIniFile->ReadString("ExtConv", bf, LogText.m_tConv[i].Key);
		sprintf(bf, "WLEN%d", i+1);
		LogText.m_tConv[i].w = pIniFile->ReadInteger("ExtConv", bf, LogText.m_tConv[i].w);
	}
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "RDEF%d", i+1);
		LogText.m_rConv[i].Key = pIniFile->ReadString("ExtConv", bf, LogText.m_rConv[i].Key);
		sprintf(bf, "RLEN%d", i+1);
		LogText.m_rConv[i].w = pIniFile->ReadInteger("ExtConv", bf, LogText.m_rConv[i].w);
	}
	as = pIniFile->ReadString("Dir", "RecSound", RecDir);
	strcpy(RecDir, as.c_str());

	for( i = 0; i < RECMENUMAX; i++ ){
		sprintf(bf, "File%d", i + 1);
		as = pIniFile->ReadString("Recent File", bf, "");
		RecentMenu.SetItemText(i, as.c_str());
	}
	Log.ReadIniFile("LogSet", pIniFile);

	KExtCmd1->Caption = pIniFile->ReadString("Program", "Name1", KExtCmd1->Caption);
	sys.m_ExtCmd[0] = pIniFile->ReadString("Program", "Command1", sys.m_ExtCmd[0]);
	KExtCmd2->Caption = pIniFile->ReadString("Program", "Name2", KExtCmd2->Caption);
	sys.m_ExtCmd[1] = pIniFile->ReadString("Program", "Command2", sys.m_ExtCmd[1]);
	KExtCmd3->Caption = pIniFile->ReadString("Program", "Name3", KExtCmd3->Caption);
	sys.m_ExtCmd[2] = pIniFile->ReadString("Program", "Command3", sys.m_ExtCmd[2]);
	KExtCmd4->Caption = pIniFile->ReadString("Program", "Name4", KExtCmd4->Caption);
	sys.m_ExtCmd[3] = pIniFile->ReadString("Program", "Command4", sys.m_ExtCmd[3]);
	UpdateExtProgram();
	LoadTNCSetup(pIniFile);
	LoadRADIOSetup(pIniFile);

	if( !Remote ){
		for( i = 0; i < CALLLISTMAX; i++ ){
			sprintf(bf, "Item%d", i+1);
			as = pIniFile->ReadString("Calls", bf, "");
			if( as.IsEmpty() ) break;
			LPSTR p;
			for( p = as.c_str(); *p; p++ ){
				if( *p == '_' ) *p = '\t';
			}
			TMenuItem *pm = new TMenuItem(this);
			pm->Caption = as.c_str();
			pm->RadioItem = FALSE;
			pm->OnClick = KCClick;
			pm->Checked = FALSE;
			pm->Enabled = TRUE;
			PopupC->Items->Add(pm);
		}
	}

//AA6YQ option, Added by JE3HHT on Sep.2010
	CAA6YQ *pAA6YQ = &pSound->FSKDEM.m_AA6YQ;
	pAA6YQ->m_fEnabled = pIniFile->ReadInteger("AA6YQ", "Enabled", pAA6YQ->m_fEnabled);
	pAA6YQ->m_bpfTaps = pIniFile->ReadInteger("AA6YQ", "TapsBPF", pAA6YQ->m_bpfTaps);
	pAA6YQ->m_bpfFW = ReadDoubleIniFile(pIniFile, "AA6YQ", "WidthBPF", pAA6YQ->m_bpfFW);
	pAA6YQ->m_befTaps = pIniFile->ReadInteger("AA6YQ", "TapsBEF", pAA6YQ->m_befTaps);
	pAA6YQ->m_befFW = ReadDoubleIniFile(pIniFile, "AA6YQ", "WidthBEF", pAA6YQ->m_befFW);
	pAA6YQ->m_afcERR = ReadDoubleIniFile(pIniFile, "AA6YQ", "afcERR", pAA6YQ->m_afcERR);
	int verAA6YQ = pIniFile->ReadInteger("AA6YQ", "Version", 0);
	if( verAA6YQ < VERAA6YQ ){
		pAA6YQ->m_bpfTaps = 512;
		pAA6YQ->m_befTaps = 256;
    }
	if( pAA6YQ->m_fEnabled ) pAA6YQ->Create();
	delete pIniFile;
}

//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ÷ÇÃèëÇ´çûÇ›
void __fastcall TMmttyWd::WriteRegister(void)
{
	char	bf[256];
	sprintf(bf, "%sMmtty.ini", BgnDir);

	try{

	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	WriteDoubleIniFile(pIniFile, "SoundCard", "SampFreq", sys.m_SampFreq);
	WriteDoubleIniFile(pIniFile, "SoundCard", "TxOffset", sys.m_TxOffset);

	AnsiString as;
	GetComboBox(as, MarkFreq);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "Mark", as);
	GetComboBox(as, ShiftFreq);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "Shift", as);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "Baud", m_asBaud);
	GetComboBox(as, HisRST);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "RST", as);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "IIRBW", m_asIIRBW);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "VCO", m_asVCOGain);
	if( !as.IsEmpty() ) pIniFile->WriteString("ComboList", "LoopFC", m_asLoopFC);

	pIniFile->WriteString("Grid", "Log", sys.m_LogGridWidths);
	pIniFile->WriteString("Grid", "QSO", sys.m_QSOGridWidths);

	pIniFile->WriteString("Help", "HTML", sys.m_HTMLHelp);
	pIniFile->WriteString("Help", "MMTTY", sys.m_Help);
	pIniFile->WriteString("Help", "MMTTYLOG", sys.m_HelpLog);
	pIniFile->WriteString("Help", "DIGITAL", sys.m_HelpDigital);
	pIniFile->WriteString("Help", "FontName", sys.m_HelpFontName);
	pIniFile->WriteInteger("Help", "FontCharset", sys.m_HelpFontCharset);
	pIniFile->WriteInteger("Help", "FontSize", sys.m_HelpFontSize);
	pIniFile->WriteInteger("Help", "UseNotePad", sys.m_HelpNotePad);

	if( Remote ){
		if( !(Remote & REMSHOWOFF) ){
			pIniFile->WriteInteger("Window", "WindowTop", Top);
			pIniFile->WriteInteger("Window", "WindowLeft", Left);
			if( Remote & REMCTRL ){
				pIniFile->WriteInteger("Window", "WindowWidth", Width);
			}
			else {
				pIniFile->WriteInteger("Window", "WindowWidthS", Width);
			}
			if( Remote & REMMENU ){
				pIniFile->WriteInteger("Window", "WindowHeightRM", Height);
			}
			else {
				pIniFile->WriteInteger("Window", "WindowHeightR", Height);
			}
		}
	}
	else {
		pIniFile->WriteInteger("Window", "WindowState", WindowState);
		pIniFile->WriteInteger("Window", "ShowMacro", KMac->Checked);
		pIniFile->WriteInteger("Window", "MemoryWindow", sys.m_MemWindow);
		if( (WindowState == wsNormal) && sys.m_MemWindow ){
			pIniFile->WriteInteger("Window", "WindowTopN", Top);
			pIniFile->WriteInteger("Window", "WindowLeftN", Left);
			pIniFile->WriteInteger("Window", "WindowWidthN", Width);
			if( !sys.m_DisWindow ){
				pIniFile->WriteInteger("Window", "WindowHeightN", Height);
			}
		}
	}

	pIniFile->WriteInteger("Window", "SetupOnTop", sys.m_SetupOnTop);  //1.70K

	pIniFile->WriteInteger("Window", "Disable", sys.m_DisWindow);
	pIniFile->WriteInteger("Window", "StayOnTop", sys.m_StayOnTop);

	pIniFile->WriteString("WindowFont", "Name", sys.m_WinFontName);
	pIniFile->WriteInteger("WindowFont", "Charset", sys.m_WinFontCharset);
	pIniFile->WriteInteger("WindowFont", "Style", sys.m_WinFontStyle);
	pIniFile->WriteInteger("WindowFont", "Adjust", sys.m_FontAdjSize);
	pIniFile->WriteString("ButtonFont", "Name", sys.m_BtnFontName);
	pIniFile->WriteInteger("ButtonFont", "Charset", sys.m_BtnFontCharset);
	pIniFile->WriteInteger("ButtonFont", "Style", sys.m_BtnFontStyle);
	pIniFile->WriteInteger("ButtonFont", "Adjust", sys.m_BtnFontAdjSize);

	pIniFile->WriteString("Font", "Name", sys.m_FontName);
	pIniFile->WriteInteger("Font", "Size", sys.m_FontSize);
	pIniFile->WriteInteger("Font", "AdjX", sys.m_FontAdjX);
	pIniFile->WriteInteger("Font", "AdjY", sys.m_FontAdjY);
	pIniFile->WriteInteger("Font", "Charset", sys.m_FontCharset);
	pIniFile->WriteInteger("Font", "Zero", sys.m_FontZero);
	pIniFile->WriteInteger("Font", "Style", sys.m_FontStyle);

	pIniFile->WriteInteger("Define", "ControlPanel", KPanel->Checked);
	pIniFile->WriteInteger("Button", "Hint", KHint->Checked);
	pIniFile->WriteInteger("Define", "FFT", pSound->m_FFTSW);
	pIniFile->WriteInteger("Define", "FFTFW", pSound->m_FFTFW);
	pIniFile->WriteInteger("Define", "FFTGain", sys.m_FFTGain);
	pIniFile->WriteInteger("Define", "FFTResp", sys.m_FFTResp);
	pIniFile->WriteInteger("Define", "UOS", rtty.m_uos);
	pIniFile->WriteInteger("Define", "CodeSet", sys.m_CodeSet);
	pIniFile->WriteInteger("Define", "XYScope", pSound->FSKDEM.m_XYScope);
	pIniFile->WriteInteger("Define", "XYScopeSize", m_XYSize);
	pIniFile->WriteInteger("Define", "XYScopeReverse", sys.m_XYInv);
	pIniFile->WriteInteger("Define", "XYQuality", m_XYQuality);
	pIniFile->WriteInteger("Define", "PanelSize", m_PanelSize);

	pIniFile->WriteInteger("Define", "AFC", sys.m_AFC);
	pIniFile->WriteInteger("Define", "AFCFixShift", sys.m_FixShift);
	pIniFile->WriteInteger("Define", "AFCSQ", sys.m_AFCSQ);
	WriteDoubleIniFile(pIniFile, "Define", "AFCTime", sys.m_AFCTime);
	WriteDoubleIniFile(pIniFile, "Define", "AFCSweep", sys.m_AFCSweep);

	pIniFile->WriteInteger("Define", "AutoCR", KENT->Checked);
	pIniFile->WriteInteger("Define", "WardWarp", KWP->Checked);
	pIniFile->WriteInteger("Define", "SendWay", m_SendWay);
	pIniFile->WriteInteger("Define", "Running", m_Running);
	pIniFile->WriteInteger("Define", "PttTimer", m_PttTimer);
	pIniFile->WriteInteger("Define", "TxNet", sys.m_TxNet);
	pIniFile->WriteInteger("Define", "TxDisRev", sys.m_TxDisRev);
	WriteDoubleIniFile(pIniFile, "Define", "LimitDxGain", sys.m_LimitGain);
	pIniFile->WriteInteger("Define", "LimitOverSampling", pSound->FSKDEM.m_LimitOverSampling);
	pIniFile->WriteInteger("Define", "LimitAGC", pSound->FSKDEM.m_LimitAGC);
	pIniFile->WriteInteger("Define", "ATC", pSound->FSKDEM.m_atc);
	pIniFile->WriteInteger("Define", "ATCTime", pSound->FSKDEM.m_atcMark.m_Max);

	pIniFile->WriteInteger("Define", "Majority", pSound->FSKDEM.m_majority);
	pIniFile->WriteInteger("Define", "IgnoreFreamError", pSound->FSKDEM.m_ignoreFream);

	pIniFile->WriteInteger("Define", "SQ", pSound->FSKDEM.GetSQ());
	WriteDoubleIniFile(pIniFile, "Define", "SQLevel", pSound->FSKDEM.GetSQLevel());


	pIniFile->WriteInteger("Define", "DefFix45", sys.m_DefFix45);
	WriteDoubleIniFile(pIniFile, "Define", "DefMarkFreq", sys.m_DefMarkFreq);
	WriteDoubleIniFile(pIniFile, "Define", "DefShift", sys.m_DefShift);
	pIniFile->WriteInteger("Define", "DefStopLen", sys.m_DefStopLen);

	WriteDoubleIniFile(pIniFile, "Define", "OutputGain", pSound->FSKMOD.GetOutputGain());

	pIniFile->WriteInteger("Define", "Rev", sys.m_Rev);


	WriteDoubleIniFile(pIniFile, "Define", "SpaceFreq", pSound->FSKDEM.GetSpaceFreq());
	WriteDoubleIniFile(pIniFile, "Define", "MarkFreq", pSound->FSKDEM.GetMarkFreq());

	pIniFile->WriteInteger("Define", "VERFFTDEM", VERFFTDEM);
	pIniFile->WriteInteger("Define", "DEMTYPE", pSound->FSKDEM.m_type);
	WriteDoubleIniFile(pIniFile, "Define", "BaudRate", pSound->FSKDEM.GetBaudRate());
	pIniFile->WriteInteger("Define", "SmoozType", pSound->FSKDEM.m_lpf);
	pIniFile->WriteInteger("Define", "SmoozOrder", pSound->FSKDEM.m_lpfOrder);
	WriteDoubleIniFile(pIniFile, "Define", "SmoozIIR", pSound->FSKDEM.m_lpffreq);
	WriteDoubleIniFile(pIniFile, "Define", "Smooz", pSound->FSKDEM.GetSmoozFreq());
	pIniFile->WriteInteger("Define", "FFTTones", pSound->FSKDEM.m_Phase.m_TONES);
	pIniFile->WriteInteger("Define", "SoundRxFifo", sys.m_SoundFifoRX );
	pIniFile->WriteInteger("Define", "SoundTxFifo", sys.m_SoundFifoTX );
	pIniFile->WriteInteger("Define", "SoundPriority", sys.m_SoundPriority);
	pIniFile->WriteInteger("Define", "SoundDevice", sys.m_SoundDevice);
	pIniFile->WriteInteger("Define", "SoundOutDevice", sys.m_SoundOutDevice);   //AA6YQ 1.66
	pIniFile->WriteInteger("Define", "SoundStereo", sys.m_SoundStereo);
	pIniFile->WriteString("Define", "SoundMMW", sys.m_SoundMMW);

	pIniFile->WriteInteger("Define", "HideFlexAudio", sys.m_HideFlexAudio);    //AA6YQ 1.70E

	pIniFile->WriteInteger("Define", "Tap", pSound->FSKDEM.GetFilterTap());
	pIniFile->WriteInteger("Define", "IIRBW", pSound->FSKDEM.m_iirfw);

	pIniFile->WriteInteger("Define", "Diddle", pSound->FSKMOD.m_diddle);

	pIniFile->WriteInteger("Define", "TxPort", sys.m_TxPort);
	pIniFile->WriteInteger("Define", "TxdJob", sys.m_TxdJob);
	pIniFile->WriteInteger("Define", "TxFixShift", sys.m_TxFixShift);
	pIniFile->WriteString("Define", "PTT", sys.m_TxRxName);
	pIniFile->WriteInteger("Define", "InvPTT", sys.m_TxRxInv);
	pIniFile->WriteInteger("Define", "TXUOS", sys.m_txuos);
	pIniFile->WriteInteger("Define", "MacroImm", sys.m_MacroImm);
	pIniFile->WriteInteger("Define", "CharLenFile", sys.m_CharLenFile);
	pIniFile->WriteInteger("Define", "DisTX", (sys.m_DisTX & 2) ? sys.m_DisTX : 0);
	pIniFile->WriteInteger("Define", "TXLoop", sys.m_echo);
	pIniFile->WriteInteger("Define", "TXBPF", pSound->FSKMOD.m_bpf);
	pIniFile->WriteInteger("Define", "TXBPFTAP", pSound->FSKMOD.m_bpftap);

	pIniFile->WriteInteger("Define", "TXLPF", pSound->FSKMOD.m_lpf);
	WriteDoubleIniFile(pIniFile, "Define", "TXLPFFreq", pSound->FSKMOD.GetLPFFreq());

	pIniFile->WriteInteger("Define", "TXWaitType", sys.m_LWait);
	pIniFile->WriteInteger("Define", "TXCharWait", pSound->FSKMOD.m_CharWait);
	pIniFile->WriteInteger("Define", "TXDiddleWait", pSound->FSKMOD.m_DiddleWait);
	pIniFile->WriteInteger("Define", "TXCharWaitDiddle", pSound->FSKMOD.m_CharWaitDiddle);
	pIniFile->WriteInteger("Define", "TXRandomDiddle", pSound->FSKMOD.m_RandomDiddle);
	pIniFile->WriteInteger("Define", "TXWaitTimerDiddle", pSound->FSKMOD.m_WaitTimer);

	pIniFile->WriteInteger("Define", "RXBPF", pSound->m_bpf);
	pIniFile->WriteInteger("Define", "RXBPFTAP", pSound->m_bpftap);
	pIniFile->WriteInteger("Define", "RXBPFAFC", pSound->m_bpfafc);
	WriteDoubleIniFile(pIniFile, "Define", "RXBPFFW", pSound->m_bpffw);

	pIniFile->WriteInteger("Define", "RXlms", pSound->m_lmsbpf);
	pIniFile->WriteInteger("Define", "RXlmsDelay", pSound->m_lms.m_lmsDelay);
	WriteDoubleIniFile(pIniFile, "Define", "RXlmsMU2", pSound->m_lms.m_lmsMU2);
	WriteDoubleIniFile(pIniFile, "Define", "RXlmsGM", pSound->m_lms.m_lmsGM);
	pIniFile->WriteInteger("Define", "RXlmsInv", pSound->m_lms.m_lmsInv);
	pIniFile->WriteInteger("Define", "RXlmsAGC", pSound->m_lms.m_lmsAGC);
	pIniFile->WriteInteger("Define", "RXlmsTAP", pSound->m_lms.m_Tap);
	pIniFile->WriteInteger("Define", "RXNotchTAP", pSound->m_lms.m_NotchTap);
	pIniFile->WriteInteger("Define", "RXlmsBPF", pSound->m_lms.m_bpf);
	pIniFile->WriteInteger("Define", "RXlmsType", pSound->m_lms.m_Type);
	pIniFile->WriteInteger("Define", "RXlmsNotch", pSound->m_lms.m_lmsNotch);
	pIniFile->WriteInteger("Define", "RXlmsNotch2", pSound->m_lms.m_lmsNotch2);
	pIniFile->WriteInteger("Define", "RXlmsTwoNotch", pSound->m_lms.m_twoNotch);

	WriteDoubleIniFile(pIniFile, "Define", "pllVcoGain", pSound->FSKDEM.m_pll.m_vcogain);
	pIniFile->WriteInteger("Define", "pllLoopOrder", pSound->FSKDEM.m_pll.m_loopOrder);
	WriteDoubleIniFile(pIniFile, "Define", "pllLoopFC", pSound->FSKDEM.m_pll.m_loopFC);
	pIniFile->WriteInteger("Define", "pllOutOrder", pSound->FSKDEM.m_pll.m_outOrder);
	WriteDoubleIniFile(pIniFile, "Define", "pllOutFC", pSound->FSKDEM.m_pll.m_outFC);

	pIniFile->WriteString("Define", "Call", sys.m_Call);

	pIniFile->WriteInteger("Log", "Switch", sys.m_log);
	pIniFile->WriteInteger("Log", "TimeStamp", sys.m_logTimeStamp);
	pIniFile->WriteInteger("Log", "TimeStampUTC", sys.m_TimeStampUTC);
	pIniFile->WriteInteger("Log", "AutoUTCOffset", sys.m_AutoTimeOffset);
	pIniFile->WriteInteger("Log", "UTCOffset", sys.m_TimeOffset);
	pIniFile->WriteInteger("Log", "UTCOffsetMin", sys.m_TimeOffsetMin);
	pIniFile->WriteInteger("Log", "LogLink", sys.m_LogLink);
	pIniFile->WriteInteger("Log", "LinkPoll", LogLink.IsPolling());
	pIniFile->WriteInteger("Log", "LinkPTT", LogLink.GetPTTEnabled());
	LogLink.SaveMMLink(pIniFile);
	pIniFile->WriteString("Log", "PathName", LogDir);

	pIniFile->WriteString("Dir", "OutFile", OutFileDir);

	pIniFile->WriteInteger("Log", "ShowTimeStamp", sys.m_ShowTimeStamp);

	pIniFile->WriteString("LogFile", "Name", Log.m_FileName);


	pIniFile->WriteInteger("WaterFall", "Palette", sys.m_Palette);

	pIniFile->WriteInteger("Input", "Height", PanelIn->Height);

	pIniFile->WriteInteger("Color", "RXBack", sys.m_ColorRXBack);
	pIniFile->WriteInteger("Color", "RXChar", sys.m_ColorRX);
	pIniFile->WriteInteger("Color", "RXTXChar", sys.m_ColorRXTX);
	pIniFile->WriteInteger("Color", "INBack", sys.m_ColorINBack);
	pIniFile->WriteInteger("Color", "INChar", sys.m_ColorIN);
	pIniFile->WriteInteger("Color", "INTXChar", sys.m_ColorINTX);

	pIniFile->WriteInteger("Color", "WaterLow", sys.m_ColorLow);
	pIniFile->WriteInteger("Color", "WaterHigh", sys.m_ColorHigh);
	pIniFile->WriteInteger("Color", "XYScope", sys.m_ColorXY);

	AnsiString	ws;

	int i;
	for(i = 0; i < kkEOF; i++ ){
		sprintf(bf, "S%d", i+1);
		pIniFile->WriteInteger("SysKey", bf, sys.m_SysKey[i]);
	}
	for(i = 0; i < 4; i++ ){
		sprintf(bf, "M%d", i+1);
		pIniFile->WriteInteger("InBtnCol", bf, sys.m_InBtnCol[i]);
		pIniFile->WriteInteger("InBtnKey", bf, sys.m_InBtnKey[i]);
		pIniFile->WriteString("InBtnName", bf, sys.m_InBtnName[i]);
		CrLf2Yen(ws, sys.m_InBtn[i]);
		pIniFile->WriteString("InBtn", bf, ws);
	}

	for( i = 0; i < 16; i++ ){
		sprintf(bf, "M%d", i+1);
		pIniFile->WriteInteger("MacroTimer", bf, sys.m_UserTimer[i]);
		pIniFile->WriteInteger("MacroCol", bf, sys.m_UserCol[i]);
		pIniFile->WriteInteger("MacroKey", bf, sys.m_UserKey[i]);
		pIniFile->WriteInteger("MacroEditKey", bf, sys.m_UserEditKey[i]);
		pIniFile->WriteString("MacroName", bf, sys.m_UserName[i]);
		CrLf2Yen(ws, sys.m_User[i]);
		pIniFile->WriteString("Macro", bf, ws);
	}

	for( i = 0; i < MSGLISTMAX; i++ ){
		sprintf(bf, "M%d", i+1);
		pIniFile->WriteInteger("MsgKey", bf, sys.m_MsgKey[i]);
		if( !sys.m_MsgName[i].IsEmpty() ){
			pIniFile->WriteString("MsgName", bf, sys.m_MsgName[i]);
			CrLf2Yen(ws, sys.m_MsgList[i]);
			pIniFile->WriteString("MsgList", bf, ws);
		}
		else {		// è¡ãéóp
			pIniFile->WriteString("MsgList", bf, "");
			pIniFile->WriteString("MsgName", bf, "");
			break;
		}
	}
	pIniFile->WriteString("Dir", "ExtLog", ExtLogDir);
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "WDEF%d", i+1);
		pIniFile->WriteString("ExtConv", bf, LogText.m_tConv[i].Key);
		sprintf(bf, "WLEN%d", i+1);
		pIniFile->WriteInteger("ExtConv", bf, LogText.m_tConv[i].w);
	}
	for( i = 0; i < TEXTCONVMAX; i++ ){
		sprintf(bf, "RDEF%d", i+1);
		pIniFile->WriteString("ExtConv", bf, LogText.m_rConv[i].Key);
		sprintf(bf, "RLEN%d", i+1);
		pIniFile->WriteInteger("ExtConv", bf, LogText.m_rConv[i].w);
	}
	pIniFile->WriteString("Dir", "RecSound", RecDir);
	for( i = 0; i < RecentMenu.GetCount(); i++ ){
		sprintf(bf, "File%d", i + 1);
		pIniFile->WriteString("Recent File", bf, RecentMenu.GetItemText(i));
	}
	Log.WriteIniFile("LogSet", pIniFile);

	pIniFile->WriteString("Program", "Name1", KExtCmd1->Caption);
	pIniFile->WriteString("Program", "Command1", sys.m_ExtCmd[0]);
	pIniFile->WriteString("Program", "Name2", KExtCmd2->Caption);
	pIniFile->WriteString("Program", "Command2", sys.m_ExtCmd[1]);
	pIniFile->WriteString("Program", "Name3", KExtCmd3->Caption);
	pIniFile->WriteString("Program", "Command3", sys.m_ExtCmd[2]);
	pIniFile->WriteString("Program", "Name4", KExtCmd4->Caption);
	pIniFile->WriteString("Program", "Command4", sys.m_ExtCmd[3]);

	SaveTNCSetup(pIniFile);
	SaveRADIOSetup(pIniFile);

	if( !Remote ){
		for( i = 0; i < PopupC->Items->Count; i++ ){
			sprintf(bf, "Item%d", i+1);
			char bbf[64];
			StrCopy(bbf, AnsiString(PopupC->Items->Items[i]->Caption).c_str(), 63);	//JA7UDE 0427
			LPSTR p;
			for( p = bbf; *p; p++ ){
				if( *p == '\t' ) *p = '_';
			}
			pIniFile->WriteString("Calls", bf, bbf);
		}
	}

//AA6YQ option, Added by JE3HHT on Sep.2010
	CAA6YQ *pAA6YQ = &pSound->FSKDEM.m_AA6YQ;
	pIniFile->WriteInteger("AA6YQ", "Version", VERAA6YQ);
	pIniFile->WriteInteger("AA6YQ", "Enabled", pAA6YQ->m_fEnabled);
	pIniFile->WriteInteger("AA6YQ", "TapsBPF", pAA6YQ->m_bpfTaps);
	WriteDoubleIniFile(pIniFile, "AA6YQ", "WidthBPF", pAA6YQ->m_bpfFW);
	pIniFile->WriteInteger("AA6YQ", "TapsBEF", pAA6YQ->m_befTaps);
	WriteDoubleIniFile(pIniFile, "AA6YQ", "WidthBEF", pAA6YQ->m_befFW);
	WriteDoubleIniFile(pIniFile, "AA6YQ", "afcERR", pAA6YQ->m_afcERR);

	pIniFile->UpdateFile();
	delete pIniFile;

	}
	catch(...){
		ErrorMB((Font->Charset != SHIFTJIS_CHARSET)?"Cannot update MMTTY.INI":"MMTTY.INIÇçXêVÇ≈Ç´Ç‹ÇπÇÒ.");
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmttyWd::IsTxIdle(void)
{
	if( sys.m_TxPort ){
		if( !pSound->FSKMOD.IsIdle() ) return 0;
		if( pComm != NULL ){
			return pComm->m_idle;
		}
		return 1;
	}
	else {
		return pSound->FSKMOD.IsIdle();
	}
}
//---------------------------------------------------------------------------
int __fastcall TMmttyWd::GetTxBufCount(void)
{
	int r = pSound->FSKMOD.GetBufCount();
	if( sys.m_TxPort && (pComm != NULL) ){
		int rr = pComm->GetBufCount();
		if( rr > r ) r = rr;
	}
	return r;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RecvJob(void)
{
	char c;
	while(1){
		int d = pSound->FSKDEM.GetData();
		if( d < 0 ){
			break;
		}
		switch(pSound->FSKDEM.m_BitLen){
			case 7:
				d &= 0x007f;
			case 8:
				if( d || Remote & REMNOCNV ){
					if( d ) PrintText.PutChar(char(d));
					PutTNC(char(d));
				}
				break;
			case 6:
			default:
				if( d >= 0 ){
					if( Remote ){
						if( Remote & REMNOCNV ) PutTNC(BYTE(d));
					}
					else {
						if( TNC.Type == 2 ) PutTNC(BYTE(d));
					}
					c = rtty.ConvAscii(d);
					if( c ){
						PrintText.PutChar(c);
						if( Remote ){
							if( !(Remote & REMNOCNV) ) PutTNC(c);
						}
						else {
							if( TNC.Type != 2 ) PutTNC(c);
						}
					}
					else {
						if( SBFIG->Down != rtty.m_fig ){
							SBFIG->Down = rtty.m_fig;
							if( Remote ) PostApp(TXM_FIGEVENT, rtty.m_fig);
						}
					}
				}
				else {
					break;
				}
				break;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::TimerTimer(TObject *Sender)
{
	if( pSound == NULL ) return;

	if( DisPaint != TRUE ){
		if( KXYScope->Checked && (DrawXY() == TRUE) ){
			PBoxXYPaint(NULL);
		}
		if( Remote && (pMap != NULL) ){
			if( !(Remote & REMDISSHARED) ){
				pMap->flagLostSound = pSound->m_Err;
				pMap->flagOverflow = SBTX->Down ? 0 : pSound->FSKDEM.m_OverFlow;
				pMap->txBuffCount = (Remote & REMVIATX) ? FifoEdit.GetLen() : GetTxBufCount();
            }
			if( Remote & REMSHOWOFF ){
				if( pSound->m_Err ) pSound->m_Err--;
				pSound->FSKDEM.m_OverFlow = 0;
			}
		}
		if( pSound->DrawFFT(pBitmapFFTIN, 0, KXYScope->Checked ? PBoxXY->Width : 0) ){
			if( pTnc ){
				pTnc->NotifyFFT(pSound->fftIN.m_fft);
            }
			if( Remote && (pMap != NULL) ){
				if( !pMap->flagFFT ){
					SetRemoteFFT();
				}
			}
			PBoxFFTINPaint(NULL);
			if( pSound->m_FFTSW ){
				if( pSound->DrawFFTWater(pBitmapWater, 0, KXYScope->Checked ? PBoxXY->Width : 0) ){
					PBoxWaterPaint(NULL);
				}
				if( sys.m_AFC && (!SBTX->Down || (sys.m_echo == 2)) ){
					if( !MarkFreq->DroppedDown && !ShiftFreq->DroppedDown ){
						if( pSound->DoAFC() ){
							m_DisEvent++;
							MarkFreq->Text = pSound->FSKDEM.GetMarkFreq();
							ShiftFreq->Text = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
							if( pRadio != NULL ) pRadio->SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
							m_DisEvent--;
							RemoteFreq();
						}
					}
				}
			}
		}
		if( pSound->fftIN.m_FFTDIS ){
			m_DisEvent++;
			KFFT->Checked = FALSE;
			pSound->m_FFTSW = 0;
			pSound->fftIN.InitFFT();
			UpdateUI();
			m_DisEvent--;
		}
	}
	DrawSQ();
    PBoxSQPaint(NULL);
	RecvJob();
	if( p_SendFileDlg != NULL ){
		if( SBTX->Down ){
			int r = p_SendFileDlg->Timer(GetTxBufCount());
			if( r ){
				delete p_SendFileDlg;
				p_SendFileDlg = NULL;
				if( r == 2 ){
					pSound->FSKMOD.DeleteTXBuf();
					if( pComm != NULL ) pComm->InitTxBuf();
				}
			}
		}
		else {
			delete p_SendFileDlg;
			p_SendFileDlg = NULL;
		}
	}
	if( SBTX->Down ){
		if( IsTxIdle() ){
			if( m_Macro ){
				switch(m_MacroStat){
					case 1:			// éÛêMêÿÇËë÷Ç¶
						if( m_MacTimer && (m_MacRetry >= 1) && (m_MacRetry <= 16) ){
							if( sys.m_UserTimer[m_MacRetry-1] ){
								m_MacTimer = (sys.m_UserTimer[m_MacRetry-1] * 100) + GetTickCount();
							}
							else {
								ClearMacroTimer();
							}
						}
						m_MacroStat = 0;
						m_Macro = 0;
						ToRX(0);
						break;
					case 2:			// åJÇËï‘Çµ
						OutputStr(m_NowStr.c_str());
						break;
					default:
						m_MacroStat = 0;
						m_Macro = 0;
						break;
				}
			}
			if( m_PttTimer && m_PttTimerCount ){
				if( DWORD(m_PttTimerCount) <= ::GetTickCount() ){
					SBTX->Down = FALSE;
					SBTXClick(NULL);
					UpdatePttTimer();
				}
			}
		}
		else if( m_PttTimer ){
			UpdatePttTimer();
		}
		if( pComm != NULL ) pComm->Timer();
	}
	else if( m_MacTimer && m_MacRetry ){
		if( DWORD(m_MacTimer) <= GetTickCount() ){
			m_MacTimer = 0;
			if( m_MacRetry ){
				MacBtnExec(m_MacRetry-1);
			}
		}
	}
	else if( KTest->Checked ){
		if( pSound->FSKMOD.GetBufCount() < 3 ){
			const char TestData[]="\r\nRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRYRY\r\n"
								  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n"
								  "-?:$!&'().,;/\x22";

			OutputStr(TestData);
		}
	}
	if( SBTX->Down && (!m_Macro) && (!m_FifoOutTimer) ){
		while( pSound->FSKMOD.GetBufCount() <= 2 ){
			if( !m_ReqRecv ){
				if( m_SendWay == 1 ){
					if( !FifoEdit.IsSAS(0) ) break;
				}
				else if( m_SendWay == 2 ){
					if( !FifoEdit.IsSAS(1) ) break;
				}
			}
			char bf[3];
_try:;
			bf[1] = 0;
			bf[0] = FifoEdit.GetChar();
			if( bf[0] != -1 ){
				if( bf[0] && (bf[0] != '#') ){
					if( bf[0] == '%' ){
						bf[1] = FifoEdit.GetChar();
						bf[2] = 0;
						if( bf[1] == '{' ){
							char bbf[1024];
							bbf[0] = '%'; bbf[1] = '{';
							LPSTR t = &bbf[2];
							while(FifoEdit.GetLen()){
								*t = FifoEdit.GetChar();
								if( *t == '}' ){
									t++;
									break;
								}
								t++;
							}
							*t = 0;
							OutputStr(bbf);
						}
						else {
							OutputStr(bf);
						}
					}
					else if( bf[0] == '^' ){
						m_FifoOutTimer = KXYScope->Checked ? 10 : 5;
						break;
					}
					else {
						OutputStr(bf);
						if( bf[0] == 0x0d ){
							bf[0] = 0x0a;
							OutputStr(bf);
						}
						if( (bf[0] == '_')||(bf[0] =='~') ){
							if( pSound->FSKMOD.GetBufCount() <= 10 ) goto _try;
						}
					}
				}
			}
			else if( m_ReqRecv && IsTxIdle() ){
				if( (pSound->FSKMOD.m_AmpVal == 1) && (sys.m_TxPort != 2) ){
					pSound->FSKMOD.m_AmpVal = 0;
                }
                else {
					ToRX(0);
                }
				break;
            }
            else {
				break;
            }
			if( FifoEdit.GetLen() < 2 ) break;
		}
	}
	if( m_PaletteTimer ){				// 256êFÉrÉfÉIÇÃéû
		m_PaletteTimer--;
		if( !m_PaletteTimer ){
			PBoxWater->Invalidate();
			PBoxXY->Invalidate();
			PBoxRx->Invalidate();
			PBoxIn->Invalidate();
		}
	}
	if( m_FifoOutTimer ) m_FifoOutTimer--;
	if( Remote ){
		if( m_RemoteTimer ){
			m_RemoteTimer--;
			if( !m_RemoteTimer ){
				Close();
			}
			else if( !(m_RemoteTimer % 5) ){
				FirstFileMapping();
				::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_THREAD, ::GetCurrentThreadId());
				::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_HANDLE, (DWORD)Handle);
			}
		}
		if( m_pRemOpenName != NULL ){
			HWND hwnd = ::FindWindow(m_OptionDialog ? NULL : "TForm", m_pRemOpenName);
			if( hwnd != NULL ){
				::ShowWindow(hwnd, SW_HIDE);
				::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
				::ShowWindow(hwnd, SW_SHOW);
				m_pRemOpenName = NULL;
			}
		}
	}
    else {
		if( LogLink.IsEnabled() ){
			if( LogLink.TimerLogLink() ){
				UpdateLogLink();
			}
			if( LogLink.IsPolling() && LogLink.IsLink() && !SBTX->Down ){
				LogLink.EventGetFreq();
			}
        }
        if( Log.m_LogSet.m_CopyHis == 2 ){
        	if( !m_BARTGTimer ){
            	m_BARTGTimer = 15;
				UpdateBARTG();
            }
            m_BARTGTimer--;
        }
	}
	if( pRadio != NULL ){
		pRadio->Timer(SBTX->Down, Timer->Interval);
		if( pRadio->IsFreqChange(AnsiString(Freq->Text).c_str()) ){	//JA7UDE 0428
			Freq->Text = pRadio->GetFreq();
			if( !Remote ){
				Log.SetFreq(&Log.m_sd, AnsiString(Freq->Text).c_str());	//JA7UDE 0428
			}
			if( Remote || (pTnc && pTnc->IsMMT()) ){
				double fq = 0;
				if( sscanf(AnsiString(Freq->Text).c_str(), "%lf", &fq) == 1 ){	//JA7UDE 0428
                	int freq = int(fq * 1000.0);
					if( Remote ) ::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_RADIOFREQ, freq);
					if( pTnc && pTnc->IsMMT() ) pTnc->GetNMMT()->m_radiofreq = freq;
				}
			}
		}
	}
	if( pTnc != NULL ){
		OnTncEvent();
        pTnc->Notify();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateBARTG(void)
{
	if( !SBQSO->Down && !SBTX->Down ){
       	SYSTEMTIME st;
        GetUTC(&st);
		if( st.wMinute != Log.m_UTC.wMinute ){
			Log.SetHisUTC();
            HisRST->Text = Log.m_sd.ur;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ClearMacroTimer(void)
{
	if( m_MacTimer ){		// É}ÉNÉçëóêMÇÃã≠êßìIÇ»íÜé~
		m_MacTimer = 0;
		GetSB(m_MacRetry-1)->Down = FALSE;
		if( (pSound != NULL) && pSound->m_Tx ) ToRX(1);
	}
	UpdatePttTimer();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateLogLink(void)
{
	if( !Remote ){
		LogLink.UpdateLink(sys.m_LogLink);
		UpdateTitle();
	}
}
//---------------------------------------------------------------------------
// êÊçsï“èWÉpÉbÉhÇÃï\é¶
// ÉtÉHÅ[ÉJÉXÇÃí≤êÆ
void __fastcall TMmttyWd::AdjustFocus(void)
{
    if( !Visible ) return;
    if( !Application->Active ) return;
	if( Remote ){
//		Application->Active = FALSE;
		if( m_OptionDialog || (m_pRemOpenName != NULL) ){
			return;
		}
        if( Remote & REMSHOWOFF ) return;
        if( Remote & REMDISFOCUS ) return;
		if( (APP_HANDLE != HWND_BROADCAST) && (Application->Active == TRUE) ){
        	::SetForegroundWindow(APP_HANDLE);
        }
	}
	else if( PanelRx->Visible ){
		PanelRx->SetFocus();
		FifoEdit.SetCursor(1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::TimerFocus(void)
{
    if( !Visible ) return;
    if( !Application->Active ) return;
	if( Remote ){
		if( ActiveControl != PanelFFT ){
//		    if( Remote & REMDISFOCUS ) return;
			if( MarkFreq->DroppedDown ) return;
			if( ShiftFreq->DroppedDown ) return;
			if( DemoBox1->DroppedDown ) return;
			if( DemoBox2->DroppedDown ) return;
			if( PanelFFT->Visible ) PanelFFT->SetFocus();
        }
    }
	else if( ActiveControl != PanelRx ){
		FifoEdit.SetCursor(0);
		if( MarkFreq->DroppedDown ) return;
		if( ShiftFreq->DroppedDown ) return;
		if( DemoBox1->DroppedDown ) return;
		if( DemoBox2->DroppedDown ) return;
		if( MsgList->DroppedDown ) return;
		if( HisCall->Focused() ) return;
		if( HisName->Focused() ) return;
		if( HisQTH->Focused() ) return;
		if( ActiveControl == HisRST ) return;
		if( ActiveControl == MyRST ) return;
		if( ActiveControl == Freq ) return;
		AdjustFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxFFTINPaint(TObject *Sender)
{
	if( PBoxFFTIN->Visible ){
		PBoxFFTIN->Canvas->Draw(0, 0, (TGraphic*)pBitmapFFTIN);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxWaterPaint(TObject *Sender)
{
	if( PBoxWater->Visible ){
		PBoxWater->Canvas->Draw(0, 0, (TGraphic*)pBitmapWater);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateRev(void)
{
	int txrev = sys.m_Rev;
	if( sys.m_TxDisRev ) txrev = 0;
	pSound->FSKMOD.SetRev(txrev);
	int rxrev = sys.m_Rev;
	if( SBTX->Down && (sys.m_echo != 2) ) rxrev = txrev;
	pSound->FSKDEM.SetRev(rxrev);
#if 0
	if( !SBTX->Down || !sys.m_TxDisRev ){
		pSound->FSKDEM.SetRev(sys.m_Rev);
		pSound->FSKMOD.SetRev(sys.m_Rev);
	}
	else {
		pSound->FSKDEM.SetRev(0);
		pSound->FSKMOD.SetRev(0);
	}
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBRevClick(TObject *Sender)
{
	sys.m_Rev = SBRev->Down;
	UpdateRev();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBDemClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->FSKDEM.m_type++;
	if( pSound->FSKDEM.m_type >= 4 ) pSound->FSKDEM.m_type = 0;
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBFIGClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	rtty.m_fig = SBFIG->Down;
	if( Remote ) PostApp(TXM_FIGEVENT, rtty.m_fig);
	AdjustFocus();
}
void __fastcall TMmttyWd::SBAFCClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	sys.m_AFC = SBAFC->Down;
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBATCClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->FSKDEM.m_atc = SBATC->Down;
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBNETClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	sys.m_TxNet = SBNET->Down;
	if( SBTX->Down ){
		UpdateNet();
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBBPFClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->m_bpf = SBBPF->Down;
	if( SBBPF->Down ) pSound->CalcBPF();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBLMSClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->m_lmsbpf = SBLMS->Down;
	if( SBLMS->Down ){
		pSound->CalcBPF();
	}
	else if( pSound->m_lms.m_twoNotch ){
		pSound->m_lms.m_lmsNotch = pSound->m_lms.m_lmsNotch2 = 0;
	}
    RemoteFreq();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBLMSMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		pSound->m_lms.m_Type = pSound->m_lms.m_Type ? 0 : 1;
		UpdateLMS();
		SBLMSClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBSQClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->FSKDEM.SetSQ(SBSQ->Down);
	UpdateUI();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::MarkFreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	double sf;
	sscanf(AnsiString(MarkFreq->Text).c_str(), "%lf", &sf);	//JA7UDE 0428
	if( (sf >= MARKL) && (sf < SPACEH) ){
		double sft = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
		pSound->FSKDEM.SetMarkFreq(sf);
		pSound->FSKMOD.SetMarkFreq(sf);
		if( pRadio != NULL ) pRadio->SetMarkFreq(sf);
		sf += sft;
		pSound->FSKDEM.SetSpaceFreq(sf);
		pSound->FSKMOD.SetSpaceFreq(sf);
		if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
	}
	RemoteFreq();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetShift(double sf)
{
	if( (sf >= SHIFTL) && (sf < 1500.0) ){
		if( sys.m_FixShift == 3 ){
			double sfq = pSound->FSKDEM.GetSpaceFreq();
			double mfq = pSound->FSKDEM.GetMarkFreq();
			mfq = ((mfq + sfq - sf) * 0.5);
			mfq = double(int(mfq + 0.5));
			pSound->FSKDEM.SetMarkFreq(mfq);
			pSound->FSKMOD.SetMarkFreq(mfq);
			if( pRadio != NULL ) pRadio->SetMarkFreq(mfq);
			m_DisEvent--;
			MarkFreq->Text = mfq;
			m_DisEvent++;
		}
		double f = pSound->FSKDEM.GetMarkFreq() + sf;
		m_DisEvent++;
		if( f >= (DemSamp*0.5) ){
			f = 1445.0;
			MarkFreq->Text = f;
			pSound->FSKDEM.SetMarkFreq(f);
			pSound->FSKMOD.SetMarkFreq(f);
			if( pRadio != NULL ) pRadio->SetMarkFreq(f);
			f += sf;
			pSound->FSKDEM.SetSpaceFreq(f);
			pSound->FSKMOD.SetSpaceFreq(f);
		}
		else {
			pSound->FSKDEM.SetSpaceFreq(f);
			pSound->FSKMOD.SetSpaceFreq(f);
		}
		if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
		RemoteFreq();
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ShiftFreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	double sf;
	sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sf);	//JA7UDE
	SetShift(sf);
}
//---------------------------------------------------------------------------
// ÉNÉCÉbÉNÉVÉtÉgÉ`ÉFÉìÉW
void __fastcall TMmttyWd::Label3Click(TObject *Sender)
{
	if( m_DisEvent ) return;

	double sf;
	sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lf", &sf);
	if( sf < 185.0 ){
		sf = 200;
	}
	else if( sf < 210.0 ){
		sf = 220;
	}
	else if( sf < 250.0 ){
		sf = 350;
	}
	else if( sf < 400.0 ){
		sf = 450;
	}
	else {
		sf = 170;
	}
//	sf = (sf < 185.0) ? 200 : 170;
	m_DisEvent++;
	ShiftFreq->Text = sf;
	SetShift(sf);
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBUOSClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	rtty.m_uos = SBUOS->Down;
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KTestClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	ClearMacroTimer();
	KTest->Checked = KTest->Checked ? 0 : 1;

	if( KTest->Checked ){
		rtty.ClearTX();
		pSound->FSKMOD.ClearTXBuf();
		pSound->FSKMOD.SetBaudRate(pSound->FSKDEM.GetBaudRate());
		pSound->FSKMOD.SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
		pSound->FSKMOD.SetSpaceFreq(pSound->FSKDEM.GetSpaceFreq());
		pSound->FSKMOD.SetRev(pSound->FSKDEM.GetRev());
		if( pComm != NULL ) pComm->EnbTX(1);
	}
	else if( !SBTX->Down ){
		if( pComm != NULL ) pComm->EnbTX(0);
	}
	pSound->m_Test = KTest->Checked;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::TBCharWaitChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	switch(sys.m_LWait){
		case 1:
			pSound->FSKMOD.m_DiddleWait = GetTBValue(TBCharWait->Position, 50, 50) + 0.5;
			break;
		default:
			pSound->FSKMOD.m_CharWait = GetTBValue(TBCharWait->Position, 50, 50) + 0.5;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::LTapClick(TObject *Sender)
{
	m_Baud = m_Baud ? FALSE : TRUE;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxRxPaint(TObject *Sender)
{
	static int flag = 0;

	if( Remote ) return;

	if( PBoxRx->Canvas->ClipRect.Top ){
		if( !flag ){
			PBoxRx->Invalidate();
			flag++;
		}
	}
	else {
		flag = 0;
	}
	if( UsrPal != NULL ){
		PBoxRx->Canvas->Brush->Bitmap = pBitmapRx;
		TRect rc;

		rc.Top = 0;
		rc.Left = 0;
		rc.Right = PBoxRx->Width;
		rc.Bottom = PBoxRx->Height;
		PBoxRx->Canvas->Brush->Bitmap = pBitmapRx;
		PBoxRx->Canvas->FillRect(rc);
	}
	PrintText.PaintCanvas();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxInPaint(TObject *Sender)
{
	static int flag = 0;

	if( Remote ) return;
	if( PBoxIn->Canvas->ClipRect.Top ){
		if( !flag ){
			PBoxIn->Invalidate();
			flag++;
		}
	}
	else {
		flag = 0;
	}
	if( UsrPal != NULL ){
		PBoxIn->Canvas->Brush->Bitmap = pBitmapIn;
		TRect rc;

		rc.Top = 0;
		rc.Left = 0;
		rc.Right = PBoxIn->Width;
		rc.Bottom = PBoxIn->Height;
		PBoxIn->Canvas->Brush->Bitmap = pBitmapIn;
		PBoxIn->Canvas->FillRect(rc);
	}
	FifoEdit.PaintCanvas();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::FormPaint(TObject *Sender)
{
	if( m_InitFirst == TRUE ){
		m_InitFirst = FALSE;
		UpdateColor();
		int f1stInst = FALSE;
		if( !Remote ){
			if( (sys.m_Call == "NOCALL") || sys.m_Call.IsEmpty() ){
	            f1stInst = TRUE;
				AnsiString as = "";
				if( sys.m_Call != "NOCALL" ){
					as = sys.m_Call;
				}
				if( InputMB("MMTTY", (Font->Charset != SHIFTJIS_CHARSET)?"Please enter your callsign":"Ç†Ç»ÇΩÇÃÉRÅ[ÉãÉTÉCÉìÇì¸óÕÇµÇƒâ∫Ç≥Ç¢.", as) == TRUE ){
					if( !as.IsEmpty() ){
						char bf[MLCALL+1];
						StrCopy(bf, as.c_str(), MLCALL);
						jstrupr(bf);
						sys.m_Call = bf;
						Log.MakePathName(ClipCall(bf));
						UpdateItem();
					}
				}
			}
			char bf[256];
			sprintf(bf, "%sARRL.DX", BgnDir);
			Cty.Load(bf);
			sprintf(bf, "%sMMCG.DEF", BgnDir);
			mmcg.LoadDef(bf);
		}
		pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
		pSound->DrawFFTWater(pBitmapWater, 1, KXYScope->Checked ? PBoxXY->Width : 0);

		if( !Remote ){
			if( !f1stInst ) Log.DoBackup();
			Log.Open(NULL, !f1stInst);		// ÉJÉåÉìÉgÉçÉOÉtÉ@ÉCÉãÇÃÉIÅ[ÉvÉì
			LogLink.UpdateLink(sys.m_LogLink);
		}
		UpdateTextData();
		if( Log.m_sd.btime ) SBQSO->Down = TRUE;
		OpenCloseCom();
		pSound->Resume();
		m_DisEvent = 0;
		if( Remote ){
			::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_THREAD, ::GetCurrentThreadId());
			::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_HANDLE, (DWORD)Handle);
			::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_START, 0x00000000);
		}
#if 0
		FILE *fp = fopen("eremote.txt", "rb");
		while(!feof(fp)){
			char c = fgetc(fp);
			PrintText.PutChar(c);
		}
		fclose(fp);
#endif
		DisPaint = FALSE;
	}
	else if( pSound != NULL ){
		if( pSound->m_susp && (!Remote) ){
			KExtResetClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ToTX(LPCSTR p)
{
	if( !SBTX->Down ){
		m_ReqRecv = 0;
		SBTX->Down = TRUE;
		if( p == NULL ){
			XMIT(1);
		}
		else {
			int f = 1;
			for( ; *p; p++ ){
				if( (*p == '~') || ((*p == '%') && (*(p+1)=='{')) ){
					f = 0;
					break;
				}
				else if( (*p != '[')&&(*p != ']')&&(*p != '\\')&&(*p != '_') ){
					break;
				}
			}
			XMIT(f);
		}
		if( !Remote ) AdjustFocus();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ToRX(int sw)
{
	if( SBTX->Down ){
		if( pTnc ) pTnc->m_PTT = 0;
		if( Remote & REMVIATX ) FifoEdit.Clear();
		if( pComm != NULL ) pComm->DisDiddle(-1);
		pSound->FSKMOD.SetDiddleTimer(-1);		// Diddle ã÷é~
		pSound->TrigBCC();
		pSound->FSKMOD.DeleteTXBuf();
		SBTX->Caption = "REQ";
		SBTX->Update();
		if( !sw ){							// ëóêMäÆóπë“Çø
			if( sys.m_TxPort != txTXDOnly ){
				for( int i = 0; (pSound->GetBCC() >= 0) && (i < 200); i++ ){
					RecvJob();
					Sleep(20);
				}
			}
            else {
				Sleep(200);
				RecvJob();
            }
		}
		pSound->m_ReqTx = 0;
		for( int i = 0; pSound->m_Tx && (i < 200); i++ ){
			RecvJob();
			Sleep(20);
		}
		m_MacroStat = 0;
		m_Macro = 0;
		m_ReqRecv = 0;
		SBTX->Caption = "TX";
		SBTX->Down = 0;
		PrintText.SetTX(0);
		if( sys.m_logTimeStamp ) PrintText.TrigTimeStamp("RX", sys.m_ShowTimeStamp);
		UpdateRev();
		if( sys.m_echo != 2 ){
			rtty.ClearRX();
			pSound->FSKDEM.ClearRXBuf();
			pSound->m_bpf = SBBPF->Down;
		}
		UpdateLimit();
		if( pComm != NULL ) pComm->SetTXRX(0);
		if( pRadio != NULL ) pRadio->SetPTT(0);
		if( pTnc != NULL ) pTnc->SetPTT(0);
		LogLink.SetPTT(0);
		SBFIG->Down = rtty.m_fig;
		if( Remote ) PostApp(TXM_FIGEVENT, rtty.m_fig);
		UpdateUI();
		TimerFocus();
		UpdateNet();
		if( Remote ){
			if( (pMap != NULL) && (sys.m_echo != 2) && !(Remote & REMDISSHARED) ){
				memset(pMap->arrayFFT, 0, sizeof(pMap->arrayFFT));
				memset(pMap->arrayX, 0, sizeof(pMap->arrayX));
				memset(pMap->arrayY, 0, sizeof(pMap->arrayY));
			}
			PostApp(TXM_PTTEVENT, 0);
		}
		if( pComm != NULL ) pComm->EnbTX(0);
	}
	if( !Remote ) AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateNet(void)
{
	if( SBTX->Down ){	// ëóêMêÿÇËë÷Ç¶éû
		if( sys.m_TxNet && (!sys.m_TxFixShift) ){
			pSound->FSKMOD.SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
			pSound->FSKMOD.SetSpaceFreq(pSound->FSKDEM.GetSpaceFreq());
		}
		else {
			m_RxMarkFreq = pSound->FSKDEM.GetMarkFreq();
			m_RxSpaceFreq = pSound->FSKDEM.GetSpaceFreq();
			double mfq;
			double sfq;
			if( sys.m_TxNet ){
				mfq = pSound->FSKDEM.GetMarkFreq();
				sfq = pSound->FSKDEM.GetSpaceFreq();
				mfq += ((sfq - mfq) - sys.m_DefShift) * 0.5;
			}
			else {
				mfq = sys.m_DefMarkFreq;
			}
			mfq = int(mfq);
			sfq = mfq + sys.m_DefShift;
			pSound->FSKMOD.SetMarkFreq(mfq);
			pSound->FSKMOD.SetSpaceFreq(sfq);
			pSound->FSKDEM.SetMarkFreq(mfq);
			pSound->FSKDEM.SetSpaceFreq(sfq);
			m_DisEvent++;
			MarkFreq->Text = mfq;
			ShiftFreq->Text = sys.m_DefShift;
			m_DisEvent--;
			RemoteFreq();
		}
	}
	else {				// éÛêMêÿÇËë÷Ç¶éû
		if( !sys.m_TxNet || sys.m_TxFixShift ){
			pSound->FSKDEM.SetMarkFreq(m_RxMarkFreq);
			pSound->FSKDEM.SetSpaceFreq(m_RxSpaceFreq);
			if( pRadio != NULL ) pRadio->SetMarkFreq(m_RxMarkFreq);
			m_DisEvent++;
			MarkFreq->Text = m_RxMarkFreq;
			ShiftFreq->Text = m_RxSpaceFreq - m_RxMarkFreq;
			m_DisEvent--;
			RemoteFreq();
		}
	}
}
//---------------------------------------------------------------------------
// ã≠êßìIÇ»éÛêM
void __fastcall TMmttyWd::SBTXOFFClick(TObject *Sender)
{
	ClearMacroTimer();
	ToRX(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::XMIT(int sw)
{
	if( !Remote && (sys.m_DisTX&1) ){
		SBTX->Down = FALSE;
		return;
    }
	m_MacroStat = 0;
	m_Macro = 0;
	m_ReqRecv = 0;
	SBTX->Caption = "REQ";
	SBTX->Update();
	pSound->FSKMOD.ClearTXBuf();
	rtty.ClearTX();
	pSound->FSKMOD.SetBaudRate(pSound->FSKDEM.GetBaudRate());
	UpdateNet();
	UpdateRev();
	if( Remote ) PostApp(TXM_PTTEVENT, 1);
	if( pComm != NULL ) pComm->SetTXRX(1);
	if( pRadio != NULL ) pRadio->SetPTT(1);
	if( pTnc != NULL ) pTnc->SetPTT(1);
	LogLink.SetPTT(1);
	if( Remote && m_RemoteDelay ) ::Sleep(m_RemoteDelay);
	pSound->m_ReqTx = 1;
	pSound->FSKMOD.m_Amp.Reset();
	pSound->FSKMOD.m_AmpVal = 1;
	pSound->FSKMOD.OutTone(sw, pSound->m_BuffSize);
	if( sys.m_echo != 2 ){
    	pSound->FSKDEM.ClearRXBuf();
		if( !SBNET->Down ){
        	pSound->m_bpf = 0;
        }
    }
#if 1
	if( sys.m_TxPort != txTXD ){
		for( int i = 0; (!pSound->m_Tx) && (i < 200); i++ ) Sleep(20);
//		pSound->FSKMOD.OutTone(sw, pSound->m_BuffSize);
	}
#else
	for( int i = 0; (!pSound->m_Tx) && (i < 200); i++ ) Sleep(20);
	pSound->FSKMOD.OutTone(sw, pSound->m_BuffSize);
#endif
    SBTX->Caption = "TX";
	if( sys.m_logTimeStamp ) PrintText.TrigTimeStamp("TX", sys.m_ShowTimeStamp);
	if( sys.m_echo != 2 ){
		pSound->FSKDEM.ClearRXBuf();
	}
	SBFIG->Down = rtty.m_fig;
	if( Remote ) PostApp(TXM_FIGEVENT, rtty.m_fig);
	UpdateUI();
	UpdateLimit();
	pSound->FSKMOD.SetDiddleTimer(SampFreq/4);		// 0.25s
	if( pComm != NULL ) pComm->EnbTX(1);
	PrintText.SetTX(1);
	UpdatePttTimer();
#if 0
	if( (sys.m_echo == 2) && sw ){
		pSound->FSKMOD.SetCount(pSound->m_BuffSize * 3);
	}
#endif
//	pSound->FSKMOD.OutTone(sw, pSound->m_BuffSize);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBTXClick(TObject *Sender)
{
	ClearMacroTimer();
	if( SBTX->Down ){
		XMIT(1);
		if( (!Remote) && KENT->Checked ) OutputStr("\r\n%L");
	}
	else if( FifoEdit.GetLen() || ((!IsTxIdle())&& (!m_Macro)) ){
		if( m_ReqRecv ){
			m_ReqRecv = 0;
			SBTX->Caption = "TX";
			SBTX->Down = 1;
		}
		else {
			if( (!Remote) && KENT->Checked ){
				char c = FifoEdit.GetLastChar();
				if( (c != 0x0a) && (c != '\\') ) WriteFifoEdit("\r\n");
			}
			m_ReqRecv = 1;
			SBTX->Caption = "WAIT";
			SBTX->Down = 1;
		}
	}
	else if( (!Remote) && KENT->Checked && (!m_Macro) ){
		OutputStr("\r\n");
		m_ReqRecv = 1;
		SBTX->Caption = "WAIT";
		SBTX->Down = 1;
	}
	else {
		SBTX->Down = 1;
		ToRX(0);
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
// ÉLÅ[É{Å[Éh
void __fastcall TMmttyWd::PushKey(char Key)
{
	Key = char(toupper(Key));
	if( KWP->Checked && (Key == ' ') ){
		char bf[256];
		ConvString(bf, FifoEdit.GetCurLine(), sizeof(bf)-1, 0);
		int len = strlen(bf);
		if( (len >= 60) || ((len  >= (FifoEdit.m_ColMax * 82/100)) && (!Remote)) ){
			FifoEdit.PutChar(Key);
			Key = 0x0d;
		}
	}
	if( (pSound->FSKDEM.m_BitLen <= 6) && (Key != '%') && (Key != 0x08) && (Key != '^') ){
		if( !rttysub.IsChar(rttysub.ConvRTTY(Key)) ){
			return;
		}
	}
	FifoEdit.PutChar(Key);
}
//---------------------------------------------------------------------------
// ÉLÅ[É{Å[Éh
void __fastcall TMmttyWd::FormKeyPress(TObject *Sender, char &Key)
{
	if( HisCall->Focused() ) return;
	if( HisName->Focused() ) return;
	if( HisQTH->Focused() ) return;
	if( ActiveControl == HisRST ) return;
	if( ActiveControl == MyRST ) return;
	if( ActiveControl == Freq ) return;
	if( Key == VK_ESCAPE ) return;

	ClearMacroTimer();
	PushKey(Key);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::FormKeyDown(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	TShiftState sc1, sc2, sa1, sa2, ss1, ss2;

	WORD nKey = WORD(Key & 0x00ff);
	sc1 << ssCtrl;
	sc2 << ssCtrl;
	sc1 *= Shift;

	sa1 << ssAlt;
	sa2 << ssAlt;
	sa1 *= Shift;

	ss1 << ssShift;
	ss2 << ssShift;
	ss1 *= Shift;

	if( sc1 == sc2  ){		// Ctrl+Any
		nKey |= 0x0100;
	}
	else if( sa1 == sa2 ){	// Alt + Any
		nKey |= 0x0200;
	}
	else if( ss1 == ss2 ){	// Shift + Any
		nKey |= 0x0400;
	}

	ClearMacroTimer();
	if( nKey == (VK_BACK | 0x100) ){
		FifoEdit.LineBackSpace();
		Key = 0;
	}
// ÉÜÅ[ÉUÅ[íËã`â¬î\Ç»ÉLÅ[
	else if( nKey == sys.m_SysKey[kkUOS] ){
		SBUOS->Down = SBUOS->Down ? 0 : 1;
		SBUOSClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkFIG] ){
		SBFIG->Down = SBFIG->Down ? 0 : 1;
		SBFIGClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkTX] ){
		SBTX->Down = SBTX->Down ? 0 : 1;
		SBTXClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkTXOFF] ){
		SBTXOFFClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkQSO] ){
		if( SBQSO->Enabled ){
			SBQSO->Down = SBQSO->Down ? 0 : 1;
			SBQSOClick(NULL);
		}
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkOnQSO] ){
		QSOIN();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkOffQSO] ){
		QSOOUT(1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkCAPTURE] ){
		CallCapture();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInitBox] ){
		SBInitClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkCall] ){
		HisCall->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkName] ){
		m_NameQTH = 0;
		UpdateUI();
		HisName->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkQTH] ){
		m_NameQTH = 1;
		UpdateUI();
		HisQTH->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRST] ){
		HisRST->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkMyRST] ){
		MyRST->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkFreq] ){
		Freq->SetFocus();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkFind] ){
		SBFindClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkClear] ){
		SBINClearClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInUp] ){
		FifoEdit.ScrollBarUp(0);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInDown] ){
		FifoEdit.ScrollBarDown(0);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInPUp] ){
		FifoEdit.ScrollBarUp(1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInPDown] ){
		FifoEdit.ScrollBarDown(1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInHome] ){
		FifoEdit.ScrollBarUp(-1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInEnd] ){
		FifoEdit.ScrollBarDown(-1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxUp] ){
		PrintText.ScrollBarUp(0);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxDown] ){
		PrintText.ScrollBarDown(0);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxPUp] ){
		PrintText.ScrollBarUp(1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxPDown] ){
		PrintText.ScrollBarDown(1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxHome] ){
		PrintText.ScrollBarUp(-1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkRxEnd] ){
		PrintText.ScrollBarDown(-1);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkCharWaitLeft] ){
		int pos = SetTBValue(pSound->FSKMOD.m_CharWait, 50, 50);
		pos-=5;
		if( pos < 0 ) pos = 0;
		int wait = GetTBValue(pos, 50, 50);
		if( pSound->FSKMOD.m_CharWait == wait ) wait--;
		if( wait < 0 ) wait = 0;
		pSound->FSKMOD.m_CharWait = wait;
		UpdateLWait();
		Key = 0;
#if 0
		if( TBCharWait->Position ){
			if( TBCharWait->Position > 5 ){
				TBCharWait->Position -= 5;
			}
			else {
				TBCharWait->Position = 0;
			}
			TBCharWaitChange(NULL);
		}
		Key = 0;
#endif
	}
	else if( nKey == sys.m_SysKey[kkCharWaitRight] ){
		int pos = SetTBValue(pSound->FSKMOD.m_CharWait, 50, 50);
		pos+=5;
		if( pos > 50 ) pos = 50;
		int wait = GetTBValue(pos, 50, 50);
		if( pSound->FSKMOD.m_CharWait == wait ) wait++;
		if( wait > 50 ) wait = 50;
		pSound->FSKMOD.m_CharWait = wait;
		UpdateLWait();
		Key = 0;
#if 0
		if( TBCharWait->Position < TBCharWait->Max ){
			if( TBCharWait->Position < (TBCharWait->Max - 5) ){
				TBCharWait->Position += 5;
			}
			else {
				TBCharWait->Position = TBCharWait->Max;
			}
			TBCharWaitChange(NULL);
		}
		Key = 0;
#endif
	}
	else if( nKey == sys.m_SysKey[kkDiddleWaitLeft] ){
		int pos = SetTBValue(pSound->FSKMOD.m_DiddleWait, 50, 50);
		pos-=5;
		if( pos < 0 ) pos = 0;
		int wait = GetTBValue(pos, 50, 50);
		if( pSound->FSKMOD.m_DiddleWait == wait ) wait--;
		if( wait < 0 ) wait = 0;
		pSound->FSKMOD.m_DiddleWait = wait;
		UpdateLWait();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkDiddleWaitRight] ){
		int pos = SetTBValue(pSound->FSKMOD.m_DiddleWait, 50, 50);
		pos+=5;
		if( pos > 50 ) pos = 50;
		int wait = GetTBValue(pos, 50, 50);
		if( pSound->FSKMOD.m_DiddleWait == wait ) wait++;
		if( wait > 50 ) wait = 50;
		pSound->FSKMOD.m_DiddleWait = wait;
		UpdateLWait();
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInHeightUp] ){
		if( PanelRx->Height > 50 ){
			PanelIn->Height++;
			UpdatePanel();
		}
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkInHeightDown] ){
		if( PanelIn->Height > 20 ){
			PanelIn->Height--;
			UpdatePanel();
		}
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkTxLTR] ){
		if( SBTX->Down ){
			FifoEdit.PutChar('%');
			FifoEdit.PutChar('L');
		}
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkTxFIG] ){
		if( SBTX->Down ){
			FifoEdit.PutChar('%');
			FifoEdit.PutChar('F');
		}
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkDecShift] ){
		int sf;
		sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lu", &sf);	//JA7UDE 0428
		sf-=2;
		m_DisEvent++;
		ShiftFreq->Text = sf;
		SetShift(sf);
		m_DisEvent--;
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkIncShift] ){
		int sf;
		sscanf(AnsiString(ShiftFreq->Text).c_str(), "%lu", &sf);	//JA7UDE 0428
		sf+=2;
		m_DisEvent++;
		ShiftFreq->Text = sf;
		SetShift(sf);
		m_DisEvent--;
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkToggleShift] ){
		Label3Click(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_SysKey[kkCList] ){
		HisCallDblClick(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_InBtnKey[0] ){
		SBIN1Click(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_InBtnKey[1] ){
		SBIN2Click(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_InBtnKey[2] ){
		SBIN3Click(NULL);
		Key = 0;
	}
	else if( nKey == sys.m_InBtnKey[3] ){
		SBIN4Click(NULL);
		Key = 0;
	}
	else {
		int i;
		for( i = 0; i < 16; i++ ){				// É}ÉNÉçÉ{É^Éì
			if( nKey == sys.m_UserKey[i] ){
				MacBtnExec(i);
				Key = 0;
				return;
			}
		}
		for( i = 0; i < MSGLISTMAX; i++ ){		// ÉÅÉbÉZÅ[ÉWÉäÉXÉg
			if( nKey == sys.m_MsgKey[i] ){
				AnsiString as = sys.m_MsgName[i];
				FindMsgList(as);
				Key = 0;
				return;
			}
		}
		for( i = 0; i < 5; i++ ){
			if( nKey == Log.m_LogSet.m_QSOMacroKey[i] ){
				if(!Log.m_LogSet.m_QSOMacroStr[i].IsEmpty()){
					ToTX(Log.m_LogSet.m_QSOMacroStr[i].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[i].c_str());
					return;
				}
			}
		}
		for( i = 0; i < 16; i++ ){				// É}ÉNÉçï“èWÉ{É^Éì
			if( nKey == sys.m_UserEditKey[i] ){
				EditMB(i);
				Key = 0;
				return;
			}
		}
	}
}
//---------------------------------------------------------------------------
// ÉLÅ[É{Å[Éh
void __fastcall TMmttyWd::FormKeyUp(TObject *Sender, WORD &Key,
	TShiftState Shift)
{
	int n;
	switch(Key){
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
			FormKeyDown(Sender, Key, Shift);
			break;
		case VK_TAB:
			AdjustFocus();
			n = 4 - (FifoEdit.m_WriteCol % 4);
			for( ; n; n-- ) FifoEdit.PutChar(' ');
			break;
	}
	Key = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetGreetingString(LPSTR t, LPCSTR pCall, int type)
{
	strcpy(t, type ? "" : "HELLO");
	if( !Cty.IsData() ) return;
	LPCSTR	p = ClipCC(pCall);
	if( *p ){
		int		n;
		if( (n = Cty.GetNoP(p))!=0 ){
			CTL *cp = Cty.GetCTL(n-1);
			if( cp->TD != NULL ){
				SYSTEMTIME	now;
				GetUTC(&now);

				WORD tim = WORD((now.wHour * 60 + now.wMinute) * 30 + now.wSecond/2);
				tim = AdjustRolTimeUTC(tim, *cp->TD);
				if( tim ){
					tim /= WORD(30);
					if( tim < 12*60 ){
						strcpy(t, type ? "GM" : "GOOD MORNING");
					}
					else if( tim < 18*60 ){
						strcpy(t, type ? "GA" : "GOOD AFTERNOON");
					}
					else {
						strcpy(t, type ? "GE" : "GOOD EVENING");
					}
				}
			}
		}

	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::StoreCWID(LPSTR &tt, char c, int &nc, int size)
{
	const USHORT _tbl[]={
		// 0       1       2       3        4       5       6       7
		0x0005, 0x8005, 0xc005, 0xe005, 0xf005, 0xf805, 0x7805, 0x3805, // 0-7
		// 8       9       :        ;       <       =       >       ?
		0x1805, 0x0805, 0x0000, 0x0000, 0xe806, 0x7005, 0xA805, 0xcc06, // 8
		// @        A       B       C       D       E       F       G
		0x0000, 0x8002, 0x7004, 0x5004, 0x6003, 0x8001, 0xd004, 0x2003, // @-G
		// H        I       J       K       L       M       N       O
		0xf004, 0xc002, 0x8004, 0x4003, 0xb004, 0x0002, 0x4002, 0x0003, // H-O
		// P        Q       R       S       T       U       V       W
		0x9004, 0x2004, 0xa003, 0xe003, 0x0001, 0xc003, 0xe004, 0x8003, // P-W
		// X        Y       Z
		0x6004, 0x4004, 0x3004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // X-Z
	};

	c = char(toupper(c));
	c &= 0x7f;
	int d, nn;
	if( c == '/' ){
		d = 0x6805;
		nn = d & 0x00ff;
	}
	else if( c == '@' ){
		d = -1;
		nn = 3;
	}
	else if( (c >= '0') && (c <= 'Z') ){
		c -= '0';
		d = _tbl[c];
		nn = d & 0x00ff;
	}
	else {
		d = -1;
		nn = 5;
	}
	if( d == -1 ){
		nc += nn;
		if( nc < (size - 1) ){
			for( ; nn; nn-- ){
				*tt++ = '~';
			}
		}
	}
	else {
		for( ; nn; nn-- ){
			if( nc < (size - 5) ){
				*tt++ = '_'; nc++;
				if( !(d & 0x8000) ){
					*tt++ = '_'; nc++;
					*tt++ = '_'; nc++;
				}
				*tt++ = '~'; nc++;
				d = d << 1;
			}
		}
	}
	nc += 2;
	if( nc < (size - 1) ){
		*tt++ = '~';
		*tt++ = '~';
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ConvString(LPSTR t, LPCSTR p, int size, int sw)
{
	int f = (HisCall->Text.IsEmpty() && Log.m_LogSet.m_Contest);
	int n, l;
	char	bf[256];
	SYSTEMTIME	now;

	for( n = 0;*p; p++ ){
		if( *p == '%' ){
			p++;
			switch(*p){
				case 'm':
					strcpy(t, sys.m_Call.c_str());
					break;
				case 'c':
					strcpy(t, f ? Log.m_asd.call: AnsiString(HisCall->Text).c_str());	//JA7UDE 0428
					break;
				case 'n':
					if( HisName->Text.IsEmpty() ){
						strcpy(t, "OM");
					}
					else {
						strcpy(t, AnsiString(HisName->Text).c_str());	//JA7UDE 0428
					}
					break;
				case 'q':
					strcpy(t, AnsiString(HisQTH->Text).c_str());	//JA7UDE 0428
					break;
				case 'r':
					strcpy(t, f ? Log.m_asd.ur: AnsiString(HisRST->Text).c_str());	//JA7UDE 0428
					break;
				case 's':
					strcpy(t, f ? Log.m_asd.my: AnsiString(MyRST->Text).c_str());	//JA7UDE 0428
					break;
				case 'R':
					strcpy(bf, f ? Log.m_asd.ur: AnsiString(HisRST->Text).c_str());	//JA7UDE 0428
					if( strlen(bf) >= 3 ){
						StrCopy(t, bf, 3);
					}
					else {
						strcpy(t, "599");
					}
					break;
				case 'N':
					strcpy(bf, f ? Log.m_asd.ur: AnsiString(HisRST->Text).c_str());	//JA7UDE 0428
					if( strlen(bf) > 3 ){
						strcpy(t, &bf[3]);
					}
					else {
						*t = 0;
					}
					break;
				case 'M':
					strcpy(bf, f ? Log.m_asd.my: AnsiString(MyRST->Text).c_str());	//JA7UDE 0428
					if( strlen(bf) > 3 ){
						strcpy(t, &bf[3]);
					}
					else {
						*t = 0;
					}
					break;
				case 'g':
					SetGreetingString(t, AnsiString(HisCall->Text).c_str(), 0);	//JA7UDE 0428
					break;
				case 'f':
					SetGreetingString(t, AnsiString(HisCall->Text).c_str(), 1);	//JA7UDE 0428
					break;
				case 'L':
					if( sw ){
						*t = '%';
						*(t+1) = 'L';
						*(t+2) = 0;
					}
					else {
						*t = 0x1f;
						*(t+1) = 0;
					}
					break;
				case 'F':
					if( sw ){
						*t = '%';
						*(t+1) = 'F';
						*(t+2) = 0;
					}
					else {
						*t = 0x1b;
						*(t+1) = 0;
					}
					break;
				case 'E':
					goto _ex;
				case 'D':
					GetUTC(&now);
					switch(Log.m_LogSet.m_DateType){
						case 2:
						case 3:
							sprintf(t, "%02u-%s-%04u", now.wDay, MONT1[now.wMonth], now.wYear);
							break;
						case 4:
						case 5:
							sprintf(t, "%s-%02u-%04u", MONT1[now.wMonth], now.wDay, now.wYear);
							break;
						default:
							sprintf(t, "%04u-%s-%02u", now.wYear, MONT1[now.wMonth], now.wDay);
							break;
					}
					break;
				case 'T':
					GetUTC(&now);
					sprintf(t, "%02u:%02u", now.wHour, now.wMinute);
					break;
				case 't':
					GetUTC(&now);
					sprintf(t, "%02u%02u", now.wHour, now.wMinute);
					break;
				case 'x':
					*t = 0;
					strcpy(bf, f ? Log.m_asd.ur: AnsiString(HisRST->Text).c_str());	//JA7UDE 0428
					if( strlen(bf) > 3 ){
						LPSTR tt;
						StrDlm(tt, &bf[3], '-');
						if( *tt ){
							strcpy(t, tt);
						}
					}
					break;
				case 'y':
					*t = 0;
					strcpy(bf, f ? Log.m_asd.ur: AnsiString(HisRST->Text).c_str());	//JA7UDE 0428
					if( strlen(bf) > 3 ){
						LPSTR p, tt;
						p = StrDlm(tt, &bf[3], '-');
						if( *p ){
							strcpy(t, p);
						}
					}
					break;
				case '{':
					{
						p++;
						LPSTR tt = t;
						int nc = n + 2;
						if( nc < (size - 1) ){
							*tt++ = '[';	// Disable Diddle
							*tt++ = '~';
						}
						for( ; *p && *p != '}'; p++ ){
							if( *p == '%' ){
								char src[3];
								src[0] = *p++;
								src[1] = *p;
								src[2] = 0;
								char bbf[1024];
								ConvString(bbf, src, sizeof(bf)-1, 0);
								LPCSTR pp;
								for( pp = bbf; *pp; pp++ ){
									StoreCWID(tt, *pp, nc, size);
								}
							}
							else {
								StoreCWID(tt, *p, nc, size);
							}
						}
						*tt = 0;
					}
					break;
				default:
					strcpy(t, "%%");
					break;
			}
			l = strlen(t);
			n += l;
			if( n >= (size - 1) ) break;
			t += l;
		}
		else if( sw || (*p != '\\') ){
			*t++ = *p;
			n++;
		}
		if( n >= (size - 1) ) break;
	}
_ex:;
	*t = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OutputStr(LPCSTR pOut)
{
	char bf[1024];
	BYTE OutBf[1024];

	if( m_NowStr.c_str() != pOut ) m_NowStr = pOut;
	ConvString(bf, pOut, sizeof(bf), 0);
	int i, len;
	switch(pSound->FSKMOD.m_BitLen){
		case 7:
		case 8:
			len = strlen(bf);
			for( i = 0; i < len; i++ ){
				pSound->FSKMOD.PutData(bf[i]);
				if( pComm != NULL ) pComm->PutChar(bf[i]);
			}
			break;
		case 6:
		default:
			len = rtty.ConvRTTY(OutBf, bf);
			for( i = 0; i < len; i++ ){
				pSound->FSKMOD.PutData(OutBf[i]);
				if( pComm != NULL ) pComm->PutChar(OutBf[i]);
			}
			break;
	}
	LPCSTR p;
	for( p = &pOut[strlen(pOut)-1]; p >= pOut; p-- ){
		if( *p == '\\' ){
			m_MacroStat = 1;
			m_Macro = 17;
			if( SBTX->Down && (pSound != NULL) ){
				pSound->FSKMOD.SetDiddleTimer(-1);
				if( pComm != NULL ) pComm->DisDiddle(-1);
			}
		}
		else if( *p == '#' ){
			m_MacroStat = 2;
			m_Macro = 17;
			if( pSound != NULL ) pSound->FSKMOD.SetDiddleTimer(0);
			if( pComm != NULL ) pComm->DisDiddle(0);
		}
		else if( (*p != 0x0d) && (*p != 0x0a) ){
			break;
		}
	}
}
//---------------------------------------------------------------------------
TSpeedButton *__fastcall TMmttyWd::GetSB(int n)
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
int __fastcall TMmttyWd::GetMacroIndex(TObject *Sender)
{
	for( int i = 0; i < 16; i++ ){
		TSpeedButton *_sb[]={SBM1, SBM2, SBM3, SBM4, SBM5, SBM6, SBM7,
							 SBM8, SBM9, SBM10, SBM11, SBM12, SBM13, SBM14, SBM15, SBM16};

		if( ((TObject *)_sb[i]) == Sender ) return i;
	}
	return -1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::MacBtnExec(int n)
{
	ClearMacroTimer();
	if( n >= 0 ){
		if( !sys.m_User[n].IsEmpty() ){
			LPCSTR p = sys.m_User[n].c_str();
			if( *p == '#' ){
				WriteFifoEdit(p+1);
			}
			else {
				ToTX(p);
				if( *p == '\\' ){
					WriteFifoEdit(p+1);
				}
				else {
					OutputStr(p);
					m_Macro = n + 1;
					if( sys.m_UserTimer[n] ){
						m_MacTimer = (sys.m_UserTimer[n] * 100) + GetTickCount();
						m_MacRetry = m_Macro;
					}
					else {
						m_MacTimer = 0;
						m_MacRetry = 0;
					}
					GetSB(n)->Down = m_MacTimer ? TRUE : FALSE;
				}
			}
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBMClick(TObject *Sender)
{
	TSpeedButton *tp = (TSpeedButton *)Sender;
	if( tp->Down ){
		tp->Down = FALSE;
		if( sys.m_DisTX&1 ) return;
		MacBtnExec(GetMacroIndex(Sender));
	}
	else {
		ClearMacroTimer();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::EditMB(int n)
{
	if( n >= 0 ){
		TEditDlgBox *pBox = new TEditDlgBox(this);
		pBox->TxBtn->Visible = TRUE;
		pBox->ButtonName->Text = sys.m_UserName[n];
		if( pBox->Execute(sys.m_User[n], sys.m_UserKey[n], &sys.m_UserCol[n], &sys.m_UserTimer[n], 1) == TRUE ){
			if( pBox->m_JobSel ){
				AnsiString as = pBox->Memo->Text.c_str();
				LPCSTR p = as.c_str();
				if( *p == '#' ){
					WriteFifoEdit(p+1);
				}
				else {
					ToTX(p);
					if( *p == '\\' ){
						WriteFifoEdit(p+1);
					}
					else {
						OutputStr(p);
						m_Macro = n + 1;
						m_MacTimer = 0;
					}
				}
			}
			else if( !pBox->ButtonName->Text.IsEmpty() ){
				sys.m_UserName[n] = pBox->ButtonName->Text;
			}
		}
		delete pBox;
		UpdateMacro();
		UpdateShortCut(FALSE);
		TopWindow(this);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBMMouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int n  = GetMacroIndex(Sender);
		EditMB(n);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::FindCall(void)
{
	CWaitCursor w;
	m_Dupe = 0;
	StrCopy(Log.m_sd.call, AnsiString(HisCall->Text).c_str(), MLCALL);	//JA7UDE 0428
	AddCall(Log.m_sd.call);
	if( Log.FindSet(&Log.m_Find, Log.m_sd.call) ){
		Log.SetFreq(&Log.m_sd, AnsiString(Freq->Text).c_str());	//JA7UDE 0428
		SDMMLOG	sd;
		Log.GetData(&sd, Log.m_Find.pFindTbl[0]);	// àÍî‘ç≈êVÇÃÉfÅ[É^
		if( Log.m_Find.m_FindCmp1Max && ((!Log.m_LogSet.m_CheckBand) || Log.FindSameBand()) ){
			m_Dupe = 1;
		}
		else {
			m_Dupe = 0;
		}
		if( (!SBQSO->Down || !Log.m_sd.name[0] ) && Log.m_LogSet.m_CopyName ) strcpy(Log.m_sd.name, sd.name);
		if( (!SBQSO->Down || !Log.m_sd.qth[0] ) && Log.m_LogSet.m_CopyQTH ) strcpy(Log.m_sd.qth, sd.qth);
		if( (!SBQSO->Down || !Log.m_sd.rem[0] ) && Log.m_LogSet.m_CopyREM ) strcpy(Log.m_sd.rem, sd.rem);
		if( (!SBQSO->Down || !Log.m_sd.qsl[0] ) && Log.m_LogSet.m_CopyQSL ) strcpy(Log.m_sd.qsl, sd.qsl);
		UpdateTextData();
		if( SBQSO->Down ){
			Log.m_Find.Ins(Log.m_CurNo);
			UpdateTextData();
			Log.PutData(&Log.m_sd, Log.m_CurNo);
		}
	}
	else {
		if( !SBQSO->Down && Log.m_LogSet.m_CopyName ) Log.m_sd.name[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyQTH ) Log.m_sd.qth[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyREM ) Log.m_sd.rem[0] = 0;
		if( !SBQSO->Down && Log.m_LogSet.m_CopyQSL ) Log.m_sd.qsl[0] = 0;
		UpdateTextData();
	}
	LPCSTR pCC = ClipCC(Log.m_sd.call);
	Log.SetOptStr(0, &Log.m_sd, Cty.GetCountry(pCC));
	Log.SetOptStr(1, &Log.m_sd, Cty.GetCont(pCC));
	UpdateCallsign();
	if( !SBQSO->Down ){
		LogLink.SetMode("RTTY");
		LogLink.SetFreq(AnsiString(Freq->Text).c_str());	//JA7UDE 0428
		LogLink.FindCall(Log.m_sd.call);
	}
	else {
		LogLink.Write(&Log.m_sd, 1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetYourCallsign(LPCSTR pCall)
{
	if( strcmpi(sys.m_Call.c_str(), pCall) ){
		HisCall->Text = pCall;
		FindCall();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetYourName(LPCSTR pName)
{
	HisName->Text = pName;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetYourQTH(LPCSTR pQTH)
{
	HisQTH->Text = pQTH;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetYourRST(LPCSTR pRST)
{
	MyRST->Text = pRST;
}
//---------------------------------------------------------------------------
static void __fastcall StoreZone(LPSTR t, LPSTR p)
{
	char	rst[4];
	char	qth[20];

	rst[0] = qth[0] = 0;
	StrCopy(rst, t, 3);
	if( !rst[0] ) strcpy(rst, "599");
	if( strlen(t) > 5 ){
		StrCopy(qth, t+5, 20);
	}
	if( !memcmp(p, "599", 3) ) p+=3;
	int d;
	sscanf(p, "%u", &d);
	sprintf(t, "%s%02u%s", rst, d, qth);
}
//---------------------------------------------------------------------------
static void __fastcall StoreQTH(LPSTR t, LPSTR p)
{
	char	rst[4];
	char	zone[3];

	rst[0] = zone[0] = 0;
	StrCopy(rst, t, 3);
	if( !rst[0] ) strcpy(rst, "599");
	if( strlen(t) > 3 ){
		StrCopy(zone, t+3, 2);
	}
	else {
		strcpy(zone, "  ");
	}
	sprintf(t, "%s%s%s", rst, zone, p);
}
//---------------------------------------------------------------------------
static void __fastcall StoreNR(LPSTR t, LPSTR p)
{
	char	nr[6];
	StrCopy(nr, p, 5);
	if( strlen(nr) < 3 ){
		int n = atoin(nr, -1);
		sprintf(nr, "%03u", n);
	}
	char	rst[4];
	char	utc[6];
	rst[0] = 0;
	LPSTR tt, pp;
	pp = StrDlm(tt, t, '-');
	StrCopy(utc, pp, 5);
	if( *tt ){
		if( (strlen(tt) > 3) && !utc[0] ){
			int h, m;
			sscanf(&tt[3], "%u", &h);
			m = h % 100;
			h = h / 100;
			if( (h >= 0) && (h < 24) && (m >= 0) && (m < 60) ){
				sprintf(utc, "%02u%02u", h, m);
			}
		}
		StrCopy(rst, tt, 3);
	}
	if( !rst[0] ) strcpy(rst, "599");
	sprintf(t, "%s%s-%s", rst, nr, utc);
}
//---------------------------------------------------------------------------
static void __fastcall StoreUTC(LPSTR t, LPSTR p)
{
	char	rst[MLRST+1];
	char	utc[6];
	StrCopy(utc, p, 5);
	int h, m;
	h = m = 0;
	if( sscanf(utc, "%u:%u", &h, &m) != 2 ){
		sscanf(utc, "%u", &h);
		m = h % 100;
		h = h / 100;
		if( (h < 0) || (h >= 24) || (m < 0) || (m >= 60) ){
			StoreNR(t, p);
			return;
		}
	}
	sprintf(utc, "%02u%02u", h, m);
	rst[0] = 0;
	LPSTR tt;
	StrDlm(tt, t, '-');
	if( *tt ){
		StrCopy(rst, tt, MLRST);
	}
	if( !rst[0] ) strcpy(rst, "599");
	sprintf(t, "%s-%s", rst, utc);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxRxMouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbMiddle ) return;
	char bf[256];
	char bbf[256];

	ClearMacroTimer();
	AdjustFocus();
	if( Button == mbRight ){		// âEÉ{É^Éì
		PrintText.ShiftText(X, Y);
		return;
	}
	PrintText.GetText(bf, X, Y, 0);
	if( bf[0] ){
		jstrupr(bf);
		if( HisCall->Text.IsEmpty() || (Log.m_LogSet.m_Contest == testPED) ){
			if( strlen(bf) >= 16 ) bf[16] = 0;
			SetYourCallsign(bf);
		}
		else if( Log.m_LogSet.m_Contest ){		// ÉRÉìÉeÉXÉgíÜ
			if( (strlen(bf) <= 2) ||
				!IsCall(bf) ||
				((strlen(bf) == 3) && !HisCall->Text.IsEmpty())
			){
				PrintText.GetText(bf, X, Y, 1);
				jstrupr(bf);
//				DelChar(bf, '/');
				if( Log.m_LogSet.m_Contest == testCQRJ ){	// CQ/RJ
					strcpy(bbf, AnsiString(MyRST->Text).c_str());	//JA7UDE 0428
					if(!IsNumbs(bf)){		// QTH
						StoreQTH(bbf, bf);
					}
					else {					// Zone
						StoreZone(bbf, bf);
					}
				}
				else if( Log.m_LogSet.m_Contest == testBARTG ){
					strcpy(bbf, AnsiString(MyRST->Text).c_str());	//JA7UDE 0428
					PrintText.GetText(bf, X, Y, 2);
					LPSTR p = bf;
					for( ; *p && !isdigit(*p); p++ );
					if( *p ){
						if( strchr(p, ':') != NULL ){
							StoreUTC(bbf, p);
						}
						else if( (strlen(p) <= 3) || (strlen(p) <= 3) || (bbf[4] == '-') ){
							StoreNR(bbf, p);
						}
						else {
							StoreUTC(bbf, p);
						}
					}
				}
				else {								// Misc
					if( (strlen(bf) >= 4) && !memcmp(bf, "599", 3) ){		// 599xx
						strcpy(bbf, bf);
					}
					else if(strlen(AnsiString(MyRST->Text).c_str()) >= 3 ){	//JA7UDE 0428
						strcpy(bbf, AnsiString(MyRST->Text).c_str());	//JA7UDE 0428
						strcpy(&bbf[3], bf);
					}
					else {
						sprintf(bbf, "599%s", bf);
					}
				}
				m_DisEvent++;
				MyRST->Text = bbf;
				m_DisEvent--;
			}
			else if( IsCall(bf) ){
				SetYourCallsign(bf);
			}
		}
		else if( IsRST(bf) ){
			if( (!Log.m_LogSet.m_DefMyRST) && SBQSO->Down && !MyRST->Text.IsEmpty() ){
				if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)? "Change MyRST ?":"MyRSTÇïœçXÇµÇ‹Ç∑Ç©?") != IDYES ) return;
			}
			SetYourRST(bf);
		}
		else if( IsCall(bf) ){
			if( strcmpi(bf, sys.m_Call.c_str()) ){
				if( SBQSO->Down && !HisCall->Text.IsEmpty() ){
					if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Change Callsign?":"ÉRÅ[ÉãÉTÉCÉìÇïœçXÇµÇ‹Ç∑Ç©?") != IDYES ) return;
				}
				SetYourCallsign(bf);
			}
		}
		else if( m_NameQTH ){
			if( SBQSO->Down && !HisQTH->Text.IsEmpty() ){
				if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Change QTH?":"ÇpÇsÇgÇïœçXÇµÇ‹Ç∑Ç©?") != IDYES ) return;
			}
			SetYourQTH(bf);
		}
		else if( IsName(bf) ){
			if( SBQSO->Down && !HisName->Text.IsEmpty() ){
				if( YesNoMB((Font->Charset != SHIFTJIS_CHARSET)?"Change Name?":"ñºëOÇïœçXÇµÇ‹Ç∑Ç©?") != IDYES ) return;
			}
			SetYourName(bf);
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ScrollBarRxChange(TObject *Sender)
{
	PrintText.ScrollBarChange();
	if( ScrollBarRx->Dragging() == FALSE ){
		TimerFocus();
	}
}

#if USEPAL
//---------------------------------------------------------------------------
// åªç›ÇÃò_óùÉpÉåÉbÉgÇï‘Ç∑ÅiTControl::GetPaletteÇÃÉIÅ[ÉoÉâÉCÉhä÷êîÅj
HPALETTE __fastcall TMmttyWd::GetPalette(void)
{
	m_PaletteQuery = 1;
	if( sys.m_Palette ){
		if( UsrPal == NULL ){
			SetColorIndex();
		}
	}
	else {
		if( UsrPal != NULL ){
			ClosePalette();
		}
	}
	if( UsrPal != NULL ){
		m_PaletteTimer = 3;
	}
	return UsrPal;
}
//---------------------------------------------------------------------------
// ÉpÉåÉbÉgÇçÌèúÇ∑ÇÈ
void __fastcall TMmttyWd::ClosePalette(void)
{
	if( UsrPal != NULL ){
		pBitmapWater->Palette = 0;
		pBitmapFFTIN->Palette = 0;
		pBitmapXY->Palette = 0;
		if( pBitmapRx != NULL ) pBitmapRx->Palette = 0;
		if( pBitmapIn != NULL ) pBitmapIn->Palette = 0;
		::DeleteObject(UsrPal);
		UsrPal = NULL;
		sys.d_PaletteMask = 0x00000000;
	}
	if( pBitmapRx != NULL ){
		PBoxRx->Canvas->Brush->Bitmap = NULL;
		PrintText.SetBitmap(NULL);
		delete pBitmapRx;
		pBitmapRx = NULL;
	}
	if( pBitmapIn != NULL ){
		PBoxIn->Canvas->Brush->Bitmap = NULL;
		FifoEdit.SetBitmap(NULL);
		delete pBitmapIn;
		pBitmapIn = NULL;
	}
}
//---------------------------------------------------------------------------
// ÉpÉåÉbÉgÇìoò^Ç∑ÇÈ
void __fastcall TMmttyWd::SetupPalette(RGBQUAD *pTbl, int max)
{
	struct { /* lgpl */
		WORD         palVersion;
		WORD         palNumEntries;
		PALETTEENTRY palPalEntry[256];
	}logpal;

	logpal.palVersion = 0x300;
	logpal.palNumEntries = WORD(max);
	int n = 0;
	for( int i = 0; i < max; i++, n++ ){
		logpal.palPalEntry[i].peRed = pTbl[n].rgbRed;
		logpal.palPalEntry[i].peGreen = pTbl[n].rgbGreen;
		logpal.palPalEntry[i].peBlue = pTbl[n].rgbBlue;
		logpal.palPalEntry[i].peFlags = NULL; //PC_NOCOLLAPSE;  /*PC_EXPLICIT;*/
	}

	ClosePalette();
	UsrPal = ::CreatePalette((LOGPALETTE *)&logpal);
	if( UsrPal != NULL ){
		sys.d_PaletteMask = 0x02000000;
		pBitmapWater->Palette = UsrPal;
		pBitmapFFTIN->Palette = UsrPal;
		pBitmapXY->Palette = UsrPal;
		pBitmapWater->IgnorePalette = FALSE;
		pBitmapFFTIN->IgnorePalette = FALSE;
		pBitmapXY->IgnorePalette = FALSE;

		pBitmapRx = new Graphics::TBitmap();
		pBitmapRx->Width = 8;
		pBitmapRx->Height = 8;
		pBitmapRx->Palette = UsrPal;
		pBitmapIn = new Graphics::TBitmap();
		pBitmapIn->Width = 8;
		pBitmapIn->Height = 8;
		pBitmapIn->Palette = UsrPal;
		pBitmapRx->IgnorePalette = FALSE;
		pBitmapIn->IgnorePalette = FALSE;
		for( int y = 0; y < 8; y++ ){
			for( int x = 0; x < 8; x++ ){
				pBitmapRx->Canvas->Pixels[x][y] = TColor(sys.m_ColorRXBack | 0x02000000);
				pBitmapIn->Canvas->Pixels[x][y] = TColor(sys.m_ColorINBack | 0x02000000);
			}
		}
		PrintText.SetBitmap(pBitmapRx);
		FifoEdit.SetBitmap(pBitmapIn);
		PBoxRx->Invalidate();
		PBoxIn->Invalidate();
	}
}
//---------------------------------------------------------------------------
// ÉJÉâÅ[ÉCÉìÉfÉbÉNÉXÇìoò^Ç∑ÇÈ
// 256ColorÇÃÉJÉâÅ[ÉCÉìÉfÉbÉNÉXÇÕÇOÅ`ÇPÇUÇ‹Ç≈
int __fastcall TMmttyWd::EntryColor(RGBQUAD *pTbl, TColor col, int n)
{
	DWORD   dd = DWORD(col);        // xBGR

	RGBQUAD rq;

	rq.rgbBlue = BYTE(dd>>16);
	rq.rgbGreen = BYTE(dd>>8);
	rq.rgbRed = BYTE(dd);
	rq.rgbReserved = 0;
	if( !(n & 0x00ff00) ){
		for( int i = 0; i < 256; i++ ){
			if( (pTbl[i].rgbBlue == rq.rgbBlue) &&
				(pTbl[i].rgbGreen == rq.rgbGreen) &&
				(pTbl[i].rgbRed == rq.rgbRed)
			){
				return FALSE;
			}
		}
	}
	n &= 0x00ff;
	pTbl[n] = rq;
	return TRUE;
}
//---------------------------------------------------------------------------
// ÉrÉbÉgÉ}ÉbÉvÇÃÉJÉâÅ[ÉeÅ[ÉuÉãÇìoò^Ç∑ÇÈ
void __fastcall TMmttyWd::SetColorIndex(void)
{
	RGBQUAD tbl[256];
	memset(tbl, 0, sizeof(tbl));

		int n = 0;
		if( EntryColor(tbl, sys.m_ColorRXBack, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorRX, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorRXTX, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorINBack, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorIN, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorINTX, n) ) n++;
		if( EntryColor(tbl, sys.m_ColorXY, n) ) n++;
		for( int i = 0; i < 128; n++, i++ ){
			tbl[n].rgbRed  =(unsigned char)(ColorTable[n]);
			tbl[n].rgbGreen=(unsigned char)(ColorTable[n] >> 8);
			tbl[n].rgbBlue =(unsigned char)(ColorTable[n] >> 16);
		}
#if 0	// ÉJÉâÅ[ââéZÇçsÇÌÇ»Ç¢ÇÃÇ≈ïsóv

		// 16Å`255ÇÕÇ∑Ç◊Çƒçï
		for( ;n < 256; n++ ){
			memset(&tbl[n], 0, sizeof(RGBQUAD));
		}

		// ÉzÉèÉCÉgÅiïKÇ∏0xffÇ…ìoò^Åj
		EntryColor(tbl, clWhite, 0x01ff);

		// äÓñ{êFÇÃìoò^
		const TColor tt[]={
			clAqua, clBlack, clBlue, clDkGray, clFuchsia, clGray, clGreen, clLime,
			clLtGray, clMaroon, clNavy, clOlive, clPurple, clRed, clSilver, clTeal,
			clWhite, clYellow,
			clActiveBorder, clActiveCaption, clAppWorkSpace, clBackground,
			clBtnFace, clBtnHighlight, clBtnShadow, clBtnText, clCaptionText,
			clGrayText, clHighlight, clHighlightText, clInactiveBorder,
			clInactiveCaption, clInactiveCaptionText, clMenu, clMenuText,
			clScrollBar, clWindow, clWindowFrame, clWindowText,
		};
		n = 254;
		for( int i = 0; i < (sizeof(tt)/sizeof(TColor)); i++ ){
			if( EntryColor(tbl, tt[i], n) ) n--;
		}
#endif
		SetupPalette(tbl, n);
}
#endif
//---------------------------------------------------------------------------
// FFT IN MOUSE Event
void __fastcall TMmttyWd::PBoxFFTINMouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( pSound == NULL ) return;
	if( sys.m_echo != 2 ){
		ClearMacroTimer();
		if( SBTX->Down ){
			AdjustFocus();
			return;
		}
	}

	double mfq = pSound->GetScreenFreq(X, PBoxFFTIN->Width, KXYScope->Checked ? PBoxXY->Width : 0);
	if( Button == mbRight ){
		if( pSound->m_lms.m_Type){
			if( !SBLMS->Down ){
				pSound->m_lms.m_lmsNotch2 = 0;
				pSound->m_lms.m_lmsNotch = int(mfq + 0.5);
				SBLMS->Down = TRUE;
				SBLMSClick(NULL);
			}
			else {
				pSound->m_lms.m_lmsNotch2 = pSound->m_lms.m_lmsNotch;
				pSound->m_lms.m_lmsNotch = int(mfq + 0.5);
				pSound->CalcBPF();
			}
            RemoteFreq();
		}
	}
	else {
		mfq = double(int(mfq + 0.5));
		if( fabs(mfq - pSound->FSKDEM.GetMarkFreq()) >= 2 ){
			double sft = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
			double sfq = mfq + sft;
			if( (mfq >= MARKL) && (sfq <= SPACEH) ){
				pSound->FSKDEM.SetMarkFreq(mfq);
				pSound->FSKDEM.SetSpaceFreq(sfq);
				if( !SBTX->Down ){
					pSound->FSKMOD.SetMarkFreq(mfq);
					pSound->FSKMOD.SetSpaceFreq(sfq);
				}
				if( pRadio != NULL ) pRadio->SetMarkFreq(mfq);
				if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
				RemoteFreq();
				UpdateItem();
			}
		}
	}
	AdjustFocus();

}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::ScrollBarInChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	FifoEdit.ScrollBarChange();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBINClearClick(TObject *Sender)
{
	MsgList->Text = "";
	FifoEdit.Clear();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::WriteFifoEdit(LPCSTR s)
{
	while((*s == '\\')||(*s == '#') ){
		if( *s == '\\' ){
			FifoEdit.Clear();
		}
		else if( !SBTX->Down ){
			ToTX(s);
		}
		s++;
	}
	LPSTR bp = new char[strlen(s)+1];
	strcpy(bp, s);
	LPSTR p;
	for( p = bp; *p; p++ ){		// %E ÇÕèIóπÇíËã`èIóπ
		if( (*p == '%') && (*(p+1) == 'E') ){
			*p = 0;
			break;
		}
	}
	if( sys.m_MacroImm ){
		char bf[1024];
		ConvString(bf, bp, sizeof(bf), 1);
		FifoEdit.PutText(bf);
	}
	else {
		FifoEdit.PutText(bp);
	}
	delete bp;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN1Click(TObject *Sender)
{
	WriteFifoEdit(sys.m_InBtn[0].c_str());
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN2Click(TObject *Sender)
{
	WriteFifoEdit(sys.m_InBtn[1].c_str());
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN3Click(TObject *Sender)
{
	WriteFifoEdit(sys.m_InBtn[2].c_str());
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN4Click(TObject *Sender)
{
	WriteFifoEdit(sys.m_InBtn[3].c_str());
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBINMouseUp(int n)
{
	TEditDlgBox *pBox = new TEditDlgBox(this);
	pBox->Caption = (Font->Charset != SHIFTJIS_CHARSET)? "Edit Button":"ì¸óÕÉ{É^Éìï“èW";
	pBox->ButtonName->Text = sys.m_InBtnName[n];
	if( pBox->Execute(sys.m_InBtn[n], sys.m_InBtnKey[n], &sys.m_InBtnCol[n], NULL, 0) == TRUE ){
		if( !pBox->ButtonName->Text.IsEmpty() ){
			sys.m_InBtnName[n] = pBox->ButtonName->Text;
		}
	}
	delete pBox;
	UpdateMacro();
	UpdateShortCut(FALSE);
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN1MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) SBINMouseUp(0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN2MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) SBINMouseUp(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN3MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) SBINMouseUp(2);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBIN4MouseUp(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ) SBINMouseUp(3);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateMsgList(void)
{
	m_DisEvent++;
	MsgList->Clear();
	int i;
	for( i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		MsgList->Items->Add(sys.m_MsgName[i]);
	}
	UpdateShortCut(FALSE);
	m_DisEvent--;
}
//---------------------------------------------------------------------------
// ÉVÉáÅ[ÉgÉJÉbÉgÇ≈íTÇ∑
void __fastcall TMmttyWd::FindMsgLst(WORD nKey)
{
	for( int i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		if( sys.m_MsgKey[i] == nKey ){
			AnsiString as = sys.m_MsgName[i];
			FindMsgList(as);
		}
	}
}
//---------------------------------------------------------------------------
// ñºèÃÇ≈íTÇ∑
void __fastcall TMmttyWd::FindMsgList(AnsiString fs)	//JA7UDE 0428
{
	if( m_DisEvent ) return;
	m_DisEvent++;
	AnsiString	as;
	int i;
	for( i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		if( sys.m_MsgName[i] == fs ){
			as = sys.m_MsgList[i];
			WriteFifoEdit(as.c_str());
			EntryMsg(fs, as, sys.m_MsgKey[i]);
			MsgList->Text = fs;
			break;
		}
	}
	AdjustFocus();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::MsgListChange(TObject *Sender)
{
	FindMsgList(AnsiString(MsgList->Text));	//JA7UDE 0428
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBINEditClick(TObject *Sender)
{
	TEditDlgBox *pBox = new TEditDlgBox(this);

	pBox->EditMsg();
	delete pBox;
	UpdateMsgList();
	TopWindow(this);
}
//---------------------------------------------------------------------------
// ÉIÉvÉVÉáÉìÉÅÉjÉÖÅ[ÇäJÇ≠
void __fastcall TMmttyWd::DoOption(TObject *Sender, int sw)
{
//	CWaitCursor	wait;
	TOptionDlg *pBox = new TOptionDlg(this);
	m_hOptionWnd = pBox->Handle;

	m_DisEvent++;
	int fontadj = sys.m_FontAdjSize;
	int Palette = sys.m_Palette;
	pBox->CheckPalette->Enabled = m_PaletteQuery;
	if( Sender == KMacro ){
		PageIndex = 3;
	}
	int stereo = sys.m_SoundStereo;
	int fiforx = sys.m_SoundFifoRX;
	int fifotx = sys.m_SoundFifoTX;
	double txoffset = sys.m_TxOffset;
	AnsiString pttname = sys.m_TxRxName;
	AnsiString MMW = sys.m_SoundMMW;
	if( Remote ){
    	if( sw ) ::EnableWindow(APP_HANDLE, FALSE);
		PostApp(TXM_SHOWSETUP, 1);
    }
	int r = pBox->Execute(&pSound->FSKDEM, &pSound->FSKMOD);
	if( Remote && sw ) ::EnableWindow(APP_HANDLE, TRUE);
	if( r ){
		RemoteStat();
		if( r == 2 ){
			PrintText.SetPaintBox(PBoxRx, ScrollBarRx);
			FifoEdit.SetPaintBox(PBoxIn, ScrollBarIn);
		}
		PrintText.AdjZero();
		if( pBox->m_ColorChange ){
			InitColorTable(sys.m_ColorLow, sys.m_ColorHigh);
			UpdateColor();
		}
		if( Palette != sys.m_Palette ){
			if( !sys.m_Palette && (UsrPal != NULL) ){
				ClosePalette();
			}
			m_ReqPaletteChange = 1;
		}
		if( (int(pSound->m_IDDevice) != sys.m_SoundDevice) ||
			(fiforx != sys.m_SoundFifoRX) ||
			(fifotx != sys.m_SoundFifoTX) ||
			(txoffset != sys.m_TxOffset) ||
            (MMW != sys.m_SoundMMW) ||
			(stereo != sys.m_SoundStereo) ){
//			pSound->m_IDDevice = sys.m_SoundDevice;
			pSound->InitSound();
		}
		pSound->TaskPriority();
		UpdateSystemFont();
		if( sys.m_SampFreq != SampFreq ){
			if( Font->Charset != SHIFTJIS_CHARSET ){
				InfoMB( "Please restart %s for the new sampling frequency.", Remote ? "application":"MMTTY");
			}
			else {
				InfoMB( "êVÇµÇ¢ÉTÉìÉvÉäÉìÉOé¸îgêîÇìKópÇ≥ÇπÇÈÇΩÇﬂÇ…%sÇçƒãNìÆÇµÇƒâ∫Ç≥Ç¢.", Remote ? "ÉAÉvÉäÉPÅ[ÉVÉáÉì":"MMTTY" );
			}
		}
		if( (pttname != sys.m_TxRxName) || ((pComm == NULL)&&strcmp(sys.m_TxRxName.c_str(), "NONE")) ){
			COMM.change = 1;
		}
		if( pRadio != NULL ) pRadio->SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
		rtty.SetCodeSet(); rttysub.SetCodeSet();
	}
	delete pBox;
    m_hOptionWnd = NULL;
	UpdateItem();
	UpdateMacro();
	UpdateLMS();
	UpdateMsgList();
	UpdateLWait();
	if( fontadj != sys.m_FontAdjSize ){
		FormResize(NULL);
	}
	OpenCloseCom();
	m_DisEvent--;
	TopWindow(this);
	AdjustFocus();
	if( Remote ) PostApp(TXM_SHOWSETUP, 0);
}
//---------------------------------------------------------------------------
// ÉIÉvÉVÉáÉìÉÅÉjÉÖÅ[ÇäJÇ≠
void __fastcall TMmttyWd::KOptionClick(TObject *Sender)
{
	DoOption(Sender, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::HisCallKeyPress(TObject *Sender, char &Key)
{
	if( (Key == 0x0d)||(Key == 0x1b) ){
		if( (Key == 0x0d) && (Sender == HisCall) ){
			FindCall();
		}
		AdjustFocus();
		Key = 0;
	}
	else {
		Key = char(toupper(Key));
	}
}
//---------------------------------------------------------------------------
// ÉIÉVÉçÉXÉRÅ[Évï\é¶
void __fastcall TMmttyWd::KOSClick(TObject *Sender)
{
	TTScope *pBox = new TTScope(this);

	m_DisEvent++;
	pBox->Execute(&pSound->FSKDEM, &pSound->FSKMOD);
	delete pBox;
	pSound->FSKDEM.m_Scope = 0;
	UpdateItem();
	UpdateMacro();
	UpdateMsgList();
	TopWindow(this);
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTClick(TObject *Sender)
{
	if( m_DisEvent ) return;

	pSound->m_FFTSW = pSound->m_FFTSW ? 0 : 1;
	KFFT->Checked = pSound->m_FFTSW;
	if( pSound->m_FFTSW ){
		pSound->fftIN.m_FFTDIS = 0;
	}
	UpdateUI();
	PBoxFFTIN->Invalidate();
	PBoxWater->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWebHHTClick(TObject *Sender)
{
	//1.70J
	//WebRef.ShowHTML("http://www33.ocn.ne.jp/~je3hht/");
	WebRef.ShowHTML("http://mm-open.org/");
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWebJARTSClick(TObject *Sender)
{
	//1.70J
	//WebRef.ShowHTML("http://jarts.web.fc2.com/");
	WebRef.ShowHTML("http://jarts.jp/");
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWebGRPClick(TObject *Sender)
{
	WebRef.ShowHTML("http://groups.yahoo.com/group/MMTTY/join");
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWebENGClick(TObject *Sender)
{
	//1.70J
	//WebRef.ShowHTML("http://mmhamsoft.amateur-radio.ca/");
	WebRef.ShowHTML("http://hamsoft.ca/");
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWebWSKClick(TObject *Sender)
{
	//1.70J
	//WebRef.ShowHTML("http://home.b01.itscom.net/ja1wsk/");
	WebRef.ShowHTML("http://home.b01.itscom.net/ja1wsk/mmtty00.html");
}
//---------------------------------------------------------------------------
// MMTTY.TxtÇÃï\é¶ÉÅÉjÉÖÅ[
void __fastcall TMmttyWd::KHlpTxtClick(TObject *Sender)
{
	ShowHelp(this, sys.m_Help.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KHlpLogClick(TObject *Sender)
{
	ShowHelp(this, sys.m_HelpLog.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KHlpDigClick(TObject *Sender)
{
	ShowHelp(this, sys.m_HelpDigital.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TMmttyWd::KRMClick(TObject *Sender)
{
	ShowHelp(this, (Font->Charset != SHIFTJIS_CHARSET) ? "EUpdate.txt" : "Update.txt");
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KVolClick(TObject *Sender)
{
	HWND hWnd = ::FindWindow("Volume Control", NULL);
	if( hWnd != NULL ){
		::PostMessage(hWnd, WM_CLOSE, 0, 0);
		::Sleep(200);
	}
	char cmd[128];

    //AA6YQ 1.66G

    if (WinVista) {
        if (Sender == KVolIn) {
            strcpy(cmd, "control.exe mmsys.cpl,,1");
            }
        else {
            strcpy(cmd, "sndvol.exe");
            }
    }
    else {

	    strcpy(cmd, "SNDVOL32.EXE");
	    if( WinNT && (Sender == KVolIn) ) strcat(cmd, " /R");
    }

    WinExec(cmd, SW_SHOW);
    
	if((!WinVista) && (!WinNT) && (Sender == KVolIn) ){
		CWaitCursor w;
		int i;
		for( i = 0; i < 30; i++ ){
			::Sleep(100);
			hWnd = ::FindWindow("Volume Control", NULL);
			if( hWnd != NULL ) break;
		}
		if( i < 30 ){
			::SetForegroundWindow(hWnd);
			::Sleep(100);
			const short _tt[]={
				VK_MENU, 'P', 'P'|0x8000, VK_MENU|0x8000,
				'R', 'R'|0x8000, VK_TAB, VK_TAB|0x8000,
				VK_DOWN, VK_DOWN|0x8000, VK_RETURN, VK_RETURN|0x8000,
				0
			};
			KeyEvent(_tt);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRcvLogClick(TObject *Sender)
{
	sys.m_log = sys.m_log ? 0 : 1;
	PrintText.MakeLogName();
	PrintText.OpenLogFile();
	if( sys.m_logTimeStamp ) PrintText.TrigTimeStamp("Start Logging");
}
//---------------------------------------------------------------------------
int __fastcall TMmttyWd::OutputFile(LPCSTR pName)
{
#if 1
	if( p_SendFileDlg != NULL ){
		delete p_SendFileDlg;
		p_SendFileDlg = NULL;
	}
	p_SendFileDlg = new TSendFileDlg(this);
	if( p_SendFileDlg->Start(pName, Top + (Height - ClientHeight) + PanelStat->Top) == FALSE ){
		delete p_SendFileDlg;
		p_SendFileDlg = NULL;
	}
	else {
		ToTX(NULL);
	}
#else
	char	bf[2048];
	AnsiString	in;
	FILE	*fp;

	CWaitCursor w;
	if( (fp = fopen(pName, "rt"))!=NULL ){
		while(!feof(fp)){
			if( fgets(bf, 2048, fp) != NULL ){
				ClipLF(bf);
				in += bf;
				in += "\r\n";
			}
		}
		fclose(fp);
	}
	else {
		ErrorMB((Font->Charset != SHIFTJIS_CHARSET) ? "'%s' was not found":"'%s'Ç™å©Ç¬Ç©ÇËÇ‹ÇπÇÒ.", pName);
		return FALSE;
	}
	ToTX(in.c_str());
	OutputStr(in.c_str());
#endif
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFileOutClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if(Font->Charset != SHIFTJIS_CHARSET){
		OpenDialog->Title = "Open Text file for TX";
		OpenDialog->Filter = "Text files(*.txt)|*.txt|All files(*.*)|*.*|";
	}
	else {
		OpenDialog->Title = "ëóêMÇ∑ÇÈÉeÉLÉXÉgÉtÉ@ÉCÉãÇäJÇ≠";
		OpenDialog->Filter = "ÉeÉLÉXÉgÉtÉ@ÉCÉã(*.txt)|*.txt|Ç∑Ç◊ÇƒÇÃÉtÉ@ÉCÉã(*.*)|*.*|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "txt";
	OpenDialog->InitialDir = OutFileDir;
	DisPaint = TRUE;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		if( OutputFile(AnsiString(OpenDialog->FileName).c_str()) == TRUE ){	//JA7UDE 0428
			SetDirName(OutFileDir, AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
		}
	}
	TopWindow(this);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSaveRxClick(TObject *Sender)
{
	if( (Font->Charset != SHIFTJIS_CHARSET) ){
		SaveDialog->Title = "Write Text file";
		SaveDialog->Filter = "Text files(*.txt)|*.txt|";
	}
	else {
		SaveDialog->Title = "ÉeÉLÉXÉgÉtÉ@ÉCÉãÇÃçÏê¨";
		SaveDialog->Filter = "ÉeÉLÉXÉgÉtÉ@ÉCÉã(*.txt)|*.txt|";
	}
	SaveDialog->FileName = "Recv.txt";
	SaveDialog->DefaultExt = "txt";
	SaveDialog->InitialDir = OutFileDir;
	DisPaint = TRUE;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		CWaitCursor w;
		AnsiString as;
		PrintText.GetString(as);
		FILE *fp = fopen(AnsiString(SaveDialog->FileName).c_str(), "wb");	//JA7UDE 0428
		if( fp != NULL ){
			fputs(as.c_str(), fp);
			if( fclose(fp) ){
				ErrorMB( (Font->Charset != SHIFTJIS_CHARSET) ? "Write Error to '%s'":"'%s'Ç…ê≥ÇµÇ≠èëÇ´Ç±ÇﬂÇ‹ÇπÇÒÇ≈ÇµÇΩ.", SaveDialog->FileName.c_str());
			}
			else {
				SetDirName(OutFileDir, AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
			}
		}
		else {
			ErrorMB((Font->Charset != SHIFTJIS_CHARSET)?"Write Error to '%s'": "'%s'ÇçÏê¨Ç≈Ç´Ç‹ÇπÇÒ.", SaveDialog->FileName.c_str());
		}
	}
	TopWindow(this);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KLogClick(TObject *Sender)
{
	DisPaint = TRUE;
	TLogListDlg *pBox = new TLogListDlg(this);
	pBox->Execute();
	Application->OnIdle = OnIdle;
	delete pBox;
	UpdateItem();
	UpdateLogLink();
	AdjustFocus();
	TopWindow(this);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateLogData(void)
{
	StrCopy(Log.m_sd.call, AnsiString(HisCall->Text).c_str(), MLCALL);	//JA7UDE 0428
	jstrupr(Log.m_sd.call);
	StrCopy(Log.m_sd.name, AnsiString(HisName->Text).c_str(), MLNAME);	//JA7UDE 0428
	StrCopy(Log.m_sd.qth, AnsiString(HisQTH->Text).c_str(), MLQTH);	//JA7UDE 0428
	StrCopy(Log.m_sd.my, AnsiString(MyRST->Text).c_str(), MLRST);	//JA7UDE 0428
	jstrupr(Log.m_sd.my);
	StrCopy(Log.m_sd.ur, AnsiString(HisRST->Text).c_str(), MLRST);	//JA7UDE 0428
	jstrupr(Log.m_sd.ur);
	Log.SetFreq(&Log.m_sd, AnsiString(Freq->Text).c_str());	//JA7UDE 0428
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateTextData(void)
{
	HisCall->Text = Log.m_sd.call;
	HisName->Text = Log.m_sd.name;
	HisQTH->Text = Log.m_sd.qth;
	HisRST->Text = Log.m_sd.ur;
	MyRST->Text = Log.m_sd.my;
	Freq->Text = Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq);
	Log.m_CurChg = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KQSOClick(TObject *Sender)
{
	UpdateLogData();
	if( strcmp(Log.m_Find.GetText(), Log.m_sd.call) ){
		CWaitCursor w;
		Log.FindSet(&Log.m_Find, Log.m_sd.call);
	}
	TQSODlgBox *pBox = new TQSODlgBox(this);
	if( pBox->Execute(&Log.m_Find, &Log.m_sd, Log.m_CurNo) == TRUE ){
		UpdateTextData();
		LogLink.Write(&Log.m_sd, 0);
	}
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateCallsign(void)
{
	if( HisCall->Text.IsEmpty() ) m_Dupe = 0;
	HisCall->Font->Color = m_Dupe ? clRed : clBlack;
	LCall->Font->Color = m_Running ? clRed : clBlack;
	KRun->Checked = m_Running;
}
//---------------------------------------------------------------------------
typedef struct {
	int		cnt;
	int		v;
	char	call[MLCALL+1];
}CLDT;
//---------------------------------------------------------------------------
static int _cmp(const void *s, const void *t)
{
	const CLDT *sp = (const CLDT *)s;
	const CLDT *tp = (const CLDT *)t;
	if( sp->cnt != tp->cnt ){
		return tp->cnt - sp->cnt;
	}
	else {
		return tp->v - sp->v;
	}
}
//---------------------------------------------------------------------------
static int IsNGCall(LPCSTR s)
{
	LPCSTR	tt[]={
		"PSE","QTH","QSO","KKK","NAME",NULL
	};
	for( int i = 0; tt[i] != NULL; i++ ){
		if( !strcmp(s, tt[i]) ) return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::CallCapture(void)
{
	CLDT	list[16];
	char	bf[1024];

	LPSTR	p, t, tt;
	int n = 0;
	int max = 0;
	memset(list, 0, sizeof(list));
	while(PrintText.GetText(bf, n) && (max < 16) ){
		n++;
		p = bf;
		int l = 0;
		while(*p){
			p = StrDlm(t, p, ' ');

			for( ; *t && (!IsCallChar(*t)); t++ );
			for( tt = t; *tt && IsCallChar(*tt); tt++);
			*tt = 0;

			if( (strlen(t) >= 3) && IsAlphas(t) && (strlen(t) < MLCALL) && ((strlen(t)<=7)||(strchr(t, '/')!=NULL)) ){
				if( !IsNGCall(t) && strcmp(t, sys.m_Call.c_str()) ){
					if( HisCall->Text.IsEmpty() || strcmp(t, AnsiString(HisCall->Text).c_str()) ){	//JA7UDE 0428
						int j;
						for( j = 0; j < max; j++ ){
							if( !strcmp(t, list[j].call) ){
								list[j].cnt++;
								break;
							}
						}
						if( j == max ){
							if( max < 16 ){
								strcpy(list[max].call, t);
								list[max].cnt = 1;
								list[max].v = l - (n*4);
								if( !IsCall(t) ){
									list[max].cnt -= 128;
								}
								if( Log.IsAlready(t) != -1 ){
									list[max].cnt -= 64;
								}
								max++;
								l++;
							}
						}
					}
				}
			}
		}
	}
	if( max ){
		qsort(list, max, sizeof(CLDT), _cmp);
		HisCall->Text = list[0].call;
#if 0
		FILE	*fp = fopen("F:\\LOG.TXT", "wt");
		for( int i = 0; i < max; i++ ){
			fprintf(fp, "[%s]\t%d,%d\n", list[i].call, list[i].cnt, list[i].v);
		}
		fclose(fp);
#endif
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::QSOIN(void)
{
	if( HisCall->Text.IsEmpty() ){
		CallCapture();
	}
	if( HisCall->Text.IsEmpty() ) return;
	SBQSO->Down = TRUE;
	ClearMacroTimer();
	UpdateLogData();
	SYSTEMTIME	now;
	char bf[256];
	GetUTC(&now);
	int Year = now.wYear % 100;
	int Month = now.wMonth;
	int Day = now.wDay;
	int Hour = now.wHour;
	UTCtoJST(Year, Month, Day, Hour);
	LogLink.SetTime(&now, 0);

	Log.m_sd.cq = m_Running ? 'A' : 'C';
	Log.m_sd.year = char(Year % 100);
	Log.m_sd.date = WORD(Month * 100 + Day);
	Log.m_sd.btime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
	if( !Log.m_sd.btime ) Log.m_sd.btime++;
	CWaitCursor w;
	if( Log.FindSet(&Log.m_Find, Log.m_sd.call) ){
		SDMMLOG	sd;
		Log.GetData(&sd, Log.m_Find.pFindTbl[0]);	// àÍî‘ç≈êVÇÃÉfÅ[É^
		if( !Log.m_sd.name[0] && Log.m_LogSet.m_CopyName ) strcpy(Log.m_sd.name, sd.name);
		if( !Log.m_sd.qth[0] && Log.m_LogSet.m_CopyQTH ) strcpy(Log.m_sd.qth, sd.qth);
		if( !Log.m_sd.rem[0] && Log.m_LogSet.m_CopyREM ) strcpy(Log.m_sd.rem, sd.rem);
		if( !Log.m_sd.qsl[0] && Log.m_LogSet.m_CopyQSL ) strcpy(Log.m_sd.qsl, sd.qsl);
	}
	Log.CopyAF();
	if( Log.m_sd.call[0] ){
		LPCSTR pCC = ClipCC(Log.m_sd.call);
		Log.SetOptStr(0, &Log.m_sd, Cty.GetCountry(pCC));
		Log.SetOptStr(1, &Log.m_sd, Cty.GetCont(pCC));
	}
	if( Log.m_LogSet.m_CopyHis == 2 ) Log.SetHisUTC();
	UpdateTextData();
	if( Log.PutData(&Log.m_sd, Log.m_CurNo) == FALSE ){
		SBQSO->Down = FALSE;
	}

	if( Log.m_Find.m_FindCmp1Max && ((!Log.m_LogSet.m_CheckBand) || Log.FindSameBand()) ){
		m_Dupe = 1;
	}
	else {
		m_Dupe = 0;
	}
	if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
		if( m_Running ){	// Running
			if( m_Dupe ){
				if( Log.m_LogSet.m_QSOMacro[2] && !Log.m_LogSet.m_QSOMacroStr[2].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[2].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[2].c_str());
				}
			}
			else {
				if( Log.m_LogSet.m_QSOMacro[0] && !Log.m_LogSet.m_QSOMacroStr[0].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[0].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[0].c_str());
				}
			}
		}
		else {				// S & P
			if( Log.m_LogSet.m_QSOMacro[3] && !Log.m_LogSet.m_QSOMacroStr[3].IsEmpty() ){
				ToTX(Log.m_LogSet.m_QSOMacroStr[3].c_str());
				OutputStr(Log.m_LogSet.m_QSOMacroStr[3].c_str());
			}
		}
	}

	Log.m_Find.Ins(Log.m_CurNo);
	Log.m_CurChg = 0;
	sprintf(bf, "Start QSO With %s", Log.m_sd.call);
	PrintText.TrigTimeStamp(bf);
	LogLink.Write(&Log.m_sd, 1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::QSOOUT(int sw)
{
	if( sw && !SBQSO->Down ){
		if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
			if( m_Running ){			// Running
				if( Log.m_LogSet.m_QSOMacro[1] && !Log.m_LogSet.m_QSOMacroStr[1].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[1].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[1].c_str());
				}
			}
			else {						// S & P
				if( Log.m_LogSet.m_QSOMacro[4] && !Log.m_LogSet.m_QSOMacroStr[4].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[4].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[4].c_str());
				}
			}
		}
		return;
	}
	SBQSO->Down = FALSE;
	ClearMacroTimer();
	UpdateLogData();
	SYSTEMTIME	now;
	char bf[256];
	GetUTC(&now);
	int Year = now.wYear % 100;
	int Month = now.wMonth;
	int Day = now.wDay;
	int Hour = now.wHour;
	UTCtoJST(Year, Month, Day, Hour);
	LogLink.SetTime(&now, 1);

	Log.m_sd.cq = m_Running ? 'A' : 'C';
	Log.m_sd.etime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
	if( !Log.m_sd.etime ) Log.m_sd.etime++;
	int r;
	if( !Log.m_sd.ur[0] ){
		if( Log.m_LogSet.m_Contest == 3 ){
			strcpy(Log.m_sd.ur, "599");
			r = IDYES;
		}
		else if( (Font->Charset != SHIFTJIS_CHARSET) ){
			r = YesNoCancelMB("No HisRST... Are you sure?");
		}
		else {
			r = YesNoCancelMB("HisRSTÇ™ê›íËÇ≥ÇÍÇƒÇ¢Ç‹ÇπÇÒ.\r\n\r\nÇ±ÇÃÉfÅ[É^ÇóLå¯Ç…ÇµÇ‹Ç∑Ç©ÅH");
		}
		switch(r){
			case IDCANCEL:
				Log.m_sd.etime = 0;
				SBQSO->Down = TRUE;
				return;
			case IDNO:
				Log.DeleteLast();
				Log.m_Find.Clear();
				Log.InitCur();
				UpdateTextData();
				AutoLogSave();
				return;
			default:
				break;
		}
	}
	if( !Log.m_sd.my[0] ){
		if( Log.m_LogSet.m_Contest == 3 ){
			strcpy(Log.m_sd.my, "599");
			r = IDYES;
		}
		else if( (Font->Charset != SHIFTJIS_CHARSET) ){
			r = YesNoCancelMB("No MyRST... Are you sure?");
		}
		else {
			r = YesNoCancelMB("MyRSTÇ™ê›íËÇ≥ÇÍÇƒÇ¢Ç‹ÇπÇÒ.\r\n\r\nÇ±ÇÃÉfÅ[É^ÇóLå¯Ç…ÇµÇ‹Ç∑Ç©ÅH");
		}
		switch(r){
			case IDCANCEL:
				Log.m_sd.etime = 0;
				SBQSO->Down = TRUE;
				return;
			case IDNO:
				Log.DeleteLast();
				Log.m_Find.Clear();
				Log.InitCur();
				UpdateTextData();
				AutoLogSave();
				return;
			default:
				break;
		}
	}
	if( !Log.m_sd.ur[0] || !Log.m_sd.my[0] ){	// ñ≥å¯ÉfÅ[É^
		Log.m_sd.send = 'I';
	}
	Log.PutData(&Log.m_sd, Log.m_CurNo);
	LogLink.Write(&Log.m_sd, 2);
	sprintf(bf, "Exit QSO With %s %s %s %s",
		Log.m_sd.call, Log.m_sd.ur, Log.m_sd.my, Log.m_sd.name, Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq)
	);
	PrintText.TrigTimeStamp(bf);

	if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
		if( m_Running ){			// Running
			if( Log.m_LogSet.m_QSOMacro[1] && !Log.m_LogSet.m_QSOMacroStr[1].IsEmpty() ){
				ToTX(Log.m_LogSet.m_QSOMacroStr[1].c_str());
				OutputStr(Log.m_LogSet.m_QSOMacroStr[1].c_str());
			}
		}
		else {						// S & P
			if( Log.m_LogSet.m_QSOMacro[4] && !Log.m_LogSet.m_QSOMacroStr[4].IsEmpty() ){
				ToTX(Log.m_LogSet.m_QSOMacroStr[4].c_str());
				OutputStr(Log.m_LogSet.m_QSOMacroStr[4].c_str());
			}
		}
	}

	memcpy(&Log.m_asd, &Log.m_sd, sizeof(Log.m_asd));
	Log.m_CurNo++;
	Log.m_CurChg = 0;
	Log.m_Find.Clear();
	Log.InitCur();
	UpdateTextData();
	AutoLogSave();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBQSOClick(TObject *Sender)
{
	ClearMacroTimer();
	UpdateLogData();
	SYSTEMTIME	now;
	char bf[256];
	GetUTC(&now);
	int Year = now.wYear % 100;
	int Month = now.wMonth;
	int Day = now.wDay;
	int Hour = now.wHour;
	UTCtoJST(Year, Month, Day, Hour);

	Log.m_sd.cq = m_Running ? 'A' : 'C';
	if( SBQSO->Down ){		// Start QSO
		LogLink.SetTime(&now, 0);
		Log.m_sd.year = char(Year % 100);
		Log.m_sd.date = WORD(Month * 100 + Day);
		Log.m_sd.btime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
		if( !Log.m_sd.btime ) Log.m_sd.btime++;
		CWaitCursor w;
		if( Log.FindSet(&Log.m_Find, Log.m_sd.call) ){
			SDMMLOG	sd;
			Log.GetData(&sd, Log.m_Find.pFindTbl[0]);	// àÍî‘ç≈êVÇÃÉfÅ[É^
			if( !Log.m_sd.name[0] && Log.m_LogSet.m_CopyName ) strcpy(Log.m_sd.name, sd.name);
			if( !Log.m_sd.qth[0] && Log.m_LogSet.m_CopyQTH ) strcpy(Log.m_sd.qth, sd.qth);
			if( !Log.m_sd.rem[0] && Log.m_LogSet.m_CopyREM ) strcpy(Log.m_sd.rem, sd.rem);
			if( !Log.m_sd.qsl[0] && Log.m_LogSet.m_CopyQSL ) strcpy(Log.m_sd.qsl, sd.qsl);
		}
		Log.CopyAF();
		if( Log.m_sd.call[0] ){
			LPCSTR pCC = ClipCC(Log.m_sd.call);
			Log.SetOptStr(0, &Log.m_sd, Cty.GetCountry(pCC));
			Log.SetOptStr(1, &Log.m_sd, Cty.GetCont(pCC));
		}
		if( Log.m_LogSet.m_CopyHis == 2 ) Log.SetHisUTC();
		UpdateTextData();
		if( Log.PutData(&Log.m_sd, Log.m_CurNo) == FALSE ){
			SBQSO->Down = FALSE;
		}

		if( Log.m_Find.m_FindCmp1Max && ((!Log.m_LogSet.m_CheckBand) || Log.FindSameBand()) ){
			m_Dupe = 1;
		}
		else {
			m_Dupe = 0;
		}
		if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
			if( m_Running ){	// Running
				if( m_Dupe ){
					if( Log.m_LogSet.m_QSOMacro[2] && !Log.m_LogSet.m_QSOMacroStr[2].IsEmpty() ){
						ToTX(Log.m_LogSet.m_QSOMacroStr[2].c_str());
						OutputStr(Log.m_LogSet.m_QSOMacroStr[2].c_str());
					}
				}
				else {
					if( Log.m_LogSet.m_QSOMacro[0] && !Log.m_LogSet.m_QSOMacroStr[0].IsEmpty() ){
						ToTX(Log.m_LogSet.m_QSOMacroStr[0].c_str());
						OutputStr(Log.m_LogSet.m_QSOMacroStr[0].c_str());
					}
				}
			}
			else {				// S & P
				if( Log.m_LogSet.m_QSOMacro[3] && !Log.m_LogSet.m_QSOMacroStr[3].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[3].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[3].c_str());
				}
			}
		}

		Log.m_Find.Ins(Log.m_CurNo);
		Log.m_CurChg = 0;
		sprintf(bf, "Start QSO With %s", Log.m_sd.call);
		PrintText.TrigTimeStamp(bf);
		LogLink.Write(&Log.m_sd, 1);
	}
	else {					// Finish QSO
		LogLink.SetTime(&now, 1);
		Log.m_sd.etime = WORD((Hour * 60 + now.wMinute) * 30 + now.wSecond/2);
		if( !Log.m_sd.etime ) Log.m_sd.etime++;
		int r;
		if( !Log.m_sd.ur[0] ){
			if( Log.m_LogSet.m_Contest == 3 ){
				strcpy(Log.m_sd.ur, "599");
				r = IDYES;
			}
			else if( (Font->Charset != SHIFTJIS_CHARSET) ){
				r = YesNoCancelMB("No HisRST... Are you sure?");
			}
			else {
				r = YesNoCancelMB("HisRSTÇ™ê›íËÇ≥ÇÍÇƒÇ¢Ç‹ÇπÇÒ.\r\n\r\nÇ±ÇÃÉfÅ[É^ÇóLå¯Ç…ÇµÇ‹Ç∑Ç©ÅH");
			}
			switch(r){
				case IDCANCEL:
					Log.m_sd.etime = 0;
					SBQSO->Down = TRUE;
					return;
				case IDNO:
					Log.DeleteLast();
					Log.m_Find.Clear();
					Log.InitCur();
					UpdateTextData();
					AutoLogSave();
					return;
				default:
					break;
			}
		}
		if( !Log.m_sd.my[0] ){
			if( Log.m_LogSet.m_Contest == 3 ){
				strcpy(Log.m_sd.my, "599");
				r = IDYES;
			}
			else if( (Font->Charset != SHIFTJIS_CHARSET) ){
				r = YesNoCancelMB("No MyRST... Are you sure?");
			}
			else {
				r = YesNoCancelMB("MyRSTÇ™ê›íËÇ≥ÇÍÇƒÇ¢Ç‹ÇπÇÒ.\r\n\r\nÇ±ÇÃÉfÅ[É^ÇóLå¯Ç…ÇµÇ‹Ç∑Ç©ÅH");
			}
			switch(r){
				case IDCANCEL:
					Log.m_sd.etime = 0;
					SBQSO->Down = TRUE;
					return;
				case IDNO:
					Log.DeleteLast();
					Log.m_Find.Clear();
					Log.InitCur();
					UpdateTextData();
					AutoLogSave();
					return;
				default:
					break;
			}
		}
		if( !Log.m_sd.ur[0] || !Log.m_sd.my[0] ){	// ñ≥å¯ÉfÅ[É^
			Log.m_sd.send = 'I';
		}
		Log.PutData(&Log.m_sd, Log.m_CurNo);
		LogLink.Write(&Log.m_sd, 2);
		sprintf(bf, "Exit QSO With %s %s %s %s",
			Log.m_sd.call, Log.m_sd.ur, Log.m_sd.my, Log.m_sd.name, Log.GetFreqString(Log.m_sd.band, Log.m_sd.fq)
		);
		PrintText.TrigTimeStamp(bf);

		if( Log.m_LogSet.m_QSOMacroFlag ){	// Auto running Macro
			if( m_Running ){			// Running
				if( Log.m_LogSet.m_QSOMacro[1] && !Log.m_LogSet.m_QSOMacroStr[1].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[1].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[1].c_str());
				}
			}
			else {						// S & P
				if( Log.m_LogSet.m_QSOMacro[4] && !Log.m_LogSet.m_QSOMacroStr[4].IsEmpty() ){
					ToTX(Log.m_LogSet.m_QSOMacroStr[4].c_str());
					OutputStr(Log.m_LogSet.m_QSOMacroStr[4].c_str());
				}
			}
		}

		memcpy(&Log.m_asd, &Log.m_sd, sizeof(Log.m_asd));
		Log.m_CurNo++;
		Log.m_CurChg = 0;
		Log.m_Find.Clear();
		Log.InitCur();
		UpdateTextData();
		AutoLogSave();
	}
}
//---------------------------------------------------------------------------
// ÉoÉìÉhÇÃïœçX
void __fastcall TMmttyWd::FreqChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	Log.SetFreq(&Log.m_sd, AnsiString(Freq->Text).c_str());	//JA7UDE 0428
	LogLink.SetFreq(AnsiString(Freq->Text).c_str());	//JA7UDE 0428
	if( !HisCall->Text.IsEmpty() ){
		FindCall();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBFindClick(TObject *Sender)
{
	if( !Log.IsOpen() ){
		if( Log.Open(NULL, TRUE) == FALSE ){
			return;
		}
	}
	FindCall();
	TQSODlgBox *pBox = new TQSODlgBox(this);
	pBox->ShowFind(&Log.m_Find);
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBFindMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( (Button == mbRight) && (!HisCall->Text.IsEmpty()) ){
		FindCall();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::HisCallChange(TObject *Sender)
{
	Log.m_Find.ClearText();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::LNameClick(TObject *Sender)
{
	m_NameQTH = m_NameQTH ? 0 : 1;
	UpdateUI();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBInitClick(TObject *Sender)
{
	AnsiString	as = Freq->Text;
	if( SBQSO->Down ){
		int r;
		if( Log.m_LogSet.m_Contest ){
			r = IDYES;
		}
		else {
			r = YesNoMB( (Font->Charset != SHIFTJIS_CHARSET) ? "Delete on this QSO. Are you sure?":"Ç±ÇÃQSOÇñ≥å¯Ç…ÇµÇ‹Ç∑Ç©ÅH" );
		}
		if( r == IDYES ){
			Log.DeleteLast();
			Log.m_Find.Clear();
			Log.InitCur();
			SBQSO->Down = FALSE;
			UpdateTextData();
		}
	}
	else {
		Log.InitCur();
		Log.m_sd.call[0] = 0;
		Log.m_sd.name[0] = 0;
		Log.m_sd.qth[0] = 0;
		UpdateTextData();
	}
	if( !as.IsEmpty() ){
		Freq->Text = as;
		Log.SetFreq(&Log.m_sd, as.c_str());
	}
#if 0
	HisCall->Text = "";
	HisName->Text = "";
	HisQTH->Text = "";
	MyRST->Text = "";
	HisRST->Text = Log.m_sd.ur;
	UpdateLogData();
#endif
	LogLink.Clear();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxInClick(TObject *Sender)
{
	ClearMacroTimer();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBHAMClick(TObject *Sender)
{
	CFSKMOD *mp = &pSound->FSKMOD;
	CFSKDEM *dp = &pSound->FSKDEM;
	dp->SetMarkFreq(sys.m_DefMarkFreq);
	mp->SetMarkFreq(sys.m_DefMarkFreq);
	dp->SetSpaceFreq(sys.m_DefMarkFreq + sys.m_DefShift);
	mp->SetSpaceFreq(sys.m_DefMarkFreq + sys.m_DefShift);
	if( pRadio != NULL ) pRadio->SetMarkFreq(sys.m_DefMarkFreq);
	if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
	if( sys.m_TxPort ){
		if( sys.m_DefFix45 ){
			if( dp->GetBaudRate() != 45.45 ) COMM.change = 1;
        }
		if( dp->m_BitLen != 5 ) COMM.change = 1;
		if( dp->m_StopLen != (sys.m_DefStopLen + 3) ) COMM.change = 1;
		if( dp->m_Parity != 0 ) COMM.change = 1;
	}
	if( sys.m_DefFix45 ) dp->SetBaudRate(45.45);
	dp->m_BitLen = 5;
	dp->m_StopLen = sys.m_DefStopLen + 3;
	dp->m_Parity = 0;
	if( sys.m_DefFix45 ) mp->SetBaudRate(45.45);
	mp->m_BitLen = 5;
	mp->m_StopLen = sys.m_DefStopLen + 3;
	mp->m_Parity = 0;
	UpdateFSK();
	UpdateItem();
	AdjustFocus();
	RemoteStat();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KOpenLogClick(TObject *Sender)
{
	OpenDialog->Options << ofCreatePrompt;
	OpenDialog->Options >> ofFileMustExist;
	if( (Font->Charset != SHIFTJIS_CHARSET) ){
		OpenDialog->Title = "Open LogData File";
		OpenDialog->Filter = "MMLOG Data Files(*.mdt)|*.mdt|";
	}
	else {
		OpenDialog->Title = "ÉçÉOÉtÉ@ÉCÉãÇÃÉIÅ[ÉvÉì";
		OpenDialog->Filter = "MMLOGÉfÅ[É^ÉtÉ@ÉCÉã(*.mdt)|*.mdt|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "mdt";
	OpenDialog->InitialDir = MMLogDir;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		Log.Close();
        Log.DoBackup();
		Log.MakeName(AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
		Log.Open(NULL, TRUE);
		UpdateItem();
		KLogClick(NULL);
	}
	TopWindow(this);
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::AutoLogSave(void)
{
	if( Log.IsOpen() && Log.m_LogSet.m_AutoSave && Log.IsEdit() ) KFlushClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFlushClick(TObject *Sender)
{
	Log.Close();
	Log.Open(NULL, TRUE);
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTWAClick(TObject *Sender)
{
	pSound->m_FFTFW = 0;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTW05Click(TObject *Sender)
{
	pSound->m_FFTFW = 1;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTW1Click(TObject *Sender)
{
	pSound->m_FFTFW = 2;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTW15Click(TObject *Sender)
{
	pSound->m_FFTFW = 3;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTW2Click(TObject *Sender)
{
	pSound->m_FFTFW = 4;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTW3Click(TObject *Sender)
{
	pSound->m_FFTFW = 5;
	pSound->DrawFFT(pBitmapFFTIN, 1, KXYScope->Checked ? PBoxXY->Width : 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::FormShow(TObject *Sender)
{
	if( Remote & REMSHOWOFF ){
		Hide();
	}
	else {
		AdjustFocus();
	}
}
//---------------------------------------------------------------------------
// HisRST Box
void __fastcall TMmttyWd::KDispVerClick(TObject *Sender)
{
	TVerDspDlg *pBox = new TVerDspDlg(this);
	pBox->ShowModal();
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KClrRxClick(TObject *Sender)
{
	PrintText.Clear();
}
//---------------------------------------------------------------------------
int __fastcall TMmttyWd::DrawXY(void)
{
	if( pSound == NULL ) return FALSE;

	TCanvas *pCanvas = pBitmapXY->Canvas;
	CScope *mp = &pSound->FSKDEM.m_XYScopeMark;
	CScope *sp = &pSound->FSKDEM.m_XYScopeSpace;

	TRect rc;
	int XL = 0;
	int XR = pBitmapXY->Width - 1;
	int YT = 0;
	int YB = pBitmapXY->Height - 1;
	int	XC = XR/2;
	int YC = YB/2;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	pCanvas->Brush->Color = clBlack;
	pCanvas->FillRect(rc);

	if( !pSound->FSKDEM.m_XYScope ) return FALSE;
	if( !sp->GetFlag() ) return FALSE;


	double	dmax = 0;
	double	dm, ds;
	int i, x, y;

	if( Remote && (pMap != NULL) && !(Remote & REMDISSHARED) ){
		if( !pMap->flagXY ){
#if 1
			double *pm = mp->pScopeData;
			double *ps = sp->pScopeData;
			LONG *tm = pMap->arrayX;
			LONG *ts = pMap->arrayY;
			if( pSound->FSKDEM.m_Limit ){
				for( i = 0; i < XYCOLLECT; i++ ){
					*tm++ = LONG(*pm++);
					*ts++ = LONG(*ps++);
				}
			}
			else {
				for( i = 0; i < XYCOLLECT; i++ ){
					*tm++ = LONG(*pm++) * 16384/2048;
					*ts++ = LONG(*ps++) * 16384/2048;
				}
			}
#else
			if( pSound->FSKDEM.m_Limit ){
				for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
					pMap->arrayX[i] = int(mp->pScopeData[i]);
					pMap->arrayY[i] = int(sp->pScopeData[i]);
				}
			}
			else {
				for( i = 0; i < XYCOLLECT; i++ ){
					pMap->arrayX[i] = int(mp->pScopeData[i]) * 16384/2048;
					pMap->arrayY[i] = int(sp->pScopeData[i]) * 16384/2048;
				}
			}
#endif
			pMap->flagXY = 1;
		}
	}
	if( Remote & REMSHOWOFF ){
		pSound->FSKDEM.m_XYScope = 0;
		mp->Collect(XYCOLLECT);
		sp->Collect(XYCOLLECT);
		pSound->FSKDEM.m_XYScope = 1;
		return FALSE;
	}
	if( pTnc && pTnc->IsMMT() ){
		pTnc->NotifyXY(mp->pScopeData, sp->pScopeData);
    }

	double *pm = mp->pScopeData;
	double *ps = sp->pScopeData;
	for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
		dm = fabs(*pm);
		ds = fabs(*ps);
		if( dmax < dm ) dmax = dm;
		if( dmax < ds ) dmax = ds;
	}
	if( pSound->FSKDEM.m_Limit ){
		if( dmax < 16384.0 ) dmax = 16384.0;
	}
	else {
		if( dmax < 2048.0 ) dmax = 2048.0;
	}
	int YW = YC * 0.8;
	int XW = XC * 0.8;
	TColor col = TColor(sys.m_ColorXY  | sys.d_PaletteMask);
	pCanvas->Pen->Color = col;

	const double _mt[]={1.25, 1.20, 1.20, 1.25};
	double	sc;

	double	SX = (XW / dmax);
	double	SY = (YW / dmax);
	double	dmt[8], dst[8];
	pm = mp->pScopeData;
	ps = sp->pScopeData;
	int inv = sys.m_XYInv;
	if( pSound->FSKDEM.m_type == 1 ) inv = inv ? 0 : 1;
	switch(m_XYQuality){
		case 0:
			if( inv ){
				for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
					x = XC + (*pm * SX);
					y = YC - (*ps * SY);
					pCanvas->Pixels[x][y] = col;
				}
			}
			else {
				for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
					x = XC + (*pm * SX);
					y = YC + (*ps * SY);
					pCanvas->Pixels[x][y] = col;
				}
			}
			break;
		case 1:
			i2m.Clear();
			i2s.Clear();
			sc = _mt[SampType];
			if( pSound->FSKDEM.GetMarkFreq() < 1600 ) sc *= 0.9;
			SX *= sc;
			SY *= sc;
			for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
				dm = *pm;
				ds = *ps;
				if( inv ) ds = -ds;
				i2m.Do(dmt, dm);
				i2s.Do(dst, ds);
				x = XC + (dmt[0] * SX);
				y = YC + (dst[0] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[1] * SX);
				y = YC + (dst[1] * SY);
				pCanvas->Pixels[x][y] = col;
			}
			break;
		case 2:
			i4m.Clear();
			i4s.Clear();
			for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
				dm = *pm;
				ds = *ps;
				if( inv ) ds = -ds;
				i4m.Do(dmt, dm);
				i4s.Do(dst, ds);
				x = XC + (dmt[0] * SX);
				y = YC + (dst[0] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[1] * SX);
				y = YC + (dst[1] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[2] * SX);
				y = YC + (dst[2] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[3] * SX);
				y = YC + (dst[3] * SY);
				pCanvas->Pixels[x][y] = col;
			}
			break;
		default:
			i8m.Clear();
			i8s.Clear();
			for( i = 0; i < XYCOLLECT; i++, pm++, ps++ ){
				dm = *pm;
				ds = *ps;
				if( inv ) ds = -ds;
				i8m.Do(dmt, dm);
				i8s.Do(dst, ds);
				x = XC + (dmt[0] * SX);
				y = YC + (dst[0] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[1] * SX);
				y = YC + (dst[1] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[2] * SX);
				y = YC + (dst[2] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[3] * SX);
				y = YC + (dst[3] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[4] * SX);
				y = YC + (dst[4] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[5] * SX);
				y = YC + (dst[5] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[6] * SX);
				y = YC + (dst[6] * SY);
				pCanvas->Pixels[x][y] = col;
				x = XC + (dmt[7] * SX);
				y = YC + (dst[7] * SY);
				pCanvas->Pixels[x][y] = col;
			}
			break;
	}
	pSound->FSKDEM.m_XYScope = 0;
	mp->Collect(XYCOLLECT);
	sp->Collect(XYCOLLECT);
	pSound->FSKDEM.m_XYScope = 1;
	return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateXYScope(void)
{
	if( KXYScope->Checked ){
		if( UsrPal != NULL ){
			ClosePalette();
			m_ReqPaletteChange = 1;
		}
		PanelXY->Visible = TRUE;
		Timer->Interval = 100;
		pBitmapXY->Palette = 0;
		delete pBitmapXY;
		int w;
		if( m_XYSize ){
			w = PanelWater->Top - PanelFFT->Top + PanelWater->Height;
			KXYBig->Checked = TRUE;
		}
		else {
			w = PanelFFT->Height - 1;
			KXYSmall->Checked = TRUE;
		}
		int left = PanelFFT->Left + PanelFFT->Width - w;
		PanelXY->SetBounds(left, PanelFFT->Top, w, w);
		pBitmapXY = new Graphics::TBitmap();
		pBitmapXY->Width = PBoxXY->Width;
		pBitmapXY->Height = PBoxXY->Height;
		if( UsrPal != NULL ){
			pBitmapXY->Palette = UsrPal;
		}
	}
	else {
		if( m_XYSize ){
			KXYBig->Checked = TRUE;
		}
		else {
			KXYSmall->Checked = TRUE;
		}
		PanelXY->Visible = FALSE;
		Timer->Interval = 200;
	}
	if( pSound == NULL ) return;
	UpdateXY2();
	if( KXYScope->Checked ){
		DrawXY();
		if( !(Remote & REMSHOWOFF) ){
			PBoxXYPaint(NULL);
		}
		pSound->FSKDEM.m_XYScopeMark.Collect(XYCOLLECT);
		pSound->FSKDEM.m_XYScopeSpace.Collect(XYCOLLECT);
	}
	pSound->FSKDEM.m_XYScope = KXYScope->Checked;
	UpdateXYQuality();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYScopeClick(TObject *Sender)
{
	KXYScope->Checked = KXYScope->Checked ? 0 : 1;
	UpdateXYScope();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxXYPaint(TObject *Sender)
{
	if( PBoxXY->Visible ){
		PBoxXY->Canvas->Draw(0, 0, (TGraphic*)pBitmapXY);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxRecClick(TObject *Sender)
{
	if( (Font->Charset != SHIFTJIS_CHARSET) ){
		SaveDialog->Title = "Record MMTTY Sound file";
		SaveDialog->Filter = "MMTTY Sound Files(*.mmv)|*.mmv|";
	}
	else {
		SaveDialog->Title = "MMTTYÉTÉEÉìÉhÉtÉ@ÉCÉãÇÃçÏê¨";
		SaveDialog->Filter = "MMTTYÉTÉEÉìÉhÉtÉ@ÉCÉã(*.mmv)|*.mmv|";
	}
	SaveDialog->FileName = "Recv.mmv";
	SaveDialog->DefaultExt = "mmv";
	SaveDialog->InitialDir = RecDir;
	DisPaint = TRUE;
	NormalWindow(this);
	if( SaveDialog->Execute() == TRUE ){
		pSound->WaveFile.Rec(AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
		if( pSound->WaveFile.m_mode == 2 ){
			KRxRec->Checked = TRUE;
			RecentAdd(AnsiString(pSound->WaveFile.m_FileName).c_str(), TRUE);	//JA7UDE 0428
		}
		SetDirName(RecDir, AnsiString(SaveDialog->FileName).c_str());	//JA7UDE 0428
	}
	TopWindow(this);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxPlayClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if( (Font->Charset != SHIFTJIS_CHARSET) ){
		OpenDialog->Title = "Play MMTTY Sound file";
		OpenDialog->Filter = "MMTTY Sound Files(*.mmv)|*.mmv|";
	}
	else {
		OpenDialog->Title = "MMTTYÉTÉEÉìÉhÉtÉ@ÉCÉãÇÃçƒê∂";
		OpenDialog->Filter = "MMTTYÉTÉEÉìÉhÉtÉ@ÉCÉã(*.mmv)|*.mmv|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "mmv";
	OpenDialog->InitialDir = RecDir;
	DisPaint = TRUE;
	NormalWindow(this);
	if( OpenDialog->Execute() == TRUE ){
		pSound->WaveFile.Play(AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
		SetDirName(RecDir, AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
	}
	DisPaint = FALSE;
	if( pSound->WaveFile.m_mode == 1 ){
		KRxPosClick(NULL);
		RecentAdd(pSound->WaveFile.m_FileName.c_str(), TRUE);
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxStopClick(TObject *Sender)
{
	pSound->WaveFile.FileClose();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYSmallClick(TObject *Sender)
{
	m_XYSize = 0;
	UpdateXYScope();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYBigClick(TObject *Sender)
{
	m_XYSize = 1;
	UpdateXYScope();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxTimeClick(TObject *Sender)
{
	SYSTEMTIME	now;
	GetLocal(&now);

	char bf[256];
	sprintf(bf, "%s%02u%02u%02u%02u.mmv", RecDir, now.wMonth, now.wDay, now.wHour, now.wMinute);
	pSound->WaveFile.Rec(bf);
	if( pSound->WaveFile.m_mode == 2 ){
		KRxTime->Checked = TRUE;
		RecentAdd(pSound->WaveFile.m_FileName.c_str(), TRUE);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxPauseClick(TObject *Sender)
{
	pSound->WaveFile.m_pause = pSound->WaveFile.m_pause ? 0 : 1;
	KRxPause->Checked = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxTopClick(TObject *Sender)
{
	pSound->WaveFile.Rewind();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KRxPosClick(TObject *Sender)
{
	TPlayDlgBox *pBox = new TPlayDlgBox(this);

	pBox->Execute(&pSound->WaveFile);
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------
// Recent Menu Click
void __fastcall TMmttyWd::OnRecentClick(TObject *Sender)
{
	LPCSTR pName = RecentMenu.FindItemText((TMenuItem *)Sender);
	if( pName != NULL ){
		char bf[256];
		strcpy(bf, pName);
		RecentAdd(bf, pSound->WaveFile.Play(bf));
		if( pSound->WaveFile.m_mode == 1 ){
			KRxPosClick(NULL);
		}
	}
}
//---------------------------------------------------------------------------
// Recent Menu Click
void __fastcall TMmttyWd::RecentAdd(LPCSTR pNew, int f)
{
	if( RecentMenu.IsAdd(pNew) ){
		RecentMenu.Delete();
		if( f != FALSE ){
			RecentMenu.Add(pNew);
		}
		else {
			RecentMenu.Delete(pNew);
		}
		RecentMenu.Insert(NULL, OnRecentClick);
	}
}
//---------------------------------------------------------------------------
// Recent Menu Update
void __fastcall TMmttyWd::UpdateRecentMenu(void)
{
	RecentMenu.Delete();
	RecentMenu.Init(KExit, KMFile, RECMENUMAX);
	RecentMenu.ExtFilter("MMV");
	RecentMenu.Insert(NULL, OnRecentClick);
}

//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KShortCutClick(TObject *Sender)
{
	if(Font->Charset != SHIFTJIS_CHARSET){
		SetMenuName("RcvLog", AnsiString(KRcvLog->Caption).c_str());	//JA7UDE 0428
		SetMenuName("FileOut", AnsiString(KFileOut->Caption).c_str());	//JA7UDE 0428
		SetMenuName("SaveRx", AnsiString(KSaveRx->Caption).c_str());	//JA7UDE 0428
		SetMenuName("OpenLog", AnsiString(KOpenLog->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Flush", AnsiString(KFlush->Caption).c_str());	//JA7UDE 0428
		SetMenuName("RecTime", AnsiString(KRxTime->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Rec", AnsiString(KRxRec->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Play", AnsiString(KRxPlay->Caption).c_str());	//JA7UDE 0428
		SetMenuName("PlayPos", AnsiString(KRxPos->Caption).c_str());	//JA7UDE 0428
		SetMenuName("PlayStop", AnsiString(KRxStop->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Paste", AnsiString(KPaste->Caption).c_str());	//JA7UDE 0428
		SetMenuName("MacBtn", AnsiString(KMac->Caption).c_str());	//JA7UDE 0428
		SetMenuName("RUN", AnsiString(KRun->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Scope", AnsiString(KOS->Caption).c_str());	//JA7UDE 0428
		SetMenuName("ClrRxWindow", AnsiString(KClrRx->Caption).c_str());	//JA7UDE 0428
		SetMenuName("LogList", AnsiString(KLog->Caption).c_str());	//JA7UDE 0428
		SetMenuName("QSOData", AnsiString(KQSO->Caption).c_str());	//JA7UDE 0428
		SetMenuName("EntTX", AnsiString(KENT->Caption).c_str());	//JA7UDE 0428
		SetMenuName("WordWrap", AnsiString(KWP->Caption).c_str());	//JA7UDE 0428
		SetMenuName("TNC", AnsiString(KTNC->Caption).c_str());	//JA7UDE 0428
		SetMenuName("Option", AnsiString(KOption->Caption).c_str());	//JA7UDE 0428
		SetMenuName("LogOpt", AnsiString(KOptLog->Caption).c_str());	//JA7UDE 0428
		SetMenuName("ExtReset", AnsiString(KExtReset->Caption).c_str());	//JA7UDE 0428
		SetMenuName("ExtSusp", AnsiString(KExtSusp->Caption).c_str());	//JA7UDE 0428

		SetMenuName("TxUp", "ScrollUp TxWindow");
		SetMenuName("TxDown", "ScrollDown TxWindow");
		SetMenuName("TxPUp", "PageUp TxWindow");
		SetMenuName("TxPDown", "PageDown TxWindow");
		SetMenuName("TxHome", "MoveTop TxWindow");
		SetMenuName("TxEnd", "MoveBottom TxWindow");

		SetMenuName("RxUp", "ScrollUp RxWindow");
		SetMenuName("RxDown", "ScrollDown RxWindow");
		SetMenuName("RxPUp", "PageUp RxWindow");
		SetMenuName("RxPDown", "PageDown RxWindow");
		SetMenuName("RxHome", "MoveTop RxWindow");
		SetMenuName("RxEnd", "MoveBottom RxWindow");

		SetMenuName("CharWaitL", "Left Char Wait");
		SetMenuName("CharWaitR", "Right Char Wait");
		SetMenuName("DiddleWaitL", "Left Diddle Wait");
		SetMenuName("DiddleWaitR", "Right Diddle Wait");

		SetMenuName("TxHeightUp", "HeightUp TxWindow");
		SetMenuName("TxHeightDown", "HeightDown TxWindow");

		SetMenuName("TxLTR", "Tx LTR");
		SetMenuName("TxFIG", "Tx FIG");

		SetMenuName("DecShift", "Dec. Shift width");
		SetMenuName("IncShift", "Inc. Shift width");
		SetMenuName("ChangeShift", "Change Shift (170/200/220/350/450)");
	}

	TShortCutDlg *pBox = new TShortCutDlg(this);
	pBox->Execute();
	delete pBox;
	UpdateShortCut(TRUE);
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetHint(TControl *tp, WORD Key)
{
	if( Key && KHint->Checked ){
		char bf[128];
		sprintf(bf, "ShortCut %s", GetKeyName(Key));
		tp->ShowHint = TRUE;
		tp->Hint = bf;
	}
	else {
		tp->Hint = "";
		tp->ShowHint = FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetShortCut(TMenuItem *mp, WORD Key)
{
	if( Key && (!Remote) ){
		if( Key & 0x100 ){
			mp->ShortCut = ShortCut(WORD(Key&0x00ff), TShiftState() << ssCtrl);
		}
		else if( Key & 0x200 ){
			mp->ShortCut = ShortCut(WORD(Key&0x00ff), TShiftState() << ssAlt);
		}
		else if( Key & 0x400 ){
			mp->ShortCut = ShortCut(WORD(Key&0x00ff), TShiftState() << ssShift);
		}
		else {
			mp->ShortCut = ShortCut(Key, TShiftState());
		}
	}
	else {
		mp->ShortCut = ShortCut(0, TShiftState());
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateShortCut(BOOL bMenu)
{
	if( bMenu ){
	SetShortCut(KRcvLog, sys.m_SysKey[kkRcvLog]);
	SetShortCut(KFileOut, sys.m_SysKey[kkFileOut]);
	SetShortCut(KSaveRx, sys.m_SysKey[kkSaveRx]);

	SetShortCut(KOpenLog, sys.m_SysKey[kkOpenLog]);
	SetShortCut(KFlush, sys.m_SysKey[kkFlush]);

	SetShortCut(KRxTime, sys.m_SysKey[kkRecTime]);
	SetShortCut(KRxRec, sys.m_SysKey[kkRec]);
	SetShortCut(KRxPlay, sys.m_SysKey[kkPlay]);
	SetShortCut(KRxPos, sys.m_SysKey[kkPlayPos]);
	SetShortCut(KRxStop, sys.m_SysKey[kkPlayStop]);

	SetShortCut(KRun, sys.m_SysKey[kkRun]);

	SetShortCut(KPaste, sys.m_SysKey[kkPaste]);
	SetShortCut(KPanel, sys.m_SysKey[kkPanel]);
	SetShortCut(KMac, sys.m_SysKey[kkMac]);
	SetShortCut(KOS, sys.m_SysKey[kkScope]);
	SetShortCut(KClrRx, sys.m_SysKey[kkClrRxWindow]);
	SetShortCut(KLog, sys.m_SysKey[kkLogList]);
	SetShortCut(KQSO, sys.m_SysKey[kkQSOData]);

	SetShortCut(KENT, sys.m_SysKey[kkEntTX]);
	SetShortCut(KWP, sys.m_SysKey[kkWordWrap]);
	SetShortCut(KTNC, sys.m_SysKey[kkTNC]);
	SetShortCut(KOption, sys.m_SysKey[kkOption]);
	SetShortCut(KOptLog, sys.m_SysKey[kkLogOption]);

	SetShortCut(KS1, sys.m_SysKey[kkPro1]);
	SetShortCut(KS2, sys.m_SysKey[kkPro2]);
	SetShortCut(KS3, sys.m_SysKey[kkPro3]);
	SetShortCut(KS4, sys.m_SysKey[kkPro4]);
	SetShortCut(KS5, sys.m_SysKey[kkPro5]);
	SetShortCut(KS6, sys.m_SysKey[kkPro6]);
	SetShortCut(KS7, sys.m_SysKey[kkPro7]);
	SetShortCut(KS8, sys.m_SysKey[kkPro8]);
	SetShortCut(KSDEF, sys.m_SysKey[kkProDef]);
	SetShortCut(KSRET, sys.m_SysKey[kkProRet]);

	SetShortCut(KExtCmd1, sys.m_SysKey[kkExtCmd1]);
	SetShortCut(KExtCmd2, sys.m_SysKey[kkExtCmd2]);
	SetShortCut(KExtCmd3, sys.m_SysKey[kkExtCmd3]);
	SetShortCut(KExtCmd4, sys.m_SysKey[kkExtCmd4]);
	SetShortCut(KExtReset, sys.m_SysKey[kkExtReset]);
	SetShortCut(KExtSusp, sys.m_SysKey[kkExtSusp]);
	}

	SetHint(SBData, sys.m_SysKey[kkQSOData]);
	SetHint(SBFIG, sys.m_SysKey[kkFIG]);
	SetHint(SBUOS, sys.m_SysKey[kkUOS]);

	SetHint(SBTX, sys.m_SysKey[kkTX]);
	SetHint(SBTXOFF, sys.m_SysKey[kkTXOFF]);
	SetHint(SBQSO, sys.m_SysKey[kkQSO]);
	SetHint(SBInit, sys.m_SysKey[kkInitBox]);
	SetHint(HisCall, sys.m_SysKey[kkCall]);
	SetHint(HisName, sys.m_SysKey[kkName]);
	SetHint(HisQTH, sys.m_SysKey[kkQTH]);
	SetHint(HisRST, sys.m_SysKey[kkRST]);
	SetHint(MyRST, sys.m_SysKey[kkMyRST]);
	SetHint(Freq, sys.m_SysKey[kkFreq]);
	SetHint(SBFind, sys.m_SysKey[kkFind]);
	SetHint(SBINClear, sys.m_SysKey[kkClear]);

	SetHint(SBIN1, sys.m_InBtnKey[0]);
	SetHint(SBIN2, sys.m_InBtnKey[1]);
	SetHint(SBIN3, sys.m_InBtnKey[2]);
	SetHint(SBIN4, sys.m_InBtnKey[3]);
	int i;
	for( i = 0; i < 16; i++ ){
		SetHint(GetSB(i), sys.m_UserKey[i]);
	}
	Application->ShowHint = KHint->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KHintClick(TObject *Sender)
{
	KHint->Checked = KHint->Checked ? 0 : 1;
	UpdateShortCut(FALSE);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KOptLogClick(TObject *Sender)
{
	TLogSetDlg *pBox = new TLogSetDlg(this);
	int contest = Log.m_LogSet.m_Contest;
	pBox->Execute();
	delete pBox;
	UpdateTitle();
	UpdateLogLink();
	if( contest != Log.m_LogSet.m_Contest ){
		AlignLogPanel();
	}
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPanelClick(TObject *Sender)
{
	KPanel->Checked = KPanel->Checked ? 0 : 1;
	UpdateControlPanel();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPanelSmallClick(TObject *Sender)
{
	if( m_PanelSize ){
		m_PanelSize = 0;
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPanelBigClick(TObject *Sender)
{
	if( m_PanelSize != 2 ){
		m_PanelSize = 2;
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPanelMediumClick(TObject *Sender)
{
	if( m_PanelSize != 1 ){
		m_PanelSize = 1;
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPanelSuperClick(TObject *Sender)
{
	if( m_PanelSize != 3 ){
		m_PanelSize = 3;
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateExtProgram(void)
{
	KExtEnt1->Caption = KExtCmd1->Caption;
	KExtDel1->Caption = KExtCmd1->Caption;
	KExtEnt2->Caption = KExtCmd2->Caption;
	KExtDel2->Caption = KExtCmd2->Caption;
	KExtEnt3->Caption = KExtCmd3->Caption;
	KExtDel3->Caption = KExtCmd3->Caption;
	KExtEnt4->Caption = KExtCmd4->Caption;
	KExtDel4->Caption = KExtCmd4->Caption;
	KExtCmd1->Enabled = !sys.m_ExtCmd[0].IsEmpty();
	KExtCmd2->Enabled = !sys.m_ExtCmd[1].IsEmpty();
	KExtCmd3->Enabled = !sys.m_ExtCmd[2].IsEmpty();
	KExtCmd4->Enabled = !sys.m_ExtCmd[3].IsEmpty();
	KExtDel1->Enabled = !sys.m_ExtCmd[0].IsEmpty();
	KExtDel2->Enabled = !sys.m_ExtCmd[1].IsEmpty();
	KExtDel3->Enabled = !sys.m_ExtCmd[2].IsEmpty();
	KExtDel4->Enabled = !sys.m_ExtCmd[3].IsEmpty();
	SetExtMenuName(0, AnsiString(KExtCmd1->Caption).c_str());	//JA7UDE 0428
	SetExtMenuName(1, AnsiString(KExtCmd2->Caption).c_str());	//JA7UDE 0428
	SetExtMenuName(2, AnsiString(KExtCmd3->Caption).c_str());	//JA7UDE 0428
	SetExtMenuName(3, AnsiString(KExtCmd4->Caption).c_str());	//JA7UDE 0428
#if 0
	HICON ic = ::ExtractIcon(HInstance, sys.m_ExtCmd[0].c_str(), 0);
//    ::DrawIcon(PBoxRx->Canvas->Handle, 0, 0, ic);
	static Graphics::TBitmap *pbmp = NULL;
	if( pbmp == NULL ) pbmp = new Graphics::TBitmap;
	pbmp->Height = GetMenuCheckMarkDimensions();
	pbmp->Width = GetMenuCheckMarkDimensions();
	::DrawIcon(pbmp->Canvas->Handle, 0, 0, ic);
	::SetMenuItemBitmaps(KExtCmd1->Handle, 0, 0, pbmp->Handle, pbmp->Handle);
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtEntClick(int n, TMenuItem *mp)
{
	AnsiString as = mp->Caption;
	int r;
	if(Font->Charset != SHIFTJIS_CHARSET){
		r = InputMB("Assign External Program", "Input Menu text", as);
	}
	else {
		r = InputMB("äOïîÉvÉçÉOÉâÉÄìoò^", "ÉÅÉjÉÖÅ[Ç…ï\é¶Ç∑ÇÈñºëOÇì¸óÕÇµÇƒâ∫Ç≥Ç¢.", as);
	}
	if( r == TRUE ){
		OpenDialog->Options >> ofCreatePrompt;
		OpenDialog->Options << ofFileMustExist;
		if( (Font->Charset != SHIFTJIS_CHARSET) ){
			OpenDialog->Title = "Select External Program";
			OpenDialog->Filter = "Program files(*.exe;*.lnk)|*.exe;*.lnk|All Files(*.*)|*.*|";
		}
		else {
			OpenDialog->Title = "é¿çsÇ∑ÇÈÉvÉçÉOÉâÉÄÇëIë";
			OpenDialog->Filter = "ÉvÉçÉOÉâÉÄÉtÉ@ÉCÉã(*.exe;*.lnk)|*.exe;*.lnk|Ç∑Ç◊ÇƒÇÃÉtÉ@ÉCÉã(*.*)|*.*|";
		}
		OpenDialog->FileName = sys.m_ExtCmd[n];
		OpenDialog->DefaultExt = "exe";
		OpenDialog->InitialDir = "\\";
		DisPaint = TRUE;
		NormalWindow(this);
		if( OpenDialog->Execute() == TRUE ){
			sys.m_ExtCmd[n] = OpenDialog->FileName;
			mp->Caption = as;
			UpdateExtProgram();
		}
		TopWindow(this);
		DisPaint = FALSE;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtEnt1Click(TObject *Sender)
{
	KExtEntClick(0, KExtCmd1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtEnt2Click(TObject *Sender)
{
	KExtEntClick(1, KExtCmd2);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtEnt3Click(TObject *Sender)
{
	KExtEntClick(2, KExtCmd3);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtEnt4Click(TObject *Sender)
{
	KExtEntClick(3, KExtCmd4);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmdClick(int n)
{
	KExtSuspClick(NULL);

	char bf[256];
	SetDirName(bf, sys.m_ExtCmd[n].c_str());
	::SetCurrentDirectory(bf);
	::WinExec(sys.m_ExtCmd[n].c_str(), SW_SHOWDEFAULT);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmd1Click(TObject *Sender)
{
	KExtCmdClick(0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmd2Click(TObject *Sender)
{
	KExtCmdClick(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmd3Click(TObject *Sender)
{
	KExtCmdClick(2);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmd4Click(TObject *Sender)
{
	KExtCmdClick(3);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtResetClick(TObject *Sender)
{
	CWaitCursor wait;
	pSound->m_suspack = 0;
	pSound->m_susp = 0;
	for( int i = 0; (i < 20) && !pSound->m_suspack; i++ ) ::Sleep(100);
	if( pSound->m_susp ) return;
	COMM.change = 1;
	RADIO.change = 1;
	TNC.change = 1;
	OpenCloseCom();
	::SetCurrentDirectory(BgnDir);
	Log.Close();
	Log.Open(NULL, TRUE);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtSuspClick(TObject *Sender)
{
	CWaitCursor wait;
	pSound->m_suspack = 0;
	pSound->m_susp = 1;
	if( pComm != NULL ){
		delete pComm;
		pComm = NULL;
	}
	if( pRadio != NULL ){
		delete pRadio;
		pRadio = NULL;
	}
	if(pTnc != NULL ){
		delete pTnc;
		pTnc = NULL;
	}
	Log.Close();
	Log.Open(NULL, TRUE);
	int i;
	for( i = 0; (i < 20) && !pSound->m_suspack; i++ ) ::Sleep(100);
	pSound->m_suspack = 0;
	DisPaint = TRUE;
	if( !(Remote & REMSHOWOFF) ){
		Application->Minimize();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtDel1Click(TObject *Sender)
{
	KExtCmd1->Caption = "External 1";
	sys.m_ExtCmd[0] = "";
	UpdateExtProgram();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtDel2Click(TObject *Sender)
{
	KExtCmd2->Caption = "External 2";
	sys.m_ExtCmd[1] = "";
	UpdateExtProgram();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtDel3Click(TObject *Sender)
{
	KExtCmd3->Caption = "External 3";
	sys.m_ExtCmd[2] = "";
	UpdateExtProgram();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtDel4Click(TObject *Sender)
{
	KExtCmd4->Caption = "External 4";
	sys.m_ExtCmd[3] = "";
	UpdateExtProgram();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::DrawSQ(void)
{
	if( pSound == NULL ) return;

	TCanvas *pCanvas = pBitmapSQ->Canvas;

	TRect rc;
	int XL = 0;
	int XR = pBitmapSQ->Width - 1;
	int YT = 0;
	int YB = pBitmapSQ->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	if( !(Remote & REMSHOWOFF) ){
		pCanvas->Brush->Color = clBlack;
		pCanvas->FillRect(rc);
	}

	rc.Left = XL;
	int over = 0;
	if( pSound->FSKDEM.m_Limit ){
		rc.Right = pSound->FSKDEM.m_avgdeff * XR / (10.0 * SQSCALE);
		if( (pSound->FSKDEM.GetSQLevel() * 10.0) > pSound->FSKDEM.m_avgdeff ){
			pCanvas->Brush->Color = clGray;
		}
		else {
			pCanvas->Brush->Color = clLime;
			over = 1;
			if( !m_FIGTimer && SBFIG->Down ){
				SBFIG->Down = FALSE;
				SBFIGClick(NULL);
			}
			m_FIGTimer = 1000/Timer->Interval;
		}
		if( m_FIGTimer ) m_FIGTimer--;
		RemoteSigLevel(pSound->FSKDEM.m_avgdeff/10.0);
	}
	else {
		rc.Right = pSound->FSKDEM.m_avgdeff * XR / SQSCALE;
		if( pSound->FSKDEM.GetSQLevel() > pSound->FSKDEM.m_avgdeff ){
			pCanvas->Brush->Color = clGray;
		}
		else {
			pCanvas->Brush->Color = clLime;
			over = 1;
		}
		RemoteSigLevel(pSound->FSKDEM.m_avgdeff);
	}
	if( Remote & REMSHOWOFF ) return;
	if( !SBSQ->Down ){
		pCanvas->Brush->Color = clLime;
	}

	if( rc.Right > XR ) rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	pCanvas->FillRect(rc);
	rc.Right = pSound->FSKDEM.GetSQLevel() * XR / SQSCALE;
	rc.Left = rc.Right;
	rc.Right++;
	rc.Top = YT;
	rc.Bottom = YB+1;
	pCanvas->Brush->Color = over ?  clBlack : clWhite;
	pCanvas->FillRect(rc);
#if BITDEBUG
	double tim = pSound->FSKDEM.m_bitCountA * 1000.0/DemSamp;
#if TXDDEBUG
	if( pComm == NULL ) return;
	char bf[32];
	pCanvas->Brush->Color = clWhite;
	pCanvas->Font->Color = clBlack;
	pCanvas->Font->Size = 12;
	sprintf(bf, "%lu", pComm->m_bitCountA);
	pCanvas->TextOut(0, 0, bf);
#else
	if( over && (tim >= 140) && (tim < 400) ){
		char bf[32];
		pCanvas->Brush->Color = clWhite;
		pCanvas->Font->Color = clBlack;
		pCanvas->Font->Size = 12;
//	sprintf(bf, "%05lf", pSound->FSKDEM.m_avgdeff);
		sprintf(bf, "%.1lf", pSound->FSKDEM.m_bitCountA * 1000.0/DemSamp);
		pCanvas->TextOut(0, 0, bf);
	}
#endif
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxSQMouseDown(TObject *Sender,
	TMouseButton Button, TShiftState Shift, int X, int Y)
{
	pSound->FSKDEM.SetSQLevel(X * SQSCALE / (pBitmapSQ->Width - 1));
	if( !SBSQ->Down ){
		DrawSQ();
        PBoxSQPaint(NULL);
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PBoxSQPaint(TObject *Sender)
{
	if( PBoxSQ->Visible ){
		PBoxSQ->Canvas->Draw(0, 0, (TGraphic*)pBitmapSQ);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::LLPFClick(TObject *Sender)
{
	if( pSound == NULL ) return;

	switch(m_DemType){
		case 0:
		case 1:
			pSound->FSKDEM.m_lpf = pSound->FSKDEM.m_lpf ? 0 : 1;
			UpdateItem();
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateXYQuality(void)
{
	switch(m_XYQuality){
		case 0:
			KXYQuality1->Checked = TRUE;
			break;
		case 1:
			KXYQuality2->Checked = TRUE;
			break;
		case 2:
			KXYQuality3->Checked = TRUE;
			break;
		default:
			KXYQuality4->Checked = TRUE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYQuality1Click(TObject *Sender)
{
	m_XYQuality = 0;
	UpdateXYQuality();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYQuality2Click(TObject *Sender)
{
	m_XYQuality = 1;
	UpdateXYQuality();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYQuality3Click(TObject *Sender)
{
	m_XYQuality = 2;
	UpdateXYQuality();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KXYQuality4Click(TObject *Sender)
{
	m_XYQuality = 3;
	UpdateXYQuality();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGLClick(TObject *Sender)
{
	sys.m_FFTGain = 0;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGMClick(TObject *Sender)
{
	sys.m_FFTGain = 1;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGHClick(TObject *Sender)
{
	sys.m_FFTGain = 2;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGTClick(TObject *Sender)
{
	sys.m_FFTGain = 3;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGS1Click(TObject *Sender)
{
	sys.m_FFTGain = 4;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGS2Click(TObject *Sender)
{
	sys.m_FFTGain = 5;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGS3Click(TObject *Sender)
{
	sys.m_FFTGain = 6;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTGS4Click(TObject *Sender)
{
	sys.m_FFTGain = 7;
	pSound->fftIN.ClearBuf();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::LCallClick(TObject *Sender)
{
	m_Running = m_Running ? 0 : 1;
	UpdateTitle();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSetHelpClick(TObject *Sender)
{
	DisPaint = TRUE;
	TSetHelpBox *pBox = new TSetHelpBox(this);

	//1.70K
	if (sys.m_WinFontCharset == SHIFTJIS_CHARSET) {
		pBox->GroupBox1->Visible=True;
		pBox->GroupBox2->Visible=True;
		pBox->GroupBox3->Visible=True;
		pBox->GroupBox4->Visible=False;
	}

	else {
		pBox->GroupBox1->Visible=False;
		pBox->GroupBox2->Visible=False;
		pBox->GroupBox3->Visible=False;
		pBox->GroupBox4->Visible=True;
	}

	if( pBox->Execute() == TRUE ){

		//AddHelpMenu();   //1.70K don't modify Help menu

		UpdateSystemFont();

	}
	delete pBox;
	TopWindow(this);
	DisPaint = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::AddHelpMenu(void)
{
	//1.70K only one Help Menu configuration

	if(sys.m_WinFontCharset != SHIFTJIS_CHARSET){
		KMHelp->Delete(0); //Delete 3 Japanese Help menu items
		KMHelp->Delete(0);
		KMHelp->Delete(0);

		pAndyMenu = new TMenuItem(NULL);
		pAndyMenu->Caption = "MMTTY Help";
		pAndyMenu->OnClick = KAndyHelp;
		KMHelp->Insert(0, pAndyMenu);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KAndyHelp(TObject *Sender)
{
#if 1
	ShowHtmlHelp();
#else
	char bf[512];

	sprintf(bf, "%s%s", BgnDir, sys.m_HTMLHelp.c_str());
	FILE *fp;
	if( (fp = fopen(bf, "rb"))!=NULL ){
		fclose(fp);
		if( !strcmpi(GetEXT(bf), "HLP") ){
			::WinHelp(Handle, bf, HELP_FINDER, 0);
		}
		else if( !strcmpi(GetEXT(bf), "CHM") ){
			::ShellExecute(Handle, "open", bf, NULL, NULL, SW_SHOWDEFAULT);
		}
		else {
			WebRef.ShowHTML(bf);
		}
	}
	else if((sys.m_WinFontCharset != SHIFTJIS_CHARSET )){
		ErrorMB( "'%s' was not found.\r\n\r\nPlease search in the MMTTY English Web Site.", sys.m_HTMLHelp.c_str());
	}
	else {
		ErrorMB( "'%s'Ç™å©Ç¬Ç©ÇËÇ‹ÇπÇÒ.\r\n\r\nMMTTY English Web Site Ç©ÇÁÉ_ÉEÉìÉçÅ[ÉhÇµÇƒâ∫Ç≥Ç¢.", sys.m_HTMLHelp.c_str());
	}
#endif
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSCharClick(TObject *Sender)
{
	m_SendWay = 0;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSWordClick(TObject *Sender)
{
	m_SendWay = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSLineClick(TObject *Sender)
{
	m_SendWay = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTRSClick(TObject *Sender)
{
	sys.m_FFTResp = 3;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTRMClick(TObject *Sender)
{
	sys.m_FFTResp = 2;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KFFTRFClick(TObject *Sender)
{
	sys.m_FFTResp = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPasteClick(TObject *Sender)
{
	TClipboard	*pClip = new TClipboard;


	//char bf[1024];
	WideChar bf[1024];	//JA7UDE 0428

	pClip->Open();
	pClip->GetTextBuf(bf, sizeof(bf));
	pClip->Close();
	delete pClip;
	//jstrupr(bf);
	_wcsupr(bf);	//JA7UDE 0428
	char bbf[1024];
	wcstombs( bbf, bf, wcslen(bf) );
	WriteFifoEdit(bbf);
	AdjustFocus();
}
//---------------------------------------------------------------------------
// TNCÉfÅ[É^éÛêMÇÃÉCÉxÉìÉg
void __fastcall TMmttyWd::OnTncEvent(TMessage &Message)
{
	if( pTnc == NULL ) return;
	switch(Message.WParam){
        case MMTMSG_CHAR:
			Message.Result = pTnc->PutRxFifo(BYTE(Message.LParam));
			break;
		case MMTMSG_PTT:
        	RxCommon(Message.LParam ? TNC_PTTON : TNC_PTTOFF);
        	break;
		case MMTMSG_COMMON:
        	RxCommon(BYTE(Message.LParam));
            break;
		case MMTMSG_CLEARFIFO:
			if( Message.LParam ){
				pTnc->ClearTxFifo();
            }
            else {
				pTnc->ClearRxFifo();
            }
        	break;
        case MMTMSG_SETSWITCH:
			Message.LParam &= ~0x00008000;
			RemoteSwitch(Message.LParam);
            break;
        case MMTMSG_SETVIEW:
			RemoteView(Message.LParam);
        	break;
		case MMTMSG_CHARTXW:
			if( Message.LParam != LF ) PushKey(char(Message.LParam));
			Message.Result = FifoEdit.GetLen();
        	break;
        case MMTMSG_WRITETXW:
			if( !Message.LParam ) break;
   			WriteFifoEdit(LPCSTR(Message.LParam));
			Message.Result = FifoEdit.GetLen();
        	break;
		case MMTMSG_SETFREQ:
			{
	        	m_DisEvent++;
                int mfq = LOWORD(Message.LParam);
                int sfq = HIWORD(Message.LParam);
				MarkFreq->Text = mfq;
    	        pSound->FSKDEM.SetMarkFreq(mfq);
				pSound->FSKDEM.SetSpaceFreq(sfq);
            	ShiftFreq->Text = sfq - mfq;
            	m_DisEvent--;
            	MarkFreqChange(NULL);
            }
			break;
        case MMTMSG_SETBAUD:
        	{
            	int dd = Message.LParam;
				if( (dd >= 2000) && (dd < 60000) ){
					pSound->FSKDEM.SetBaudRate(double(dd)/100.0);
					pSound->FSKMOD.SetBaudRate(double(dd)/100.0);
					SelectCombo(FALSE);
					UpdateFSK();
				}
            }
        	break;
        case MMTMSG_SETSQ:
			pSound->FSKDEM.SetSQLevel(Message.LParam);
        	break;
		case MMTMSG_SETNOTCH:
			pSound->m_lms.m_lmsNotch2 = HIWORD(Message.LParam);
			pSound->m_lms.m_lmsNotch = LOWORD(Message.LParam);
			if( SBLMS->Down ) pSound->CalcBPF();
            RemoteFreq();
        	break;
        case MMTMSG_SETDEFFREQ:
			sys.m_DefMarkFreq = LOWORD(Message.LParam);
            sys.m_DefShift = HIWORD(Message.LParam);
            break;
        case MMTMSG_SETPTTTIMER:
			m_PttTimer = Message.LParam;
			UpdatePttTimer();
			KPttTim->Checked = m_PttTimer ? 1 : 0;
			break;
		case MMTMSG_SETTNCTYPE:
        	TNC.Type = LOWORD(Message.LParam);
            if( TNC.Type >= 3 ) TNC.Type = 0;
            TNC.TncMode = HIWORD(Message.LParam);
			if( TNC.Type == 2 ){
				TNC.TncMode = 0;
            }
            else if( TNC.TncMode ){
				CmdKAM("");
            }
            break;
        case MMTMSG_SETPRIORITY:
        	Message.Result = ::SetThreadPriority((HANDLE)pTnc->Handle, Message.LParam);
        	break;
		case MMTMSG_SETLENGTH:
			if( (Message.LParam >= 5) && (Message.LParam <= 8) ){
	        	pSound->FSKDEM.m_BitLen = Message.LParam;
    	        pSound->FSKMOD.m_BitLen = Message.LParam;
        	    UpdateFSK();
            }
            break;
		case MMTMSG_GETPROFILENAME:
			Message.Result = NULL;
			if( (Message.LParam >= 0) && (Message.LParam < 8) ){
				TMenuItem *tp = GetKS(Message.LParam);
                if( tp->Enabled ){
					Message.Result = (DWORD)tp->Caption.c_str();
                }
            }
        	break;
        case MMTMSG_LOADPROFILE:
			ReadProfile(Message.LParam, NULL);
			UpdateItem();
        	break;
        case MMTMSG_SAVEPROFILE:
           	if( !Message.LParam ) break;
			{
            	char bf[1024];
                StrCopy(bf, LPCSTR(Message.LParam), sizeof(bf)-1);
                LPSTR t, p;
                p = StrDlm(t, bf);
                int n = atoin(t, -1);
				if( (n >= 0) && (n < 8) ){
					TMenuItem *tp = GetKS(n);
                    LPCSTR dp;
                    if( *p ){
                    	dp = p;
                    }
                    else {
						char bbf[32];
						sprintf(bbf, "Profile %d", n + 1);
                        dp = bbf;
                    }
                   	tp->Caption = dp;
					tp->Enabled = *p;
	        		WriteProfile(n, dp, *p);
                    UpdateProfile();
				}
            }
            break;
        case MMTMSG_SETRESO:
        	{
				MSG msg;
                msg.wParam = RXM_SETRESO;
                msg.lParam = Message.LParam;
                RemoteMMTTY(msg);
            }
            break;
        case MMTMSG_SETLPF:
        	{
				MSG msg;
                msg.wParam = RXM_SETLPF;
                msg.lParam = Message.LParam;
                RemoteMMTTY(msg);
            }
            break;
        default:
		   	OnTncEvent();
            Message.Result = FALSE;
        	break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetNMMT(void)
{
	if( pTnc->IsMMT() ){
    	NMMT *pNMMT = pTnc->GetNMMT();
		pNMMT->m_notch1 = pSound->m_lms.m_lmsNotch;
        pNMMT->m_notch2 = pSound->m_lms.m_lmsNotch2;
		pNMMT->m_txbuflen = GetTxBufCount() + strlen(m_TNCStg.c_str());
        pNMMT->m_codeswitch = GetRemoteSwitch();
        pNMMT->m_codeview = GetRemoteView();
        pNMMT->m_txwlen = FifoEdit.GetLen();
        pNMMT->m_baud = pSound->FSKDEM.GetBaudRate() * 100;
        pNMMT->m_bitlength = pSound->FSKDEM.m_BitLen;
        pNMMT->m_modetnc = MAKELONG(TNC.Type, TNC.TncMode);
        pNMMT->m_fig = rtty.m_fig;
        pNMMT->m_defmark = sys.m_DefMarkFreq;
		pNMMT->m_defshift = sys.m_DefShift;
    }
}
//---------------------------------------------------------------------------
// TNCÉfÅ[É^éÛêMÇÃÉCÉxÉìÉg
//
void __fastcall TMmttyWd::OnTncEvent(void)
{
	if( pTnc == NULL ) return;

	UpdateTNCPTT();
	SetNMMT();
	int txbuflen = GetTxBufCount();
	if( SBTX->Down && (TNC.flwXON || TNC.flwCTS) ){
		if( txbuflen >= (MODBUFMAX*3/4) ) return;
	}
	char c;
	while(pTnc->m_rxcnt){
		c = pTnc->m_rxbuf[pTnc->m_rxrp];
		pTnc->m_rxrp++;
		if( pTnc->m_rxrp >= sizeof(pTnc->m_rxbuf) ) pTnc->m_rxrp = 0;
		pTnc->m_rxcnt--;
		if( !RxCommon(c) ){
			switch(TNC.Type){
				case 0:
					RxTnc241(c);
					break;
				case 1:
					RxKAM(c);
					break;
				case 2:
					if( SBTX->Down ){
						pSound->FSKMOD.PutData(BYTE(c));
					}
					break;
            }
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateTNCPTT(void)
{
	if( TNC.Type == 2 ){
		if( pTnc->m_PTT ){
			if( !SBTX->Down ) ToTX("");
		}
		else {
			if( SBTX->Down && IsTxIdle() ) ToRX(0);
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::TncConvers(void)
{
	if( TNC.TncMode ){
       	switch(TNC.Type){
			case 0:
				pTnc->OutStr("K\r");
				CmdTnc241("K");
               	break;
            case 1:
				pTnc->OutStr("RTTY\r");
				CmdKAM("RTTY");
               	break;
        }
    }
   	TNC.TncMode = 0;
}
//---------------------------------------------------------------------------
BOOL __fastcall TMmttyWd::RxCommon(BYTE c)
{
	BOOL r = TRUE;
	switch(c){
		case TNC_PTTON:
			ClearMacroTimer();
        	TncConvers();
            if( TNC.Type == 2 ){
				pTnc->m_PTT = TRUE;
				UpdateTNCPTT();
            }
            else {
				if( m_TNCStg.IsEmpty() ){
					ToTX(NULL);
				}
                else {
					ToTX(m_TNCStg.c_str());
					OutputStr(m_TNCStg.c_str());
					m_TNCStg = "";
                }
            }
        	break;
		case TNC_PTTOFF:
			ClearMacroTimer();
        	TncConvers();
			if( TNC.Type == 2 ){
	        	pTnc->m_PTT = FALSE;
				UpdateTNCPTT();
            }
            else {
				ToRX(1);
				m_TNCStg = "";
            }
        	break;
		case TNC_PTTFLUSH:
			ClearMacroTimer();
        	TncConvers();
			if( IsTxIdle() ){
            	if( TNC.Type == 2 ){
					pTnc->m_PTT = FALSE;
					UpdateTNCPTT();
                }
                else {
					ToRX(0);
                }
            }
			else {
				m_ReqRecv = 1;
			}
        	break;
		case TNC_CLEARBUF:
			pSound->FSKMOD.DeleteTXBuf();
			m_TNCStg = "";
        	break;
        case TNC_AFCON:
			SBAFC->Down = TRUE;
            SBAFCClick(NULL);
        	break;
		case TNC_AFCOFF:
			SBAFC->Down = FALSE;
            SBAFCClick(NULL);
			break;
		case TNC_NETON:
			SBNET->Down = TRUE;
            SBNETClick(NULL);
        	break;
		case TNC_NETOFF:
			SBNET->Down = FALSE;
            SBNETClick(NULL);
        	break;
		case TNC_REVON:
			SBRev->Down = TRUE;
            SBRevClick(NULL);
        	break;
        case TNC_REVOFF:
			SBRev->Down = FALSE;
            SBRevClick(NULL);
        	break;
		case TNC_HAM:
			SBHAMClick(NULL);
        	break;
        case TNC_ECHOON:
			TNC.Echo = 2;
        	break;
        case TNC_ECHOOFF:
			TNC.Echo = 0;
        	break;
        case TNC_CANCELPTTFLUSH:
			m_ReqRecv = 0;
        	break;
		default:
        	r = FALSE;
        	break;
    }
    return r;
}
void __fastcall TMmttyWd::RxTnc241(char c)
{
	char bf[256];

	switch(c){
		case 0x12:		// Ctrl+R
			if( TNC.TncMode ) pTnc->OutStr("\r");
			RxCommon(TNC_PTTOFF);
//			ToRX(0);
//			TNC.TncMode = 0;
//			m_TNCStg = "";
			break;
		case 0x14:		// Ctrl+T
			if( TNC.TncMode ) pTnc->OutStr("\r");
			RxCommon(TNC_PTTON);
//			TNC.TncMode = 0;
//			ToTX(NULL);
//			if( !m_TNCStg.IsEmpty() ){
//				OutputStr(m_TNCStg.c_str());
//				m_TNCStg = "";
//			}
			break;
		case 0x03:
			if( TNC.TncMode ) pTnc->OutStr("\r");
			TNC.TncMode = 1;
			pTnc->OutStr("\rcmd:");
			m_TNCStg = "";
			break;
		case 0x04:		// Ctrl+D
			RxCommon(TNC_PTTFLUSH);
//			if( IsTxIdle() ){
//				ToRX(0);
//			}
//			else {
//				m_ReqRecv = 1;
//			}
//			TNC.TncMode = 0;
			break;
		case 0x18:		// Ctrl+X
        	RxCommon(TNC_CLEARBUF);
//			pSound->FSKMOD.DeleteTXBuf();
//			m_TNCStg = "";
			break;
		default:
			if( TNC.TncMode ){		// CommandÉÇÅ[Éh
				if( TNC.Echo ) pTnc->PutChar(c);
				m_cmdTNC.WriteChar(c);
				if( m_cmdTNC.LoadText(bf, sizeof(bf)) ){
					CmdTnc241(bf);
				}
			}
			else {					// ÉÅÉbÉZÅ[ÉWÉÇÅ[Éh
				if( TNC.Echo == 1 ) pTnc->PutChar(c);
				if( (c != LF) && (c != '#') ){
					bf[0] = c;
					bf[1] = 0;
					if( SBTX->Down ){OutputStr(bf);}else {m_TNCStg += bf[0];}
					if( c == CR ){
						bf[0] = LF;
						if( SBTX->Down ){OutputStr(bf);}else {m_TNCStg += bf[0];}
					}
				}
			}
			break;
	}
}
void __fastcall TMmttyWd::RxKAM(char c)
{
	char bf[256];
	static int MODE;
	int		dd;

	switch(c){
		case 0x03:		// Ctrl+C
			if( TNC.TncMode ){
				pTnc->OutStr("cmd:");
				m_TNCStg = "";
			}
			else {
				MODE = 1;
			}
			break;
		case 0x0e:		// Ctrl+N
			OutputStr("%F");
			break;
		case 0x0c:		// Ctrl+L
			OutputStr("%L");
			break;
		case 0x18:		// Ctrl+X
			RxCommon(TNC_CLEARBUF);
//            pSound->FSKMOD.DeleteTXBuf();
//			m_TNCStg = "";
			break;
		default:
			if( TNC.TncMode ){		// CommandÉÇÅ[Éh
				if( TNC.Echo ) pTnc->PutChar(c);
				m_cmdTNC.WriteChar(c);
				if( m_cmdTNC.LoadText(bf, sizeof(bf)) ){
					CmdKAM(bf);
				}
			}
			else if( MODE ){
				MODE = 0;
				c = char(toupper(c));
				switch(c){
					case 'S':	// shift change
						SetShift(170.0);
						m_DisEvent++;
						ShiftFreq->Text = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
						m_DisEvent--;
						sscanf(AnsiString(ShiftFreq->Text).c_str(), "%d", &dd);	//JA7UDE 0428
						pTnc->OutStr("-170 SHIFT-");
						break;
					case 'I':	// revers shift
						SBRev->Down = SBRev->Down ? 0 : 1;
						SBRevClick(NULL);
						break;
					case 'T':	// TX
						RxCommon(TNC_PTTON);
//						ToTX(NULL);
//						if( !m_TNCStg.IsEmpty() ){
//							OutputStr(m_TNCStg.c_str());
//							m_TNCStg = "";
//						}
						break;
					case 'R':	// RX
						RxCommon(TNC_PTTOFF);
//						ToRX(0);
//						m_TNCStg = "";
						break;
					case 'E':	// RX after TX
						RxCommon(TNC_PTTFLUSH);
//						if( IsTxIdle() ){
//							ToRX(0);
//						}
//						else {
//							m_ReqRecv = 1;
//						}
						break;
					case 'X': 	// To cmd
						pTnc->OutStr("cmd:");
						TNC.TncMode = 1;
						m_TNCStg = "";
						break;
					default:
						if( isdigit(c) ){
							const int tbl[]={
								45, 45, 50, 57, 75, 100, 110, 150, 200, 300
							};
							dd = tbl[c & 0x0f];
							pSound->FSKDEM.SetBaudRate(dd != 45 ? dd : 45.45);
							pSound->FSKMOD.SetBaudRate(dd != 45 ? dd : 45.45);
							SelectCombo(FALSE);
						}
				}
			}
			else {					// ÉÅÉbÉZÅ[ÉWÉÇÅ[Éh
				if( TNC.Echo == 1 ) pTnc->PutChar(c);
				if( (c != LF) && (c != '#') ){
					bf[0] = c;
					bf[1] = 0;
					if( SBTX->Down ){OutputStr(bf);}else {m_TNCStg += bf[0];}
					if( c == CR ){
						bf[0] = LF;
						if( SBTX->Down ){OutputStr(bf);}else {m_TNCStg += bf[0];}
					}
				}
			}
			break;
	}
}
void __fastcall TMmttyWd::CmdTnc241(LPSTR p)
{
	int dd;

	jstrupr(p);
	LPSTR	pCmd;
	p = SkipSpace(StrDlm(pCmd, SkipSpace(p), ' '));
	if( !strcmp(pCmd, "MODE") ){
		TNC.TncMode = 2;
		pTnc->OutLine("MODE     was TTY R");
	}
	else if( !strcmp(pCmd, "SHIFT") ){
		pTnc->OutLine("SHIFT    was %s", ShiftFreq->Text.c_str());
		if( !strcmp(p, "M") ){
			dd = 170;
		}
		else {
			sscanf(p, "%d", &dd);
		}
		SetShift(double(dd));
		m_DisEvent++;
		ShiftFreq->Text = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
		m_DisEvent--;
	}
	else if( !strcmp(pCmd, "RXREV") || !strcmp(pCmd, "TXREV") ){
		pTnc->OutLine("%s    was %s", pCmd, SBRev->Down ? "OFF":"ON");
		if( !strcmp(p, "ON" ) ){
			SBRev->Down = 0;
			SBRevClick(NULL);
		}
		else {
			SBRev->Down = 1;
			SBRevClick(NULL);
		}
	}
	else if( !strcmp(pCmd, "RTBAUD") ){
		pTnc->OutLine("RTBAUD   was %u", int(pSound->FSKDEM.GetBaudRate()));
		sscanf(p, "%d", &dd);
		if( (dd >= 20) && (dd < 600) ){
			pSound->FSKDEM.SetBaudRate(dd != 45 ? dd : 45.45);
			pSound->FSKMOD.SetBaudRate(dd != 45 ? dd : 45.45);
			SelectCombo(FALSE);
		}
	}
	else if( !strcmp(pCmd, "K") ){
		TNC.TncMode = 0;
		m_TNCStg = "";
	}
	if( TNC.TncMode ){
		if( TNC.TncMode == 1 ){
			pTnc->OutStr("cmd:");
		}
		else {
			pTnc->OutStr("tty:");
		}
	}
	else {
		pTnc->OutStr("\r");
	}
}
void __fastcall TMmttyWd::CmdKAM(LPSTR p)
{
	int dd;

	jstrupr(p);
	LPSTR	pCmd;
	p = SkipSpace(StrDlm(pCmd, SkipSpace(p), ' '));
	if( !strcmp(pCmd, "RTTY") ){
		TNC.TncMode = 0;
		pTnc->OutLine("-RTTY %u-", int(pSound->FSKDEM.GetBaudRate()));
		m_TNCStg = "";
	}
	else if( !strcmp(pCmd, "SPACE") ){
		pTnc->OutLine("SPACE    was %s", pSound->FSKDEM.GetSpaceFreq());
		sscanf(p, "%d", &dd);
		pSound->FSKDEM.SetSpaceFreq(dd);
		pSound->FSKMOD.SetSpaceFreq(dd);
		UpdateUI();
	}
	else if( !strcmp(pCmd, "MARK") ){
		pTnc->OutLine("MARK     was %s", pSound->FSKDEM.GetMarkFreq());
		sscanf(p, "%d", &dd);
		pSound->FSKDEM.SetMarkFreq(dd);
		pSound->FSKMOD.SetMarkFreq(dd);
		if( pRadio != NULL ) pRadio->SetMarkFreq(dd);
		UpdateUI();
	}
	if( TNC.TncMode ){
		pTnc->OutStr("cmd:");
	}
	else {
		pTnc->OutStr("\r");
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PutTNC(char c)
{
	if( (pTnc != NULL) && !TNC.TncMode ){
		if( !SBTX->Down || ((TNC.Echo == 2) && (TNC.Type != 2)) || ((TNC.Type==2)&&(sys.m_echo==2)) ){
			pTnc->PutChar(c);
		}
	}
	if( Remote ){
		PostApp(TXM_CHAR, (unsigned char)c);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OpenCloseTNC(void)
{
	if( pTnc != NULL ) delete pTnc;
	pTnc = NULL;
	TNC.change = 0;
	KTNC->Checked = FALSE;
	if( Remote ) return;

	if( strcmp(TNC.StrPort, "NONE" ) ){
		COMMPARA	cm;
		SetFSKPara(&cm);
		pTnc = new CCtnc(TRUE);
		pTnc->Open(&TNC, Handle, CM_CTNC, &cm);
		if( (TNC.Type == 2) && (pTnc->m_CreateON != TRUE) ){
			if( (cm.BitLen == 5) && (cm.Stop == TWOSTOPBITS) ){
				cm.Stop = ONE5STOPBITS;		// set 1.5bits
				pTnc->Open(&TNC, Handle, CM_CTNC, &cm);
			}
		}
		if( pTnc->m_CreateON != TRUE ){
			delete pTnc;
			pTnc = NULL;
			ErrorMB( (Font->Charset != SHIFTJIS_CHARSET)? "Cannot open '%s' for TNC":"TNCñÕã[É|Å[Ég'%s'Ç™ÉIÅ[ÉvÉìÇ≈Ç´Ç‹ÇπÇÒ.", TNC.StrPort);
		}
		else {
			sys.m_DisTX &= ~1;
			KTNC->Checked = TRUE;
			if( TNC.Type != 2 ){
				pTnc->OutStr("\r\n" VERTTL "\r\n%sTNC emulation.\r\n", pTnc->IsMMT() ? "Custom " : "");
				if( !pTnc->IsMMT() ){
					if( TNC.Type ){
						pTnc->OutStr("Command 'RTTY'\r\n");
						pTnc->OutStr("To RX:	Ctrl+C,R\r\n");
						pTnc->OutStr("To TX:	Ctrl+C,T\r\n");
					}
					else {
						pTnc->OutStr("Command 'K'\r\n");
						pTnc->OutStr("To RX:	Ctrl+R\r\n");
						pTnc->OutStr("To TX:	Ctrl+T\r\n");
					}
                }
				pTnc->OutStr("\r\ncmd:");
				TNC.TncMode = 1;
			}
			else {
				TNC.TncMode = 0;
			}
			m_TNCStg = "";
			SetNMMT();
            RemoteFreq();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KTNCClick(TObject *Sender)
{
	TTNCSetDlg *pBox = new TTNCSetDlg(this);

	if( pBox->Execute() == TRUE ){
		FormResize(NULL);
		UpdateStayOnTop();
		OpenCloseCom();
	}
	delete pBox;
	TopWindow(this);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteFreq(void)
{
	if( !Remote && !pTnc ) return;

	int mark, space;

	if( SBTX->Down && (sys.m_echo != 2) ){
		mark = DWORD(pSound->FSKMOD.GetMarkFreq());
		space = DWORD(pSound->FSKMOD.GetSpaceFreq());
	}
	else {
		mark = DWORD(pSound->FSKDEM.GetMarkFreq());
		space = DWORD(pSound->FSKDEM.GetSpaceFreq());
	}
	if( pTnc && pTnc->IsMMT() ){
    	pTnc->NotifyFreq(mark, space);
    }
    if( !Remote ) return;
	if( m_RemoteMark != mark ){
		PostApp(TXM_MARK, mark);
		m_RemoteMark = mark;
	}
	if( m_RemoteSpace != space ){
		PostApp(TXM_SPACE, space);
		m_RemoteSpace = space;
	}
	mark = pSound->m_lms.m_lmsNotch + (pSound->m_lms.m_lmsNotch2 << 16);
	if( mark != m_RemoteNotch ){
		PostApp(TXM_NOTCH, mark);
		m_RemoteNotch = mark;
	}
	if( int(sys.m_DefShift) != m_RemoteDefShift ){
		PostApp(TXM_DEFSHIFT, sys.m_DefShift);
		m_RemoteDefShift = sys.m_DefShift;
	}
}
//---------------------------------------------------------------------------
DWORD __fastcall TMmttyWd::GetRemoteSwitch(void)
{
	DWORD d = pSound->FSKDEM.m_type & 0x03;	//b0-b1
	d |= SBAFC->Down ? 0x0004 : 0;			//b2
	d |= SBNET->Down ? 0x0008 : 0;			//b3
	d |= SBATC->Down ? 0x0010 : 0;			//b4
	d |= SBBPF->Down ? 0x0020 : 0;			//b5
	d |= SBLMS->Down ? 0x0040 : 0;			//b6
	d |= SBSQ->Down ? 0x0080 : 0;			//b7
	d |= SBRev->Down ? 0x0100 : 0;			//b8
	d |= SBUOS->Down ? 0x0200 : 0;			//b9
	d |= (sys.m_FixShift << 10);			//b10-b11
	d |= pSound->FSKDEM.m_lpf ? 0x1000 : 0;	//b12
	d |= pSound->m_lms.m_Type ? 0x2000 : 0; //b13
	d |= pSound->m_lms.m_twoNotch ? 0x4000 : 0;	// b14
	d |= (Remote & REMVIATX) ? 0x8000 : 0;	// b15
	d |= KWP->Checked ? 0x10000 : 0;	// b16
	d |= (m_SendWay << 17);				// b17-b18
	// Added by JE3HHT on Sep.2010
	d |= pSound->FSKDEM.m_AA6YQ.m_fEnabled ? 0x80000 : 0;	// b19
	return d;
}
//---------------------------------------------------------------------------
DWORD __fastcall TMmttyWd::GetRemoteView(void)
{
	DWORD d = pSound->m_FFTFW & 0x0007;		// 0000000000000xxx
	d |= ((sys.m_FFTGain&0x0003) << 4);		// 0000000000xx0000
	d |= ((sys.m_FFTResp - 1) << 6);		// 00000000xx000000
	d |= (m_XYSize << 8);					// 0000000x00000000
	d |= (m_XYQuality << 10);				// 0000xx0000000000
	d |= KXYScope->Checked ? 0x1000 : 0;	// 000x000000000000
	d |= ((sys.m_FFTGain&0x000c) << 11);	// 0xx0000000000000
    return d;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteStat(void)
{
	int d;
	if( Remote ){
		d = Width + (Height << 16);
		if( m_RemoteHeight != d ){
			PostApp(TXM_HEIGHT, d);
			m_RemoteHeight = d;
		}
		d = DWORD(pSound->FSKDEM.GetBaudRate() * 100);
		if( m_RemoteBaud != d ){
			PostApp(TXM_BAUD, d);
			m_RemoteBaud = d;
		}
    }
	RemoteFreq();		// MMTÇÃÇΩÇﬂÇ…åƒÇ—èoÇµÇ‹Ç∑
    if( !Remote ) return;
	d = GetRemoteSwitch();
	if( m_RemoteSwitch != d ){
		PostApp(TXM_SWITCH, d);
		m_RemoteSwitch = d;
	}
	d = GetRemoteView();
	if( m_RemoteView != d ){
		PostApp(TXM_VIEW, d);
		m_RemoteView = d;
	}
	d = DWORD(pSound->FSKDEM.m_iirfw) + (DWORD(pSound->FSKDEM.GetFilterTap()) << 16);
	if( m_RemoteIIR != d ){
		PostApp(TXM_RESO, d);
		m_RemoteIIR = d;
	}
	d = DWORD(pSound->FSKDEM.GetSmoozFreq())+(DWORD(pSound->FSKDEM.m_lpffreq) << 16);
	if( m_RemoteLPF != d ){
		PostApp(TXM_LPF, d);
		m_RemoteLPF = d;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteSwitch(DWORD d)
{
	pSound->FSKDEM.m_type = d & 0x0003;
	pSound->m_lms.m_Type = d & 0x2000 ? 1 : 0; UpdateLMS();
	pSound->m_lms.m_twoNotch = d & 0x4000 ? 1 : 0;
	SBAFC->Down = d & 0x0004 ? 1 : 0; SBAFCClick(NULL);
	SBNET->Down = d & 0x0008 ? 1 : 0; SBNETClick(NULL);
	SBATC->Down = d & 0x0010 ? 1 : 0; SBATCClick(NULL);
	SBBPF->Down = d & 0x0020 ? 1 : 0; SBBPFClick(NULL);
	SBLMS->Down = d & 0x0040 ? 1 : 0; SBLMSClick(NULL);
	SBSQ->Down = d & 0x0080 ? 1 : 0; pSound->FSKDEM.SetSQ(SBSQ->Down);
	SBRev->Down = d & 0x0100 ? 1 : 0; SBRevClick(NULL);
	SBUOS->Down = d & 0x0200 ? 1 : 0; SBUOSClick(NULL);
	sys.m_FixShift = (d >> 10) & 0x0003;
	pSound->FSKDEM.m_lpf = d & 0x1000 ? 1 : 0;
	if( d & 0x8000 ){ Remote |= REMVIATX;} else { Remote &= ~REMVIATX;}
	KWP->Checked = d & 0x10000 ? 1 : 0;
	m_SendWay = (d >> 17) & 0x03;

	// Added by JE3HHT on Sep.2010
	int f = (d & 0x80000) ? 1 : 0;			// b19
    if( f != pSound->FSKDEM.m_AA6YQ.m_fEnabled ){
		pSound->FSKDEM.m_AA6YQ.m_fEnabled = f;
		if( f ) pSound->FSKDEM.m_AA6YQ.Create();
    }
   	UpdateItem();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteView(DWORD d)
{
	m_DisEvent++;
	pSound->m_FFTFW = d & 0x0007;
	sys.m_FFTGain = (d >> 4) & 0x0003;
	sys.m_FFTGain |= (d >> 11) & 0x000c;
	sys.m_FFTResp = ((d >> 6) & 0x0003) + 1;
	m_XYSize = (d >> 8) & 0x0001;
	m_XYQuality = (d >> 10) & 0x0003;
	KXYScope->Checked = d & 0x1000 ? 1 : 0;
	UpdateXYScope();
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteSigLevel(DWORD d)
{
	if( !Remote && !pTnc ) return;

	DWORD sq = pSound->FSKDEM.GetSQLevel();
	if( Remote ){
		PostApp(TXM_LEVEL, (sq << 16) + d);
    }
    else if( pTnc ){
		pTnc->NotifyLevel(sq, d);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SetRemoteFFT(void)
{
	if( Remote & REMDISSHARED ) return;

	int i, x;
	double smp = pMap->smpFFT ? 8000 : 11025;
	if( sys.m_FFTGain >= 4 ){
	if( FFT_SIZE < 2048 ){
		smp *= 0.5;
		for( i = 0; i < 2048; i++ ){
			x = (i * smp / SampFreq) + 0.5;
			if( x < FFT_SIZE ){
				pMap->arrayFFT[i] =  pSound->fftIN.m_fft[x] * 0.1;
			}
			else {
				for( ; i < 2048; i++ ) pMap->arrayFFT[i] =  0;
			}
		}
		pMap->smpFreq = pMap->smpFFT ? 8000 : 11025;
	}
	else if( SampType || pMap->smpFFT ){
		for( i = 0; i < 2048; i++ ){
			x = (i * smp / SampFreq) + 0.5;
			if( x < FFT_SIZE ){
				pMap->arrayFFT[i] =  pSound->fftIN.m_fft[x] * 0.1;
			}
			else {
				for( ; i < 2048; i++ ) pMap->arrayFFT[i] =  0;
			}
		}
		pMap->smpFreq = pMap->smpFFT ? 8000 : 11025;
	}
	else {
		for( i = 0; i < 2048; i++ ){
			pMap->arrayFFT[i] =  pSound->fftIN.m_fft[i] * 0.1;
		}
		pMap->smpFreq = SampFreq;
	}
	}
	else {
	if( FFT_SIZE < 2048 ){
		smp *= 0.5;
		for( i = 0; i < 2048; i++ ){
			x = (i * smp / SampFreq) + 0.5;
			if( x < FFT_SIZE ){
				pMap->arrayFFT[i] =  pSound->fftIN.m_fft[x];
			}
			else {
				for( ; i < 2048; i++ ) pMap->arrayFFT[i] =  0;
			}
		}
		pMap->smpFreq = pMap->smpFFT ? 8000 : 11025;
	}
	else if( SampType || pMap->smpFFT ){
		for( i = 0; i < 2048; i++ ){
			x = (i * smp / SampFreq) + 0.5;
			if( x < FFT_SIZE ){
				pMap->arrayFFT[i] =  pSound->fftIN.m_fft[x];
			}
			else {
				for( ; i < 2048; i++ ) pMap->arrayFFT[i] =  0;
			}
		}
		pMap->smpFreq = pMap->smpFFT ? 8000 : 11025;
	}
	else {
		memcpy(pMap->arrayFFT, pSound->fftIN.m_fft, sizeof(pMap->arrayFFT));
		pMap->smpFreq = SampFreq;
	}
	}
	pMap->flagFFT = 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::InitRemoteStat(void)
{
	m_RemoteSwitch = -1;
	m_RemoteView = -1;
	m_RemoteBaud = -1;
	m_RemoteHeight = -1;
	m_RemoteIIR = -1;
	m_RemoteLPF = -1;
	m_RemoteMark = -1;
	m_RemoteSpace = -1;
	m_RemoteNotch = -1;
	m_RemoteDefShift = -1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteMMTTY(tagMSG &Msg)
{
	char	bf[16];
	double	d;
	int		di;

	switch(Msg.wParam){
		case RXM_HANDLE:		// ÉEÉCÉìÉhÉEÉnÉìÉhÉãÇÃí ím
			m_RemoteTimer = 0;
			APP_HANDLE = (HWND)Msg.lParam;
			if( APP_HANDLE != HWND_BROADCAST){
				::SetForegroundWindow(APP_HANDLE);
				InitRemoteStat();
				RemoteStat();
				PostApp(TXM_PTTEVENT, SBTX->Down);
			}
			break;
		case RXM_REQHANDLE:		// ÉEÉCÉìÉhÉEÉnÉìÉhÉãÇÃóvãÅ
			::PostMessage(APP_HANDLE, MSG_MMTTY, TXM_HANDLE, (DWORD)Handle);
			break;
		case RXM_PTT:			// ëóéÛÇÃêÿÇËë÷Ç¶
			switch(Msg.lParam){
				case 0:
                case 5:
					if( Remote & REMVIATX ) FifoEdit.Clear();
					ToRX(Msg.lParam ? 0 : 1);
					break;
				case 1:
					if( ((Remote & REMVIATX) && FifoEdit.GetLen()) || !IsTxIdle() ){
						m_ReqRecv = 1;
					}
					else {
						ToRX(0);
					}
					break;
				case 2:
					ToTX(NULL);
					break;
				case 3:
					ToTX("~");
					break;
				case 4:
					if( Remote & REMVIATX ) FifoEdit.Clear();
					pSound->FSKMOD.DeleteTXBuf();
					if( pComm != NULL ) pComm->InitTxBuf();
					break;
			}
			break;
		case RXM_CHAR:
			bf[0] = char(Msg.lParam);
			if( Remote & REMNOCNV ){
				pSound->FSKMOD.PutData(bf[0]);
			}
			else if( Remote & REMVIATX ){
				if( bf[0] != LF ) PushKey(bf[0]);
			}
			else if( bf[0] != '#' ){
				bf[1] = 0;
				OutputStr(bf);
			}
			break;
		case RXM_EXIT:
			pMap = NULL;
			Close();
			break;
		case RXM_WINPOS:
			Top = Msg.lParam >> 16;
			Left = Msg.lParam & 0x0000ffff;
			break;
		case RXM_WIDTH:
			di = Msg.lParam & 0x0000ffff;
			if( di > 10 ) Width = di;
			di = Msg.lParam >> 16;
			if( di > 10 ) Height = di;
			break;
		case RXM_REQPARA:
			InitRemoteStat();
			RemoteStat();
			break;
		case RXM_SETBAUD:
			if( (Msg.lParam > 10) && (Msg.lParam <= 120000) ){
				d = Msg.lParam;
				d /= 100.0;
				pSound->FSKMOD.SetBaudRate(d);
				pSound->FSKDEM.SetBaudRate(d);
				SelectCombo(FALSE);
			}
			RemoteStat();
			break;
		case RXM_SETMARK:
			if( (Msg.lParam > 100) && (Msg.lParam < 3000) ){
				d = Msg.lParam;
				if( !SBTX->Down || (sys.m_echo != 2) ) pSound->FSKMOD.SetMarkFreq(d);
				pSound->FSKDEM.SetMarkFreq(d);
				if( pRadio != NULL ) pRadio->SetMarkFreq(d);
				if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
				m_DisEvent++;
				MarkFreq->Text = int(d);
				ShiftFreq->Text = int(pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq());
				m_DisEvent--;
			}
			RemoteStat();
			break;
		case RXM_SETSPACE:
			if( (Msg.lParam > 100) && (Msg.lParam < 3000) ){
				d = Msg.lParam;
				if( !SBTX->Down || (sys.m_echo != 2) ) pSound->FSKMOD.SetSpaceFreq(d);
				pSound->FSKDEM.SetSpaceFreq(d);
				if( SBBPF->Down || SBLMS->Down ) pSound->CalcBPF();
				m_DisEvent++;
				MarkFreq->Text = int(pSound->FSKDEM.GetMarkFreq());
				ShiftFreq->Text = int(pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq());
				m_DisEvent--;
			}
			RemoteStat();
			break;
		case RXM_SETSWITCH:
			RemoteSwitch(Msg.lParam);
			break;
		case RXM_SETHAM:
			SBHAMClick(NULL);
			break;
		case RXM_SHOWSETUP:
			if( !m_OptionDialog ){
				m_OptionDialog++;
				DoOption(NULL, Msg.lParam);
				m_OptionDialog--;
				::SetForegroundWindow(APP_HANDLE);
			}
            else if( m_hOptionWnd ){
				::SetForegroundWindow(m_hOptionWnd);
            }
			break;
		case RXM_SETVIEW:
			RemoteView(Msg.lParam);
			break;
		case RXM_SETSQLVL:
			if( Msg.lParam >= SQSCALE ) Msg.lParam = SQSCALE - 1;
			pSound->FSKDEM.SetSQLevel(Msg.lParam);
			break;
		case RXM_SHOW:
			if( Msg.lParam ){
				if( !(Remote & REMSHOWOFF) ){
					Show();
					if( Remote & REMNOMOVE ){
						FormResize(NULL);
					}
				}
			}
			else {
				Hide();
			}
			break;
		case RXM_SETFIG:
			SBFIG->Down = Msg.lParam;
			SBFIGClick(NULL);
			break;
		case RXM_SETRESO:
			{
				int f = 0;
				di = Msg.lParam & 0x0000ffff;
				if( (di > 0) && (di < 500) ){
					pSound->FSKDEM.SetIIR(di);
					f = 1;
				}
				di = Msg.lParam >> 16;
				if( (di > 0) && (di <= 512) ){
					pSound->FSKDEM.SetFilterTap(di);
					f = 1;
				}
				if( f ) SelectCombo(FALSE);
			}
			break;
		case RXM_SETLPF:
			di = Msg.lParam & 0x0000ffff;
			if( (di > 10) && (di < 1000) ){
				pSound->FSKDEM.SetSmoozFreq(di);
			}
			di = Msg.lParam >> 16;
			if( (di > 10) && (di <= 1000) ){
				pSound->FSKDEM.m_lpffreq = di;
			}
			SelectCombo(FALSE);
			break;
		case RXM_SETTXDELAY:
			di = Msg.lParam & 0x0000ffff;
			if( (di >= 0) && (di < 10000) ){
				m_RemoteDelay = di;
			}
			break;
		case RXM_UPDATECOM:
			UpdateComarray();
			break;
		case RXM_SUSPEND:
			if( Msg.lParam ){
				KExtSuspClick(NULL);
				if( !(Remote & REMSHOWOFF) ){
					Hide();
				}
			}
			else {
				KExtResetClick(NULL);
				if( !(Remote & REMSHOWOFF) ){
					Show();
				}
			}
			break;
		case RXM_NOTCH:
			pSound->m_lms.m_lmsNotch2 = pSound->m_lms.m_lmsNotch;
			pSound->m_lms.m_lmsNotch = Msg.lParam & 0x0000ffff;
			if( SBLMS->Down ) pSound->CalcBPF();
            RemoteFreq();
			break;
		case RXM_PROFILE:
			RemoteProfile(Msg.lParam);
			break;
		case RXM_TIMER:
			if( Msg.lParam & 0xffff0000 ){
				KPttTimClick(NULL);
			}
			else {
				m_PttTimer = Msg.lParam & 0x0000ffff;
				UpdatePttTimer();
				KPttTim->Checked = m_PttTimer ? 1 : 0;
			}
			break;
        case RXM_ENBFOCUS:
			if( Msg.lParam ){
				Remote &= ~REMDISFOCUS;
            }
            else {
				Remote |= REMDISFOCUS;
            }
        	break;
        case RXM_SETDEFFREQ:
			sys.m_DefMarkFreq = LOWORD(Msg.lParam);
            sys.m_DefShift = HIWORD(Msg.lParam);
        	break;
        case RXM_SETLENGTH:
			if( (Msg.lParam >= 5) && (Msg.lParam <= 8) ){
	        	pSound->FSKDEM.m_BitLen = Msg.lParam;
    	        pSound->FSKMOD.m_BitLen = Msg.lParam;
        	    UpdateFSK();
            }
            break;
		case RXM_ENBSHARED:
        	if( Msg.lParam ){
				Remote &= ~REMDISSHARED;
            }
            else {
				Remote |= REMDISSHARED;
            }
        	break;
		case RXM_PTTFSK:		// Added by JE3HHT on Sep.2010
			if( sys.m_TxPort != (Msg.lParam & 0x03) ){
				sys.m_TxPort = Msg.lParam & 0x03;
				COMM.change = 1;
				OpenCloseCom();
            }
			break;
		case RXM_SOUNDSOURCE:		// Added by AA6YQ 1.70F
			if( sys.m_SoundStereo != (Msg.lParam & 0x03) ){
				sys.m_SoundStereo = Msg.lParam & 0x03;
				pSound->InitSound();
            }
			break;
		default:
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateMacroShow(int sw)
{
	if( Remote ) return;

	GroupM->Visible = KMac->Checked;
	PanelMac->Visible = KMac->Checked ? 0 : 1;
	TSpeedButton *tp;
	int i;
	if( GroupM->Visible == TRUE ){
		if( SBM1->Parent != GroupM ){
			for( i = 0; i < 16; i++ ){
				tp = GetSB(i);
				PanelMac->RemoveControl(tp);
				GroupM->InsertControl(tp);
				tp->Parent = GroupM;
			}
		}
	}
	else {
		if( SBM1->Parent != PanelMac ){
			for( i = 0; i < 16; i++ ){
				tp = GetSB(i);
				GroupM->RemoveControl(tp);
				PanelMac->InsertControl(tp);
				tp->Parent = PanelMac;
			}
		}
	}
	if( sw ){
		PanelMac->Align = alNone;
		PanelQSO->Align = alNone;
		PanelRx->Align = alNone;
		PanelIn->Align = alNone;
		PanelStat->Align = alNone;

		PanelMac->Align = alTop;
		PanelQSO->Align = alTop;
		PanelRx->Align = alTop;
		if( !sys.m_DisWindow ){
			PanelIn->Align = alBottom;
			PanelStat->Align = alBottom;
		}
		FormResize(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KMacClick(TObject *Sender)
{
	KMac->Checked = KMac->Checked ? 0 : 1;
	UpdateMacroShow(1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KENTClick(TObject *Sender)
{
	KENT->Checked = KENT->Checked ? 0 : 1;
}
#if 0
//---------------------------------------------------------------------------
// RadioÉfÅ[É^éÛêMÇÃÉCÉxÉìÉg
//
//    	pRadio = new CCradio(TRUE);
//     	pRadio->Open(PortTnc, &ParaTnc, Handle, CM_COMM_UP);
void __fastcall TMmttyWd::CMCradio(TMessage *Message)
{
	if( pRadio == NULL ) return;

	char bf[RADIO_COMBUFSIZE];
	DWORD	len = pRadio->RecvLen();
	if( len >= RADIO_COMBUFSIZE ) len = RADIO_COMBUFSIZE;
	pRadio->Read((BYTE *)bf, len);
}
#endif
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::OpenCloseRadio(void)
{
	if( !RADIO.change ) return;
	if( pRadio != NULL ) delete pRadio;
	pRadio = NULL;
	RADIO.change = 0;
	if( strcmp(RADIO.StrPort, "NONE" ) ){
		pRadio = new CCradio(TRUE);
		pRadio->Open(&RADIO, Handle, CM_CMMR, CM_CRADIO);
		if( pRadio->m_CreateON != TRUE ){
			delete pRadio;
			pRadio = NULL;
			if( !Remote || !m_RemoteTimer ){
				ErrorMB( (Font->Charset != SHIFTJIS_CHARSET)? "Can't open '%s' for Radio commands":"RadioÉRÉ}ÉìÉhÉ|Å[Ég'%s'Ç™ÉIÅ[ÉvÉìÇ≈Ç´Ç‹ÇπÇÒ.", RADIO.StrPort);
			}
		}
		else {
			pRadio->SetMarkFreq(pSound->FSKDEM.GetMarkFreq());
			pRadio->SendCommand(RADIO.CmdInit.c_str());
			if( RADIO.PollScan ){
				m_DisEvent++;
				Freq->Text = "???";
				m_DisEvent--;
				RADIO.Cmdxx = 0x00;
				pRadio->m_ScanAddr = 1;
			}
		}
	}
	if( (pMap != NULL) && !(Remote & REMDISSHARED) ) StrCopy(pMap->comRadio, RADIO.StrPort, sizeof(pMap->comRadio)-1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::HisCallClick(TObject *Sender)
{
	ClearMacroTimer();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateLWait(void)
{
	if( pSound->FSKMOD.m_CharWaitDiddle ) sys.m_LWait = 3;
	m_DisEvent++;
	switch(sys.m_LWait){
		case 0:			// Char.wait
			LWait->Caption = "Char. wait";
			TBCharWait->Position = SetTBValue(pSound->FSKMOD.m_CharWait, 50, 50);
			break;
		case 1:			// Diddle. wait
			LWait->Caption = "Diddle wait";
			TBCharWait->Position = SetTBValue(pSound->FSKMOD.m_DiddleWait, 50, 50);
			break;
		case 2:			// Char+Diddle
			LWait->Caption = "Both wait";
			TBCharWait->Position = SetTBValue(pSound->FSKMOD.m_CharWait, 50, 50);
			break;
		case 3:			// No diddle wait
			LWait->Caption = "Disable wait";
			TBCharWait->Position = SetTBValue(pSound->FSKMOD.m_CharWait, 50, 50);
			break;
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
// WaitÇÃÉNÉäÉbÉN
void __fastcall TMmttyWd::LWaitClick(TObject *Sender)
{
	sys.m_LWait++;
	if( sys.m_LWait >= 4 ) sys.m_LWait = 0;
	pSound->FSKMOD.m_CharWaitDiddle = ( sys.m_LWait == 3 ) ? 1 : 0;
	UpdateLWait();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KWPClick(TObject *Sender)
{
	KWP->Checked = KWP->Checked ? 0 : 1;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBDataMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		int n = Log.m_CurNo - 1;
		if( n < 0 ){
			KQSOClick(NULL);
			return;
		}

		TQSODlgBox *pBox = new TQSODlgBox(this);
		SDMMLOG	sd;
		Log.GetData(&sd, n);
		CLogFind	Find;

		Log.FindSet(&Find, sd.call);

		pBox->Execute(&Find, &sd, n);
		delete pBox;
		TopWindow(this);
		if( !SBQSO->Down && Log.m_CurNo ){
			Log.SetLastPos();
			UpdateTextData();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::PostApp(WPARAM wParam, LPARAM lParam)
{
	if(APP_HANDLE != HWND_BROADCAST){
		::PostMessage(APP_HANDLE, MSG_MMTTY, wParam, lParam);
	}
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmttyWd::GetKS(int n)
{
	TMenuItem *__KS[]={
		KS1, KS2, KS3, KS4, KS5, KS6, KS7, KS8,
	};
	return __KS[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmttyWd::GetKDS(int n)
{
	TMenuItem *__KDS[]={
		KDS1, KDS2, KDS3, KDS4, KDS5, KDS6, KDS7, KDS8,
	};
	return __KDS[n];
}
//---------------------------------------------------------------------------
TMenuItem *__fastcall TMmttyWd::GetKES(int n)
{
	TMenuItem *__KES[]={
		KES1, KES2, KES3, KES4, KES5, KES6, KES7, KES8,
	};
	return __KES[n];
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdateProfile(void)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *p = GetKS(i);
		GetKDS(i)->Caption = p->Caption;
		GetKES(i)->Caption = p->Caption;
		GetKDS(i)->Enabled = p->Enabled;
		SetProMenuName(i, AnsiString(p->Caption).c_str());	//JA7UDE -428
	}
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ©ÇÁÇÃì«Ç›èoÇµ
void __fastcall TMmttyWd::ReadProfileList(void)
{
	char	bf[256];
	AnsiString	as;

	sprintf(bf, "%sUserPara.ini", BgnDir);
	TMemIniFile	*pIniFile = new TMemIniFile(bf);
	char	key[32];
	int i;
	for( i = 0; i < 8; i++ ){
		sprintf(key, "Define%d", i);
		TMenuItem *p = GetKS(i);
		as = pIniFile->ReadString(key, "Name", p->Caption);
		p->Caption = as.c_str();
		p->Enabled = pIniFile->ReadInteger(key, "Enabled", p->Enabled);
		if( pMap != NULL ){
			StrCopy(pMap->ProfileName[i], p->Enabled ? as.c_str() : "", 63);
		}
	}
	if( pMap != NULL ){
		for( ; i < 16; i++ ){
			sprintf(key, "Define%d", i);
			as = pIniFile->ReadString(key, "Name", "");
			StrCopy(pMap->ProfileName[i], as.c_str(), 63);
		}
	}
	delete pIniFile;
	UpdateProfile();
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ©ÇÁÇÃì«Ç›èoÇµ
void __fastcall TMmttyWd::ReadProfile(int n, LPCSTR pName)
{
	char	bf[256];
	AnsiString	as, ws;

	if( n == 1024 ){
		strcpy(bf, pName);
	}
	else {
		sprintf(bf, "%sUserPara.ini", BgnDir);
	}
	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	char	key[32];
	sprintf(key, "Define%d", n);

	pSound->Suspend();

	sys.m_FixShift = pIniFile->ReadInteger(key, "AFCFixShift", sys.m_FixShift);
	sys.m_AFC = pIniFile->ReadInteger(key, "AFC", sys.m_AFC);
	sys.m_AFCSQ = pIniFile->ReadInteger(key, "AFCSQ", sys.m_AFCSQ);
	sys.m_AFCTime = ReadDoubleIniFile(pIniFile, key, "AFCTime", sys.m_AFCTime);
	sys.m_AFCSweep = ReadDoubleIniFile(pIniFile, key, "AFCSweep", sys.m_AFCSweep);

	rtty.m_uos = pIniFile->ReadInteger(key, "UOS", rtty.m_uos);

	sys.m_TxNet = pIniFile->ReadInteger(key, "TxNet", sys.m_TxNet);
	sys.m_TxDisRev = pIniFile->ReadInteger(key, "TxDisRev", sys.m_TxDisRev);
	sys.m_LimitGain = ReadDoubleIniFile(pIniFile, key, "LimitDxGain", sys.m_LimitGain);
	pSound->FSKDEM.m_LimitOverSampling = pIniFile->ReadInteger(key, "LimitOverSampling", pSound->FSKDEM.m_LimitOverSampling);
	pSound->FSKDEM.m_LimitAGC = pIniFile->ReadInteger(key, "LimitAGC", pSound->FSKDEM.m_LimitAGC);
	pSound->FSKDEM.m_atc = pIniFile->ReadInteger(key, "ATC", pSound->FSKDEM.m_atc);
	pSound->FSKDEM.m_atcMark.m_Max = pIniFile->ReadInteger(key, "ATCTime", pSound->FSKDEM.m_atcMark.m_Max);
	pSound->FSKDEM.m_atcSpace.m_Max = pSound->FSKDEM.m_atcMark.m_Max;

	pSound->FSKDEM.m_majority = pIniFile->ReadInteger(key, "Majority", pSound->FSKDEM.m_majority);
	pSound->FSKDEM.m_ignoreFream = pIniFile->ReadInteger(key, "IgnoreFreamError", pSound->FSKDEM.m_ignoreFream);

	pSound->FSKDEM.SetSQ(pIniFile->ReadInteger(key, "SQ", pSound->FSKDEM.GetSQ()));
	pSound->FSKDEM.SetSQLevel(ReadDoubleIniFile(pIniFile, key, "SQLevel", pSound->FSKDEM.GetSQLevel()));

	sys.m_DefMarkFreq = ReadDoubleIniFile(pIniFile, key, "DefMarkFreq", sys.m_DefMarkFreq);
	sys.m_DefShift = ReadDoubleIniFile(pIniFile, key, "DefShift", sys.m_DefShift);
	sys.m_DefStopLen = pIniFile->ReadInteger(key, "DefStopLen", sys.m_DefStopLen);

	pSound->FSKMOD.SetOutputGain(ReadDoubleIniFile(pIniFile, key, "OutputGain", pSound->FSKMOD.GetOutputGain()));

	sys.m_echo = pIniFile->ReadInteger(key, "TXLoop", sys.m_echo);
	pSound->FSKMOD.m_bpftap = pIniFile->ReadInteger(key, "TXBPFTAP", pSound->FSKMOD.m_bpftap);
	pSound->FSKMOD.m_lpf = pIniFile->ReadInteger(key, "TXLPF", pSound->FSKMOD.m_lpf);
	pSound->FSKMOD.SetLPFFreq(ReadDoubleIniFile(pIniFile, key, "TXLPFFreq", pSound->FSKMOD.GetLPFFreq()));
	pSound->FSKMOD.CalcBPF();

	sys.m_LWait = pIniFile->ReadInteger(key, "TXWaitType", sys.m_LWait);
	pSound->FSKMOD.m_CharWait = pIniFile->ReadInteger(key, "TXCharWait", pSound->FSKMOD.m_CharWait);
	pSound->FSKMOD.m_DiddleWait = pIniFile->ReadInteger(key, "TXDiddleWait", pSound->FSKMOD.m_DiddleWait);
	pSound->FSKMOD.m_CharWaitDiddle = pIniFile->ReadInteger(key, "TXCharWaitDiddle", pSound->FSKMOD.m_CharWaitDiddle);
	pSound->FSKMOD.m_RandomDiddle = pIniFile->ReadInteger(key, "TXRandomDiddle", pSound->FSKMOD.m_RandomDiddle);
	pSound->FSKMOD.m_WaitTimer = pIniFile->ReadInteger(key, "TXWaitTimerDiddle", pSound->FSKMOD.m_WaitTimer);

	sys.m_Rev = pIniFile->ReadInteger(key, "Rev", sys.m_Rev);

	double	oldshift = pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq();
	double	sft = ReadDoubleIniFile(pIniFile, key, "ShiftFreq", oldshift);
	double	mark = pSound->FSKDEM.GetMarkFreq();
	mark -= (sft - oldshift) * 0.5;
	if( sft < oldshift ){
		mark = int(mark);
	}
	else {
		mark = int(mark+0.5);
	}
	double space = mark + sft;
	pSound->FSKDEM.SetSpaceFreq(space);
	pSound->FSKDEM.SetMarkFreq(mark);
	pSound->FSKMOD.SetMarkFreq(mark);
	pSound->FSKMOD.SetSpaceFreq(space);
	m_RxMarkFreq = mark;
	m_RxSpaceFreq = space;

	pSound->m_bpf = pIniFile->ReadInteger(key, "RXBPF", pSound->m_bpf);
	pSound->m_bpftap = pIniFile->ReadInteger(key, "RXBPFTAP", pSound->m_bpftap);
	pSound->m_bpfafc = pIniFile->ReadInteger(key, "RXBPFAFC", pSound->m_bpfafc);
	pSound->m_bpffw = ReadDoubleIniFile(pIniFile, key, "RXBPFFW", pSound->m_bpffw);

	pSound->m_lmsbpf = pIniFile->ReadInteger(key, "RXlms", pSound->m_lmsbpf);
	pSound->m_lms.m_lmsDelay = pIniFile->ReadInteger(key, "RXlmsDelay", pSound->m_lms.m_lmsDelay);
	pSound->m_lms.m_lmsMU2 = ReadDoubleIniFile(pIniFile, key, "RXlmsMU2", pSound->m_lms.m_lmsMU2);
	pSound->m_lms.m_lmsGM = ReadDoubleIniFile(pIniFile, key, "RXlmsGM", pSound->m_lms.m_lmsGM);
	pSound->m_lms.m_lmsAGC = pIniFile->ReadInteger(key, "RXlmsAGC", pSound->m_lms.m_lmsAGC);
	pSound->m_lms.m_lmsInv = pIniFile->ReadInteger(key, "RXlmsInv", pSound->m_lms.m_lmsInv);
	pSound->m_lms.m_Tap = pIniFile->ReadInteger(key, "RXlmsTAP", pSound->m_lms.m_Tap);
	pSound->m_lms.m_NotchTap = pIniFile->ReadInteger(key, "RXNotchTAP", pSound->m_lms.m_NotchTap);
	pSound->m_lms.m_bpf = pIniFile->ReadInteger(key, "RXlmsBPF", pSound->m_lms.m_bpf);
	pSound->m_lms.m_Type = pIniFile->ReadInteger(key, "RXlmsType", pSound->m_lms.m_Type);
	pSound->m_lms.m_lmsNotch = pIniFile->ReadInteger(key, "RXlmsNotch", pSound->m_lms.m_lmsNotch);
	pSound->m_lms.m_lmsNotch2 = pIniFile->ReadInteger(key, "RXlmsNotch2", pSound->m_lms.m_lmsNotch2);
	pSound->m_lms.m_twoNotch = pIniFile->ReadInteger(key, "RXlmsTwoNotch", pSound->m_lms.m_twoNotch);
	if( !pSound->m_lmsbpf && pSound->m_lms.m_twoNotch ) pSound->m_lms.m_lmsNotch = pSound->m_lms.m_lmsNotch2 = 0;

	pSound->CalcBPF();

	pSound->FSKDEM.m_type = pIniFile->ReadInteger(key, "DEMTYPE", pSound->FSKDEM.m_type);
	pSound->FSKDEM.SetBaudRate(ReadDoubleIniFile(pIniFile, key, "BaudRate", pSound->FSKDEM.GetBaudRate()));
	pSound->FSKDEM.m_lpf = pIniFile->ReadInteger(key, "SmoozType", pSound->FSKDEM.m_lpf);
	pSound->FSKDEM.m_lpfOrder = pIniFile->ReadInteger(key, "SmoozOrder", pSound->FSKDEM.m_lpfOrder);
	pSound->FSKDEM.SetLPFFreq(ReadDoubleIniFile(pIniFile, key, "SmoozIIR", pSound->FSKDEM.m_lpffreq));
	pSound->FSKDEM.SetSmoozFreq(ReadDoubleIniFile(pIniFile, key, "Smooz", pSound->FSKDEM.GetSmoozFreq()));

	pSound->FSKDEM.m_BitLen = pIniFile->ReadInteger(key, "TTYBitLen", pSound->FSKDEM.m_BitLen);
	pSound->FSKDEM.m_StopLen = pIniFile->ReadInteger(key, "TTYStopLen", pSound->FSKDEM.m_StopLen);
	pSound->FSKDEM.m_Parity = pIniFile->ReadInteger(key, "TTYParity", pSound->FSKDEM.m_Parity);
	pSound->FSKMOD.m_BitLen = pSound->FSKDEM.m_BitLen;
	pSound->FSKMOD.m_StopLen = pSound->FSKDEM.m_StopLen;
	pSound->FSKMOD.m_Parity = pSound->FSKDEM.m_Parity;

	pSound->FSKDEM.SetFilterTap(pIniFile->ReadInteger(key, "Tap", pSound->FSKDEM.GetFilterTap()));
	pSound->FSKDEM.m_iirfw = pIniFile->ReadInteger(key, "IIRBW", pSound->FSKDEM.m_iirfw);
	pSound->FSKDEM.SetIIR(pSound->FSKDEM.m_iirfw);
	pSound->FSKDEM.m_Phase.m_TONES = pIniFile->ReadInteger(key, "FFTTones", pSound->FSKDEM.m_Phase.m_TONES);
	pSound->FSKDEM.m_Phase.Create();

	pSound->FSKDEM.m_pll.SetVcoGain(ReadDoubleIniFile(pIniFile, key, "pllVcoGain", pSound->FSKDEM.m_pll.m_vcogain));
	pSound->FSKDEM.m_pll.m_loopOrder = pIniFile->ReadInteger(key, "pllLoopOrder", pSound->FSKDEM.m_pll.m_loopOrder);
	pSound->FSKDEM.m_pll.m_loopFC = ReadDoubleIniFile(pIniFile, key, "pllLoopFC", pSound->FSKDEM.m_pll.m_loopFC);
	pSound->FSKDEM.m_pll.m_outOrder = pIniFile->ReadInteger(key, "pllOutOrder", pSound->FSKDEM.m_pll.m_outOrder);
	pSound->FSKDEM.m_pll.m_outFC = ReadDoubleIniFile(pIniFile, key, "pllOutFC", pSound->FSKDEM.m_pll.m_outFC);
	pSound->FSKDEM.m_pll.MakeLoopLPF();
	pSound->FSKDEM.m_pll.MakeOutLPF();

	pSound->FSKMOD.m_diddle = pIniFile->ReadInteger(key, "Diddle", pSound->FSKMOD.m_diddle);

	sys.m_TxFixShift = pIniFile->ReadInteger(key, "TxFixShift", sys.m_TxFixShift);
	sys.m_TxRxInv = pIniFile->ReadInteger(key, "InvPTT", sys.m_TxRxInv);
	sys.m_txuos = pIniFile->ReadInteger(key, "TXUOS", sys.m_txuos);

//AA6YQ option, Added by JE3HHT on Sep.2010
	CAA6YQ *pAA6YQ = &pSound->FSKDEM.m_AA6YQ;
	pAA6YQ->m_fEnabled = pIniFile->ReadInteger(key, "AA6YQ.Enabled", 0);	// 0 = for old profiles
	pAA6YQ->m_bpfTaps = pIniFile->ReadInteger(key, "AA6YQ.TapsBPF", pAA6YQ->m_bpfTaps);
	pAA6YQ->m_bpfFW = ReadDoubleIniFile(pIniFile, key, "AA6YQ.WidthBPF", pAA6YQ->m_bpfFW);
	pAA6YQ->m_befTaps = pIniFile->ReadInteger(key, "AA6YQ.TapsBEF", pAA6YQ->m_befTaps);
	pAA6YQ->m_befFW = ReadDoubleIniFile(pIniFile, key, "AA6YQ.WidthBEF", pAA6YQ->m_befFW);
	pAA6YQ->m_afcERR = ReadDoubleIniFile(pIniFile, key, "AA6YQ.afcERR", pAA6YQ->m_afcERR);
	int verAA6YQ = pIniFile->ReadInteger(key, "AA6YQ.Version", 0);
	if( verAA6YQ < VERAA6YQ ){
		pAA6YQ->m_bpfTaps = 512;
		pAA6YQ->m_befTaps = 256;
    }
	pAA6YQ->Create();

	pSound->Resume();
	delete pIniFile;
	if( Remote ) PostApp(TXM_PROFILE, 0x00000000 + n);
	COMM.change = 1;
	UpdateFSK();
    RemoteStat();
}
//---------------------------------------------------------------------------
// ÉåÉWÉXÉgÉäÇ÷ÇÃèëÇ´èoÇµ
void __fastcall TMmttyWd::WriteProfile(int n, LPCSTR pName, int Flag)
{
	char	bf[256];
	AnsiString	as, ws;

	if( n == 1024 ){
		strcpy(bf, pName);
		pName = sys.m_Call.c_str();
	}
	else {
		sprintf(bf, "%sUserPara.ini", BgnDir);
	}
	try {

	TMemIniFile	*pIniFile = new TMemIniFile(bf);

	char	key[32];
	sprintf(key, "Define%d", n);

	if( !Flag ){
		pIniFile->EraseSection(key);
		pIniFile->WriteInteger(key, "Enabled", Flag);
		if( (pMap != NULL) && !(Remote & REMDISSHARED) && (n >= 0) && (n < 16) ){
			pMap->ProfileName[n][0] = 0;
		}
	}
	else {
	pIniFile->WriteString(key, "Name", pName);
	pIniFile->WriteInteger(key, "Enabled", Flag);
	pIniFile->WriteInteger(key, "AFC", sys.m_AFC);
	pIniFile->WriteInteger(key, "AFCFixShift", sys.m_FixShift);
	pIniFile->WriteInteger(key, "AFCSQ", sys.m_AFCSQ);
	WriteDoubleIniFile(pIniFile, key, "AFCTime", sys.m_AFCTime);
	WriteDoubleIniFile(pIniFile, key, "AFCSweep", sys.m_AFCSweep);

	pIniFile->WriteInteger(key, "UOS", rtty.m_uos);

	pIniFile->WriteInteger(key, "TxNet", sys.m_TxNet);
	pIniFile->WriteInteger(key, "TxDisRev", sys.m_TxDisRev);
	WriteDoubleIniFile(pIniFile, key, "LimitDxGain", sys.m_LimitGain);
	pIniFile->WriteInteger(key, "LimitOverSampling", pSound->FSKDEM.m_LimitOverSampling);
	pIniFile->WriteInteger(key, "LimitAGC", pSound->FSKDEM.m_LimitAGC);
	pIniFile->WriteInteger(key, "ATC", pSound->FSKDEM.m_atc);
	pIniFile->WriteInteger(key, "ATCTime", pSound->FSKDEM.m_atcMark.m_Max);

	pIniFile->WriteInteger(key, "Majority", pSound->FSKDEM.m_majority);
	pIniFile->WriteInteger(key, "IgnoreFreamError", pSound->FSKDEM.m_ignoreFream);

	pIniFile->WriteInteger(key, "SQ", pSound->FSKDEM.GetSQ());
	WriteDoubleIniFile(pIniFile, key, "SQLevel", pSound->FSKDEM.GetSQLevel());

	WriteDoubleIniFile(pIniFile, key, "DefMarkFreq", sys.m_DefMarkFreq);
	WriteDoubleIniFile(pIniFile, key, "DefShift", sys.m_DefShift);
	pIniFile->WriteInteger(key, "DefStopLen", sys.m_DefStopLen);

	WriteDoubleIniFile(pIniFile, key, "OutputGain", pSound->FSKMOD.GetOutputGain());

	pIniFile->WriteInteger(key, "TXLoop", sys.m_echo);
	pIniFile->WriteInteger(key, "TXBPFTAP", pSound->FSKMOD.m_bpftap);

	pIniFile->WriteInteger(key, "TXLPF", pSound->FSKMOD.m_lpf);
	WriteDoubleIniFile(pIniFile, key, "TXLPFFreq", pSound->FSKMOD.GetLPFFreq());

	pIniFile->WriteInteger(key, "TXWaitType", sys.m_LWait);
	pIniFile->WriteInteger(key, "TXCharWait", pSound->FSKMOD.m_CharWait);
	pIniFile->WriteInteger(key, "TXDiddleWait", pSound->FSKMOD.m_DiddleWait);
	pIniFile->WriteInteger(key, "TXCharWaitDiddle", pSound->FSKMOD.m_CharWaitDiddle);
	pIniFile->WriteInteger(key, "TXRandomDiddle", pSound->FSKMOD.m_RandomDiddle);
	pIniFile->WriteInteger(key, "TXWaitTimerDiddle", pSound->FSKMOD.m_WaitTimer);
	pIniFile->WriteInteger(key, "Rev", sys.m_Rev);

	WriteDoubleIniFile(pIniFile, key, "ShiftFreq", pSound->FSKDEM.GetSpaceFreq() - pSound->FSKDEM.GetMarkFreq());
	pIniFile->WriteInteger(key, "RXBPF", pSound->m_bpf);
	pIniFile->WriteInteger(key, "RXBPFTAP", pSound->m_bpftap);
	pIniFile->WriteInteger(key, "RXBPFAFC", pSound->m_bpfafc);
	WriteDoubleIniFile(pIniFile, key, "RXBPFFW", pSound->m_bpffw);

	pIniFile->WriteInteger(key, "RXlms", pSound->m_lmsbpf);
	pIniFile->WriteInteger(key, "RXlmsDelay", pSound->m_lms.m_lmsDelay);
	WriteDoubleIniFile(pIniFile, key, "RXlmsMU2", pSound->m_lms.m_lmsMU2);
	WriteDoubleIniFile(pIniFile, key, "RXlmsGM", pSound->m_lms.m_lmsGM);
	pIniFile->WriteInteger(key, "RXlmsAGC", pSound->m_lms.m_lmsAGC);
	pIniFile->WriteInteger(key, "RXlmsInv", pSound->m_lms.m_lmsInv);
	pIniFile->WriteInteger(key, "RXlmsTAP", pSound->m_lms.m_Tap);
	pIniFile->WriteInteger(key, "RXNotchTAP", pSound->m_lms.m_NotchTap);
	pIniFile->WriteInteger(key, "RXlmsBPF", pSound->m_lms.m_bpf);
	pIniFile->WriteInteger(key, "RXlmsType", pSound->m_lms.m_Type);
	pIniFile->WriteInteger(key, "RXlmsNotch", pSound->m_lms.m_lmsNotch);
	pIniFile->WriteInteger(key, "RXlmsNotch2", pSound->m_lms.m_lmsNotch2);
	pIniFile->WriteInteger(key, "RXlmsTwoNotch", pSound->m_lms.m_twoNotch);

	pIniFile->WriteInteger(key, "DEMTYPE", pSound->FSKDEM.m_type);
	WriteDoubleIniFile(pIniFile, key, "BaudRate", pSound->FSKDEM.GetBaudRate());
	pIniFile->WriteInteger(key, "TTYBitLen", pSound->FSKDEM.m_BitLen);
	pIniFile->WriteInteger(key, "TTYStopLen", pSound->FSKDEM.m_StopLen);
	pIniFile->WriteInteger(key, "TTYParity", pSound->FSKDEM.m_Parity);
	pIniFile->WriteInteger(key, "SmoozType", pSound->FSKDEM.m_lpf);
	pIniFile->WriteInteger(key, "SmoozOrder", pSound->FSKDEM.m_lpfOrder);
	WriteDoubleIniFile(pIniFile, key, "SmoozIIR", pSound->FSKDEM.m_lpffreq);
	WriteDoubleIniFile(pIniFile, key, "Smooz", pSound->FSKDEM.GetSmoozFreq());

	pIniFile->WriteInteger(key, "Tap", pSound->FSKDEM.GetFilterTap());
	pIniFile->WriteInteger(key, "IIRBW", pSound->FSKDEM.m_iirfw);
	pIniFile->WriteInteger(key, "FFTTones", pSound->FSKDEM.m_Phase.m_TONES);

	WriteDoubleIniFile(pIniFile, key, "pllVcoGain", pSound->FSKDEM.m_pll.m_vcogain);
	pIniFile->WriteInteger(key, "pllLoopOrder", pSound->FSKDEM.m_pll.m_loopOrder);
	WriteDoubleIniFile(pIniFile, key, "pllLoopFC", pSound->FSKDEM.m_pll.m_loopFC);
	pIniFile->WriteInteger(key, "pllOutOrder", pSound->FSKDEM.m_pll.m_outOrder);
	WriteDoubleIniFile(pIniFile, key, "pllOutFC", pSound->FSKDEM.m_pll.m_outFC);

	pIniFile->WriteInteger(key, "Diddle", pSound->FSKMOD.m_diddle);

	pIniFile->WriteInteger(key, "TxFixShift", sys.m_TxFixShift);
	pIniFile->WriteInteger(key, "InvPTT", sys.m_TxRxInv);
	pIniFile->WriteInteger(key, "TXUOS", sys.m_txuos);

//AA6YQ option, Added by JE3HHT on Sep.2010
	CAA6YQ *pAA6YQ = &pSound->FSKDEM.m_AA6YQ;
	pIniFile->WriteInteger(key, "AA6YQ.Version", VERAA6YQ);
	pIniFile->WriteInteger(key, "AA6YQ.Enabled", pAA6YQ->m_fEnabled);
	pIniFile->WriteInteger(key, "AA6YQ.TapsBPF", pAA6YQ->m_bpfTaps);
	WriteDoubleIniFile(pIniFile, key, "AA6YQ.WidthBPF", pAA6YQ->m_bpfFW);
	pIniFile->WriteInteger(key, "AA6YQ.TapsBEF", pAA6YQ->m_befTaps);
	WriteDoubleIniFile(pIniFile, key, "AA6YQ.WidthBEF", pAA6YQ->m_befFW);
	WriteDoubleIniFile(pIniFile, key, "AA6YQ.afcERR", pAA6YQ->m_afcERR);

	}
	pIniFile->UpdateFile();
	delete pIniFile;
	if( Remote ) PostApp(TXM_PROFILE, (Flag ? 0x00010000 : 0x00020000) + n);

	}
	catch(...){
		ErrorMB((Font->Charset != SHIFTJIS_CHARSET)?"Cannot update UserPara.INI":"UserPara.INIÇçXêVÇ≈Ç´Ç‹ÇπÇÒ.");
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::WriteProfile(int n)
{
	AnsiString as;
	TMenuItem *tp = NULL;
	if( (n >= 0) && (n < 8) ){
		tp = GetKS(n);
		as = tp->Caption;
	}
	else {
		char	bf[256];

		sprintf(bf, "%sUserPara.ini", BgnDir);
		TMemIniFile	*pIniFile = new TMemIniFile(bf);
		char	key[32];
		sprintf(key, "Define%d", n);
		as = pIniFile->ReadString(key, "Name", "User profile");
		delete pIniFile;
	}
	int r;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		r = InputMB("Assign profile", "Profile name", as);
	}
	else {
		r = InputMB("ÉvÉçÉtÉ@ÉCÉãÇÃìoò^", "ÉvÉçÉtÉ@ÉCÉãÇÃñºëO", as);
	}
	if( r == TRUE ){
		if( tp != NULL ){
			tp->Caption = as;
			tp->Enabled = TRUE;
		}
		WriteProfile(n, as.c_str(), TRUE);
		if( (pMap != NULL) && !(Remote & REMDISSHARED) && (n >= 0) && (n < 16) ){
			StrCopy(pMap->ProfileName[n], as.c_str(), 63);
		}
		UpdateProfile();
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KESClick(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKES(i);
		if( tp == (TMenuItem *)Sender ){
			WriteProfile(i);
			break;
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KDSClick(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKDS(i);
		if( tp == (TMenuItem *)Sender ){
			tp = GetKS(i);
			tp->Enabled = FALSE;
			char bf[32];
			sprintf(bf, "Profile %d", i + 1);
			tp->Caption = bf;
			WriteProfile(i, bf, FALSE);
			UpdateProfile();
			break;
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSClick(TObject *Sender)
{
	int i;
	for( i = 0; i < 8; i++ ){
		TMenuItem *tp = GetKS(i);
		if( tp == (TMenuItem *)Sender ){
			ReadProfile(i, NULL);
			UpdateItem();
			break;
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::RemoteProfile(DWORD flag)
{
	char bf[128];
	TMenuItem	*tp;

	int n = flag & 0x0000ffff;
	switch(flag & 0xffff0000){
		case 0x00000000:
			if( ((n >= 0)&&(n <= 15)) || ((n>=1025)&&(n<=1026)) ){
				ReadProfile(n, NULL);
				UpdateItem();
			}
			break;
		case 0x00010000:
			if( (n >= 0)&&(n <= 15) ){
				if( pMap != NULL ){
					StrCopy(bf, pMap->ProfileName[n], 63);
				}
				else {
					sprintf(bf, "Remote %d", n + 1);
				}
				if( n < 8 ){
					tp = GetKS(n);
					tp->Caption = bf;
					tp->Enabled = TRUE;
				}
				WriteProfile(n, bf, TRUE);
				UpdateProfile();
			}
			break;
		case 0x00020000:
			if( (n >= 0)&&(n <= 15) ){
				sprintf(bf, "Profile %d", n + 1);
				if( n < 8 ){
					tp = GetKS(n);
					tp->Caption = bf;
					tp->Enabled = FALSE;
				}
				WriteProfile(n, bf, FALSE);
				UpdateProfile();
			}
			break;
		case 0x00030000:
			if( !m_OptionDialog ){
				KSLDClick(NULL);
			}
			break;
		case 0x00040000:
			if( !m_OptionDialog ){
				KSSVClick(NULL);
			}
			break;
		case 0x00050000:
			if( (n >= 0)&&(n <= 15) ){
				WriteProfile(n);
			}
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSSVClick(TObject *Sender)
{
	if( (Font->Charset != SHIFTJIS_CHARSET) ){
		SaveDialog->Title = "Write Profile";
		SaveDialog->Filter = "Profiles(*.pro)|*.pro|";
	}
	else {
		SaveDialog->Title = "ÉvÉçÉtÉ@ÉCÉãÇÃçÏê¨";
		SaveDialog->Filter = "ÉvÉçÉtÉ@ÉCÉã(*.pro)|*.pro|";
	}
	char bf[128];
	sprintf(bf, "%s.pro", ClipCall(sys.m_Call.c_str()));
	SaveDialog->FileName = bf;
	SaveDialog->DefaultExt = "pro";
	SaveDialog->InitialDir = BgnDir;
	DisPaint = TRUE;
	NormalWindow(this);
	m_OptionDialog++;
	if( Remote ){
		m_pRemOpenName = AnsiString(SaveDialog->Title).c_str();	//JA7UDE 0428
		PostApp(TXM_SHOWPROFILE, 1);
	}
	if( SaveDialog->Execute() == TRUE ){
		CWaitCursor w;
		WriteProfile(1024, AnsiString(SaveDialog->FileName).c_str(), TRUE);	//JA7UDE 0428
	}
	m_pRemOpenName = NULL;
	TopWindow(this);
	m_OptionDialog--;
	DisPaint = FALSE;
	AdjustFocus();
	if( Remote ) PostApp(TXM_SHOWPROFILE, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSLDClick(TObject *Sender)
{
	OpenDialog->Options >> ofCreatePrompt;
	OpenDialog->Options << ofFileMustExist;
	if(Font->Charset != SHIFTJIS_CHARSET){
		OpenDialog->Title = "Load Profile";
		OpenDialog->Filter = "Profiles(*.pro)|*.pro|All files(*.*)|*.*|";
	}
	else {
		OpenDialog->Title = "ÉvÉçÉtÉ@ÉCÉãÇÃÉçÅ[Éh";
		OpenDialog->Filter = "ÉvÉçÉtÉ@ÉCÉã(*.pro)|*.pro|Ç∑Ç◊ÇƒÇÃÉtÉ@ÉCÉã(*.*)|*.*|";
	}
	OpenDialog->FileName = "";
	OpenDialog->DefaultExt = "pro";
	OpenDialog->InitialDir = BgnDir;
	DisPaint = TRUE;
	NormalWindow(this);
	m_OptionDialog++;
	if( Remote ){
		m_pRemOpenName = AnsiString(OpenDialog->Title).c_str();	//JA7UDE 0428
		PostApp(TXM_SHOWPROFILE, 1);
    }
	if( OpenDialog->Execute() == TRUE ){
		CWaitCursor w;
		ReadProfile(1024, AnsiString(OpenDialog->FileName).c_str());	//JA7UDE 0428
		UpdateItem();
	}
	m_pRemOpenName = NULL;
	TopWindow(this);
	m_OptionDialog--;
	DisPaint = FALSE;
	AdjustFocus();
	if( Remote ) PostApp(TXM_SHOWPROFILE, 0);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSDEFClick(TObject *Sender)
{
	ReadProfile(1025, NULL);
	UpdateItem();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KSRETClick(TObject *Sender)
{
	ReadProfile(1026, NULL);
	UpdateItem();
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KPttTimClick(TObject *Sender)
{
	AnsiString as;

	as = m_PttTimer;
	if( InputMB("MMTTY", (Font->Charset != SHIFTJIS_CHARSET)?"Enter PTT timer value(sec). (0 = OFF)":"PTTÉ^ÉCÉ}Å[éûä‘ÅiïbÅjÇì¸óÕÇµÇƒâ∫Ç≥Ç¢. (0 = OFF)", as) == TRUE ){
		if( !as.IsEmpty() ){
			int d;
			sscanf(as.c_str(), "%lu", &d);
			m_PttTimer = d;
			UpdatePttTimer();
			KPttTim->Checked = m_PttTimer ? 1 : 0;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::UpdatePttTimer(void)
{
	m_PttTimerCount = ::GetTickCount() + (m_PttTimer * 1000);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KROFClick(TObject *Sender)
{
	AnsiString	as = LogDir;
	if( InputMB("MMTTY", (Font->Charset != SHIFTJIS_CHARSET)?"Please enter folder name":"ÉtÉHÉãÉ_ñºÇì¸óÕÇµÇƒâ∫Ç≥Ç¢.", as) == TRUE ){
		StrCopy(LogDir, as.c_str(), 255);
		if( sys.m_log ){
			PrintText.MakeLogName();
			PrintText.OpenLogFile();
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KROTClick(TObject *Sender)
{
	sys.m_logTimeStamp = sys.m_logTimeStamp ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KROTUClick(TObject *Sender)
{
	sys.m_TimeStampUTC = sys.m_TimeStampUTC ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KROTDClick(TObject *Sender)
{
	sys.m_ShowTimeStamp = sys.m_ShowTimeStamp ? FALSE : TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KMFileClick(TObject *Sender)
{
	KRcvLog->Checked = sys.m_log;
	KROT->Checked = sys.m_logTimeStamp;
	KROTU->Checked = sys.m_TimeStampUTC;
	KROTD->Checked = sys.m_ShowTimeStamp;
	KFlush->Enabled = Log.IsOpen() && Log.IsEdit();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::AddCall(LPCSTR p)
{
	SYSTEMTIME	now;
	GetLocal(&now);
	char bf[128];
	sprintf(bf, "%02u:%02u\t%s", now.wHour, now.wMinute, p);

	int i;
	TMenuItem *pm;
	for( i = 0; i < PopupC->Items->Count; i++ ){
		pm = PopupC->Items->Items[i];
		if( !strcmp(AnsiString(pm->Caption).c_str()+6, p) ){	//JA7UDE 0428
			if( !i ){
				pm->Caption = bf;
				return;
			}
			else {
				PopupC->Items->Delete(i);
				break;
			}
		}
	}
	pm = new TMenuItem (this);
	pm->Caption = bf;
	pm->RadioItem = FALSE;
	pm->OnClick = KCClick;
	pm->Checked = FALSE;
	pm->Enabled = TRUE;
	if( (i >= PopupC->Items->Count) && (PopupC->Items->Count == CALLLISTMAX) ){
		PopupC->Items->Delete(CALLLISTMAX-1);
	}
	PopupC->Items->Insert(0, pm);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KCClick(TObject *Sender)
{
	TMenuItem *pm = (TMenuItem *)Sender;
	if( strcmp(AnsiString(HisCall->Text).c_str(), (AnsiString(pm->Caption).c_str()+6)) ){	//JA7UDE 0428
		HisCall->Text = pm->Caption.c_str() + 6;
		FindCall();
	}
}
//---------------------------------------------------------------------------
/*
void __fastcall TMmsstv::SBRXIDMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		if( PopupC->Items->Count ){
			TControl *tp = (TControl *)Sender;
			PopupC->Popup(Left + GBLog->Left + tp->Left + SBRXID->Width/2, m_MW + Top + GBLog->Top + tp->Top);
		}
	}
}
*/
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::HisCallDblClick(TObject *Sender)
{
	if( PopupC->Items->Count ){
		int MW = Height - ClientHeight;
		PopupC->Popup(Left + PanelQSO->Left + HisCall->Left + HisCall->Width/2, Top + MW + PanelQSO->Top + HisCall->Top + HisCall->Height/2);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::LCallMouseDown(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		HisCallDblClick(NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SelectCombo(int sw)
{
	m_DisEvent++;

	// DemoBox1ÇÃëIë
	if( m_Baud ){	// Baudï\é¶
		if( sw ) SetComboBox(DemoBox1, m_asBaud.c_str());
		DemoBox1->Text = pSound->FSKDEM.GetBaudRate();
	}
	else {
		switch(pSound->FSKDEM.m_type){
			case 0:		// IIR
				if( sw ) SetComboBox(DemoBox1, m_asIIRBW.c_str());
				DemoBox1->Text = pSound->FSKDEM.m_iirfw;
				break;
			case 1:
				if( sw ) SetComboBox(DemoBox1, m_asFilterTap.c_str());
				DemoBox1->Text = pSound->FSKDEM.GetFilterTap();
				break;
			case 2:
				if( sw ) SetComboBox(DemoBox1, m_asVCOGain.c_str());
				DemoBox1->Text = pSound->FSKDEM.m_pll.m_vcogain;
				break;
			case 3:		// FFT
				if( sw ) SetComboBox(DemoBox1, "2,3,4,5,6");
				DemoBox1->Text = pSound->FSKDEM.m_Phase.m_TONES;
				break;
		}
	}

	// DemoBox2ÇÃëIë
	switch(pSound->FSKDEM.m_type){
		case 0:		// IIR or FIR
		case 1:
    	case 3:
			if( sw ) SetComboBox(DemoBox2, "20,22.5,25,30,35,40,45,50,60,70,80,100,150,200,300,600");
			if( pSound->FSKDEM.m_lpf ){
				DemoBox2->Text = pSound->FSKDEM.m_lpffreq;
			}
			else {
				DemoBox2->Text = pSound->FSKDEM.GetSmoozFreq();
			}
			break;
		case 2:		// PLL
			if( sw ) SetComboBox(DemoBox2, m_asLoopFC.c_str());
			DemoBox2->Text = pSound->FSKDEM.m_pll.m_loopFC;
			break;
	}
	m_ShowBaud = m_Baud;
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::DemoBox1Change(TObject *Sender)
{
	if( m_DisEvent ) return;

	double d;
	sscanf(AnsiString(DemoBox1->Text).c_str(), "%lf", &d);	//JA7UDE 0428
	if( m_Baud ){	// Baud
		if( d > 0 ){
			if( sys.m_TxPort && (pSound->FSKDEM.GetBaudRate() != d) ) COMM.change = 1;
			pSound->FSKDEM.SetBaudRate(d);
			pSound->FSKMOD.SetBaudRate(d);
			RemoteStat();
			UpdateFSK();
		}
	}
	else {
		switch(pSound->FSKDEM.m_type){
			case 0:		// IIR
				if( (d > 0) && (d <= 500.0) ){
					pSound->FSKDEM.SetIIR(d);
				}
				break;
			case 1:
				{
					int di = int(d + 0.1);
					if( (di >= 2) && (di <= TAPMAX) ){
						pSound->FSKDEM.SetFilterTap(di);
					}
				}
				break;
			case 2:
				if( (d > 0) && (d <= 1000.0) ){
					pSound->FSKDEM.m_pll.SetVcoGain(d);
				}
				break;
			case 3:		// FFT
				if( (d > 2) && (d <= 6.0) ){
					pSound->FSKDEM.m_Phase.SetTones(d);
				}
				break;
		}
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::DemoBox2Change(TObject *Sender)
{
	if( m_DisEvent ) return;

	double d;
	sscanf(AnsiString(DemoBox2->Text).c_str(), "%lf", &d);	//JA7UDE 0428
	switch(pSound->FSKDEM.m_type){
		case 0:
		case 1:
        case 3:
			if( (d >= 20.0) && (d <= 1500.0) ){
				if( pSound->FSKDEM.m_lpf ){
					pSound->FSKDEM.SetLPFFreq(d);
				}
				else {
					pSound->FSKDEM.SetSmoozFreq(d);
				}
			}
			break;
		case 2:
			if( (d > 0) && (d <= 2000.0) ){
				pSound->FSKDEM.m_pll.m_loopFC = d;
				pSound->FSKDEM.m_pll.MakeLoopLPF();
			}
			break;
	}
	AdjustFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::CMWheel(TMessage Message)
{
	if( Remote ) return;

	int z = Message.WParam >> 16;
	z /= 120;
	while( z ){
		if( z > 0 ){
			PrintText.ScrollBarUp(1);
			z--;
		}
		else {
			PrintText.ScrollBarDown(1);
			z++;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::SBTXMouseDown(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if( Button == mbRight ){
		KOptionClick(KMacro);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::CMMML(TMessage Message)
{
	if( sys.m_LogLink != 2 ) return;
	if( LogLink.m_pLink == NULL ) return;

	switch(Message.WParam){
		case MML_NOTIFYSESSION:
			LogLink.NotifySession((LPCSTR)Message.LParam);
			UpdateLogLink();
			break;
		case MML_QRETURN:
			if( !LogLink.IsLink() ) return;
			if( LogLink.QReturn(&Log.m_sd, (const mmLOGDATA *)Message.LParam) ){
				UpdateTextData();
			}
			break;
		case MML_VFO:
			if( !LogLink.IsLink() ) return;
			if( Message.LParam ){
				Freq->Text = (LPCSTR)Message.LParam;
			}
			break;
	}
	Message.Result = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::CMMMR(TMessage Message)
{
	if( pRadio == NULL ) return;

	switch(Message.WParam){
		case MMR_DEFCOMMAND:
			{
				LPCSTR p;
				switch(Message.LParam){
					case 1:
						p = RADIO.CmdRx.c_str();
						break;
					case 2:
						p = RADIO.CmdTx.c_str();
						break;
					default:
						p = RADIO.CmdInit.c_str();
						break;
				}
				pRadio->SendCommand(p);
			}
			break;
		case MMR_VFO:
			pRadio->UpdateFreq(double(Message.LParam)/10000.0);
			break;
	}
	Message.Result = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KTXClick(TObject *Sender)
{
	int dis = (sys.m_DisTX&1) ? FALSE : TRUE;
	sys.m_DisTX &= ~1;
    sys.m_DisTX |= dis;
    if( dis && SBTX->Down ){
		SBTXOFFClick(NULL);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KMOptClick(TObject *Sender)
{
	KTest->Enabled = !SBTX->Down;
	switch(m_SendWay){
		case 0:
			KSChar->Checked = TRUE;
			break;
		case 1:
			KSWord->Checked = TRUE;
			break;
		default:
			KSLine->Checked = TRUE;
			break;
	}
	KTX->Checked = (sys.m_DisTX&1);
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KViewClick(TObject *Sender)
{
	switch(m_PanelSize){
		case 0:
			KPanelSmall->Checked = TRUE;
			break;
		case 1:
			KPanelMedium->Checked = TRUE;
			break;
		case 3:
			KPanelSuper->Checked = TRUE;
			break;
		default:
			KPanelBig->Checked = TRUE;
			break;
	}

	switch(sys.m_FFTGain){
		case 0:
			KFFTGL->Checked = TRUE;
			break;
		case 1:
			KFFTGM->Checked = TRUE;
			break;
		case 2:
			KFFTGH->Checked = TRUE;
			break;
		case 3:
			KFFTGT->Checked = TRUE;
			break;
		case 4:
			KFFTGS1->Checked = TRUE;
			break;
		case 5:
			KFFTGS2->Checked = TRUE;
			break;
		case 6:
			KFFTGS3->Checked = TRUE;
			break;
		default:
			KFFTGS4->Checked = TRUE;
			break;
	}
	switch(sys.m_FFTResp){
		case 1:
			KFFTRF->Checked = TRUE;
			break;
		case 2:
			KFFTRM->Checked = TRUE;
			break;
		default:
			KFFTRS->Checked = TRUE;
			break;
	}
	switch(pSound->m_FFTFW){
		case 0:
			KFFTWA->Checked = TRUE;
			break;
		case 1:
			KFFTW05->Checked = TRUE;
			break;
		case 2:
			KFFTW1->Checked = TRUE;
			break;
		case 3:
			KFFTW15->Checked = TRUE;
			break;
		case 4:
			KFFTW2->Checked = TRUE;
			break;
		default:
			KFFTW3->Checked = TRUE;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TMmttyWd::KExtCmdClick(TObject *Sender)
{
	KExtReset->Enabled = pSound->m_susp;
	KExtSusp->Enabled = !pSound->m_susp;
}
//---------------------------------------------------------------------------


