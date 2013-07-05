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

#include "ShortCut.h"
#include "LogFile.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TShortCutDlg *ShortCutDlg;
static	int	CurRow = 0;
static	int	TopRow = 0;
//---------------------------------------------------------------------
__fastcall TShortCutDlg::TShortCutDlg(TComponent* AOwner)
	: TForm(AOwner)
{
	FormStyle = ((TForm *)AOwner)->FormStyle;
	m_DisEvent = 1;
	Font->Name = ((TForm *)AOwner)->Font->Name;
	Font->Charset = ((TForm *)AOwner)->Font->Charset;
	if( Font->Charset != SHIFTJIS_CHARSET ){
		Caption = "Assign ShortCut";
		DelBtn->Caption = "Del";
		CheckBtn->Caption = "Check Dupe";
		OKBtn->Caption = "Close";
	}
	ShortCutName->Clear();
	int i;
	for( i = 0; KEYTBL[i].Key; i++ ){
		ShortCutName->Items->Add(ToDXKey(KEYTBL[i].pName));
	}
	m_DisEvent = 0;
	SBHelp->Visible = !JanHelp.IsEmpty();
}

typedef struct {
	LPCSTR	pSys;
	LPCSTR	pUser;
	WORD	*pKey;
}DEFTBL;

DEFTBL def[]={
	{"RcvLog","受信記録（日付名）ファイル", &sys.m_SysKey[kkRcvLog]},
	{"FileOut","テキストファイル送信", &sys.m_SysKey[kkFileOut]},
	{"SaveRx","受信画面の内容をファイルにセーブ", &sys.m_SysKey[kkSaveRx]},

	{"OpenLog","新しいログファイルのオープン", &sys.m_SysKey[kkOpenLog]},
	{"Flush","ディスクにフラッシュ", &sys.m_SysKey[kkFlush]},

	{"RecTime","受信サウンドを時刻名のファイルに記録", &sys.m_SysKey[kkRecTime]},
	{"Rec","受信サウンドをファイルに記録", &sys.m_SysKey[kkRec]},
	{"Play","受信サウンドをファイルから再生", &sys.m_SysKey[kkPlay]},
	{"PlayPos","再生位置調整", &sys.m_SysKey[kkPlayPos]},
	{"PlayStop","記録／再生の終了", &sys.m_SysKey[kkPlayStop]},

	{"Paste","送信画面への貼り付け", &sys.m_SysKey[kkPaste]},
	{"Panel","Control Panel", &sys.m_SysKey[kkPanel]},
	{"MacBtn","Macro ボタン", &sys.m_SysKey[kkMac]},
	{"Scope","オシロスコープ", &sys.m_SysKey[kkScope]},
	{"ClrRxWindow","受信画面のクリア", &sys.m_SysKey[kkClrRxWindow]},
	{"LogList","LogList", &sys.m_SysKey[kkLogList]},
	{"QSOData","QSOData", &sys.m_SysKey[kkQSOData]},
	{"EntTX","TXボタンで自動改行送信", &sys.m_SysKey[kkEntTX]},
	{"WordWrap","キーボードのワードラップ", &sys.m_SysKey[kkWordWrap]},
	{"RUN","ランニングモード", &sys.m_SysKey[kkRun]},
	{"TNC","TNC模擬モード設定", &sys.m_SysKey[kkTNC]},
	{"Option","設定画面", &sys.m_SysKey[kkOption]},
	{"LogOpt","ログ設定画面", &sys.m_SysKey[kkLogOption]},

	{"Profile1","Profile 1", &sys.m_SysKey[kkPro1]},
	{"Profile2","Profile 2", &sys.m_SysKey[kkPro2]},
	{"Profile3","Profile 3", &sys.m_SysKey[kkPro3]},
	{"Profile4","Profile 4", &sys.m_SysKey[kkPro4]},
	{"Profile5","Profile 5", &sys.m_SysKey[kkPro5]},
	{"Profile6","Profile 6", &sys.m_SysKey[kkPro6]},
	{"Profile7","Profile 7", &sys.m_SysKey[kkPro7]},
	{"Profile8","Profile 8", &sys.m_SysKey[kkPro8]},
	{"ProfileDef","MMTTY default", &sys.m_SysKey[kkProDef]},
	{"ProfileRet","Return to the startup", &sys.m_SysKey[kkProRet]},

	{"ExtCmd1","External 1", &sys.m_SysKey[kkExtCmd1]},
	{"ExtCmd2","External 2", &sys.m_SysKey[kkExtCmd2]},
	{"ExtCmd3","External 3", &sys.m_SysKey[kkExtCmd3]},
	{"ExtCmd4","External 4", &sys.m_SysKey[kkExtCmd4]},
	{"ExtReset","サスペンドの解除", &sys.m_SysKey[kkExtReset]},
	{"ExtSusp","サスペンド", &sys.m_SysKey[kkExtSusp]},

	{"FIG","FIG Button", &sys.m_SysKey[kkFIG]},
	{"UOS","UOS Button", &sys.m_SysKey[kkUOS]},
	{"TX","TX Button", &sys.m_SysKey[kkTX]},
	{"TXOFF","TXOFF Button", &sys.m_SysKey[kkTXOFF]},
	{"QSO","QSO Button", &sys.m_SysKey[kkQSO]},
	{"OnQSO","QSO Button ON", &sys.m_SysKey[kkOnQSO]},
	{"OffQSO","QSO Button OFF", &sys.m_SysKey[kkOffQSO]},
	{"Capture","Capture Callsign", &sys.m_SysKey[kkCAPTURE]},
	{"InitBox","Init Button", &sys.m_SysKey[kkInitBox]},
	{"CALL","Call Box", &sys.m_SysKey[kkCall]},
	{"NAME","Name Box", &sys.m_SysKey[kkName]},
	{"QTH","QTH Box", &sys.m_SysKey[kkQTH]},
	{"RST","HisRST Box", &sys.m_SysKey[kkRST]},
	{"MyRST","MyRST Box", &sys.m_SysKey[kkMyRST]},
	{"Freq","Frequency Box", &sys.m_SysKey[kkFreq]},
	{"Find","Find Button", &sys.m_SysKey[kkFind]},
	{"Clear","Clear Button", &sys.m_SysKey[kkClear]},

	{"TxUp","入力画面 ↑", &sys.m_SysKey[kkInUp]},
	{"TxDown","入力画面 ↓", &sys.m_SysKey[kkInDown]},
	{"TxPUp","入力画面 ページ↑", &sys.m_SysKey[kkInPUp]},
	{"TxPDown","入力画面 ページ↓", &sys.m_SysKey[kkInPDown]},
	{"TxHome","入力画面 先頭", &sys.m_SysKey[kkInHome]},
	{"TxEnd","入力画面 末尾", &sys.m_SysKey[kkInEnd]},

	{"RxUp","受信画面 ↑", &sys.m_SysKey[kkRxUp]},
	{"RxDown","受信画面 ↓", &sys.m_SysKey[kkRxDown]},
	{"RxPUp","受信画面 ページ↑", &sys.m_SysKey[kkRxPUp]},
	{"RxPDown","受信画面 ページ↓", &sys.m_SysKey[kkRxPDown]},
	{"RxHome","受信画面 先頭", &sys.m_SysKey[kkRxHome]},
	{"RxEnd","受信画面 末尾", &sys.m_SysKey[kkRxEnd]},

	{"CharWaitL","文字Wait←", &sys.m_SysKey[kkCharWaitLeft]},
	{"CharWaitR","文字Wait→", &sys.m_SysKey[kkCharWaitRight]},
	{"DiddleWaitL","DiddleWait←", &sys.m_SysKey[kkDiddleWaitLeft]},
	{"DiddleWaitR","DiddleWait→", &sys.m_SysKey[kkDiddleWaitRight]},

	{"TxHeightUp","入力画面拡大", &sys.m_SysKey[kkInHeightUp]},
	{"TxHeightDown","入力画面縮小", &sys.m_SysKey[kkInHeightDown]},

	{"TxLTR","LTR送信", &sys.m_SysKey[kkTxLTR]},
	{"TxFIG","FIG送信", &sys.m_SysKey[kkTxFIG]},

	{"DecShift","SHIFT幅 狭める", &sys.m_SysKey[kkDecShift]},
	{"IncShift","SHIFT幅 広げる", &sys.m_SysKey[kkIncShift]},
	{"ChangeShift","SHIFT幅変更 (170/200/220/350/450)", &sys.m_SysKey[kkToggleShift]},

	{"CallList","Callsign list", &sys.m_SysKey[kkCList]},
};

