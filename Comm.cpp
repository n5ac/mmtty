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

#include "Comm.h"
#include "ComLib.h"

#define	WAITSTAT	0

#define	DEFFSOUND	3

COMMPARA	COMM;
void InitCOMMPara(void)
{
	COMM.change = 1;
}

__fastcall CComm::CComm(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	m_CreateON = FALSE;	// クリエイトフラグ
	m_Command = 0;
	m_fHnd = NULL;		// ファイルハンドル
	m_inv = 0;
	m_Execute = 0;
	m_txwp = m_txrp = m_txcnt = 0;
	m_TxEnb = 0;

	m_figout = 0;
	m_idle = 0;
	m_WaitTimerCount = 0;
	m_nextcount = 0;

	pMod = NULL;
	m_pEXT = NULL;
}

void __fastcall CComm::WaitTXD(int w)
{
	if( w ){
#if 0
		ULONG tim = ::GetTickCount();
		ULONG w = ((pMod->m_ReCount*(pMod->m_BitLen + 1.5) ) + (w * pMod->m_ReCount/3)) * 1000 / SampFreq;
		tim += w;
		if( w > 10 )::Sleep(w - 10);
		while(tim >= ::GetTickCount()) ::Sleep(0);
#else
		::Sleep(((pMod->m_ReCount*(pMod->m_BitLen + 1.5) ) + (w * pMod->m_ReCount/3)) * 1000 / SampFreq);
#endif
	}
}

void __fastcall CComm::NextBuf(void)
{
	m_txrp++;
	if( m_txrp >= COMM_TXBUFSIZE ){
		m_txrp = 0;
	}
	m_txcnt--;
	FSKCount1++;
	FSKCount--;
}

void __fastcall CComm::EnbTX(int sw)
{
	m_TxEnb = 0;
	if( !sw ){
		FSKCount1 = 1000;
		FSKCount2 = 0;
	}
	else {
		m_WaitTimerCount = 4;
		m_FirstGaurd = TRUE;
		FSKCount1 = 0;
		FSKCount2 = 0;
		FSKDeff = 0;
	}
	m_txwp = m_txrp = m_txcnt = 0;
	FSKCount = 0;
	m_figout = 0;
	m_idle = 0;
	m_TxEnb = sw;
}

