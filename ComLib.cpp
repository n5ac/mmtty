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

#include "ComLib.h"
#include "rtty.h"
#include "LogFile.h"
//---------------------------------------------------------------------------
enum TFontPitch { fpDefault, fpVariable, fpFixed };	//JA7UDE 0427
//---------------------------------------------------------------------------
//#define	SYSMB	1			// MessageBox 0-Application, 1-System
//
char	BgnDir[256];		// 起動ディレクトリ
char	LogDir[256];		// 受信記録のディレクトリ
char	OutFileDir[256];	// テキスト送信ファイルのディエクトリ
char	MMLogDir[256];		// MMLOGファイルのディレクトリ
char	ExtLogDir[256];		// エクスポートファイルのディレクトリ
char	RecDir[256];		// 録音ファイルのディレクトリ
AnsiString	JanHelp;

int     WinNT;
int     WinVista;           //AA6YQ 1.66G

int		FSKCount;
int		FSKCount1;
int		FSKCount2;
int		FSKDeff;

double	SampFreq = 11025.0;	// サンプリング周波数
double	SampBase = 11025.0;
double	DemSamp = 11025.0*0.5;
int		DemOver = 1;
int		SampType = 0;
int     SampSize = 1024;
int		FFT_SIZE=2048;

SYSSET	sys;
LCID	lcid;				// ロケール情報
DWORD ColorTable[128];
int		DisPaint;
int		Remote;
//---------------------------------------------------------------------------
void __fastcall ShowHtmlHelp(LPCSTR pContext)
{
	char bf[512];

	sprintf(bf, "%s%s", BgnDir, sys.m_HTMLHelp.c_str());
	FILE *fp;
	if( (fp = fopen(bf, "rb"))!=NULL ){
		fclose(fp);

		HWND hd;
		if( Screen->ActiveForm != NULL ){
			hd = Screen->ActiveForm->Handle;
		}
		else {
			hd = Application->MainForm->Handle;
		}

		//1.70K look for CHM first

		if( !strcmpi(GetEXT(bf), "CHM") ){
			if( pContext ){
				char cmd[512];
				sprintf(cmd, "HH.EXE %s::/%s", bf, pContext);
				::WinExec(cmd, SW_SHOWDEFAULT);
			}
			else {
				::ShellExecute(hd, "open", bf, NULL, NULL, SW_SHOWDEFAULT);
			}
		}

		else if( !strcmpi(GetEXT(bf), "HLP") ){
			::WinHelp(hd, bf, HELP_FINDER, 0);
		}

		else {
			CWebRef	WebRef;
			WebRef.ShowHTML(bf);
		}
	}
	else if((sys.m_WinFontCharset != SHIFTJIS_CHARSET )){
		ErrorMB( "'%s' was not found.\r\n\r\nPlease search in the MMTTY English Web Site.", sys.m_HTMLHelp.c_str());
	}
	else {
		ErrorMB( "'%s'が見つかりません.\r\n\r\nMMTTY English Web Site からダウンロードして下さい.", sys.m_HTMLHelp.c_str());
	}

}
#if 0		// Delete by JE3HHT on 29.Sep.2010
void ShowHelp(int index)
{
	if( !JanHelp.IsEmpty() ){
		HWND hd;
		if( Screen->ActiveForm != NULL ){
			hd = Screen->ActiveForm->Handle;
		}
		else {
			hd = Application->MainForm->Handle;
		}
		if( index != -1 ){
			::WinHelp(hd, JanHelp.c_str(), HELP_CONTEXT, index);
		}
		else {
			::WinHelp(hd, JanHelp.c_str(), HELP_FINDER, 0);
		}
	}
}
#endif
//---------------------------------------------------------------------------
void InitSampType(void)
{
	if( SampFreq >= 11600.0 ){
		SampType = 3;
		SampBase = 12000.0;
		DemSamp = SampFreq * 0.5;
		DemOver = 1;
		FFT_SIZE = 2048;
		SampSize = (12000*1024)/11025;
	}
	else if( SampFreq >= 10000.0 ){
		SampType = 0;
		SampBase = 11025.0;
		DemSamp = SampFreq * 0.5;
		DemOver = 1;
		FFT_SIZE = 2048;
		SampSize = 1024;
	}
	else if( SampFreq >= 7000.0 ){
		SampType = 1;
		SampBase = 8000.0;
		DemSamp = SampFreq;
		DemOver = 0;
		FFT_SIZE = 1024;
		SampSize = (8000*1024)/11025;
	}
	else if( SampFreq >= 5000.0 ){
		SampType = 2;
		SampBase = 6000.0;
		DemSamp = SampFreq;
		DemOver = 0;
		FFT_SIZE = 1024;
		SampSize = (6000*1024)/11025;
	}
}
//---------------------------------------------------------------------------
int IsFile(LPCSTR pName)
{
	if( FileExists( pName ) )	//JA7UDE 0428
		return 1;
	else
		return 0;
	/*
	FILE *fp;
	fp = fopen(pName, "rb");
	if( fp != NULL ){
		fclose(fp);
		return 1;
	}
	else {
		return 0;
	}
	*/
}
//---------------------------------------------------------------------------
int SetTimeOffsetInfo(int &Hour, int &Min)
{
	TIME_ZONE_INFORMATION tz;

	int off;
	switch(GetTimeZoneInformation(&tz)){
		case TIME_ZONE_ID_STANDARD:
			off = tz.StandardBias;
			break;
		case TIME_ZONE_ID_DAYLIGHT:
			off = tz.DaylightBias;
			break;
		default:
			return FALSE;
	}
	if( off >= 0 ){
		Hour = -(off / 60);
		Min = (off % 60);
	}
	else {
		off = -off;
		Hour = off / 60;
		Min = off % 60;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void AddjustOffset(SYSTEMTIME *tp)
{
	if( sys.m_TimeOffset || sys.m_TimeOffsetMin ){
//typedef struct _SYSTEMTIME {  /* st */
//    WORD wYear;
//    WORD wMonth;
//    WORD wDayOfWeek;
//    WORD wDay;
//    WORD wHour;
//    WORD wMinute;
//    WORD wSecond;
//    WORD wMilliseconds;
//} SYSTEMTIME;


		LPCSTR	mt;
		int Hour = tp->wHour;
		int Min = tp->wMinute;
		Min += sys.m_TimeOffsetMin;
		if( Min >= 60 ){
			Hour++;
			Min -= 60;
		}
		else if( Min < 0 ){
			Hour--;
			Min += 60;
		}
		tp->wMinute = WORD(Min);
		Hour += sys.m_TimeOffset;
		if( Hour >= 24 ){
			tp->wHour = WORD(Hour - 24);
			tp->wDay++;
			if( tp->wYear % 4 ){
				mt = MONN;
			}
			else {
				mt = MONU;
			}
			if( tp->wDay > mt[tp->wMonth] ){
				tp->wDay = 1;
				tp->wMonth++;
				if( tp->wMonth > 12 ){
					tp->wMonth = 1;
					tp->wYear++;
				}
			}
		}
		else if( Hour < 0 ){
			tp->wHour = WORD(Hour + 24);
			tp->wDay--;
			if( tp->wDay < 1 ){
				tp->wMonth--;
				if( tp->wMonth < 1 ){
					tp->wMonth = 12;
					tp->wYear--;
				}
				if( tp->wYear % 4 ){
					tp->wDay = MONN[tp->wMonth];
				}
				else {
					tp->wDay = MONU[tp->wMonth];
				}
			}
		}
		else {
			tp->wHour = WORD(Hour);
		}
	}
}
//---------------------------------------------------------------------------
void GetUTC(SYSTEMTIME *tp)
{
	::GetSystemTime(tp);
	AddjustOffset(tp);
}
//---------------------------------------------------------------------------
void GetLocal(SYSTEMTIME *tp)
{
	::GetLocalTime(tp);
	AddjustOffset(tp);
}
//---------------------------------------------------------------------------
LPSTR StrDupe(LPCSTR s)
{
	LPSTR p = new char[strlen(s)+1];
	strcpy(p, s);
	return p;
}

//---------------------------------------------------------------------------
void InitColorTable(TColor cl, TColor ch)
{
	int		br = (ch & 0x000000ff);
	int		bg = (ch & 0x0000ff00) >> 8;
	int		bb = (ch & 0x00ff0000) >> 16;
	int		er = (cl & 0x000000ff);
	int		eg = (cl & 0x0000ff00) >> 8;
	int		eb = (cl & 0x00ff0000) >> 16;

	int		brw = er - br + 1;
	int		bgw = eg - bg + 1;
	int		bbw = eb - bb + 1;
	int		r, g, b;

	ColorTable[0] = DWORD( br | (bg<<8) | (bb<<16) );
	int i;
	for( i = 1; i < 127; i++ ){
		r = (br + (i * brw/128)) & 0x000000ff;
		g = (bg + (i * bgw/128)) & 0x000000ff;
		b = (bb + (i * bbw/128)) & 0x000000ff;
		ColorTable[i] = DWORD( r | (g<<8) | (b<<16) );
	}
	ColorTable[127] = DWORD( er | (eg<<8) | (eb<<16) );
}

const DEFKEYTBL KEYTBL[]={
	{ VK_F1, "F1" },
	{ VK_F2, "F2" },
	{ VK_F3, "F3" },
	{ VK_F4, "F4" },
	{ VK_F5, "F5" },
	{ VK_F6, "F6" },
	{ VK_F7, "F7" },
	{ VK_F8, "F8" },
	{ VK_F9, "F9" },
	{ VK_F10, "F10" },
	{ VK_F11, "F11" },
	{ VK_F12, "F12" },

	{ VK_LEFT, "←"},
	{ VK_RIGHT, "→"},
	{ VK_UP, "↑"},
	{ VK_DOWN, "↓"},
	{ VK_PRIOR, "PageUp"},
	{ VK_NEXT, "PageDown"},
	{ VK_HOME, "Home" },
	{ VK_END, "End" },
	{ VK_INSERT, "Insert" },
	{ VK_DELETE, "Delete" },
	{ VK_HELP, "Help" },
	{ VK_ESCAPE, "ESC" },
	{ VK_PAUSE, "PAUSE" },


	{ VK_F1 | 0x400, "Shift+F1" },
	{ VK_F2 | 0x400, "Shift+F2" },
	{ VK_F3 | 0x400, "Shift+F3" },
	{ VK_F4 | 0x400, "Shift+F4" },
	{ VK_F5 | 0x400, "Shift+F5" },
	{ VK_F6 | 0x400, "Shift+F6" },
	{ VK_F7 | 0x400, "Shift+F7" },
	{ VK_F8 | 0x400, "Shift+F8" },
	{ VK_F9 | 0x400, "Shift+F9" },
	{ VK_F10 | 0x400, "Shift+F10" },
	{ VK_F11 | 0x400, "Shift+F11" },
	{ VK_F12 | 0x400, "Shift+F12" },

	{ VK_LEFT | 0x400, "Shift+←"},
	{ VK_RIGHT | 0x400, "Shift+→"},
	{ VK_UP | 0x400, "Shift+↑"},
	{ VK_DOWN | 0x400, "Shift+↓"},
	{ VK_PRIOR | 0x400, "Shift+PageUp"},
	{ VK_NEXT | 0x400, "Shift+PageDown"},
	{ VK_HOME | 0x400, "Shift+Home" },
	{ VK_END | 0x400, "Shift+End" },
	{ VK_INSERT | 0x400, "Shift+Insert" },
	{ VK_DELETE | 0x400, "Shift+Delete" },
	{ VK_HELP | 0x400, "Shift+Help" },
	{ VK_ESCAPE | 0x400, "Shift+ESC" },
	{ VK_PAUSE | 0x400, "Shift+PAUSE" },

	{ VK_F1 | 0x100, "Ctrl+F1" },
	{ VK_F2 | 0x100, "Ctrl+F2" },
	{ VK_F3 | 0x100, "Ctrl+F3" },
	{ VK_F4 | 0x100, "Ctrl+F4" },
	{ VK_F5 | 0x100, "Ctrl+F5" },
	{ VK_F6 | 0x100, "Ctrl+F6" },
	{ VK_F7 | 0x100, "Ctrl+F7" },
	{ VK_F8 | 0x100, "Ctrl+F8" },
	{ VK_F9 | 0x100, "Ctrl+F9" },
	{ VK_F10 | 0x100, "Ctrl+F10" },
	{ VK_F11 | 0x100, "Ctrl+F11" },
	{ VK_F12 | 0x100, "Ctrl+F12" },
	{ '1' | 0x100, "Ctrl+1" },
	{ '2' | 0x100, "Ctrl+2" },
	{ '3' | 0x100, "Ctrl+3" },
	{ '4' | 0x100, "Ctrl+4" },
	{ '5' | 0x100, "Ctrl+5" },
	{ '6' | 0x100, "Ctrl+6" },
	{ '7' | 0x100, "Ctrl+7" },
	{ '8' | 0x100, "Ctrl+8" },
	{ '9' | 0x100, "Ctrl+9" },
	{ '0' | 0x100, "Ctrl+0" },
	{ 'A' | 0x100, "Ctrl+A" },
	{ 'B' | 0x100, "Ctrl+B" },
	{ 'C' | 0x100, "Ctrl+C" },
	{ 'D' | 0x100, "Ctrl+D" },
	{ 'E' | 0x100, "Ctrl+E" },
	{ 'F' | 0x100, "Ctrl+F" },
	{ 'G' | 0x100, "Ctrl+G" },
//    { 'H' | 0x100, "Ctrl+H" },
//    { 'I' | 0x100, "Ctrl+I" },
	{ 'K' | 0x100, "Ctrl+K" },
	{ 'L' | 0x100, "Ctrl+L" },
//    { 'M' | 0x100, "Ctrl+M" },
	{ 'N' | 0x100, "Ctrl+N" },
	{ 'O' | 0x100, "Ctrl+O" },
	{ 'P' | 0x100, "Ctrl+P" },
	{ 'Q' | 0x100, "Ctrl+Q" },
	{ 'R' | 0x100, "Ctrl+R" },
	{ 'S' | 0x100, "Ctrl+S" },
	{ 'T' | 0x100, "Ctrl+T" },
	{ 'U' | 0x100, "Ctrl+U" },
	{ 'V' | 0x100, "Ctrl+V" },
	{ 'W' | 0x100, "Ctrl+W" },
	{ 'X' | 0x100, "Ctrl+X" },
	{ 'Y' | 0x100, "Ctrl+Y" },
	{ 'Z' | 0x100, "Ctrl+Z" },

	{ VK_LEFT | 0x100, "Ctrl+←"},
	{ VK_RIGHT | 0x100, "Ctrl+→"},
	{ VK_UP | 0x100, "Ctrl+↑"},
	{ VK_DOWN | 0x100, "Ctrl+↓"},
	{ VK_PRIOR | 0x100, "Ctrl+PageUp"},
	{ VK_NEXT | 0x100, "Ctrl+PageDown"},
	{ VK_HOME | 0x100, "Ctrl+Home" },
	{ VK_END | 0x100, "Ctrl+End" },
	{ VK_INSERT | 0x100, "Ctrl+Insert" },
	{ VK_DELETE | 0x100, "Ctrl+Delete" },
	{ VK_HELP | 0x100, "Ctrl+Help" },
	{ VK_ESCAPE | 0x100, "Ctrl+ESC" },

	{ VK_F1 | 0x200, "Alt+F1" },
	{ VK_F2 | 0x200, "Alt+F2" },
	{ VK_F3 | 0x200, "Alt+F3" },
	{ VK_F4 | 0x200, "Alt+F4" },
	{ VK_F5 | 0x200, "Alt+F5" },
	{ VK_F6 | 0x200, "Alt+F6" },
	{ VK_F7 | 0x200, "Alt+F7" },
	{ VK_F8 | 0x200, "Alt+F8" },
	{ VK_F9 | 0x200, "Alt+F9" },
	{ VK_F10 | 0x200, "Alt+F10" },
	{ VK_F11 | 0x200, "Alt+F11" },
	{ VK_F12 | 0x200, "Alt+F12" },
	{ '1' | 0x200, "Alt+1" },
	{ '2' | 0x200, "Alt+2" },
	{ '3' | 0x200, "Alt+3" },
	{ '4' | 0x200, "Alt+4" },
	{ '5' | 0x200, "Alt+5" },
	{ '6' | 0x200, "Alt+6" },
	{ '7' | 0x200, "Alt+7" },
	{ '8' | 0x200, "Alt+8" },
	{ '9' | 0x200, "Alt+9" },
	{ '0' | 0x200, "Alt+0" },
	{ 'A' | 0x200, "Alt+A" },
	{ 'B' | 0x200, "Alt+B" },
	{ 'C' | 0x200, "Alt+C" },
	{ 'D' | 0x200, "Alt+D" },
	{ 'E' | 0x200, "Alt+E" },
	{ 'F' | 0x200, "Alt+F" },
	{ 'G' | 0x200, "Alt+G" },
	{ 'H' | 0x200, "Alt+H" },
	{ 'I' | 0x200, "Alt+I" },
	{ 'K' | 0x200, "Alt+K" },
	{ 'L' | 0x200, "Alt+L" },
	{ 'M' | 0x200, "Alt+M" },
	{ 'N' | 0x200, "Alt+N" },
	{ 'O' | 0x200, "Alt+O" },
	{ 'P' | 0x200, "Alt+P" },
	{ 'Q' | 0x200, "Alt+Q" },
	{ 'R' | 0x200, "Alt+R" },
	{ 'S' | 0x200, "Alt+S" },
	{ 'T' | 0x200, "Alt+T" },
	{ 'U' | 0x200, "Alt+U" },
	{ 'V' | 0x200, "Alt+V" },
	{ 'W' | 0x200, "Alt+W" },
	{ 'X' | 0x200, "Alt+X" },
	{ 'Y' | 0x200, "Alt+Y" },
	{ 'Z' | 0x200, "Alt+Z" },

	{ VK_LEFT | 0x200, "Alt+←"},
	{ VK_RIGHT | 0x200, "Alt+→"},
	{ VK_UP | 0x200, "Alt+↑"},
	{ VK_DOWN | 0x200, "Alt+↓"},
	{ VK_PRIOR | 0x200, "Alt+PageUp"},
	{ VK_NEXT | 0x200, "Alt+PageDown"},
	{ VK_HOME | 0x200, "Alt+Home" },
	{ VK_END | 0x200, "Alt+End" },
	{ VK_INSERT | 0x200, "Alt+Insert" },
	{ VK_DELETE | 0x200, "Alt+Delete" },
	{ VK_HELP | 0x200, "Alt+Help" },
	{ VK_ESCAPE | 0x200, "Alt+ESC" },
	{ VK_PAUSE | 0x200, "Alt+PAUSE" },
	{ 0, "" },
};
//---------------------------------------------------------------------------
LPCSTR ToDXKey(LPCSTR s)
{
	if( (sys.m_WinFontCharset != SHIFTJIS_CHARSET) && *s ){
		LPSTR		p;
		static char bf[32];
		strcpy(bf, s);
		if( (p = strstr(bf, "←"))!=NULL ){
			strcpy(p, "ArrowLeft");
			return bf;
		}
		else if( (p = strstr(bf, "→"))!=NULL ){
			strcpy(p, "ArrowRight");
			return bf;
		}
		else if( (p = strstr(bf, "↓"))!=NULL ){
			strcpy(p, "ArrowDown");
			return bf;
		}
		else if( (p = strstr(bf, "↑"))!=NULL ){
			strcpy(p, "ArrowUp");
			return bf;
		}
	}
	return s;
}
//---------------------------------------------------------------------------
LPCSTR ToJAKey(LPCSTR s)
{
	if( (sys.m_WinFontCharset != SHIFTJIS_CHARSET) && *s ){
		LPSTR		p;
		static char bf[32];
		strcpy(bf, s);
		if( (p = strstr(bf, "ArrowLeft"))!=NULL ){
			strcpy(p, "←");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowRight"))!=NULL ){
			strcpy(p, "→");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowDown"))!=NULL ){
			strcpy(p, "↓");
			return bf;
		}
		else if( (p = strstr(bf, "ArrowUp"))!=NULL ){
			strcpy(p, "↑");
			return bf;
		}
	}
	return s;
}
//---------------------------------------------------------------------------
LPCSTR GetKeyName(WORD Key)
{
	int i;
	for( i = 0; KEYTBL[i].Key; i++ ){
		if( Key == KEYTBL[i].Key ){
			break;
		}
	}
	return ToDXKey(KEYTBL[i].pName);
}
//---------------------------------------------------------------------------
WORD GetKeyCode(LPCSTR pName)
{
	LPCSTR p = ToJAKey(pName);
	int i;
	for( i = 0; KEYTBL[i].Key; i++ ){
		if( !strcmp(p, KEYTBL[i].pName) ) break;
	}
	return KEYTBL[i].Key;
}
//---------------------------------------------------------------------------
LPCSTR __fastcall ConvAndChar(LPSTR t, LPCSTR p)
{
	LPSTR s = t;

    while(*p){
		if( *p == '&' ){
			*t++ = *p;
        }
		*t++ = *p++;
    }
    *t = 0;
    return s;
}
//---------------------------------------------------------------------------
int SetTBValue(double d, double dmax, int imax)
{
	if( d ){
		int dd = int(sqrt(d * dmax * dmax / imax) + 0.5);
		if( dd > imax ) dd = imax;
		return dd;
	}
	else {
		return 0;
	}
}

double GetTBValue(int d, double dmax, int imax)
{
	return (d * d) * imax/ (dmax * dmax);
}

LPUSTR jstrupr(LPUSTR s)
{
	LPUSTR	p = s;
	int kf;

	for( kf = 0; *p; p++ ){
		if( kf ){
			kf = 0;
		}
		else if( _mbsbtype((unsigned char *)p, 0) == _MBC_LEAD ){
			kf = 1;
		}
		else {
			*p = (unsigned char)toupper(*p);
		}
	}
	return s;
}
/*#$%
===============================================================
	時刻を調整する
---------------------------------------------------------------
	t : 時刻(UTC)
	c : 時差コード
---------------------------------------------------------------
	ローカルタイム
---------------------------------------------------------------
	A-Z
	a-z 	+30min
===============================================================
*/
WORD AdjustRolTimeUTC(WORD tim, char c)
{
	const	char	tdf[]={
/*		A B C D E F G H I J K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z	*/
		1,2,3,4,5,6,7,8,9,9,10,11,12,23,22,21,20,19,18,17,16,15,14,13,12,0
	};
	int	cc;

	cc = toupper(c);
	if( (cc >= 'A')&&(cc <= 'Z') ){
		// JST to UTC
#if 0
		if( tim >= (9*60*30) ){
			tim -= WORD(9 * 60 * 30);
		}
		else {
			tim += WORD(15 * 60 * 30);
		}
#endif
		tim /= WORD(30);
		tim += WORD(tdf[cc-'A'] * 60);
		if( c >= 'a' ) tim += WORD(30);
		while( tim >= (24 * 60) ){
			tim -= WORD(24 * 60);
		}
		tim *= WORD(30);
		if( !tim ) tim++;
	}
	else {
		tim = 0;
	}
	return tim;
}
///----------------------------------------------------------------
///	ウィンドウをクライアントセンターにする
void FormCenter(TForm *tp, int XW, int YW)
{
	int top = (YW - tp->Height)/2;
	int left = (XW - tp->Width)/2;
	if( top < 0 ) top = 0;
	if( left < 0 ) left = 0;
	tp->Top = top;
	tp->Left = left;
}
//---------------------------------------------------------------------------
void __fastcall SetGroupEnabled(TGroupBox *gp)
{
	int enb = gp->Enabled;
	TControl *tp;
	int i;
	for( i = 0; i < gp->ControlCount; i++ ){
		tp = gp->Controls[i];
		if( tp != NULL ){
			tp->Enabled = enb;
			if( tp->ClassNameIs("TGroupBox") ){
				SetGroupEnabled((TGroupBox *)tp);
			}
		}
	}
	gp->Font->Color = gp->Enabled ? clBlack : clGrayText;
	for( i = 0; i < gp->ControlCount; i++ ){
		tp = gp->Controls[i];
		if( tp != NULL ){
			if( tp->ClassNameIs("TComboBox") ){
				((TComboBox *)tp)->SelLength = 0;
			}
		}
	}
}
//---------------------------------------------------------------------------
void SetButtonCaption(TSpeedButton *tb, AnsiString &as, int maxlen, int fontsize, TColor col)
{
	char bf[256];
    ConvAndChar(bf, as.c_str());
	int len = strlen(as.c_str());
	if( (!len) || (len <= maxlen) ){
		fontsize = fontsize + sys.m_BtnFontAdjSize;
	}
	else {
		int size = fontsize * maxlen * 1.1 / len;
		if( size < 6 ) size = 6;
		fontsize = size + sys.m_BtnFontAdjSize;
	}
    tb->Font->Size = fontsize;
	tb->Font->Color = col;
//    if( tb->Caption != bf ) tb->Caption = bf;
    tb->Caption = bf;
}
/*#$%
========================================================
	最後の文字コードを返す
--------------------------------------------------------
	p : 文字列のポインタ
--------------------------------------------------------
	文字コード
--------------------------------------------------------
========================================================
*/
char *lastp(char *p)
{
	if( *p ){
		for( ; *p; p++ );
		p--;
		return(p);
	}
	else {
		return(p);
	}
}
/*#$%
========================================================
	末尾のスペースとＴＡＢを取る
--------------------------------------------------------
	s : 文字列のポインタ
--------------------------------------------------------
	文字列のポインタ
--------------------------------------------------------
========================================================
*/
char *clipsp(char *s)
{
	char	*p;

	if( *s ){
		for( p = lastp(s); p >= s; p-- ){
			if( (*p == ' ')||(*p == 0x09) ){
				*p = 0;
			}
			else {
				break;
			}
		}
	}
	return(s);
}
//---------------------------------------------------------------------------
LPCSTR _strdmcpy(LPSTR t, LPCSTR p, char c)
{
	for( ; *p && (*p != c); p++, t++ ) *t = *p;
	*t = 0;
	if( *p == c ) p++;
	return(p);
}
const char *StrDlmCpy(char *t, const char *p, char Dlm, int len)
{
	const char _tt1[]="[{(｢<";
	const char _tt2[]="]})｣>";
	const char	*pp;
	int			r = FALSE;

	char	Key;
	if( (pp = strchr(_tt2, Dlm))!=NULL ){
		Key = _tt1[pp - _tt2];
	}
	else {
		Key = 0;
	}
	int	f, k;
	for( f = k = 0; *p;  p++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( *p == Key ){
			f++;
		}
		else if( *p == Dlm ){
			if( f ){
				f--;
			}
			else {
				r = TRUE;
				p++;
				break;
			}
		}
		if( len ){
			*t++ = *p;
			len--;
		}
	}
	*t = 0;
	return (r == TRUE) ? p : NULL;
}

const char *StrDlmCpyK(char *t, const char *p, char Dlm, int len)
{
	const char _tt1[]="[{(｢<";
	const char _tt2[]="]})｣>";
	const char	*pp;
	int			r = FALSE;

	char	Key;
	if( (pp = strchr(_tt2, Dlm))!=NULL ){
		Key = _tt1[pp - _tt2];
	}
	else {
		Key = 0;
	}
	int	f, k;
	for( f = k = 0; *p;  p++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( (pp = strchr(_tt1, *p))!=NULL ){
			Key = _tt2[pp - _tt1];
			f++;
		}
		else if( f && (*p == Key) ){
			f--;
			Key = 0;
		}
		else if( *p == Dlm ){
			if( !f ){
				r = TRUE;
				p++;
				break;
			}
		}
		if( len ){
			*t++ = *p;
			len--;
		}
	}
	*t = 0;
	return (r == TRUE) ? p : NULL;
}
//---------------------------------------------------------------------------
void StrCopy(LPSTR t, LPCSTR s, int n)
{
	for( ; *s && n; n--, s++, t++){
		*t = *s;
	}
	*t = 0;
}
//---------------------------------------------------------------------------
char LastC(LPCSTR p)
{
	char c = 0;

	if( *p ){
		c = *(p + strlen(p) - 1);
	}
	return c;
}

/*#$%
========================================================
	拡張子を得る
--------------------------------------------------------
	p : 文字列のポインタ
--------------------------------------------------------
	文字コード
--------------------------------------------------------
========================================================
*/
LPCSTR GetEXT(LPCSTR Fname)
{
	if( *Fname ){
		LPCSTR p = Fname + (strlen(Fname) - 1);
		for( ; p > Fname; p-- ){
			if( *p == '.' ) return p+1;
		}
	}
	return "";
}

void SetEXT(LPSTR pName, LPSTR pExt)
{
	if( *pName ){
		LPSTR	p;

		for( p = &pName[strlen(pName)-1]; p >= pName; p-- ){
			if( *p == '.' ){
				strcpy(p, pExt);
				return;
			}
		}
		strcat(pName, pExt);
	}
}

void SetCurDir(LPSTR t, int size)
{
	if( !::GetCurrentDirectory(size-1, t) ){
		*t = 0;
	}
	else {
		if( LastC(t) != '\\' ){
			strcat(t, "\\");
		}
	}
}

void SetDirName(LPSTR t, LPCSTR pName)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char ext[_MAX_EXT];
	AnsiString	Dir;

	::_splitpath( pName, drive, dir, name, ext );
	Dir = drive;
	Dir += dir;
	strncpy(t, Dir.c_str(), 128);
}

LPCSTR StrDbl(double d)
{
	static	char	bf[64];
	LPSTR	p;

	sprintf(bf, "%lf", d);
	for( p = &bf[strlen(bf)-1]; p >= bf; p-- ){
		if( (*p == '0')&&(*(p-1)!='.') ){
			*p = 0;
		}
		else {
			break;
		}
	}
	return bf;
}

LPCSTR StrDblE(double d)
{
	static	char	bf[32];

	sprintf(bf, "%le", d);
	return bf;
}

double ReadDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d)
{
	AnsiString cs = StrDblE(d);
	AnsiString rs = p->ReadString(c1, c2, cs);
	if( sscanf(rs.c_str(), "%lf", &d) != 1 ) d = 0.0;
	return d;
}

void WriteDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d)
{
	AnsiString cs = StrDblE(d);
	p->WriteString(c1, c2, cs);
}

///----------------------------------------------------------------
///  コメントを削除（スペースおよびＴＡＢも削除）
///
void ClipLF(LPSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp == LF)||(*sp == CR) ){
			*sp = 0;
			break;
		}
	}
}

LPSTR FillSpace(LPSTR s, int n)
{
	LPSTR p = s;
	int i = 0;
	for( ; *p && (i < n); i++, p++ );
	for( ; i < n; i++, p++ ){
		*p = ' ';
	}
	*p = 0;
	return s;
}
///----------------------------------------------------------------
///  ホワイトスペースのスキップ
///
LPSTR SkipSpace(LPSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp != ' ')&&(*sp != TAB) ) break;
	}
	return sp;
}

LPCSTR SkipSpace(LPCSTR sp)
{
	for( ; *sp; sp++ ){
		if( (*sp != ' ')&&(*sp != TAB) ) break;
	}
	return sp;
}

///----------------------------------------------------------------
///  デリミッタ分解を行う
///
LPSTR StrDlm(LPSTR &t, LPSTR p)
{
	return StrDlm(t, p, ',');
}

///----------------------------------------------------------------
///  デリミッタ分解を行う
///
LPSTR StrDlm(LPSTR &t, LPSTR p, char c)
{
	int		f, k;
	LPSTR	d1=NULL;
	LPSTR	d2=NULL;

	t = p;
	f = k = 0;
	while(*p){
		if( k ){															// 漢字２バイト目
			k = 0;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
		}
		else if( *p == 0x22 ){
			if( !f ){
				if( d1 == NULL ) d1 = p+1;
				f++;
			}
			else {
				d2 = p;
				f--;
			}
		}
		else if( !f && (*p == c) ){
			*p = 0;
			p++;
			break;
		}
		p++;
	}
	if( (d1!=NULL)&&(d2!=NULL) ){
		if( ((t+1)==d1) && ( ((p-2)==d2)||((p-1)==d2) ) ){
			t = d1;
			*d2 = 0;
		}
	}
	return(p);
}