void SetMenuName(LPCSTR pkey, LPCSTR p)
{
	int i;
	for( i = 0; i < AN(def); i++ ){
		if( !strcmp(def[i].pSys, pkey) ){
			def[i].pUser = p;
			return;
		}
	}
}

void SetExtMenuName(int n, LPCSTR p)
{
	int i;
	for( i = 0; i < AN(def); i++ ){
		if( !strcmp(def[i].pSys, "ExtCmd1") ){
			def[i+n].pUser = p;
			return;
		}
	}
}

void SetProMenuName(int n, LPCSTR p)
{
	int i;
	for( i = 0; i < AN(def); i++ ){
		if( !strcmp(def[i].pSys, "Profile1") ){
			def[i+n].pUser = p;
			return;
		}
	}
}

const DEFTBL *GetDP(int n)
{
static	DEFTBL	udef;
static	char	bf[32];
static	WORD	dummy = 0;

	if( n < AN(def) ){
		return &def[n];
	}
	n -= AN(def);
	if( n < 4 ){
		sprintf(bf, "IN%u", n + 1);
		udef.pSys = bf;
		udef.pUser = sys.m_InBtnName[n].c_str();
		udef.pKey = &sys.m_InBtnKey[n];
		return &udef;
	}
	n -= 4;
	if( n < 16 ){
		sprintf(bf, "M%u", n + 1);
		udef.pSys = bf;
		udef.pUser = sys.m_UserName[n].c_str();
		udef.pKey = &sys.m_UserKey[n];
		return &udef;
	}
	n -= 16;
	if( n < MSGLISTMAX ){
		sprintf(bf, "ML%u", n + 1);
		udef.pSys = bf;
		udef.pUser = sys.m_MsgName[n].c_str();
		udef.pKey = &sys.m_MsgKey[n];
		return &udef;
	}
	n -= MSGLISTMAX;
	if( n < 5 ){
		sprintf(bf, "QM%u", n + 1);
		udef.pSys = bf;
		udef.pUser = (sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"QSO Button Macro":"QSO Button連動マクロ";
		udef.pKey = &Log.m_LogSet.m_QSOMacroKey[n];
		return &udef;
	}
	n -= 5;
	if( n < 16 ){
		sprintf(bf, "MEdit%u", n + 1);
		udef.pSys = bf;
		udef.pUser = sys.m_UserName[n].c_str();
		udef.pKey = &sys.m_UserEditKey[n];
		return &udef;
	}
	udef.pSys = "";
	udef.pUser = "";
	udef.pKey = &dummy;
	return &udef;
}
//---------------------------------------------------------------------
void __fastcall TShortCutDlg::UpdateUI(int n)
{
	if( n >= 0 ){
		const DEFTBL *dp = GetDP(n);
		m_DisEvent++;
		ShortCutName->Text = GetKeyName(*dp->pKey);
		m_DisEvent--;
	}
}
//---------------------------------------------------------------------
void __fastcall TShortCutDlg::Execute(void)
{
	Grid->RowCount = AN(def) + 4 + 16 + 64 + 5 + 16 + 1;
	if( CurRow ){
		Grid->Row = CurRow;
		Grid->TopRow = TopRow;
	}
	UpdateUI(Grid->Row - 1);
	ShowModal();
	CurRow = Grid->Row;
	TopRow = Grid->TopRow;
}
//---------------------------------------------------------------------------
// 重複のチェック
int __fastcall TShortCutDlg::IsDupe(int n)
{
#if 0
	FILE *fp = fopen("F:\\LIST.TXT", "wt");
	const DEFTBL *dp;
	for( int j = 0; j < Grid->RowCount-1; j++ ){
		dp = GetDP(j);
		fprintf(fp, "%s : %s\n", dp->pSys, dp->pUser);
	}
	fclose(fp);
#endif

	WORD	ks;
	const DEFTBL	*s;
	const DEFTBL	*t;
	int j;
	s = GetDP(n);
	ks = *s->pKey;
	if( ks ){
		for( j = 0; j < Grid->RowCount - 1; j++ ){
			if( n != j ){
				t = GetDP(j);
				if( t->pKey ){
					if( ks == *t->pKey ){
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
//---------------------------------------------------------------------
void __fastcall TShortCutDlg::GridDrawCell(TObject *Sender, int Col,
	int Row, TRect &Rect, TGridDrawState State)
{
	char	bf[256];

	Grid->Canvas->FillRect(Rect);
	int X = Rect.Left + 4;
	int Y = Rect.Top + 2;

	if( Row ){
		Row--;
		const DEFTBL *dp = GetDP(Row);
		bf[0] = 0;
		switch(Col){
			case 0:		// 内部名
				strcpy(bf, dp->pSys);
				break;
			case 1:		// ユーザ名
				strcpy(bf, dp->pUser);
				break;
			case 2:		// ショートカット
				strcpy(bf, GetKeyName(*dp->pKey));
				if( IsDupe(Row) ){
					Grid->Canvas->Font->Color = clRed;
				}
				break;
		}
		Grid->Canvas->TextOut(X, Y, bf);
	}
	else if( Font->Charset != SHIFTJIS_CHARSET ){
		LPCSTR	_tte[]={
			"Internal","Define Name","Key"
		};
		Grid->Canvas->TextOut(X, Y, _tte[Col]);
	}
	else {		// タイトル
		LPCSTR	_tt[]={
			"内部名","ユーザー定義名","割り当てキー"
		};
		Grid->Canvas->TextOut(X, Y, _tt[Col]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TShortCutDlg::GridSelectCell(TObject *Sender, int Col,
	int Row, bool &CanSelect)
{
	if( Row ){
		UpdateUI(Row - 1);
	}
}
//---------------------------------------------------------------------------
void __fastcall TShortCutDlg::ShortCutNameChange(TObject *Sender)
{
	if( m_DisEvent ) return;

	m_DisEvent++;
	int n = Grid->Row - 1;
	if( n >= 0 ){
		const DEFTBL *dp = GetDP(n);
		*dp->pKey = GetKeyCode(AnsiString(ShortCutName->Text).c_str());	//JA7UDE 0428
		Grid->Invalidate();
	}
	m_DisEvent--;
}
//---------------------------------------------------------------------------
void __fastcall TShortCutDlg::CheckBtnClick(TObject *Sender)
{
	WORD	ks;
	AnsiString	as;
	const DEFTBL	*s;
	const DEFTBL	*t;
	int i, j;
	for( i = 0; i < Grid->RowCount - 1; i++ ){
		s = GetDP(i);
		ks = *s->pKey;
		if( ks ){
			as = s->pSys;
			for( j = i + 1; j < Grid->RowCount - 1; j++ ){
				t = GetDP(j);
				if( t->pKey ){
					if( ks == *t->pKey ){
						Grid->Row = j + 1;
						WarningMB((Font->Charset != SHIFTJIS_CHARSET)?"Duplicated key '%s' and '%s'":"'%s' と '%s' が重複しています.", as.c_str(), t->pSys);
						return;
					}
				}
			}
		}
	}
	InfoMB( (Font->Charset != SHIFTJIS_CHARSET)?"No duplicated":"重複するショートカットキーはありません." );
}
//---------------------------------------------------------------------------
void __fastcall TShortCutDlg::DelBtnClick(TObject *Sender)
{
	int n = Grid->Row - 1;
	if( n >= 0 ){
		const DEFTBL *dp = GetDP(n);
		*dp->pKey = 0;
		UpdateUI(n);
		Grid->Invalidate();
	}
}
//---------------------------------------------------------------------------
void __fastcall TShortCutDlg::SBHelpClick(TObject *Sender)
{
	ShowHtmlHelp("keyboardshortcuts.htm");
}
//---------------------------------------------------------------------------
