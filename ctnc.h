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



///----------------------------------------------------------
///  TNCシミュレーションクラス
///
///  (C) JE3HHT Makoto.Mori
///
//---------------------------------------------------------------------------
#ifndef CtncH
#define CtncH
#include "ComLib.h"
#include "comm.h"
#include "Mmlink.h"
//---------------------------------------------------------------------------
#include <Classes.hpp>
typedef struct {
	int		change;

	char	StrPort[32];	// ポートの名前
	int		BaudRate;		// ボーレート
	int		BitLen;			// 0-7Bit, 1-8Bit
	int		Stop;			// 0-1Bit, 1-2Bit
	int		Parity;			// 0-PN, 1-PE, 2-PO
	int		flwXON;			// Xon/Xoff ON
	int		flwCTS;			// CTS-RTS ON

	int		Echo;
	int		Type;			// 0-TNC241, 2-KAM
	int		TncMode;		// 0-Message, 1-Cmd, 2-TTy
}CTNCPARA;
extern	CTNCPARA	TNC;
//---------------------------------------------------------------------------
enum {
	TNC_PTTON = 0x00f0,
    TNC_PTTOFF,
    TNC_PTTFLUSH,
    TNC_CLEARBUF,
	TNC_AFCON,
    TNC_AFCOFF,
    TNC_NETON,
    TNC_NETOFF,
    TNC_REVON,
    TNC_REVOFF,
    TNC_HAM,
    TNC_ECHOON,
    TNC_ECHOOFF,
	TNC_CANCELPTTFLUSH,
};
enum {
	MMTMSG_NULL,
	MMTMSG_CHAR,
	MMTMSG_PTT,
    MMTMSG_COMMON,
	MMTMSG_CLEARFIFO,
    MMTMSG_SETSWITCH,
    MMTMSG_SETVIEW,
    MMTMSG_CHARTXW,
    MMTMSG_WRITETXW,
    MMTMSG_SETFREQ,
    MMTMSG_SETBAUD,
    MMTMSG_SETSQ,
    MMTMSG_SETNOTCH,
    MMTMSG_SETDEFFREQ,
    MMTMSG_SETPTTTIMER,
    MMTMSG_SETTNCTYPE,
    MMTMSG_SETPRIORITY,
    MMTMSG_SETLENGTH,
	MMTMSG_SETRESO,
    MMTMSG_SETLPF,
    MMTMSG_GETPROFILENAME,
    MMTMSG_LOADPROFILE,
    MMTMSG_SAVEPROFILE,
};
#pragma pack(push, 1)
typedef struct {
	DWORD	m_markfreq;
	DWORD	m_spacefreq;
	DWORD	m_siglevel;
    DWORD	m_sqlevel;
    DWORD	m_txbuflen;
    DWORD	m_codeswitch;
    DWORD	m_codeview;
    DWORD	m_notch1;
    DWORD	m_notch2;
    DWORD	m_baud;
    DWORD	m_bitlength;
    DWORD	m_txwlen;
    DWORD	m_sampfreq;
    DWORD	m_demfreq;
    DWORD	m_radiofreq;
    DWORD	m_modetnc;
    DWORD	m_fig;
    DWORD	m_defmark;
    DWORD	m_defshift;
	DWORD	m_Reserved[45];
}NMMT;
#pragma pack(pop)
typedef DWORD (__stdcall *tmmtQueryTnc)(LPLONG pType, LPLONG pEcho);
typedef BOOL (__stdcall *tmmtOpen)(HWND hWnd, UINT uMsg, LONG type);
typedef void (__stdcall *tmmtClose)(void);
typedef void (__stdcall *tmmtSetPTT)(LONG tx);
typedef BOOL (__stdcall *tmmtIsTxBusy)(void);
typedef void (__stdcall *tmmtPutChar)(BYTE c);
typedef LONG (__stdcall *tmmtGetRxLen)(void);
typedef BYTE (__stdcall *tmmtGetChar)(void);
typedef void (__stdcall *tmmtNotify)(NMMT *pNMMT);
typedef void (__stdcall *tmmtNotifyFFT)(const int *pFFT, LONG size, LONG sampfreq);
typedef void (__stdcall *tmmtNotifyXY)(const LONG *pXY);
#define	ntNOTIFY	1
#define	ntNOTIFYFFT	2
#define	ntNOTIFYXY	4
class CMMTnc
{
private:
	AnsiString		m_ItemName;

    DWORD			m_QueryResult;
	//HANDLE			m_hLib;
	HINSTANCE			m_hLib;
	LONG			m_XY[XYCOLLECT*2];
public:
	NMMT			m_NMMT;
private:
	tmmtQueryTnc	fmmtQueryTnc;
	tmmtOpen		fmmtOpen;
	tmmtClose       fmmtClose;
	tmmtSetPTT      fmmtSetPTT;
	tmmtIsTxBusy    fmmtIsTxBusy;
	tmmtPutChar     fmmtPutChar;
	tmmtGetRxLen    fmmtGetRxLen;
	tmmtGetChar     fmmtGetChar;
	tmmtNotify		fmmtNotify;
	tmmtNotifyFFT	fmmtNotifyFFT;
	tmmtNotifyXY	fmmtNotifyXY;

private:
	void __fastcall FreeLib(void);
	FARPROC __fastcall GetProc(LPCSTR pName);
public:
	__fastcall CMMTnc(void);
	__fastcall ~CMMTnc();
public:
	BOOL __fastcall Open(LPCSTR pLibName, HWND hWnd, UINT uMsg);
	void __fastcall Close(void);
	inline BOOL __fastcall IsLib(void){ return (m_hLib != NULL);};
	inline LPCSTR __fastcall GetItemName(void){return m_ItemName.c_str();};

