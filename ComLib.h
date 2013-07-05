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



#ifndef ComLibH
#define ComLibH
/* JA7UDE 0427 */
#include <inifiles.hpp>
#include <Grids.hpp>
#include <Graphics.hpp>	//JA7UDE 0427
#include <Buttons.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>
#include <mbstring.h>
#include <SHELLAPI.H>

#define	VERID	"Ver1.68"       //AA6YQ 1.65D->1.66G,  JE3HHT 1.67
#define	VERBETA	"A"
#define	VERTTL2	"MMTTY "VERID VERBETA
#define	VERTTL  VERTTL2" (C) JE3HHT 2000-2010."

#define SETUPTITLE "Setup MMTTY "VERID VERBETA      //AA6YQ 1.66C
#define SETUPTITLEREMOTE "Setup "VERID VERBETA      //AA6YQ 1.66C

#define SHIFTL	10
#define	MARKL	300
#define	SPACEH	2700
#define	XYCOLLECT	512		// XY-Scope のデータ収集サイズ

#define	REMMENU		1
#define	REMCTRL		2
#define	REMSHOWOFF	4
#define	REMNOMOVE	8
#define	REMNOPTT	16
#define	REMNOCNV	32
#define	REMVIATX	64
#define	REMDISFOCUS	128
#define	REMDISSHARED	256
#define	REMDISSTAYONTOP	512

#define	FSBOLD		1
#define	FSITALIC	2

enum {
	txSound,
	txTXD,
	txTXDOnly,
};

extern	int     WinNT;
extern  int     WinVista;            //AA6YQ 1.66G

extern	int		FSKCount;
extern	int		FSKCount1;
extern	int		FSKCount2;
extern	int		FSKDeff;

extern	LCID	lcid;				// ロケール情報
extern	int		DisPaint;
extern	int		Remote;

extern	double	SampFreq;			// サンプリング周波数
extern	double	SampBase;
extern	double	DemSamp;
extern	int		SampType;
extern	int		DemOver;
extern	int		FFT_SIZE;
extern	int     SampSize;

extern	char	BgnDir[256];
extern	char	LogDir[256];
extern	char	OutFileDir[256];
extern	char	MMLogDir[256];
extern	char	ExtLogDir[256];
extern	char	RecDir[256];		// 録音ファイルのディレクトリ

extern	AnsiString	JanHelp;

extern	const char	MONN[];
extern	const char	MONU[];

#ifndef LPCUSTR
typedef const unsigned char *	LPCUSTR;
typedef unsigned char *	LPUSTR;
#endif

extern	DWORD ColorTable[128];

#define ABS(c)	(((c)<0)?(-(c)):(c))
#define AN(p)	(sizeof(p)/sizeof(*(p)))
#define	CR		0x0d
#define	LF		0x0a
#define	TAB		'\t'

typedef struct {
	WORD	Key;
	LPCSTR	pName;
}DEFKEYTBL;
extern const DEFKEYTBL KEYTBL[];

enum {
	kkRcvLog,
	kkFileOut,
	kkSaveRx,

	kkOpenLog,
	kkFlush,

	kkRecTime,
	kkRec,
	kkPlay,
	kkPlayPos,	// Ctrl+A
	kkPlayStop,
	kkScope,
	kkLogList,
	kkQSOData,
	kkOption,
	kkLogOption,
	kkPanel,
	kkExtCmd1,
	kkExtCmd2,
	kkExtCmd3,
	kkExtCmd4,
	kkExtReset,
	kkExtSusp,

	kkFIG,
	kkUOS,
	kkTX,		// F9
	kkTXOFF,	// F8
	kkQSO,
	kkCall,		// Ctrl+C
	kkName,		// Ctrl+N
	kkQTH,		// Ctrl+Q
	kkRST,		// Ctrl+R
	kkFind,		// Ctrl+F
	kkClear,	// F1

	kkInUp,
	kkInDown,
	kkInPUp,
	kkInPDown,
	kkInHome,
	kkInEnd,
	kkRxUp,
	kkRxDown,
	kkRxPUp,
	kkRxPDown,
	kkRxHome,
	kkRxEnd,

	kkCharWaitLeft,
	kkCharWaitRight,
	kkDiddleWaitLeft,
	kkDiddleWaitRight,

	kkInHeightUp,
	kkInHeightDown,

	kkMyRST,
	kkInitBox,

	kkTxLTR,
	kkTxFIG,

