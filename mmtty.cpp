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



//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("Main.cpp", MmttyWd);
USEUNIT("ComLib.cpp");
USEUNIT("fir.cpp");
USEUNIT("Sound.cpp");
USEUNIT("Rtty.cpp");
USEUNIT("Wave.cpp");
USEUNIT("Fft.cpp");
USEFORM("Scope.cpp", TScope);
USEFORM("Option.cpp", OptionDlg);
USEUNIT("Comm.cpp");
USEFORM("EditDlg.cpp", EditDlgBox);
USERES("mmtty.res");
USEFORM("TextEdit.cpp", TextEditDlg);
USEUNIT("LogFile.cpp");
USEFORM("LogList.cpp", LogListDlg);
USEFORM("QsoDlg.cpp", QSODlgBox);
USEUNIT("country.cpp");
USEUNIT("mmcg.cpp");
USEFORM("MmcgDlg.cpp", MmcgDlgBox);
USEFORM("FreqDisp.cpp", FreqDispDlg);
USEFORM("VerDsp.cpp", VerDspDlg);
USEUNIT("LogConv.cpp");
USEFORM("ConvDef.cpp", ConvDefDlg);
USEFORM("PlayDlg.cpp", PlayDlgBox);
USEFORM("ShortCut.cpp", ShortCutDlg);
USEFORM("LogSet.cpp", LogSetDlg);
USEFORM("ClockAdj.cpp", ClockAdjDlg);
USEFORM("SetHelp.cpp", SetHelpBox);
USEUNIT("ctnc.cpp");
USEFORM("TncSet.cpp", TNCSetDlg);
USEFORM("SendFile.cpp", SendFileDlg);
USEUNIT("cradio.cpp");
USEFORM("radioset.cpp", RADIOSetDlg);
USEFORM("MacroKey.cpp", MacroKeyDlg);
USEFORM("TxdDlg.cpp", TxdDlgBox);
USEFORM("InputWin.cpp", InputWinDlg);
USEUNIT("Loglink.cpp");
USEUNIT("MMlink.cpp");
USEFILE("mml.h");
USEFILE("mmrp.h");
USEFILE("mmw.h");
USEUNIT("Hamlog5.cpp");
USEFORM("TH5Len.cpp", TH5LenDlg);
USEUNIT("CLX.cpp");
//---------------------------------------------------------------------------
#define DISDUPE		1
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if DISDUPE
	LPSTR pCmd = GetCommandLine();
    if( strstr(pCmd, "-Z") == NULL ){
		HWND hWnd = FindWindow("TMmttyWd", NULL);
		if( hWnd != NULL ){
			if( !IsWindowVisible(hWnd) ) return 0;
		}
    }
#endif
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMmttyWd), &MmttyWd);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	return 0;
}
//---------------------------------------------------------------------------