	void __fastcall SetPTT(int ptt);
	void __fastcall PutChar(BYTE c);
	BYTE __fastcall GetChar(void);
	BOOL __fastcall IsTxBusy(void);
	LONG __fastcall GetRxLen(void);
	void __fastcall Notify();
	void __fastcall NotifyFFT(const int *pFFT);
	void __fastcall NotifyXY(const double *pX, const double *pY);
};

//---------------------------------------------------------------------------
void LoadTNCSetup(TMemIniFile *pIniFile);
void SaveTNCSetup(TMemIniFile *pIniFile);
//#define	CR	0x0d
//#define	LF	0x0a
#define	CTNC_CLOSE	1
#define	TNC_COMBUFSIZE	4096
#define	TNC_TXBUFSIZE	1024
#define	TNC_RXBUFSIZE	1024
class CCtnc : public TThread
{
public:
	BOOL	m_CreateON;		// クリエイトフラグ
	DCB		m_dcb;			// ＤＣＢ
	HANDLE	m_fHnd;			// ファイルハンドル
	HWND	m_wHnd;			// 親のウインドウハンドル
	UINT	m_ID;			// メッセージのＩＤ番号
	volatile	int	m_Command;		// スレッドへのコマンド
	BOOL	m_TxAbort;		// 送信中止フラグ
	AnsiString	Name;

	char	m_txbuf[TNC_TXBUFSIZE];
	int		m_txcnt;
	int		m_txwp;
	int		m_txrp;
	int		m_timeout;

	char	m_rxbuf[TNC_RXBUFSIZE];
	int		m_rxcnt;
	int		m_rxwp;
	int		m_rxrp;

    CMMTnc	*m_pMMT;

	COMMPARA	cm;
	BOOL	m_PTT;
protected:
	void virtual __fastcall Execute();
public:
	__fastcall CCtnc(bool CreateSuspended);
	__fastcall ~CCtnc(){
		Close();
	};
	inline BOOL IsOpen(void){
		return m_CreateON;
	};
	inline void UpdateHandle(HWND hwnd){
		m_wHnd = hwnd;
	};
	BOOL __fastcall Open(CTNCPARA *cp, HWND hwnd, UINT nID, COMMPARA *pp);
	void __fastcall Close(void);
	void __fastcall ReqClose(void);
	void __fastcall WaitClose(void);
	BOOL __fastcall GetPTT(void);
	DWORD __fastcall RecvLen(void);
	int __fastcall TxBusy(void);
	int __fastcall IORead(BYTE *p, DWORD len);
	int __fastcall IOPutChar(BYTE c);

	void __fastcall ClearRxFifo(void);
	void __fastcall ClearTxFifo(void);
	BOOL __fastcall PutRxFifo(BYTE c);
	void __fastcall Write(void *p, DWORD len);
	void __fastcall PutChar(char c);
	void OutStr(LPCSTR fmt, ...);
	void OutLine(LPCSTR fmt, ...);

    void __fastcall NotifyFreq(LONG mark, LONG space);
	void __fastcall SetPTT(int tx);
    inline BOOL IsMMT(void){return m_pMMT!=NULL;};
	NMMT* __fastcall GetNMMT(void){return &m_pMMT->m_NMMT;};
	void __fastcall NotifyLevel(LONG sq, LONG lvl);
	inline void __fastcall Notify(void){
		if( m_pMMT ) m_pMMT->Notify();
    }
	inline void __fastcall NotifyFFT(const int *pFFT){
		if( m_pMMT ) m_pMMT->NotifyFFT(pFFT);
    }
    inline void __fastcall NotifyXY(const double *pX, const double *pY){
		if( m_pMMT ) m_pMMT->NotifyXY(pX, pY);
    }
};

void InitTNCPara(void);

///---------------------------------------------------------
///  テキスト文字列ＦＩＦＯ
class CTextFifo
{
private:
	LPSTR	bp, wp, rp;
	int		Cnt;
	int		Max;
	int 	Line;
public:
	inline CTextFifo(int max = TNC_COMBUFSIZE){
		Max = max;
		bp = new char[Max];
		wp = rp = bp;
		Cnt = 0;
		Line = 0;
	};
	inline ~CTextFifo(){
		delete[] bp;
	};
	inline void WriteChar(char c){
		if( Cnt < Max ){
			if( c != LF ){
				*wp = c;
				wp++;
				if( wp >= (bp+Max) ){
					wp = bp;
				}
				Cnt++;
			}
			if( c == CR ){
				Line++;
			}
		}
	}
	inline void WriteString(LPCSTR sp){
		for( ; *sp; sp++ ){
			WriteChar(*sp);
		}
	};
	inline int GetLine(void){
		return Line;
	};
	inline int LoadText(LPSTR tp, int len){
		if( !Line ) return FALSE;
		while(1){
			char c = *rp;
			rp++;
			if( rp >= (bp+Max) ) rp = bp;
			Cnt--;

			if( c == CR ){
				*tp = 0;
				Line--;
				break;
			}
			else if( (len - 1) > 0 ){
				*tp++ = c;
				len--;
			}
		}
		return TRUE;
	};
};
#endif