	kkDecShift,
	kkIncShift,
	kkToggleShift,
	kkClrRxWindow,
	kkFreq,
	kkRun,
	kkPaste,
	kkTNC,
	kkMac,
	kkEntTX,
	kkWordWrap,
	kkOnQSO,
	kkOffQSO,
	kkCAPTURE,

	kkPro1,
	kkPro2,
	kkPro3,
	kkPro4,
	kkPro5,
	kkPro6,
	kkPro7,
	kkPro8,
	kkProDef,
	kkProRet,

	kkCList,
	kkEOF,
};
#define	MSGLISTMAX	64
typedef struct {
	WORD		m_SysKey[kkEOF];

	int			m_SoundPriority;
	int			m_SoundDevice;
    int         m_SoundOutDevice;   //AA6YQ 1.66
	int			m_SoundStereo;
	int			m_SoundFifoRX;
	int			m_SoundFifoTX;
	AnsiString	m_SoundMMW;

	AnsiString	m_Call;
	AnsiString	m_User[16];
	AnsiString	m_UserName[16];
	WORD		m_UserKey[16];
	WORD		m_UserEditKey[16];
	TColor		m_UserCol[16];
	int			m_UserTimer[16];
	int			m_SBFontSize;
	int			m_SBINFontSize;

	AnsiString	m_FontName;
	int			m_FontAdjX;
	int			m_FontAdjY;
	int			m_FontSize;
	int			m_FontCharset;
	int			m_FontZero;
	int			m_FontStyle;

	int			m_FontAdjSize;
	int			m_BtnFontAdjSize;

	int			m_WindowState;

	AnsiString	m_MsgName[MSGLISTMAX];
	AnsiString	m_MsgList[MSGLISTMAX];
	WORD		m_MsgKey[MSGLISTMAX];

	AnsiString	m_InBtn[4];
	AnsiString	m_InBtnName[4];
	WORD		m_InBtnKey[4];
	TColor		m_InBtnCol[4];
	int			m_MacroImm;

	AnsiString	m_ExtCmd[4];

	AnsiString	m_TxRxName;
	int		m_TxRxInv;
	int		m_txuos;
	int		m_dblsft;

	int		m_lmsbpf;

	double	m_DefMarkFreq;
	double	m_DefShift;
	int		m_DefStopLen;	// 0-1Bit, 1-1.42

	int		m_echo;

	int		m_AFC;
	int		m_FixShift;
	int		m_AFCSQ;
	double	m_AFCTime;
	double	m_AFCSweep;
	int		m_AFCNet;

	int		m_TxNet;
	int		m_Rev;
	int		m_TxDisRev;

	double	m_LimitGain;

	int		m_log;
	int		m_logTimeStamp;
	int		m_ShowTimeStamp;
	int		m_TimeStampUTC;
	int		m_AutoTimeOffset;
	int		m_TimeOffset;
	int		m_TimeOffsetMin;
	int		m_LogLink;

	AnsiString	m_LogName;

	int		m_TxPort;		// 0-Sound, 1-DTR, 2-TXD
	int		m_TxdStop;
	int		m_TxdJob;
	int		m_TxFixShift;
	int		m_CodeSet;
	int		m_DisTX;

	int		m_Palette;
	DWORD	d_PaletteMask;

	int		m_XYInv;

	TColor	m_ColorRXBack;
	TColor	m_ColorRX;
	TColor	m_ColorRXTX;
	TColor	m_ColorINBack;
	TColor	m_ColorIN;
	TColor	m_ColorINTX;

	TColor	m_ColorLow;
	TColor	m_ColorHigh;

	TColor	m_ColorXY;

	AnsiString	m_WinFontName;
	BYTE		m_WinFontCharset;
	int			m_WinFontStyle;
	AnsiString	m_BtnFontName;
	BYTE		m_BtnFontCharset;
	int			m_BtnFontStyle;

	int			m_FFTGain;
	int			m_FFTResp;
	double		m_SampFreq;
	double		m_TxOffset;

	AnsiString	m_HTMLHelp;
	AnsiString	m_Help;
	AnsiString	m_HelpLog;
	AnsiString	m_HelpDigital;
	AnsiString	m_HelpFontName;
	BYTE		m_HelpFontCharset;
	int			m_HelpFontSize;
	int			m_HelpNotePad;

	int			m_CharLenFile;
	int			m_DisWindow;
	int			m_StayOnTop;
	int			m_MemWindow;

	int			m_LWait;
	AnsiString	m_LogGridWidths;
	AnsiString	m_QSOGridWidths;

    int			m_DefFix45;
}SYSSET;