void __fastcall CComm::Timer(void)
{
	if( m_CreateON ){
		if( sys.m_TxPort ){
			if( m_fHnd == NULL ) return;
			if( !m_TxEnb ) return;
			if( m_DisDiddle != pMod->m_DisDiddle ){
				m_DisDiddle = pMod->m_DisDiddle;
			}
		}
	}
}
//-----------------------------------------------------------------
// DTRで符号送信用
void __fastcall CComm::OutData(BYTE d)
{
	const BYTE tbl[]={
		0x00, 0x10, 0x08, 0x18,	// 00000 10000 01000 11000
		0x04, 0x14, 0x0c, 0x1c,	// 00100 10100 01100 11100
		0x02, 0x12, 0x0a, 0x1a, // 00010 10010 01010 11010
		0x06, 0x16, 0x0e, 0x1e, // 00110 10110 01110 11110
		0x01, 0x11, 0x09, 0x19, // 00001 10001 01001 11001
		0x05, 0x15, 0x0d, 0x1d, // 00101 10101 01101 11101
		0x03, 0x13, 0x0b, 0x1b, // 00011 10011 01011 11011
		0x07, 0x17, 0x0f, 0x1f, // 00111 10111 01111 11111
	};
	if( m_CreateON == FALSE ) return;

	if( pMod->m_BitLen <= 6 ){
		d = tbl[d & 0x001f];
	}

	if( sys.m_TxdJob >= 2 ){
		if( m_nextcount ){
			while( m_nextcount >= ::GetTickCount() ){
				if( m_Command == COMM_CLOSE ) return;
				::Sleep(1);
			}
		}
		m_nextcount = ::GetTickCount() + m_addcount;
	}
	if( m_pEXT != NULL ){
		m_pEXT->PutChar(d);
	}
	else {
		TransmitCommChar(m_fHnd, d);
	}
#if BITDEBUG
	m_bitCountA = GetTickCount() - m_bitCount;
	m_bitCount = GetTickCount();
#endif
}
//---------------------------------------------------------------------------
void __fastcall CComm::Execute()
{
	m_Execute = 1;
    int BitLen = pMod->m_BitLen;
	//---- スレッドのコードをここに記述 ----
//	Priority = tpLower;
	while(1){
_try:;
		if( Terminated == TRUE ){
			return;
		}
		if( m_Command == COMM_CLOSE ){
			m_Command = 0;
			return;
		}
		if( m_CreateON == TRUE ){
			if( !m_TxEnb ){
				::Sleep(10);
			}
			else if( !TxBusy() ){
				if( m_FirstGaurd ){		// 最初のガード時間の確保
					m_FirstGaurd = 0;
					::Sleep(250);
					if( m_DisDiddle == -1 ) m_DisDiddle = 0;
				}
				else if( m_txcnt && (FSKDeff <= DEFFSOUND) ){	// 送信データあり
					m_DisDiddle = pMod->m_DisDiddle;
					m_Data = m_txbuf[m_txrp];
					if( m_figout && (m_fig == 0x1b) && (m_Data != 0x1f) ){
						m_Data = 0x1b;
					}
					else {
						switch(m_Data){
							case 0xff:		// Mark
								::Sleep((pMod->m_ReCount * 3 * 1000) / SampFreq);
								NextBuf();
								goto _try;
							case 0xfe:		// キャリア断続
								NextBuf();
								goto _try;
							case 0xfd:		// disable diddle
								m_DisDiddle = 0;
								NextBuf();
								goto _try;
							case 0xfc:		// enable diddle
								m_DisDiddle = 1;
								NextBuf();
								goto _try;
							case 0x1b:
							case 0x1f:
								m_fig = m_Data;
								break;
						}
						NextBuf();
					}
					m_figout = 0;
					if( !pMod->m_CharWaitDiddle ){
						WaitTXD(pMod->m_CharWait);
					}
					m_WaitTimerCount = 4;
					OutData(m_Data);
				}
				else if( (BitLen < 6) && ((FSKDeff > DEFFSOUND) || (pMod->m_diddle && !m_DisDiddle)) ){
					if( FSKDeff > DEFFSOUND ){
						WaitTXD(pMod->m_CharWait);
					}
					else if( (sys.m_LWait == 2) || !pMod->m_WaitTimer || m_WaitTimerCount ){
						if( !pMod->m_CharWaitDiddle ){
							if( sys.m_LWait == 2 ){
								WaitTXD(pMod->m_CharWait);
							}
							else if( pMod->m_DiddleWait ){
								WaitTXD(pMod->m_DiddleWait);
							}
						}
					}
					m_idle = 1;
					switch(pMod->m_diddle){
						case 1:		// BLK
							if( pMod->m_RandomDiddle && !(rand() & 3) ){
								m_Data = 0x1f;
								if( pMod->m_BitLen <= 6 ) m_figout = 1;
							}
							else {
								m_Data = 0x00;
							}
							break;
						default:	// LTR
							if( pMod->m_RandomDiddle && !(rand() & 3) ){
								m_Data = 0x00;
							}
							else {
								m_Data = 0x1f;
								if( pMod->m_BitLen <= 6 ) m_figout = 1;
							}
							break;
					}
					if( m_WaitTimerCount ) m_WaitTimerCount--;
					OutData(m_Data);
				}
				else {
					if( !m_idle ){
						::Sleep((pMod->m_ReCount * pMod->m_BitLen)*1000/SampFreq);
					}
					m_idle = 1;
				}
				::Sleep(10);
			}
			else {
				::Sleep(1);
			}
		}
		else {
			::Sleep(10);
		}
	}
}
/*#$%
==============================================================
	通信回線をクローズする
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
==============================================================
*/
void __fastcall CComm::Close(void)
{
	if( m_CreateON == TRUE ){
		if( m_Execute ){
			m_TxEnb = 0;
			m_Command = COMM_CLOSE;	// スレッド終了コマンド
			Priority = tpNormal;		//スレッドは通常の優先度である
#if 0
			DWORD tim = GetTickCount();
			while( m_Command && (GetTickCount() < (tim + 3000)) ){			// スレッド終了待ち
				::Sleep(1);
			}
#else
			WaitFor();
#endif
			FSKCount1+=1000;
		}
		if( m_pEXT != NULL ){
			delete m_pEXT;
			m_pEXT = NULL;
		}
		else {
			::CloseHandle(m_fHnd);
		}
		m_CreateON = FALSE;
	}
}
void __fastcall CComm::ReqClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_Execute ){
			m_TxEnb = 0;
			m_Command = COMM_CLOSE;	// スレッド終了コマンド
			Priority = tpNormal;		//スレッドは通常の優先度である
			FSKCount1+=1000;
		}
	}
}
void __fastcall CComm::WaitClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_Execute && m_Command ){
#if 0
			DWORD tim = GetTickCount();
			while( GetTickCount() < (tim + 3000) ){		// 1[s]のウエイト
				if( !m_Command ) break;
				::Sleep(1);
			}
#else
			WaitFor();
#endif
		}
		if( m_pEXT != NULL ){
			delete m_pEXT;
			m_pEXT = NULL;
		}
		else {
			::CloseHandle(m_fHnd);
		}
		m_CreateON = FALSE;
	}
}
/*#$%
==============================================================
	通信回線をオープンする
--------------------------------------------------------------
PortName : 回線の名前
pCP		 : COMMPARAのポインタ（ヌルの時はデフォルトで初期化）
RBufSize : 受信バッファのサイズ(default=2048)
TBufSize : 送信バッファのサイズ(default=2048)
--------------------------------------------------------------
TRUE/FALSE
--------------------------------------------------------------
==============================================================
*/
BOOL __fastcall CComm::Open(LPCTSTR PortName, int inv, COMMPARA *cp)
{

	if( m_CreateON == TRUE ) Close();
	m_Execute = 0;
	m_fHnd = ::CreateFile( PortName, GENERIC_READ | GENERIC_WRITE,
						0, NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
	);
	if( m_fHnd == INVALID_HANDLE_VALUE ){
		m_pEXT = new CEXTFSK(PortName);
		if( m_pEXT->IsLib() ){
			LONG para;
			if( cp != NULL ){
				para = (cp->Baud << 16);
				para |= cp->Stop;
				para |= (cp->BitLen << 2);
			}
			else {
				para = (45 << 16);
			}
			m_pEXT->Open(para);
			m_CreateON = TRUE;
			return TRUE;
		}
		else {
			delete m_pEXT;
			m_pEXT = NULL;
		}
		return FALSE;
	}
	m_inv = inv;
	// setup device buffers
	if( ::SetupComm( m_fHnd, DWORD(1024), DWORD(2) ) == FALSE ){
		::CloseHandle(m_fHnd);
		return FALSE;
	}

	// purge any information in the buffer
	::PurgeComm( m_fHnd, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	// set up for overlapped I/O
	COMMTIMEOUTS TimeOut;

	TimeOut.ReadIntervalTimeout = 0xffffffff;
	TimeOut.ReadTotalTimeoutMultiplier = 0;
	TimeOut.ReadTotalTimeoutConstant = 0;
	TimeOut.WriteTotalTimeoutMultiplier = 0;
	TimeOut.WriteTotalTimeoutConstant = 20000;
//	TimeOut.WriteTotalTimeoutConstant = 1;
	if( !::SetCommTimeouts( m_fHnd, &TimeOut ) ){
		::CloseHandle( m_fHnd );
		return FALSE;
	}
	::GetCommState( m_fHnd, &m_dcb );
	m_dcb.BaudRate = (cp != NULL) ? cp->Baud : 9600;
	m_dcb.fBinary = TRUE;
	m_dcb.ByteSize = BYTE((cp != NULL) ? cp->BitLen : 8);
	m_dcb.Parity = BYTE((cp != NULL) ? cp->Parity : NOPARITY);
	m_dcb.StopBits = BYTE((cp != NULL ) ? cp->Stop : ONESTOPBIT);
//	Application->MainForm->Caption = int(cp ? cp->Stop == TWOSTOPBITS : FALSE);
	m_dcb.XonChar = 0x11;	// XON
	m_dcb.XoffChar = 0x13;	// XOFF
	m_dcb.fParity = 0;
	m_dcb.fOutxCtsFlow = FALSE;
	m_dcb.fInX = m_dcb.fOutX = FALSE;
	m_dcb.fOutxDsrFlow = FALSE;
	m_dcb.EvtChar = 0x0d;

	m_dcb.fRtsControl = m_inv ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;		// 送信禁止
	m_dcb.fDtrControl = m_inv ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;		// 送信禁止

//	m_dcb.fTXContinueOnXoff = TRUE;
	m_dcb.XonLim = USHORT(1024/4);			// 1/4 of RBufSize
	m_dcb.XoffLim = USHORT(1024*3/4);		// 3/4 of RBufSize
	m_dcb.DCBlength = sizeof( DCB );

	if( m_dcb.StopBits != ONESTOPBIT ){
		m_addcount = DWORD((1000.0/double(m_dcb.BaudRate)) * double(m_dcb.ByteSize + 2.7));
	}
	else {
		m_addcount = DWORD((1000.0/double(m_dcb.BaudRate)) * double(m_dcb.ByteSize + 2.2));
	}
#if 0	// debug
	m_dcb.BaudRate = 9600;
	m_dcb.ByteSize = 8;
#endif

	if( !::SetCommState( m_fHnd, &m_dcb ) ){
		::CloseHandle( m_fHnd );
		return FALSE;
	}

	// get any early notifications
	if( !::SetCommMask( m_fHnd, EV_RXFLAG ) ){
		::CloseHandle(m_fHnd);
		return FALSE;
	}
	m_CreateON = TRUE;
	return TRUE;
}
//-----------------------------------------------------------------
// PTT切り替え用
void __fastcall CComm::SetTXRX(int sw)
{
	if( m_CreateON == FALSE ) return;

	if( m_inv ) sw = sw ? 0 : 1;
	if( m_pEXT != NULL ){
		m_pEXT->SetPTT(sw);
	}
	else if( sw ){
		::EscapeCommFunction(m_fHnd, SETRTS);
		::EscapeCommFunction(m_fHnd, SETDTR);
	}
	else {
		::EscapeCommFunction(m_fHnd, CLRRTS);
		::EscapeCommFunction(m_fHnd, CLRDTR);
	}
}
#if 0
//-----------------------------------------------------------------
// DTRで符号送信用
void __fastcall CComm::SetDTR(int sw)
{
	if( m_CreateON == FALSE ) return;

	if( sw ){
		::EscapeCommFunction(m_fHnd, SETDTR);
	}
	else {
		::EscapeCommFunction(m_fHnd, CLRDTR);
	}
}
#endif
//-----------------------------------------------------------------
// 送信ビジーかどうか調べる   TRUE : 送信ビジー状態
int __fastcall CComm::TxBusy(void)
{
	if( m_CreateON == FALSE ) return 0;

	if( m_pEXT != NULL ){
		return m_pEXT->IsTxBusy();
	}
	else {
		COMSTAT ComStat;
		DWORD	dwErrorFlags;

		ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
		return ComStat.fTxim;
	}
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
==============================================================
*/
void __fastcall CComm::PutChar(BYTE c)
{
	if( (m_CreateON == TRUE) && m_Execute ){
		if( m_txcnt < COMM_TXBUFSIZE ){
			m_txbuf[m_txwp] = c;
			m_txwp++;
			if( m_txwp >= COMM_TXBUFSIZE ) m_txwp = 0;
			m_txcnt++;
			m_idle = 0;
			FSKCount++;
		}
	}
}

#if 0
/*#$%
==============================================================
	受信バッファ内の格納データ長を得る
--------------------------------------------------------------
--------------------------------------------------------------
	データの長さ
--------------------------------------------------------------
==============================================================
*/
DWORD CComm::RecvLen(void)
{
	if( m_pEXT != NULL ){
		return m_pEXT->IsEmpty(0) ? 0 : 1;
	}
	else {
		COMSTAT ComStat;
		DWORD	dwErrorFlags;

		::ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
		return ComStat.cbInQue;
	}
}
#endif

/*******************************************************************
	EXTFSK.DLL
*******************************************************************/
__fastcall CEXTFSK::CEXTFSK(LPCSTR pName)
{
	char Name[128];
	sprintf(Name, "%s.%s", pName, strcmpi(pName, "EXTFSK") ? "fsk" : "dll");
	fextfskOpen	= NULL;
	fextfskClose = NULL;
	fextfskIsTxBusy = NULL;
	fextfskPutChar = NULL;
	fextfskSetPTT = NULL;

	m_hLib = ::LoadLibrary(Name);
	if( m_hLib != NULL ){
		fextfskOpen	= (extfskOpen)GetProc("_extfskOpen");
		fextfskClose = (extfskClose)GetProc("_extfskClose");
		fextfskIsTxBusy = (extfskIsTxBusy)GetProc("_extfskIsTxBusy");
		fextfskPutChar = (extfskPutChar)GetProc("_extfskPutChar");
		fextfskSetPTT = (extfskSetPTT)GetProc("_extfskSetPTT");
	}
}

//---------------------------------------------------------------------
__fastcall CEXTFSK::~CEXTFSK()
{
	if( m_hLib != NULL ){
		Close();
		::FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}

//---------------------------------------------------------------------
FARPROC CEXTFSK::GetProc(LPCSTR pName)
{
	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
	}
	return fn;
}

long __fastcall CEXTFSK::Open(long para)
{
	if( !m_hLib || !fextfskOpen ) return FALSE;
	return fextfskOpen(para);
}

void __fastcall CEXTFSK::Close(void)
{
	if( !m_hLib || !fextfskClose ) return;
	fextfskClose();
}

long __fastcall CEXTFSK::IsTxBusy(void)
{
	if( !m_hLib || !fextfskIsTxBusy ) return FALSE;
	return fextfskIsTxBusy();
}

void __fastcall CEXTFSK::PutChar(BYTE c)
{
	if( !m_hLib || !fextfskPutChar ) return;
	fextfskPutChar(c);
}

void __fastcall CEXTFSK::SetPTT(long tx)
{
	if( !m_hLib || !fextfskSetPTT ) return;
	fextfskSetPTT(tx);
}


