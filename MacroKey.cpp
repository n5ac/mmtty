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

#include "MacroKey.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TMacroKeyDlg *MacroKeyDlg;
MACKEY	mackeycom[]={
	{1, "_","マーク信号の送信","Transmit a mark signal"},
	{1, "~","AFSKキャリア停止","Stop AFSK carrier"},
	{1, "[","Diddleの禁止","Disable diddle"},
	{1, "]","Diddleの許可","Enable diddle"},
	{1, "%m","My callsign", NULL},
	{1, "%c","His/her callsign", NULL},
	{1, "%r","His/her RST", NULL},
	{1, "%n","His/her name", NULL},
	{1, "%q","His/her QTH", NULL},
	{1, "%s","My RST", NULL},
	{1, "%R","His/her RSTのRST部分のみ","Only the RST part of his/her RST"},
	{1, "%N","His/her RSTのコンテストナンバ部分のみ","Only the contest number part of his/her RST"},
	{1, "%M","My RSTのコンテストナンバ部分のみ","Only the contest number part of my RST"},
	{1, "%g","GOOD MORNING/AFTERNOON/EVENING", NULL},
	{1, "%f","GM/GA/GE", NULL},
	{1, "%L","LTRコードの強制送信","Force transmit LTR code"},
	{1, "%F","FIGコードの強制送信","Force transmit FIG code"},
	{1, "%D","UTC date (e.g., 2000-SEP-05)", NULL},
	{1, "%T","UTC time (e.g., 12:34)", NULL},
	{1, "%t","UTC time (e.g., 1234)", NULL},
	{1, "%x","Only the 1st number(001) for BARTG", NULL},
	{1, "%y","Only the 2nd number(UTC) for BARTG", NULL},
	{1, "%{","CWID (e.g., %{DE %m})", NULL},
	{-1, "","<行末のスペースを削除>", "<Delete space with end of line>"},
	{0, NULL, NULL},
};
MACKEY	mackeybtn[]={
	{3, "\\","先頭に記述してTxWindow経由で送信","Sends via the TX window (at the top)"},
	{2, "\\","末尾に記述して受信に切りかえる","Switches to receive (at the last)"},
	{3, "#","先頭に記述してTxWindowに全文をコピー","Copies the message to the TX window (at the top)"},
	{2, "#","末尾に記述して繰り返し","Repeats the macro (at the last)"},
	{0, NULL, NULL},
};
MACKEY	mackeymsg[]={
	{3, "\\","先頭に記述してTxWindowをクリア","Clears the TX window (at the top)"},
	{2, "\\","末尾に記述して受信に切りかえる","Switches to receive (at the last)"},
	{3, "#","先頭に記述して送信に切りかえる","Switches to transmit (at the top)"},
	{1, "^","約１秒間の停止","Wait for 1 second"},
	{0, NULL, NULL},
};
MACKEY	mackeyusr[]={
	{2, "\\","末尾に記述して受信に切りかえる","Switches to receive (at the last)"},
	{0, NULL, NULL},
};
static int PosPos;	//JA7UDE 0428
static int TopPos;
//---------------------------------------------------------------------
__fastcall TMacroKeyDlg::TMacroKeyDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		CancelBtn->Caption = "Cancel";
	}
}
//---------------------------------------------------------------------
void __fastcall TMacroKeyDlg::UpdateUI(int row)
{
	int r = row - 1;
	if( r >= 0 ){
		r = mackey[r].r;
		if( r == -1 ){
			OKBtn->Caption = (Font->Charset != SHIFTJIS_CHARSET) ? "Execute" : "実行";
		}
		else {
			OKBtn->Caption = (Font->Charset != SHIFTJIS_CHARSET) ? "Insert" : "挿入";
		}
		OKBtn->Enabled = TRUE;
	}
	else {
		OKBtn->Caption = (Font->Charset != SHIFTJIS_CHARSET) ? "Insert" : "挿入";
		OKBtn->Enabled = FALSE;
	}
}
//---------------------------------------------------------------------
int __fastcall TMacroKeyDlg::AddMacKey(MACKEY *mp, int n)
{
	for( ; mp->r; mp++, n++ ){
		mackey[n] = *mp;
	}
	return n;
}
//---------------------------------------------------------------------
int __fastcall TMacroKeyDlg::Execute(AnsiString &as, int sw)
{
	int n = 0;
	switch(sw){
		case 0:
			n = AddMacKey(mackeymsg, n);
			break;
		case 1:
			n = AddMacKey(mackeybtn, n);
			break;
		case 2:
			n = AddMacKey(mackeyusr, n);
			break;
	}
	n = AddMacKey(mackeycom, n);
	Grid->RowCount = 1 + n;
	if( (PosPos > 0) && (PosPos < Grid->RowCount) ){
		Grid->Row = PosPos;
		Grid->TopRow = TopPos;
	}
	UpdateUI(Grid->Row);
	int r = ShowModal();
	PosPos = Grid->Row;
	TopPos = Grid->TopRow;
	if( r == IDOK ){
		if( PosPos ){
			r = PosPos - 1;
			as = mackey[r].pKey;
			return mackey[r].r;
		}
	}
	return 0;
}
//---------------------------------------------------------------------
void __fastcall TMacroKeyDlg::GridDrawCell(TObject *Sender, int Col, int Row,
	TRect &Rect, TGridDrawState State)
{
	char	bf[256];

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		bf[0] = 0;
		LPCSTR pCom;
		switch(Col){
			case 0:
				strcpy(bf, mackey[Row].pKey);
				break;
			case 1:
				if( Font->Charset != SHIFTJIS_CHARSET ){
					pCom = mackey[Row].pEng;
					if( pCom == NULL ){
						pCom = mackey[Row].pJpn;
					}
				}
				else {
					pCom = mackey[Row].pJpn;
				}
				strcpy(bf, pCom);
				break;
		}
		Grid->Canvas->TextOut(X, Y, bf);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"Key","Comments",
		};
		Grid->Canvas->TextOut(X, Y, _tt[Col]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMacroKeyDlg::GridSelectCell(TObject *Sender, int Col,
	int Row, bool &CanSelect)
{
	UpdateUI(Row);
}
//---------------------------------------------------------------------------