///----------------------------------------------------------------
///
void ChgString(LPSTR t, char a, char b)
{
	for( ; *t; t++ ){
		if( *t == a ) *t = b;
	}
}

///----------------------------------------------------------------
///
void DelChar(LPSTR t, char a)
{
	for( ; *t; t++ ){
		if( *t == a ){
			strcpy(t, t+1);
			t--;
		}
	}
}

/*#$%
========================================================
	デシマルアスキーを数値に変換する
--------------------------------------------------------
	p : 文字列のポインタ
	n : 変換桁数
--------------------------------------------------------
	数値
--------------------------------------------------------
========================================================
*/
int atoin(const char *p, int n)
{
	int	d;

	for( d = 0; *p && n; p++, n-- ){
		d *= 10;
		d += (*p & 0x0f);
	}
	return(d);
}
/*#$%
========================================================
	１６進アスキーを数値に変換する
--------------------------------------------------------
	p : 文字列のポインタ
	n : 変換桁数
--------------------------------------------------------
	数値
--------------------------------------------------------
========================================================
*/
int htoin(const char *p, int n)
{
	if( *p == 'x' ) return 0;
	int	d;

	for( d = 0; *p && n; p++, n-- ){
		d = d << 4;
		d += (*p & 0x0f);
		if( *p >= 'A' ) d += 9;
	}
	return(d);
}
///----------------------------------------------------------------
HWND GetMBHandle(int &flag)
{
	HWND hd;
	if( Screen->ActiveForm != NULL ){
		hd = Screen->ActiveForm->Handle;
	}
	else {
		hd = NULL;
	}
	flag = MB_SETFOREGROUND;
	if( Remote ) flag |= MB_TOPMOST;
	flag |= (hd == NULL) ? MB_TASKMODAL : MB_APPLMODAL;
	return hd;
}
///----------------------------------------------------------------
///  メッセージの表示
///
void InfoMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	flag |= (MB_OK | MB_ICONINFORMATION);
	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, "MMTTY", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
}
///----------------------------------------------------------------
///  エラーメッセージの表示
///
void ErrorMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	flag |= (MB_OK | MB_ICONEXCLAMATION);
	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, (sys.m_WinFontCharset != SHIFTJIS_CHARSET) ? "Error":"ｴﾗｰ", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
}

///----------------------------------------------------------------
///  警告メッセージの表示
///
void WarningMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	flag |= (MB_OK | MB_ICONEXCLAMATION);
	Application->NormalizeTopMosts();
	::MessageBox(hd, bf, (sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"Warning":"警告", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
}

///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int YesNoMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	if( Screen->ActiveForm != NULL ) NormalWindow(Screen->ActiveForm);
	flag |= (MB_YESNO | MB_ICONQUESTION);
	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMTTY", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
	return r;
}

///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int YesNoCancelMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	flag |= (MB_YESNOCANCEL | MB_ICONQUESTION);
	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMTTY", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
	return r;
}

///----------------------------------------------------------------
///  実行選択メッセージの表示
///
int OkCancelMB(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);

	int	flag;
	HWND hd = GetMBHandle(flag);
	DisPaint = TRUE;
	flag |= (MB_OKCANCEL | MB_ICONQUESTION);
	Application->NormalizeTopMosts();
	int r = ::MessageBox(hd, bf, "MMTTY", flag);
	Application->RestoreTopMosts();
	DisPaint = FALSE;
	return r;
}
int RemoveL2(LPSTR t, LPSTR ss, LPCSTR pKey, int size)
{
	char	c;
	int		k;
	LPCSTR	pp;
	LPSTR	s;
	const char	_tt1[]="[{(｢<";
	const char	_tt2[]="]})｣>";

	int		len = strlen(pKey);
	char ac = ';';
	for( k = 0, s = ss; *s; s++ ){
		if( k ){															// 漢字２バイト目
			k = 0;
			ac = 0x1e;
		}
		else if(  _mbsbtype((const unsigned char *)s, 0) == _MBC_LEAD ){	// 漢字１バイト目
			k = 1;
			if( (len >= 2) && (strchr(" ,./;:*\t[{(｢<]})｣>", ac)!=NULL) && (!strnicmp(s, pKey, len)) ){
				pp = s + len;
				if( (pp = strchr(_tt1, *pp))!=NULL ){
					c = _tt2[pp - _tt1];
					if( *(s+len+1) ){
						StrDlmCpy(t, s+len+1, c, size);
						if( *t ){
							strcpy((ac == ' ')?(s-1):s, s+len+2+strlen(t));
							return TRUE;
						}
					}
				}
			}
		}
		else if( (strchr(" ,./;:*\t\x1e[{(｢<]})｣>", ac)!=NULL) && (!strnicmp(s, pKey, len)) ){
			pp = s + len;
			if( (pp = strchr(_tt1, *pp))!=NULL ){
				c = _tt2[pp - _tt1];
				if( *(s+len+1) ){
					StrDlmCpy(t, s+len+1, c, size);
					if( *t ){
						strcpy((ac == ' ')?(s-1):s, s+len+2+strlen(t));
						return TRUE;
					}
				}
			}
			ac = *s;
		}
		else {
			ac = *s;
		}
	}
	return FALSE;
}