extern	SYSSET	sys;

#define	TEXTLINEMAX	512
class CPrintText
{
private:
	TPaintBox	*pPaintBox;
	TScrollBar	*pScroll;
	TCanvas		*pCanvas;
	Graphics::TBitmap *pBitmap;

	LPSTR		pList[TEXTLINEMAX];
	LPSTR		pColList[TEXTLINEMAX];

	int			m_jisfont;

	int			m_Top;
	int			m_Line;
	int			m_WriteLine;
	int			m_Col;
	int			m_Count;

	int			m_FH;
	int			m_FW;
	int			m_LineMax;
	int			m_ColMax;

	int			m_Scroll;

	TRect		m_DestRC;
	TRect		m_SourceRC;
	TRect		m_FillRC;

	int			m_Cursor;
	int			m_DisEvent;

	int			m_TX;

	int			m_TimeStampTrig;
	SYSTEMTIME	m_TrigTime;
	AnsiString	m_TrigName;

	FILE		*m_fp;

	void Scroll(void);
	void DrawCursor(int sw);
	void CursorOff(void);
	void CursorOn();
	void WriteTimeStamp(void);

public:
	CPrintText();
	~CPrintText();

	inline void SetTX(int sw){
		m_TX = sw;
	};
	inline void SetBitmap(Graphics::TBitmap *p){
		pBitmap = p;
	};
	inline int GetPos(int n){
		n += m_Top;
		if( n >= TEXTLINEMAX ) n -= TEXTLINEMAX;
		return n;
	};
	inline int GetNext(int n){
		n++;
		if( n >= TEXTLINEMAX ) n = 0;
		return n;
	};
	inline int GetBefore(int n){
		n--;
		if( n < 0 ) n = TEXTLINEMAX - 1;
		return n;
	};
	inline int AdjustPos(int n){
		while( n < 0 ) n+= TEXTLINEMAX;
		while( n >= TEXTLINEMAX ) n -= TEXTLINEMAX;
		return n;
	};
	void Clear();

	void IncCount(void);
	void SetPaintBox(TPaintBox *tp, TScrollBar *sp);
	void AdjZero(void);

	void PutChar(char c);
	void GetString(AnsiString &as);
	void PaintCanvas(void);
	void SetCursor(sw);
	void GetText(LPSTR t, int x, int y, int sw);
	int GetText(LPSTR t, int y);
	void ShiftText(int x, int y);

	void ScrollBarChange(void);
	void ScrollBarUp(int page);
	void ScrollBarDown(int page);
	void WriteFile(char c);
	void WriteFile(LPCSTR p);
	void MakeLogName(void);
	void OpenLogFile(void);
	void CloseLogFile(void);
	void TrigTimeStamp(LPCSTR pName, int sw = 0);
	void TrigTimeStampImm(LPCSTR pName);
};

#define	FIFOLINEMAX	128
class CFifoEdit
{
	TPaintBox	*pPaintBox;
	TScrollBar	*pScroll;
	TCanvas		*pCanvas;
	Graphics::TBitmap *pBitmap;

	LPSTR		pList[FIFOLINEMAX];

	int			m_Line;			// 画面内の行位置

	int			m_DispTop;		// 表示トップ行の位置
	int			m_DispLast;		// 表示最終行の位置

	int			m_WriteLine;	// 書きこみ行の位置

	int			m_ReadLine;		// 取りだし行の位置
	int			m_ReadCol;		// 取りだしカラム位置

	int			m_FH;
	int			m_FW;
	int			m_LineMax;

	TRect		m_DestRC;
	TRect		m_SourceRC;
	TRect		m_FillRC;

	int			m_Cursor;
	int			m_DisEvent;
public:
	int			m_ColMax;
	int			m_WriteCol;		// 書きこみカラム位置
private:
	void Scroll(void);
	void DrawCursor(int sw);
	void CursorOff(void);
	void CursorOn();
	int DeleteTop(void);
	void UpdateScroll(void);
	int BackSpace(void);

public:
	CFifoEdit();
	~CFifoEdit();

	inline void SetBitmap(Graphics::TBitmap *p){
		pBitmap = p;
	};
	void Clear(void);
	void GetString(AnsiString &as);

