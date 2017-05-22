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

#include "VerDsp.h"
#include "ComLib.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TVerDspDlg *VerDspDlg;
//--------------------------------------------------------------------- 
__fastcall TVerDspDlg::TVerDspDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	IconImage->Picture->Assign(Application->Icon);
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
//	Version->Font->Name = ((TForm *)AOwner)->Font->Name;
//	Version->Font->Charset = ((TForm *)AOwner)->Font->Charset;
	LFree->Font->Name = ((TForm *)AOwner)->Font->Name;
	LFree->Font->Charset = ((TForm *)AOwner)->Font->Charset;

	//if( Font->Charset != SHIFTJIS_CHARSET ){    //1.70K always display this information in English
		Caption = "Version Information";
		LFree->Caption = "Free Software";
		LTNX->Font->Size = LTNX->Font->Size - 1; 
	//}

	Version->Caption = VERTTL;
	LTNX->Caption =
	"------ Programming ------\r\n"
	"Stu, K6TU (Updated 1.68A -> 1.70A)\r\n"
	"Paul, G3WYW (Updated 1.70A -> 1.70B)\r\n"
	"Dave, AA6YQ (updated 1.65D -> 1.66G, 1.70B -> 1.70K)\r\n\r\n"
	"------ Help, FAQ and Remote mode, etc... ------\r\n"
	"Jan, KX2A  Ken, VE5KC  Bill, KB4IJ  Andy, K3UK(KB2EOQ)\r\n"
	"Bill, K5YG    Phil, GU0SUP,  Josef, OK2WO  Oba, JA7UDE\r\n"
	"Bill, W7TI  Ed, K4SB  Eddie, W6/G0AZT  Geoff, G3NPA  Bob, K4CY\r\n"
	"Eike, DM3ML  Antonio, EA1MV  Edoardo, I5PAC  Joe, W4TV\r\n\r\n"
	"------ Japanese friends ------\r\n"
	"Fumio, JA1IQV  Nori, JA1WSK  Oba, JA7UDE\r\n"
	"Kaz, 7L1CUH   Nobu, JA2BQX  Hiro, JH1BIH,  Isao JK1IQK\r\n\r\n"
	"------ Reflector and Web sites ------\r\n"
	"MMTTY User's group  http://groups.yahoo.com/group/MMTTY\r\n"
	"MMTTY-Web  http://hamsoft.ca\r\n"
	"JARTS-Web  http://jarts.jp\r\n"
	"MMTTY open source  http://mm-open.org/\r\n"
	"\r\nAll friends of Radio Amateurs\r\n"
	"And my wife Midori, JI3IVL, for her invisible help."
	;
}
//---------------------------------------------------------------------