void AddL2(LPSTR t, LPCSTR pKey, LPCSTR s, UCHAR c1, UCHAR c2, int size)
{
	if( *s ){
		int len = strlen(t);
		int lenkey = strlen(pKey);
		int lenData = strlen(s);
		if( (len + lenkey + lenData + 2) < size ){
			t += len;
			if( len ) *t++ = ' ';
			if( *pKey ){
				strcpy(t, pKey);
				t += lenkey;
			}
			*t++ = c1;
			strcpy(t, s);
			t += lenData;
			*t++ = c2;
			*t = 0;
		}
	}
}
///----------------------------------------------------------------
///  数字が含まれるかどうか調べる
///
int IsNumbs(LPCSTR p)
{
	for( ; *p; p++ ){
		if( isdigit(*p) ) return 1;
	}
	return 0;
}
///----------------------------------------------------------------
///  数字が含まれるかどうか調べる
///
int IsAlphas(LPCSTR p)
{
	for( ; *p; p++ ){
		if( isalpha(*p) ) return 1;
	}
	return 0;
}
///----------------------------------------------------------------
///  RSTかどうか調べる
///
int IsRST(LPCSTR p)
{
	if( strlen(p) < 3 ) return 0;
	if( (*p < '1') || (*p > '5') ) return 0;
	p++;
	if( (*p < '1') || (*p > '9') ) return 0;
	p++;
	if( (*p < '1') || (*p > '9') ) return 0;
	return 1;
}
///----------------------------------------------------------------
int IsCallChar(char c)
{
	if( !isalpha(c) && !isdigit(c) && (c != '/') ) return 0;
	if( islower(c) ) return 0;
	return 1;
}
///----------------------------------------------------------------
///  コールサインかどうか調べる
///
int IsCall(LPCSTR p)
{
	int l = strlen(p);
	if( l > 16 ) return 0;
	if( l < 3 ) return 0;
	if( isdigit(*p) ){					// 先頭が数字
		if( l <= 3 ) return 0;				// 3文字以下の時NG
		if( isdigit(*(p+1)) ) return 0;		// 2文字目が数字の時NG
	}
	if( isdigit(LastC(p)) ){			// 最後が数字
		if( l <= 4 ) return 0;				// ４文字以下の時NG
//      if( !strchr(p, '/')==NULL ) return0;	// /が含まれていない時NG
//		if( p[l-2] != '/' ) return 0;		// 最後の１つ前が/以外の時NG
	}
	int i;
	for( i = 0; i < l-1; i++, p++ ){
		if( isdigit(*p) ) return 1;		// 数字が含まれていればOK
	}
	return 0;
}
///----------------------------------------------------------------
///  名前かどうか調べる
///
int IsName(LPCSTR p)
{
	if( strlen(p) >= MLNAME ) return 0;
	for( ; *p; p++ ){
		if( !isalpha(*p) ) return 0;
	}
	return 1;
}
/*#$%
======================================================
	ＪＡとＤＸの区別をする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	1 : JA
	0 : DX (JD1 INCLUDEED)
------------------------------------------------------
======================================================
*/
int IsJA(const char *s)
{
	if( (!strcmp(s, "8J1RL"))||(!strcmp(s, "8J1RM"))||(strstr(s, "/JD1")!=NULL) ){
		return(0);
	}
	else if( strchr(s, '/') ){
		char	bf[MLCALL+1];
		StrCopy(bf, s, MLCALL);
		char	*p;
		char	*t;
		for( p = bf; *p; ){
			p = StrDlm(t, p, '/');
			if( *t ){
				if( (strlen(t) >= 2) && isdigit((unsigned char)LastC(t)) ){
					if( *t == 'J' ){
						t++;
						if( *t == 'D' ) return(0);
						if( (*t >= 'A')&&(*t <= 'S' ) ) return(1);
					}
					else if( *t == '7' ){
						t++;
						if( (*t >= 'J')&&(*t <= 'N' ) ) return(1);
					}
					else if( *t == '8' ){
						t++;
						if( (*t >= 'J')&&(*t <= 'N' ) ) return(1);
					}
					else {
						return 0;
					}
				}
			}
		}
	}
	if( *s == 'J' ){
		s++;
		if( *s == 'D' ) return(0);
		if( (*s >= 'A')&&(*s <= 'S' ) ) return(1);
	}
	else if( *s == '7' ){
		s++;
		if( (*s >= 'J')&&(*s <= 'N' ) ) return(1);
	}
	else if( *s == '8' ){
		s++;
		if( (*s >= 'J')&&(*s <= 'N' ) ) return(1);
	}
	return(0);
}
/*#$%
======================================================
	コールサインをクリップする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	クリップコールのポインタ
------------------------------------------------------
======================================================
*/
LPCSTR ClipCall(LPCSTR s)
{
	static	char	bf[MLCALL+1];
	LPCSTR	p1, p2;

	if( (p1=strchr(s, '/'))!=NULL ){
		if( (p2=strchr(p1+1, '/'))!=NULL ){	/* ３分割	*/
			if( (int(strlen(p2+1)) < int((p2 - p1)+1)) || (!IsCall(p2+1)) ){		/* 最後より途中が長い	*/
				if( ((p2-p1) < (p1-s))||(!IsCall(p1+1)) ){	/* 途中より最初が長い	*/
					StrCopy(bf, s, MLCALL);
					*strchr(bf, '/') = 0;
					return(bf);
				}
				else {
					strcpy(bf, p1+1);
					*strchr(bf, '/') = 0;
					return(bf);
				}
			}
			else if( int(strlen(p2+1)) < int((p1 - s)+1) ){	/* 最後より最初が長い	*/
				StrCopy(bf, s, MLCALL);
				*strchr(bf, '/') = 0;
				return(bf);
			}
			else {
				return(p2+1);
			}
		}
		else {								/* ２分割	*/
			if( (int(strlen(p1+1)) < int((p1 - s)+1)) || (!IsCall(p1+1)) ){
				StrCopy(bf, s, MLCALL);
				*strchr(bf, '/') = 0;
				return(bf);
			}
			else {
				return(p1+1);
			}
		}
	}
	else {
		return(s);
	}
}

/*#$%
======================================================
	ポータブル表記を調べる
------------------------------------------------------
	p : コールサイン文字列のポインタ
------------------------------------------------------
	数値のポインタ
------------------------------------------------------
======================================================
*/
static LPSTR chkptb(LPSTR p)
{
	if( *p ){
		p = lastp(p);
		if( isdigit(*p) && ((*(p-1))=='/') ){
			return(p);
		}
	}
	return(NULL);
}

/*#$%
======================================================
	ポータブル表記の入れ替え
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
------------------------------------------------------
	元の文字列を破壊する
======================================================
*/
void chgptb(LPSTR s)
{
	LPSTR	p, t;

	if( (p = chkptb(s))!=NULL ){
		t = p;
		for( p--; p > s; p-- ){
			if( isdigit(*p) ){
				*p = *t;
				t--;
				*t = 0;
				break;
			}
		}
	}
}

/*#$%
======================================================
	コールサインからカントリの元をクリップする
------------------------------------------------------
	s : コールサイン文字列のポインタ
------------------------------------------------------
	カントリのポインタ
------------------------------------------------------
======================================================
*/
LPCSTR ClipCC(LPCSTR s)
{
	static	char	bf[MLCALL+1];
	LPSTR	p, t;

	StrCopy(bf, s, MLCALL);
	chgptb(bf);			/* ポータブル表記の入れ替え	*/
	for( p = bf; *p; ){
		if( *p ){
			p = StrDlm(t, p, '/');
			if( (strlen(t) > 1) && (isdigit(*t) || isdigit(LastC(t))) ) return(t);
		}
	}
	for( p = bf; *p; ){
		if( *p ){
			p = StrDlm(t, p, '/');
			if( (strlen(t) > 1) && (*t!='Q') && strcmp(t, "MM") ) return(t);
		}
	}
	return ClipCall(s);
}

///----------------------------------------------------------------
///  文字列変換
///
void Yen2CrLf(AnsiString &ws, AnsiString cs)
{
	ws = "";
	LPCSTR	p;
	int		f;
	int		dlm = 0;

	p = cs.c_str();
	if( *p == 0x22 ){
		p++;
		dlm++;
	}
	for( f = 0; *p; p++ ){
		if( f ){
			f = 0;
			ws += *p;
		}
		else if(  _mbsbtype((const unsigned char *)p, 0) == _MBC_LEAD ){
			f = 1;
			ws += *p;
		}
		else if( *p == '\\' ){
			switch(*(p+1)){
			case 'r':
				ws += "\r";
				p++;
				break;
			case 'n':
				ws += "\n";
				p++;
				break;
			case '\\':
				ws += "\\";
				p++;
				break;
			default:
				p++;
				ws += *p;
				break;
			}
		}
		else if( !dlm || (*p!=0x22) || *(p+1) ){
			ws += *p;
		}
	}
}

void CrLf2Yen(AnsiString &ws, AnsiString cs)
{
	ws = "\x22";
	LPCSTR	p;

	for( p = cs.c_str(); *p; p++ ){
		if( *p == 0x0d ){
			ws += "\\r";
		}
		else if( *p == 0x0a ){
			ws += "\\n";
		}
		else if( *p == '\\' ){
			ws += "\\\\";
		}
		else {
			ws += *p;
		}
	}
	ws += "\x22";
}


//---------------------------------------------------------------------------
// リストへの定型メッセージの登録（常に最新が先頭になる）
void EntryMsg(AnsiString ttl, AnsiString as, WORD Key)
{
	if( ttl.IsEmpty() ) return;
	if( as.IsEmpty() ) return;
	int i, j;

	// 検索
	for( i = 0; i < MSGLISTMAX; i++ ){
		if( sys.m_MsgName[i].IsEmpty() ) break;
		if( sys.m_MsgName[i] == ttl ) break;
	}
	// 一致したものを削除
	for( j = i; j < (MSGLISTMAX - 1); j++ ){
		if( sys.m_MsgName[j].IsEmpty() ) break;
		sys.m_MsgList[j] = sys.m_MsgList[j+1];
		sys.m_MsgName[j] = sys.m_MsgName[j+1];
		sys.m_MsgKey[j] = sys.m_MsgKey[j+1];
	}
	// 全体を１つ後ろにずらす
	for( j = MSGLISTMAX - 1; j > 0; j-- ){
		sys.m_MsgList[j] = sys.m_MsgList[j-1];
		sys.m_MsgName[j] = sys.m_MsgName[j-1];
		sys.m_MsgKey[j] = sys.m_MsgKey[j-1];
	}
	sys.m_MsgName[0] = ttl;
	sys.m_MsgList[0] = as;
	sys.m_MsgKey[0] = Key;
}

//---------------------------------------------------------------------------
// コンボＢＯＸに文字列を設定する
void SetComboBox(TComboBox *pCombo, LPCSTR pList)
{
	pCombo->Clear();
	LPSTR s = strdup(pList);
	LPSTR p = s;
	LPSTR t;
	while(*p){
		p = StrDlm(t, p);
		clipsp(t);
		t = SkipSpace(t);
		if( *t ) pCombo->Items->Add(t);
	}
	free(s);
}

//---------------------------------------------------------------------------
// コンボＢＯＸに文字列を設定する
void GetComboBox(AnsiString &as, TComboBox *pCombo)
{
	as = "";
	int i;
	for( i = 0; i < pCombo->Items->Count; i++ ){
		if( i ) as += ',';
		as += pCombo->Items->Strings[i];
	}
}
//---------------------------------------------------------------------------
// グリッドの間隔を設定する
void SetGridWidths(TStringGrid *pGrid, LPCSTR pList)
{
	LPSTR s = strdup(pList);
	LPSTR p = s;
	LPSTR t;
	int i;
	for( i = 0; (i < pGrid->ColCount) && *p; i++ ){
		p = StrDlm(t, p);
		clipsp(t);
		t = SkipSpace(t);
		if( *t ){
			int w = atoin(t, -1);
			pGrid->ColWidths[i] = w;
		}
	}
	free(s);
}
//---------------------------------------------------------------------------
// グリッドの間隔を読みこむ
void GetGridWidths(AnsiString &as, TStringGrid *pGrid)
{
	as = "";
	int i;
	for( i = 0; i < pGrid->ColCount; i++ ){
		if( i ) as += ',';
		as += pGrid->ColWidths[i];
	}
}

int FontStyle2Code(TFontStyles style)
{
	int code = 0;

	TFontStyles fa;
	TFontStyles fb;

	fa << fsBold;
	fb << fsBold;
	fa *= style;
	if( fa == fb ) code |= FSBOLD;
	fa >> fsBold;
	fb >> fsBold;
	fa << fsItalic;
	fb << fsItalic;
	fa *= style;
	if( fa == fb ) code |= FSITALIC;
	return code;
}

TFontStyles Code2FontStyle(int code)
{
	TFontStyles fs;

	if( code & FSBOLD ) fs << fsBold;
	if( code & FSITALIC ) fs << fsItalic;
	return fs;
}

static int FontZH;
static int FontZW;
static int FontZOX;
static int FontZOY;

LPCSTR GetZeroCode(void)
{
	static char bf[3];
	bf[2] = 0;
	switch(sys.m_FontCharset){
		case SHIFTJIS_CHARSET:
			bf[0] = 0x83;
			bf[1] = 0xd3;
			break;
		case HANGEUL_CHARSET:
			bf[0] = 0xa8;    // 0xa9
			bf[1] = 0xaa;
			break;
		case CHINESEBIG5_CHARSET:   // 台湾
			bf[0] = 0xa3;
			bf[1] = 0x58;
			break;
		case 134:                   // 中国語簡略
			bf[0] = 0xa6;
			bf[1] = 0xd5;
			break;
		default:
			bf[0] = char('ﾘ');
			bf[1] = 0;
		break;
	}
	return bf;
}