	int GetLen(void);
	char GetChar(void);
	LPCSTR GetCurLine(void);
	char GetLastChar(void);


	void SetPaintBox(TPaintBox *tp, TScrollBar *sp);
	void PutChar(char c);
	void PutText(LPCSTR p);
	void PaintCanvas(void);
	void SetCursor(sw);
	void ScrollBarChange(void);
	void ScrollBarUp(int page);
	void ScrollBarDown(int page);
	int LineBackSpace(void);
	int IsSAS(int sw);
};

///---------------------------------------------------------
///  コントロールのアラインの管理クラス
class CAlign
{
public:
	int		BTop, BLeft;
	int		BWidth, BHeight;
	int		OTop, OLeft;
	int		OWidth, OHeight;
	int		OFontHeight;
	double	m_FontAdj;

	TControl	*tp;
	TFont		*fp;
public:
	inline CAlign(void){
		tp = NULL;
		fp = NULL;
		m_FontAdj = 1.0;
	};
	inline ~CAlign(){
	};
	void InitControl(TControl *p, TControl *pB, TFont *pF = NULL);
	void InitControl(TControl *p, RECT *rp, TFont *pF = NULL);
	void NewAlign(TControl *pB);
	inline double GetFontAdj(void){return fabs(m_FontAdj);};
	inline TControl *GetControl(void){return tp;};
	void NewAlign(TControl *pB, double hx);
	void NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs);
};

///---------------------------------------------------------
///  コントロールのアラインの管理クラス
class CAlignList
{
private:
	int		Max;
	int		Cnt;
	CAlign	**AlignList;
	void Alloc(void);
public:
	CAlignList(void);
	~CAlignList();
	void EntryControl(TControl *tp, TControl *pB, TFont *pF = NULL);
	void EntryControl(TControl *tp, RECT *rp, TFont *pF = NULL);
	void NewAlign(TControl *pB);
	double GetFontAdj(TControl *pB);
	void NewAlign(TControl *pB, TControl *pS, double hx);
	void NewFont(AnsiString &FontName, BYTE Charset, TFontStyles fs);
};

class CAlignGrid
{
private:
	int		BWidth;
	int		BHeight;
	int		BRowHeight;

	int		Max;
	int		*bp;
public:
	inline CAlignGrid(void){
		Max = 0;
		bp = NULL;
	};
	inline ~CAlignGrid(){
		if( bp != NULL ){
			delete bp;
		}
	};
	void InitGrid(TStringGrid *p);
	void NewAlign(TStringGrid *p);
};

class CWebRef
{
private:
	AnsiString	HTML;
public:
	CWebRef();
	inline bool IsHTML(void){
		return !HTML.IsEmpty();
	};
	void ShowHTML(LPCSTR url);
};

class CWaitCursor
{
private:
	TCursor sv;
public:
	CWaitCursor();
	~CWaitCursor();
	void Delete(void);
	void Wait(void);
};

///-------------------------------------------------------
/// CRecentMenuクラス
#define	RECMENUMAX	4	// 処理できる最大の数
class CRecentMenu
{
private:
public:
	int		InsPos;		// メニュー挿入位置の番号
	int		Max;		// 処理する数
	AnsiString	Caption[RECMENUMAX];
	TMenuItem	*pMenu;
	TMenuItem	*Items[RECMENUMAX+1];

public:
	CRecentMenu();
	void Init(int pos, TMenuItem *pmenu, int max);
	void Init(TMenuItem *pi, TMenuItem *pmenu, int max);
	int GetCount(void);
	LPCSTR GetItemText(int n);
	void SetItemText(int n, LPCSTR p);
	int IsAdd(LPCSTR pNew);
	void Add(LPCSTR pNew);
	void ExtFilter(LPCSTR pExt);
	void Clear(void);
	void Insert(TObject *op, Classes::TNotifyEvent pEvent);
	void Delete(void);
	void Delete(LPCSTR pName);
	int FindItem(TMenuItem *mp);
	LPCSTR FindItemText(TMenuItem *mp);
};

void __fastcall ShowHtmlHelp(LPCSTR pContext = NULL);
//void ShowHelp(int index);
void InitSampType(void);
LPCSTR __fastcall ConvAndChar(LPSTR t, LPCSTR p);

void GetUTC(SYSTEMTIME *tp);
void GetLocal(SYSTEMTIME *tp);
LPSTR StrDupe(LPCSTR s);

void InitColorTable(TColor cl, TColor ch);
LPCSTR GetKeyName(WORD Key);
WORD GetKeyCode(LPCSTR pName);
LPCSTR ToDXKey(LPCSTR s);
LPCSTR ToJAKey(LPCSTR s);

int SetTBValue(double d, double dmax, int imax);
double GetTBValue(int d, double dmax, int imax);
LPUSTR jstrupr(LPUSTR s);
inline LPSTR jstrupr(LPSTR s){return (LPSTR)jstrupr(LPUSTR(s));};

int IsFile(LPCSTR pName);
int SetTimeOffsetInfo(int &Hour, int &Min);
WORD AdjustRolTimeUTC(WORD tim, char c);
void __fastcall SetGroupEnabled(TGroupBox *gp);
void SetButtonCaption(TSpeedButton *tb, AnsiString &as, int maxlen, int fontsize, TColor col);
void FormCenter(TForm *tp, int XW, int YW);
char *lastp(char *p);
char *clipsp(char *s);
LPCSTR _strdmcpy(LPSTR t, LPCSTR p, char c);
const char *StrDlmCpy(char *t, const char *p, char Dlm, int len);
const char *StrDlmCpyK(char *t, const char *p, char Dlm, int len);
void StrCopy(LPSTR t, LPCSTR s, int n);
char LastC(LPCSTR p);
LPCSTR GetEXT(LPCSTR Fname);
void SetEXT(LPSTR pName, LPSTR pExt);
void SetCurDir(LPSTR t, int size);
void SetDirName(LPSTR t, LPCSTR pName);
LPCSTR StrDbl(double d);
LPCSTR StrDblE(double d);
double ReadDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d);
void WriteDoubleIniFile(TMemIniFile *p, LPCSTR c1, LPCSTR c2, double d);
void ClipLF(LPSTR sp);
LPSTR FillSpace(LPSTR s, int n);
LPSTR SkipSpace(LPSTR sp);
LPCSTR SkipSpace(LPCSTR sp);
LPSTR StrDlm(LPSTR &t, LPSTR p);
LPSTR StrDlm(LPSTR &t, LPSTR p, char c);
void ChgString(LPSTR t, char a, char b);
void DelChar(LPSTR t, char a);
int atoin(const char *p, int n);
int htoin(const char *p, int n);
void InfoMB(LPCSTR fmt, ...);
void ErrorMB(LPCSTR fmt, ...);
void WarningMB(LPCSTR fmt, ...);
int YesNoMB(LPCSTR fmt, ...);
int YesNoCancelMB(LPCSTR fmt, ...);
int OkCancelMB(LPCSTR fmt, ...);

int RemoveL2(LPSTR t, LPSTR ss, LPCSTR pKey, int size);
void AddL2(LPSTR t, LPCSTR pKey, LPCSTR s, UCHAR c1, UCHAR c2, int size);

void Yen2CrLf(AnsiString &ws, AnsiString cs);
void CrLf2Yen(AnsiString &ws, AnsiString cs);

void EntryMsg(AnsiString ttl, AnsiString as, WORD Key);	//JA7UDE 0428
void SetComboBox(TComboBox *pCombo, LPCSTR pList);
void GetComboBox(AnsiString &as, TComboBox *pCombo);
void SetGridWidths(TStringGrid *pGrid, LPCSTR pList);
void GetGridWidths(AnsiString &as, TStringGrid *pGrid);

int IsNumbs(LPCSTR p);
int IsAlphas(LPCSTR p);
int IsRST(LPCSTR p);
int IsCallChar(char c);
int IsCall(LPCSTR p);
int IsName(LPCSTR p);
int IsJA(const char *s);
LPCSTR ClipCall(LPCSTR s);
LPCSTR ClipCC(LPCSTR s);

/* JA7UDE 0427
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p), c);};
inline LPUSTR StrDlm(LPUSTR &t, LPUSTR p){return (LPUSTR)StrDlm(LPSTR(t), LPSTR(p));};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p, char c){return (LPUSTR)StrDlm(LPSTR(t), p, c);};
inline LPUSTR StrDlm(LPUSTR &t, LPSTR p){return (LPUSTR)StrDlm(LPSTR(t), p);};
*/

void TopWindow(TForm *tp);
void NormalWindow(TForm *tp);

int FontStyle2Code(TFontStyles style);
TFontStyles Code2FontStyle(int code);

void KeyEvent(const short *p);
#endif