void DrawChar(TCanvas *pCanvas, int x, int y, char c)
{
	char bf[3];
	bf[1] = 0;
	int size = 0;
	if( (c == '0') && sys.m_FontZero ){
		strcpy(bf, GetZeroCode());
		if( bf[1] ){
			size = pCanvas->Font->Size;
			pCanvas->Font->Size = size * 7 / 8;
			x += FontZOX;
			y += FontZOY;
		}
	}
	else {
		bf[0] = c;
	}

	::SetBkMode(pCanvas->Handle, TRANSPARENT);
	pCanvas->TextOut(x, y, bf);
	if( size ){
		pCanvas->Font->Size = size;
	}
}

//---------------------------------------------------------------------------
void TopWindow(TForm *tp)
{
	if( tp->FormStyle == fsStayOnTop ){
		::SetWindowPos(tp->Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}
void NormalWindow(TForm *tp)
{
	if( tp->FormStyle == fsStayOnTop ){
		::SetWindowPos(tp->Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}
//---------------------------------------------------------------------------
// メイン画面表示処理クラス
CPrintText::CPrintText()
{
	pPaintBox = NULL;
	pCanvas = NULL;
	pBitmap = NULL;

	m_TX = 0;

	m_Scroll = 0;
	m_Top = 0;
	m_Line = 0;
	m_Col = 0;
	m_Count = 0;
	m_WriteLine = 0;
	m_DisEvent = 0;
	m_fp = NULL;
	memset(pList, 0, sizeof(pList));
	memset(pColList, 0, sizeof(pColList));
}

CPrintText::~CPrintText()
{
	int i;
	for( i = 0; i < TEXTLINEMAX; i++ ){
		if( pList[i] != NULL ) delete[] pList[i];   	//JA7UDE 0428
		pList[i] = NULL;
		if( pColList[i] != NULL ) delete[] pColList[i];	//JA7UDE 0428
		pColList[i] = NULL;
	}
}

void CPrintText::Clear(void)
{
	m_Top = 0;
	m_Line = 0;
	m_Col = 0;
	m_Count = 0;
	m_WriteLine = 0;

	int i;
	for( i = 0; i < TEXTLINEMAX; i++ ){
		*pList[i] = char(i ? -1 : 0);
	}
	for( i = 0; i < TEXTLINEMAX; i++ ){
		*pColList[i] = 0;
	}
	if( pScroll != NULL ) pScroll->Enabled = FALSE;
	if( pPaintBox != NULL ) pPaintBox->Invalidate();
}

void CPrintText::CursorOff(void)
{
	if( m_Cursor ){
		DrawCursor(0);
	}
}

void CPrintText::CursorOn()
{
	if( m_Cursor ){
		DrawCursor(1);
	}
}

void CPrintText::DrawCursor(int sw)
{
	if( GetPos(m_Line) != m_WriteLine ) return;

	TRect rc;

	rc.Left = m_Col * m_FW + 1;
	rc.Right = rc.Left + m_FW - 1;
	rc.Bottom = (m_Line * m_FH) + m_FH - 1;
	rc.Top = rc.Bottom - 3;

	if( sw ){
		pCanvas->Brush->Color = sys.m_ColorRX;
		pCanvas->FillRect(rc);
	}
	else {
		if( pBitmap != NULL ){
			pCanvas->Brush->Bitmap = pBitmap;
		}
		else {
			pCanvas->Brush->Color = sys.m_ColorRXBack;
		}
		pCanvas->FillRect(rc);
//		pCanvas->Font->Color = sys.m_ColorRX;
		if( (pList[m_WriteLine] != NULL) && (*pList[m_WriteLine]!=-1) ){
			LPSTR p = pList[m_WriteLine];
			LPSTR pc = pColList[m_WriteLine];
			if( int(strlen(p)) > m_Col ){
				pCanvas->Font->Color = pc[m_Col] ? sys.m_ColorRXTX : sys.m_ColorRX;
				DrawChar(pCanvas, m_Col * m_FW, m_Line * m_FH, p[m_Col]);
			}
		}
	}
}

void CPrintText::SetCursor(int sw)
{
	if( m_Cursor != sw ){
		DrawCursor(sw);
		m_Cursor = sw;
	}
}

void CPrintText::AdjZero(void)
{
	if( pCanvas == NULL ) return;

	LPCSTR p = GetZeroCode();
	if( sys.m_FontZero && (*(p+1)) ){
		pCanvas->Font->Size = sys.m_FontSize * 7 / 8;
		FontZH = pCanvas->TextHeight(p) + sys.m_FontAdjY;
		FontZW = pCanvas->TextWidth(p) + sys.m_FontAdjX;
		FontZOX = (m_FW - FontZW)/2;
		FontZOY = (m_FH - FontZH)/2;
	}
	pCanvas->Font->Size = sys.m_FontSize;
}

void CPrintText::SetPaintBox(TPaintBox *tp, TScrollBar *sp)
{
	pPaintBox = tp;
	pScroll = sp;

	pScroll->Align = alRight;
	pScroll->EnableAlign();
	pCanvas = pPaintBox->Canvas;

	if( !sys.m_FontName.IsEmpty() ){
		pPaintBox->Font->Name = sys.m_FontName;
		pCanvas->Font->Name = sys.m_FontName;
	}
	pPaintBox->Font->Charset = BYTE(sys.m_FontCharset);
	pCanvas->Font->Charset = BYTE(sys.m_FontCharset);

	TFontStyles fs = Code2FontStyle(sys.m_FontStyle);
	pPaintBox->Font->Style = fs;
	pCanvas->Font->Style = fs;

	pPaintBox->Font->Pitch = fpFixed;
	pCanvas->Font->Pitch = fpFixed;
	LPCSTR p = GetZeroCode();
	if( sys.m_FontZero && (*(p+1)) ){
		pPaintBox->Font->Size = sys.m_FontSize * 7 / 8;
		pCanvas->Font->Size = sys.m_FontSize * 7 / 8;
		FontZH = pCanvas->TextHeight(p) + sys.m_FontAdjY;
		FontZW = pCanvas->TextWidth(p) + sys.m_FontAdjX;
	}

	pPaintBox->Font->Size = sys.m_FontSize;
	pCanvas->Font->Size = sys.m_FontSize;

	m_FH = pCanvas->TextHeight("M") + sys.m_FontAdjY;
	m_FW = pCanvas->TextWidth("M") + sys.m_FontAdjX;
	if( sys.m_FontZero && (*(p+1)) ){
		FontZOX = (m_FW - FontZW)/2;
		FontZOY = (m_FH - FontZH)/2;
	}
	m_LineMax = pPaintBox->Height / m_FH;
	int OldColMax = m_ColMax;
	if( Remote ){
		m_ColMax = 80;
	}
	else {
		m_ColMax = pPaintBox->Width / m_FW;
	}
	m_DestRC.Left = 0; m_DestRC.Right = pPaintBox->Width - 1;
	m_SourceRC.Left = 0; m_SourceRC.Right = pPaintBox->Width - 1;
	m_FillRC.Left = 0; m_FillRC.Right = pPaintBox->Width - 1;

	m_DestRC.Top = 0; m_DestRC.Bottom = m_FH * (m_LineMax - 1);
	m_SourceRC.Top = m_FH; m_SourceRC.Bottom = m_FH * m_LineMax;
	m_FillRC.Top = m_FH * (m_LineMax - 1); m_FillRC.Bottom = m_FH * m_LineMax;
	if( pBitmap != NULL ){
		pCanvas->Brush->Bitmap = pBitmap;
	}
	else {
		pCanvas->Brush->Color = sys.m_ColorRXBack;
	}

	if( m_ColMax != OldColMax ){
		int i;
		int CopyLen = (m_ColMax > OldColMax) ? OldColMax : m_ColMax;
		for( i = 0; i < TEXTLINEMAX; i++ ){
			if( pList[i] != NULL ){
				LPSTR p = new char[m_ColMax + 1];
				StrCopy(p, pList[i], CopyLen);
				delete pList[i];
				pList[i] = p;

				p = new char[m_ColMax + 1];
				memcpy(p, pColList[i], CopyLen);
				delete pColList[i];
				pColList[i] = p;
			}
			else {
				pList[i] = new char[m_ColMax + 1];
				*pList[i] = char(i ? -1 : 0);
				pColList[i] = new char[m_ColMax + 1];
				*pColList[i] = 0;
			}
		}
	}
	if( m_Col >= m_ColMax ){
		m_Col = m_ColMax - 1;
	}
	if( m_Count >= m_LineMax ){
		m_Line = m_LineMax - 1;
		m_Top = AdjustPos(m_WriteLine - m_LineMax + 1);
		if( GetPos(m_Line+1) == m_WriteLine ){
			m_DisEvent++;
			pScroll->Max = m_Count - m_LineMax + 1;
			pScroll->Position = pScroll->Max;
			pScroll->LargeChange = WORD(m_LineMax * 2 / 3);
			pScroll->SmallChange = 1;
			pScroll->Enabled = TRUE;
			m_DisEvent--;
		}
	}
	else {
		m_Line = m_WriteLine;
		m_Top = 0;
		pScroll->Enabled = FALSE;
	}
	pPaintBox->Invalidate();
}

void CPrintText::Scroll(void)
{
	if( !pPaintBox->Visible ) return;

#if 1
	if( ::GetForegroundWindow() != Application->MainForm->Handle ){
		pPaintBox->Invalidate();
//		Application->MainForm->Caption = "Invalidate";
	}
	else {
		pCanvas->CopyRect(m_DestRC, pCanvas, m_SourceRC);
		pCanvas->FillRect(m_FillRC);
//		Application->MainForm->Caption = "CopyRect";
	}
#else
	pCanvas->CopyRect(m_DestRC, pCanvas, m_SourceRC);
	pCanvas->FillRect(m_FillRC);
#endif
}

//------------------------------------------
// 0: / で分離しない
// 1: / で分離
void CPrintText::GetText(LPSTR t, int x, int y, int sw)
{
	char	*bp = new char[m_ColMax * 3 + 1];

	*t = 0;
	int col = (x / m_FW) + m_ColMax;
	int line = y / m_FH;

	int pos = GetPos(line);

	memset(bp, 0, m_ColMax * 3 + 1);

	memcpy(bp, pList[GetBefore(pos)], m_ColMax);
	if( (*bp == -1) || ((int)strlen(bp) < m_ColMax) ){
		memset(bp, 0, m_ColMax);
	}

	memcpy(bp+m_ColMax, pList[pos], m_ColMax);
	if( bp[m_ColMax] == -1 ) bp[m_ColMax] = 0;
	int len = strlen(&bp[m_ColMax]);
	memset(&bp[m_ColMax + len], 0, m_ColMax - len);

	memcpy(bp+m_ColMax+m_ColMax, pList[GetNext(pos)], m_ColMax);
	if( bp[m_ColMax+m_ColMax] == -1 ){
		bp[m_ColMax+m_ColMax] = 0;
	}

	LPSTR	p;
	for( p = &bp[col]; p >= bp; p-- ){
		if( !isalpha(*p) && !isdigit(*p) && ((*p != '/')||sw) && ((sw != 2) || (*p != ':')) ){
			break;
		}
	}
	p++;
	if( isalpha(*p) || isdigit(*p) ){
		strcpy(t, p);
		for( p = t; *p; p++ ){
			if( !isalpha(*p) && !isdigit(*p) && ((*p != '/')||sw) && ((sw != 2) || (*p != ':')) ){
				*p = 0;
				break;
			}
		}
	}
	delete bp;
}

//------------------------------------------
int CPrintText::GetText(LPSTR t, int y)
{
	if( y > m_Count ) return 0;

	y = AdjustPos(m_WriteLine - y);
	if( *pList[y] ){
		if( *pColList[y] ){
			return 0;
		}
		else {
			strcpy(t, pList[y]);
		}
	}
	else {
		*t = 0;
	}
	return 1;
}

void CPrintText::ShiftText(int x, int y)
{
	CRTTY	rtty;

	int col = x / m_FW;
	int line = y / m_FH;

	int pos = GetPos(line);

	LPSTR s = pList[pos];
	if( *s == -1 ) return;
	if( (int)strlen(s) <= col ) return;

	LPSTR p;
	for( p = &s[col-1]; p >= s; p-- ){
		if( *p != ' ' ){
			*p = rtty.InvShift(*p);
		}
		else {
			break;
		}
	}
	for( p = &s[col]; *p; p++ ){
		if( *p != ' ' ){
			*p = rtty.InvShift(*p);
		}
		else {
			break;
		}
	}
	pPaintBox->Invalidate();
}

void CPrintText::GetString(AnsiString &as)
{
	as = "";

	LPSTR	p;
	int l = m_WriteLine;
	if( m_Count < TEXTLINEMAX ){
		l = m_WriteLine - m_Count - 1;
	}
	l = AdjustPos(l);
	do {
		l = GetNext(l);
		p = pList[l];
		if( *p != -1 ){
			as += pList[l];
			as += "\r\n";
		}
	}while((l != m_WriteLine));
}

void CPrintText::PaintCanvas(void)
{
	if( pCanvas == NULL ) return;

	CursorOff();
	int i, j, x, y;
	LPSTR	p, pc;
//	pCanvas->Font->Color = sys.m_ColorRX;
	for( y = i = 0; i <= m_Line; i++, y += m_FH ){
		p = pList[GetPos(i)];
		pc = pColList[GetPos(i)];
		if( *p != -1 ){
			for( x = j = 0; j < m_ColMax; j++, x += m_FW, p++, pc++ ){
				if( *p ){
					pCanvas->Font->Color = *pc ? sys.m_ColorRXTX : sys.m_ColorRX;
					DrawChar(pCanvas, x, y, *p);
				}
				else {
					break;
				}
			}
		}
		else {
			break;
		}
	}
	CursorOn();
}

void CPrintText::IncCount(void)
{
	m_WriteLine = GetNext(m_WriteLine);
	*(pList[m_WriteLine]) = 0;
	*(pList[GetNext(m_WriteLine)]) = -1;
	if( m_Count < TEXTLINEMAX ){
		m_Count++;
		if( m_Count >= m_LineMax ){
			if( GetPos(m_Line+1) == m_WriteLine ){
				m_DisEvent++;
				pScroll->Max = m_Count - m_LineMax + 1;
				pScroll->Position = pScroll->Max;
				pScroll->LargeChange = WORD(m_LineMax * 2 / 3);
				pScroll->SmallChange = 1;
				pScroll->Enabled = TRUE;
				m_DisEvent--;
			}
		}
	}
	if( m_Count >= m_LineMax ){
		if( GetPos(m_Line+1) == m_WriteLine ){
			m_Top = GetNext(m_Top);
		}
	}
}

void CPrintText::ScrollBarChange(void)
{
	if( m_DisEvent ) return;

	int n = pScroll->Max - pScroll->Position;
	if( n > (TEXTLINEMAX - m_LineMax - 1) ) n = TEXTLINEMAX - m_LineMax - 1;
	m_Top = m_WriteLine - m_LineMax - n + 1;
	while( m_Top < 0 ) m_Top += TEXTLINEMAX;
	while( m_Top >= TEXTLINEMAX ) m_Top -= TEXTLINEMAX;
	if( pScroll->Max == pScroll->Position ){
		m_DisEvent++;
		int max = m_Count - m_LineMax + 1;
		pScroll->SetParams(max, 0, max);
		m_DisEvent--;
	}
	pPaintBox->Invalidate();
}

void CPrintText::ScrollBarUp(int page)
{
	if( pScroll->Enabled == TRUE ){
		if( pScroll->Position ){
			int n;
			if( page == 0 ){
				n = 1;
			}
			else if( page == 1 ){
				n = m_LineMax/2;
			}
			else {
				n = pScroll->Position;
			}
			if( pScroll->Position < n ) n = pScroll->Position;
			pScroll->Position = pScroll->Position - n;
		}
	}
}

void CPrintText::ScrollBarDown(int page)
{
	if( pScroll->Enabled == TRUE ){
		if( pScroll->Position != pScroll->Max ){
			int n;
			if( page == 0 ){
				n = 1;
			}
			else if( page == 1 ){
				n = m_LineMax/2;
			}
			else {
				n = pScroll->Max - pScroll->Position;
			}
			if( (pScroll->Position + n) > pScroll->Max ) n = pScroll->Max - pScroll->Position;
			pScroll->Position = pScroll->Position + n;
		}
	}
}

void CPrintText::TrigTimeStampImm(LPCSTR pName)
{
	if( m_fp == NULL ) return;

	m_TimeStampTrig = 0;
	m_TrigName = pName;
	if( sys.m_TimeStampUTC ){
		GetUTC(&m_TrigTime);
	}
	else {
		GetLocal(&m_TrigTime);
	}
	m_TimeStampTrig = 1;

	if( m_Col ){
		WriteFile("\r\n");
	}
	WriteTimeStamp();
}

void CPrintText::TrigTimeStamp(LPCSTR pName, int sw)
{
	if( !sw && (m_fp == NULL) ) return;

	m_TimeStampTrig = 0;
	m_TrigName = pName;
	if( sys.m_TimeStampUTC ){
		GetUTC(&m_TrigTime);
	}
	else {
		GetLocal(&m_TrigTime);
	}
	m_TimeStampTrig = 1;
	if( sw ){
		m_TimeStampTrig = 0;
		char bf[256];
		LPCSTR p;

		if( m_Col ){
			PutChar(0x0d);
			PutChar(0x0a);
		}
		sprintf(bf, "<%02u%02u%02u %02u:%02u:%02u %s>\r\n",
			m_TrigTime.wYear % 100, m_TrigTime.wMonth, m_TrigTime.wDay,
			m_TrigTime.wHour, m_TrigTime.wMinute, m_TrigTime.wSecond,
			m_TrigName.c_str()
		);
		for( p = bf; *p; p++ ){
			PutChar(*p);
		}
	}
	else if( !m_Col ){
		WriteTimeStamp();
	}
}

void CPrintText::WriteTimeStamp(void)
{
	if( m_fp == NULL ) return;

	char bf[256];

	if( m_TimeStampTrig ){
		m_TimeStampTrig = 0;
		sprintf(bf, "<%02u%02u%02u %02u:%02u:%02u %s>\r\n",
			m_TrigTime.wYear % 100, m_TrigTime.wMonth, m_TrigTime.wDay,
			m_TrigTime.wHour, m_TrigTime.wMinute, m_TrigTime.wSecond,
			m_TrigName.c_str()
		);
		WriteFile(bf);
	}
//typedef struct _SYSTEMTIME {  /* st */
//    WORD wYear;
//    WORD wMonth;
//    WORD wDayOfWeek;
//    WORD wDay;
//    WORD wHour;
//    WORD wMinute;
//    WORD wSecond;
//    WORD wMilliseconds;
//} SYSTEMTIME;
}

void CPrintText::WriteFile(LPCSTR p)
{
	if( m_fp == NULL ) return;

	for( ; *p; p++ ){
		WriteFile(*p);
	}
}

void CPrintText::WriteFile(char c)
{
	if( m_fp != NULL ){
		fputc(c & 0x007f, m_fp);
	}
}

void CPrintText::PutChar(char c)
{
	int show = (GetPos(m_Line) == m_WriteLine) ? 1 : 0;

	CursorOff();
	if( c == 0x0a ){
		if( m_Scroll != 'o' ){
			WriteFile("\r\n");
			WriteTimeStamp();
			IncCount();
			m_Col = 0;
			if( show ){
				m_Line++;
				if( m_Line >= m_LineMax ){
					m_Line--;
					Scroll();
				}
				m_Scroll = c;
			}
		}
		else {
			m_Scroll = 0;
		}
	}
	else if( c == 0x0d ){
		if( m_Scroll != 'o' ){
			m_Col = 0;
			m_Scroll = c;
		}
	}
	else if( c ){
		if( m_Scroll ){
			if( m_Scroll == 0x0d ){
				PutChar(0x0a);
				CursorOff();
				m_Scroll = 0;
			}
			m_Col = 0;
			m_Scroll = 0;
		}
		LPSTR pt = pList[m_WriteLine];
		WriteFile(c);
		pt[m_Col] = c;
		pt[m_Col + 1] = 0;
		LPSTR pc = pColList[m_WriteLine];
		pc[m_Col] = BYTE(m_TX);
		if( show ){
			pCanvas->Font->Color = m_TX ? sys.m_ColorRXTX : sys.m_ColorRX;
			DrawChar(pCanvas, m_Col * m_FW, m_Line * m_FH, c);
		}
		m_Col++;
		if( m_Col >= m_ColMax ){
			m_Col = 0;
			IncCount();
			if( show ){
				m_Line++;
				if( m_Line >= m_LineMax ){
					WriteFile("\r\n");
					WriteTimeStamp();
					m_Line--;
					Scroll();
				}
				m_Scroll = 'o';
			}
		}
	}
	CursorOn();
}

void CPrintText::MakeLogName(void)
{
	char bf[256];

//typedef struct _SYSTEMTIME {  /* st */
//    WORD wYear;
//    WORD wMonth;
//    WORD wDayOfWeek;
//    WORD wDay;
//    WORD wHour;
//    WORD wMinute;
//    WORD wSecond;
//    WORD wMilliseconds;
//} SYSTEMTIME;

	SYSTEMTIME	st;

	GetLocal(&st);
	sprintf(bf, "%s%02u%02u%02u.txt", LogDir, st.wYear % 100, st.wMonth, st.wDay);
	sys.m_LogName = bf;
}

void CPrintText::OpenLogFile(void)
{
	CloseLogFile();
	if( sys.m_LogName.IsEmpty() ) return;
	if( !sys.m_log ) return;

	m_fp = fopen(sys.m_LogName.c_str(), "ab");
	if( m_fp == NULL ){
		ErrorMB( "%s がオープンできません.", sys.m_LogName.c_str() );
		return;
	}
	fprintf(m_fp, "\r\n");
}

void CPrintText::CloseLogFile(void)
{
	if( m_fp != NULL ){
		if( fclose(m_fp) ){
			m_fp = NULL;
			ErrorMB( "%s が正しく作成できませんでした.", sys.m_LogName.c_str() );
		}
		m_fp = NULL;
	}
}

//---------------------------------------------------------------------------
// キー入力画面表示処理クラス
CFifoEdit::CFifoEdit()
{
	pPaintBox = NULL;
	pCanvas = NULL;
	pScroll = NULL;
	pBitmap = NULL;

	m_DispTop = 0;
	m_Line = 0;
	m_WriteLine = 0;
	m_WriteCol = 0;
	m_ReadLine = 0;
	m_ReadCol = 0;
	m_DisEvent = 0;
	m_Cursor = 1;
	memset(pList, 0, sizeof(pList));
}

CFifoEdit::~CFifoEdit()
{
	int i;
	for( i = 0; i < FIFOLINEMAX; i++ ){
		if( pList[i] != NULL ) delete[] pList[i];
		pList[i] = NULL;
	}
}

void CFifoEdit::CursorOff(void)
{
	if( m_Cursor ){
		DrawCursor(0);
	}
}

void CFifoEdit::CursorOn()
{
	if( m_Cursor ){
		DrawCursor(1);
	}
}

void CFifoEdit::DrawCursor(int sw)
{
	if( m_WriteLine > (m_DispTop + m_Line) ) return;

	TRect rc;

	rc.Left = m_WriteCol * m_FW + 1;
	rc.Right = rc.Left + m_FW - 1;
	rc.Bottom = (m_Line * m_FH) + m_FH - 1;
	rc.Top = rc.Bottom - 3;

	if( sw ){
		pCanvas->Brush->Color = sys.m_ColorIN;
		pCanvas->FillRect(rc);
	}
	else {
		if( pBitmap != NULL ){
			pCanvas->Brush->Bitmap = pBitmap;
		}
		else {
			pCanvas->Brush->Color = sys.m_ColorINBack;
		}
		pCanvas->FillRect(rc);
		pCanvas->Font->Color = sys.m_ColorIN;
		if( m_WriteLine >= FIFOLINEMAX ) return;
		if( (pList[m_WriteLine] != NULL) && (*pList[m_WriteLine]!=-1) ){
			LPSTR p = pList[m_WriteLine];
			if( int(strlen(p)) > m_WriteCol ){
				DrawChar(pCanvas, m_WriteCol * m_FW, m_Line * m_FH, p[m_WriteCol]);
			}
		}
	}
}

void CFifoEdit::SetCursor(int sw)
{
	if( m_Cursor != sw ){
		DrawCursor(sw);
		m_Cursor = sw;
	}
}

void CFifoEdit::SetPaintBox(TPaintBox *tp, TScrollBar *sp)
{
	pPaintBox = tp;
	pScroll = sp;

	pScroll->Align = alRight;
	pScroll->EnableAlign();
	pCanvas = pPaintBox->Canvas;

	if( !sys.m_FontName.IsEmpty() ){
		pPaintBox->Font->Name = sys.m_FontName;
		pCanvas->Font->Name = sys.m_FontName;
	}

	TFontStyles fs = Code2FontStyle(sys.m_FontStyle);
	pPaintBox->Font->Style = fs;
	pCanvas->Font->Style = fs;

	pPaintBox->Font->Charset = BYTE(sys.m_FontCharset);
	pCanvas->Font->Charset = BYTE(sys.m_FontCharset);
	pPaintBox->Font->Pitch = fpFixed;
	pCanvas->Font->Pitch = fpFixed;
	pPaintBox->Font->Size = sys.m_FontSize;
	pCanvas->Font->Size = sys.m_FontSize;

	m_FH = pCanvas->TextHeight("M") + sys.m_FontAdjY;
	m_FW = pCanvas->TextWidth("M") + sys.m_FontAdjX;

	m_LineMax = pPaintBox->Height / m_FH;
	int OldColMax = m_ColMax;
	if( Remote ){
		m_ColMax = 80;
	}
	else {
		m_ColMax = pPaintBox->Width / m_FW;
	}
	m_DestRC.Left = 0; m_DestRC.Right = pPaintBox->Width - 1;
	m_SourceRC.Left = 0; m_SourceRC.Right = pPaintBox->Width - 1;
	m_FillRC.Left = 0; m_FillRC.Right = pPaintBox->Width - 1;

	m_DestRC.Top = 0; m_DestRC.Bottom = m_FH * (m_LineMax - 1);
	m_SourceRC.Top = m_FH; m_SourceRC.Bottom = m_FH * m_LineMax;
	m_FillRC.Top = m_FH * (m_LineMax - 1); m_FillRC.Bottom = m_FH * m_LineMax;
	if( pBitmap != NULL ){
		pCanvas->Brush->Bitmap = pBitmap;
	}
	else {
		pCanvas->Brush->Color = sys.m_ColorINBack;
	}

	if( OldColMax != m_ColMax ){
		int i;
		int CopyLen = (m_ColMax > OldColMax) ? OldColMax : m_ColMax;
		for( i = 0; i < FIFOLINEMAX; i++ ){
			if( pList[i] != NULL ){
				LPSTR p = new char[m_ColMax + 1];
				StrCopy(p, pList[i], CopyLen);
				delete pList[i];
				pList[i] = p;
			}
			else {
				pList[i] = new char[m_ColMax + 1];
				*pList[i] = 0;
			}
		}
	}
	if( m_WriteCol >= m_ColMax ){
		m_WriteCol = m_ColMax - 1;
	}
	if( m_WriteLine >= m_LineMax ){
		m_Line = m_LineMax - 1;
		m_DispTop = m_WriteLine - m_LineMax + 1;
	}
	else {
		m_Line = m_WriteLine;
		m_DispTop = 0;
	}
	UpdateScroll();
	pPaintBox->Invalidate();
}

void CFifoEdit::Scroll(void)
{
	pCanvas->CopyRect(m_DestRC, pCanvas, m_SourceRC);
	pCanvas->FillRect(m_FillRC);
}

void CFifoEdit::PaintCanvas(void)
{
	if( pCanvas == NULL ) return;

	CursorOff();
	int i, j, x, y, line;
	LPSTR	p;
	line = m_DispTop;
	for( i = y = 0; i <= m_Line; i++, line++, y += m_FH ){
		if( line >= FIFOLINEMAX ) break;
		p = pList[line];
		for( x = j = 0; j < m_ColMax; j++, x += m_FW, p++ ){
			if( *p ){
				::SetBkMode(pCanvas->Handle, TRANSPARENT);
				if( (line < m_ReadLine) ||
					((line == m_ReadLine) && (j < m_ReadCol) )
				){
					pCanvas->Font->Color = sys.m_ColorINTX;
				}
				else {
					pCanvas->Font->Color = sys.m_ColorIN;
				}
				DrawChar(pCanvas, x, y, *p);
			}
			else {
				break;
			}
		}
	}
	CursorOn();
}

void CFifoEdit::ScrollBarChange(void)
{
	if( m_DisEvent ) return;

	int n = pScroll->Max - pScroll->Position;
	m_DispTop = m_WriteLine - m_LineMax - n + 1;
	if( m_DispTop < 0 ) m_DispTop = 0;
	if( m_DispTop > (m_WriteLine - m_LineMax) ) m_DispTop = m_WriteLine - m_LineMax + 1;
	if( pScroll->Max == pScroll->Position ){		// 最終行の時
		m_DisEvent++;
		int max = m_WriteLine - m_LineMax + 1;
		pScroll->SetParams(max, 0, max);
		m_DisEvent--;
	}
	pPaintBox->Invalidate();
}

void CFifoEdit::ScrollBarUp(int page)
{
	if( pScroll->Enabled == TRUE ){
		if( pScroll->Position ){
			int n;
			if( page == 0 ){
				n = 1;
			}
			else if( page == 1 ){
				n = m_LineMax/2;
			}
			else {
				n = pScroll->Position;
			}
			if( pScroll->Position < n ) n = pScroll->Position;
			pScroll->Position = pScroll->Position - n;
		}
	}
}

void CFifoEdit::ScrollBarDown(int page)
{
	if( pScroll->Enabled == TRUE ){
		if( pScroll->Position != pScroll->Max ){
			int n;
			if( page == 0 ){
				n = 1;
			}
			else if( page == 1 ){
				n = m_LineMax/2;
			}
			else {
				n = pScroll->Max - pScroll->Position;
			}
			if( (pScroll->Position + n) > pScroll->Max ) n = pScroll->Max - pScroll->Position;
			pScroll->Position = pScroll->Position + n;
		}
	}
}

//---------------------------------------------------------------
// スクロールバーの更新
void CFifoEdit::UpdateScroll(void)
{
	m_DisEvent++;
	if( m_WriteLine >= m_LineMax ){
		int max = m_WriteLine - m_LineMax + 1;
		pScroll->SetParams(max, 0, max);
		pScroll->LargeChange = WORD(m_LineMax * 2 / 3);
		pScroll->SmallChange = 1;
		pScroll->Enabled = TRUE;
	}
	else if( pScroll->Enabled ){
		pScroll->Enabled = FALSE;
	}
	m_DisEvent--;
}
//---------------------------------------------------------------
// すべてクリアする
void CFifoEdit::Clear(void)
{
	m_Line = 0;			// 画面内の行位置

	m_DispTop = 0;								// 表示トップ行の位置

	m_WriteLine = 0;	// 書きこみ行の位置
	m_WriteCol = 0;		// 書きこみカラム位置

	m_ReadLine = 0;		// 取りだし行の位置
	m_ReadCol = 0;		// 取りだしカラム位置

	int i;
	for( i = 0; i < FIFOLINEMAX; i++ ){
		memset(pList[i], 0, m_ColMax);
	}
	if( pScroll != NULL ) pScroll->Enabled = FALSE;
	if( pPaintBox != NULL ) pPaintBox->Invalidate();
}

//---------------------------------------------------------------
// 先頭からの文字列を非破壊で取り出す
void CFifoEdit::GetString(AnsiString &as)
{
	as = "";
	for( int i = 0; i <= m_WriteLine; i++ ){
		if( i ) as += "\r\n";
		if( i >= FIFOLINEMAX ) break;
		if( *pList[i] ) as += pList[i];
	}
}
//---------------------------------------------------------------
// 取りだし可能な文字数を得る
int CFifoEdit::GetLen(void)
{
	int mr = m_ReadLine;
	int n = 0;
	while(mr < m_WriteLine){
		n += strlen(pList[mr]);
		mr++;
	}
	n += (m_WriteCol - m_ReadCol);
	return n;
}
//---------------------------------------------------------------
// 最後の文字を得る
char CFifoEdit::GetLastChar(void)
{
	if( m_ReadLine > m_WriteLine ) return 0;		// 送出文字なし
	if( m_ReadLine == m_WriteLine ){	// 同一行の時
		if( m_WriteCol <= m_ReadCol ) return 0; 	// 送出文字なし
	}
	if( !m_WriteCol ) return 0x0a;

	LPSTR p;
	p = pList[m_WriteLine];
	p += m_WriteCol - 1;
	return *p;
}
//---------------------------------------------------------------
// スペースが含まれているかどうか調べる
int CFifoEdit::IsSAS(int sw)
{
	if( m_ReadLine > m_WriteLine ) return 1;		// 送出文字なし
	if( m_ReadLine == m_WriteLine ){	// 同一行の時
		if( m_WriteCol <= m_ReadCol ) return 1; 	// 送出文字なし
	}
	if( m_ReadLine < m_WriteLine ) return 1;		// 複数行のとき


	LPSTR p;
	p = pList[m_ReadLine];
	p += m_ReadCol;
	if( sw ){
		for( ; *p; p++ ){
			if( *p == '\\' ) return 1;
		}
		return 0;
	}
	if( !*p ) return 1;
	for( ; *p; p++ ){
		if( *p == ' ' ) return 1;
		if( *p == '\\' ) return 1;
	}
	return 0;
}
//---------------------------------------------------------------
// 先頭の一文字を取り出す
char CFifoEdit::GetChar(void)
{
	LPSTR p;
	char c;

	if( m_ReadLine > m_WriteLine ) return -1;		// 送出文字なし
	if( m_ReadLine == m_WriteLine ){	// 同一行の時
		if( m_WriteCol <= m_ReadCol ) return -1; 	// 送出文字なし
	}

	p = pList[m_ReadLine];
	p += m_ReadCol;

	if( !*p ){		// 改行
		if( m_ReadCol ){
			c = *(p-1);
			if( ((c == '_')||(c == '~')) && (m_ReadCol == m_ColMax) ){
				m_ReadLine++;
				m_ReadCol = 0;
				return 0;
			}
		}
		c = 0x0d;
		m_ReadLine++;
		m_ReadCol = 0;
	}
	else {
		if( *p == '%' ){		// 特殊記号
			if( (m_ReadLine >= m_WriteLine) && (m_WriteCol <= (m_ReadCol + 1)) ){
				return 0;		// 送信保留
			}
		}
		c = *p;
		if( (m_ReadLine >= m_DispTop) && (m_ReadLine <= (m_DispTop + m_ColMax - 1)) ){
			pCanvas->Font->Color = sys.m_ColorINTX;
			::SetBkMode(pCanvas->Handle, TRANSPARENT);
			DrawChar(pCanvas, m_ReadCol * m_FW, (m_ReadLine - m_DispTop) * m_FH, c);
		}
		m_ReadCol++;
	}
	return c;
}

//---------------------------------------------------------------
// 先頭の一文字の非破壊に取り出す
LPCSTR CFifoEdit::GetCurLine(void)
{
	return pList[m_WriteLine];
}

//---------------------------------------------------------------
// バックスペース処理
int CFifoEdit::BackSpace(void)
{
	if( m_ReadLine > m_WriteLine ) return FALSE;		// 送出文字なし
	if( m_ReadLine == m_WriteLine ){	// 同一行の時
		if( m_WriteCol <= m_ReadCol ) return FALSE; 	// 送出文字なし
	}
	if( m_WriteCol ){
		m_WriteCol--;
	}
	else {
		m_WriteLine--;
		if( m_DispTop ){
			m_DispTop--;
		}
		else {
			m_Line--;
		}
		m_WriteCol = strlen(pList[m_WriteLine]);
		if( m_WriteCol >= m_ColMax ) m_WriteCol--;
		UpdateScroll();
	}
	LPSTR p = pList[m_WriteLine];
	p += m_WriteCol;
	*p = 0;
	pPaintBox->Invalidate();
	return TRUE;
}

//---------------------------------------------------------------
// 1行バックスペース処理
int CFifoEdit::LineBackSpace(void)
{
	if( m_ReadLine > m_WriteLine ) return FALSE;		// 送出文字なし
	if( m_ReadLine == m_WriteLine ){	// 同一行の時
		if( m_WriteCol <= m_ReadCol ) return FALSE; 	// 送出文字なし
	}

	if( m_WriteCol ){
		while(m_WriteCol){
			if( BackSpace() == FALSE ) return FALSE;
		}
		return TRUE;
	}
	else {
		return BackSpace();
	}
}

//---------------------------------------------------------------
// 先頭の１行を抹消する
int CFifoEdit::DeleteTop(void)
{
	if( m_ReadLine ){		// 送出済み
		int i;
		for( i = 0; i < (FIFOLINEMAX - 1); i++ ){
			strcpy(pList[i], pList[i+1]);
		}
		*pList[i] = 0;
		m_ReadLine--;
		m_WriteLine--;
		if( m_WriteLine <= (m_DispTop + m_Line) ){
			m_DispTop--;
		}
		else {
			pPaintBox->Invalidate();
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void CFifoEdit::PutChar(char c)
{
	int show = (m_WriteLine > (m_DispTop + m_Line)) ? 0 : 1;

	if( show && (c == 0x08) ){	// BSの時
		if( BackSpace() == TRUE ) return;
		c = 'X';
	}
	if( m_WriteLine >= FIFOLINEMAX ){		// バッファフルの時
		if( DeleteTop() == FALSE ) return;
	}
	CursorOff();
	if( c != 0x0a ){		// LFは処理しない
		if( c == 0x0d ){
			m_WriteLine++;
			m_WriteCol = 0;
			if( show ){				// 表示中の時
				m_Line++;
				if( m_Line >= m_LineMax ){
					m_Line--;
					Scroll();
					m_DispTop++;
					UpdateScroll();
				}
			}
		}
		else {
			LPSTR pt = pList[m_WriteLine];
			pt[m_WriteCol] = c;
			pt[m_WriteCol + 1] = 0;
			if( show ){
				pCanvas->Font->Color = sys.m_ColorIN;
				::SetBkMode(pCanvas->Handle, TRANSPARENT);
				DrawChar(pCanvas, m_WriteCol * m_FW, m_Line * m_FH, c);
			}
			m_WriteCol++;
			if( m_WriteCol >= m_ColMax ){
				m_WriteCol = 0;
				m_WriteLine++;
				if( show ){				// 表示中の時
					m_Line++;
					if( m_Line >= m_LineMax ){
						m_Line--;
						Scroll();
						m_DispTop++;
						UpdateScroll();
					}
				}
			}
		}
	}
	CursorOn();
}

void CFifoEdit::PutText(LPCSTR p)
{
	for( ; *p; p++ ){
		if( *p != 0x0a ){
			PutChar(*p);
		}
	}
}

///----------------------------------------------------------------
///  CAlignクラス
void CAlign::InitControl(TControl *p, TControl *pB, TFont *pF /*= NULL*/)
{
	tp = p;
	OTop = p->Top;
	OLeft = p->Left;
	OWidth = p->Width;
	OHeight = p->Height;
	if( pF != NULL ){
		fp = pF;
		OFontHeight = pF->Height;
	}

	BTop = pB->Top;
	BLeft = pB->Left;
	BWidth = pB->ClientWidth;
	BHeight = pB->ClientHeight;
}

void CAlign::InitControl(TControl *p, RECT *rp, TFont *pF /*= NULL*/)
{
	tp = p;
	OTop = p->Top;
	OLeft = p->Left;
	OWidth = p->Width;
	OHeight = p->Height;
	if( pF != NULL ){
		fp = pF;
		OFontHeight = pF->Height;
	}

	BTop = rp->top;
	BLeft = rp->left;
	BWidth = rp->right - rp->left + 1;
	BHeight = rp->bottom - rp->top + 1;
}

void CAlign::NewAlign(TControl *pB)
{
	double Sx = double(pB->ClientWidth)/double(BWidth);
	double Sy = double(pB->ClientHeight)/double(BHeight);
	tp->SetBounds(int(OLeft * Sx), int(OTop * Sy), int(OWidth * Sx), int(OHeight * Sy));
	if( fp != NULL ){
		if( Sx > Sy ) Sx = Sy;
		fp->Height = int(OFontHeight * Sx) - sys.m_FontAdjSize;
		m_FontAdj = Sx;
		if( tp->ClassNameIs("TComboBox") ){
			((TComboBox *)tp)->SelLength = 0;
		}
	}
}

void CAlign::NewAlign(TControl *pB, double hx)
{
	double Sx = double(pB->ClientWidth)/double(BWidth);
	double Sy = double(pB->ClientHeight)/double(BHeight);
	Sy *= hx;
	tp->SetBounds(int(OLeft * Sx), int(OTop * Sy), int(OWidth * Sx), int(OHeight * Sy));
	if( fp != NULL ){
		if( Sx > Sy ) Sx = Sy;
		fp->Height = int(OFontHeight * Sx) - sys.m_FontAdjSize;
		m_FontAdj = Sx;
		if( tp->ClassNameIs("TComboBox") ){
			((TComboBox *)tp)->SelLength = 0;
		}
	}
}

void CAlign::NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs)
{
	if( fp != NULL ){
		fp->Name = FontName;
		fp->Charset = Charset;
		fp->Style = fs;
		if( tp->ClassNameIs("TComboBox") ){
			((TComboBox *)tp)->SelLength = 0;
		}
	}
}
///----------------------------------------------------------------
///  CAlignListクラス
CAlignList::CAlignList(void)
{
	Max = 0;
	Cnt = 0;
	AlignList = NULL;
}

CAlignList::~CAlignList(){
	if( AlignList != NULL ){
		for( int i = 0; i < Cnt; i++ ){
			delete AlignList[i];
		}
		delete[] AlignList;
		AlignList = NULL;
	}
}

void CAlignList::Alloc(void)
{
	if( Cnt >= Max ){
		Max = Max ? Max * 2 : 16;
		CAlign **np = (CAlign **)new BYTE[sizeof(CAlign*) * Max];
		if( AlignList != NULL ){
			for( int i = 0; i < Cnt; i++ ){
				np[i] = AlignList[i];
			}
			delete[] AlignList;
		}
		AlignList = np;
	}
	AlignList[Cnt] = new CAlign;
}
void CAlignList::EntryControl(TControl *tp, TControl *pB, TFont *pF /*= NULL*/)
{
	Alloc();
	AlignList[Cnt]->InitControl(tp, pB, pF);
	Cnt++;
}

void CAlignList::EntryControl(TControl *tp, RECT *rp, TFont *pF /*= NULL*/){
	Alloc();
	AlignList[Cnt]->InitControl(tp, rp, pF);
	Cnt++;
}

void CAlignList::NewAlign(TControl *pB)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		AlignList[i]->NewAlign(pB);
	}
}

double CAlignList::GetFontAdj(TControl *pB)
{
	for( int i = 0; i < Cnt; i++ ){
		if( AlignList[i]->GetControl() == pB ){
			return AlignList[i]->GetFontAdj();
		}
	}
	return 1.0;
}

void CAlignList::NewAlign(TControl *pB, TControl *pS, double hx)
{
	for( int i = 0; i < Cnt; i++ ){
		if( AlignList[i]->GetControl() == pS ){
			AlignList[i]->NewAlign(pB, hx);
			break;
		}
	}
}
void CAlignList::NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs)
{
	if( AlignList == NULL ) return;
	for( int i = 0; i < Cnt; i++ ){
		AlignList[i]->NewFont(FontName, Charset, fs);
	}
}

///----------------------------------------------------------------
///  CAlignGridクラス
void CAlignGrid::InitGrid(TStringGrid *p)
{
	BWidth = p->Width;
	BHeight = p->Height;
	BRowHeight = p->DefaultRowHeight;
	Max = p->ColCount;
	bp = new int[Max];
	for( int i = 0; i < Max; i++ ){
		bp[i] = p->ColWidths[i];
	}
}

void CAlignGrid::NewAlign(TStringGrid *p)
{
	double Sx = double(p->Width)/double(BWidth);
	double Sy = double(p->Height)/double(BHeight);
	p->DefaultRowHeight = int(BRowHeight * Sy);
	for( int i = 0; (i < Max) && (i < p->ColCount); i++ ){
		p->ColWidths[i] = int(bp[i] * Sx);
	}
}


///------------------------------------------------------
///
///CWebRef クラス
///
void MakeCommand(LPSTR t, LPCSTR s, LPCSTR p)
{
	for( ;*s; s++ ){
		if( *s == '%' ){
			s++;
			if( *s == '%' ){
				*t++ = '%';
			}
			else if( (p != NULL) && (*s == '1') ){
				strcpy(t, p);
				t += strlen(t);
				p = NULL;
			}
		}
		else {
			*t++ = *s;
		}
	}
	*t = 0;
	if( p != NULL ){
		*t++ = ' ';
		strcpy(t, p);
	}
}

CWebRef::CWebRef()
{
	HTML = "";

	HKEY hkey=NULL;	/* オープン キーのハンドル	*/

	char    bf[512], name[512];
	ULONG   namelen, len;
	if( !RegOpenKeyEx(HKEY_CLASSES_ROOT, "http", 0, KEY_READ, &hkey) ){
		if( !RegOpenKeyEx(hkey, "shell", 0, KEY_READ, &hkey) ){
			if( !RegOpenKeyEx(hkey, "open", 0, KEY_READ, &hkey) ){
				if( !RegOpenKeyEx(hkey, "command", 0, KEY_READ, &hkey) ){
					namelen = sizeof(name);
					len = sizeof(bf);
					if( !::RegEnumValue(hkey, 0, name, &namelen, NULL,
					NULL, (LPBYTE)bf, &len) ){
						HTML = bf;
					}
				}
			}
		}
	}
	::RegCloseKey(hkey);
}

void CWebRef::ShowHTML(LPCSTR url)
{

	char    cmd[1024];

	MakeCommand(cmd, HTML.c_str(), url);
	::WinExec(cmd, SW_SHOWDEFAULT);
}


///------------------------------------------------------
///
///CWaitCursor クラス
///
CWaitCursor::CWaitCursor()
{
	sv = Screen->Cursor;
	Screen->Cursor = crHourGlass;
}

CWaitCursor::~CWaitCursor()
{
	Screen->Cursor = sv;
}

void CWaitCursor::Delete(void)
{
	Screen->Cursor = sv;
}

void CWaitCursor::Wait(void)
{
	Screen->Cursor = crHourGlass;
}

///-------------------------------------------------------
/// CRecentMenuクラス

CRecentMenu::CRecentMenu()
{
	InsPos = 0;
	Max = RECMENUMAX;
	int i;
	for( i = 0; i < RECMENUMAX; i++ ){
		Caption[i] = "";
		Items[i] = NULL;
	}
	Items[i] = NULL;
}
void CRecentMenu::Init(int pos, TMenuItem *pmenu, int max)
{
	InsPos = pos;
	pMenu = pmenu;
	Max = max;
}
void CRecentMenu::Init(TMenuItem *pi, TMenuItem *pmenu, int max)
{
	Init(pmenu->IndexOf(pi), pmenu, max);
}
int CRecentMenu::GetCount(void)
{
	int i, n;
	for( i = n = 0; i < RECMENUMAX; i++, n++ ){
		if( Caption[i].IsEmpty() ) break;
	}
	return n;
}
LPCSTR CRecentMenu::GetItemText(int n)
{
	if( (n >= 0) && (n < RECMENUMAX) ){
		return Caption[n].c_str();
	}
	else {
		return NULL;
	}
}
void CRecentMenu::SetItemText(int n, LPCSTR p)
{
	if( (n >= 0) && (n < RECMENUMAX) ){
		Caption[n] = p;
	}
}
int CRecentMenu::IsAdd(LPCSTR pNew)
{
	return strcmp(Caption[0].c_str(), pNew);
}
void CRecentMenu::Add(LPCSTR pNew)
{
	if( !IsAdd(pNew) ) return;
	int i;
	for( i = 1; i < RECMENUMAX; i++ ){
		if( !strcmp(pNew, Caption[i].c_str()) ){
			for( ; i < (RECMENUMAX - 1); i++ ){
				Caption[i] = Caption[i+1];
			}
			Caption[i] = "";
			break;
		}
	}
	for( i = RECMENUMAX - 1; i > 0; i-- ){
		Caption[i] = Caption[i-1];
	}
	Caption[0] = pNew;
}
void CRecentMenu::Delete(LPCSTR pName)
{
	int i;
	for( i = 0; i < RECMENUMAX; i++ ){
		if( !strcmp(pName, Caption[i].c_str()) ){
			for( ; i < (RECMENUMAX - 1); i++ ){
				Caption[i] = Caption[i+1];
			}
			Caption[i] = "";
			break;
		}
	}
}
void CRecentMenu::ExtFilter(LPCSTR pExt)
{
	int i, j;

	for( i = 0; i < RECMENUMAX; i++ ){
		if( (!Caption[i].IsEmpty()) && strcmpi(GetEXT(Caption[i].c_str()), pExt) ){
			for( j = i; j < (RECMENUMAX - 1); j++ ){
				Caption[j] = Caption[j+1];
			}
			Caption[j] = "";
			i--;
		}
	}
}
void CRecentMenu::Clear(void)
{
	Delete();
	for( int i = 0; i < RECMENUMAX; i++ ){
		Caption[i] = "";
	}
}
void CRecentMenu::Insert(TObject *op, Classes::TNotifyEvent pEvent)
{
	int n = GetCount();
	int i;
	int pos = InsPos;
	if( n > Max ) n = Max;
	char bf[256];
	for( i = 0; i < n; i++, pos++ ){
		Items[i] = new TMenuItem((TComponent *)op);
		sprintf(bf, "&%d %s", (i + 1) % 10, Caption[i].c_str());
		Items[i]->Caption = bf;
		Items[i]->OnClick = pEvent;
		pMenu->Insert(pos, Items[i]);
	}
	if( n ){
		Items[i] = new TMenuItem((TComponent *)op);
		Items[i]->Caption = "-";
		Items[i]->OnClick = NULL;
		pMenu->Insert(pos, Items[n]);
	}
}
void CRecentMenu::Delete(void)
{
	for( int i = 0; i < RECMENUMAX+1; i++ ){
		if( Items[i] != NULL ){
			pMenu->Delete(InsPos);
			Items[i] = NULL;
		}
	}
}
int CRecentMenu::FindItem(TMenuItem *mp)
{
	for( int i = 0; i < RECMENUMAX; i++ ){
		if( Items[i] == mp ){
			return i;
		}
	}
	return -1;
}
LPCSTR CRecentMenu::FindItemText(TMenuItem *mp)
{
	return GetItemText(FindItem(mp));
}

void KeyEvent(const short *p)
{
	for( ; *p; p++ ){
		keybd_event(BYTE(*p), 0, *p&0x8000 ? KEYEVENTF_KEYUP : 0, 0);
	}
}

