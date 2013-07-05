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

#include "Ctnc.h"
#include "ComLib.h"

#define	WAITSTAT	0

CTNCPARA	TNC;
void InitTNCPara(void)
{
	strcpy(TNC.StrPort, "NONE");	// ポートの名前
	TNC.BaudRate = 1200;			// ボーレート
	TNC.BitLen = 1;					// 0-7Bit, 1-8Bit
	TNC.Stop = 0;					// 0-1Bit, 1-2Bit
	TNC.Parity = 0;					// 0-PN, 1-PE, 2-PO
	TNC.flwXON = 1;					// Xon/Xoff ON
	TNC.flwCTS = 0;					// CTS-RTS ON

	TNC.Echo = 2;
	TNC.Type = (lcid != LANG_JAPANESE) ? 1 : 0;		// 0-TNC241, 1-KAM
	TNC.TncMode = 1;				// Command mode
	TNC.change = 1;
}
void LoadTNCSetup(TMemIniFile *pIniFile)
{
	AnsiString as = TNC.StrPort;
	as = pIniFile->ReadString("TNC", "PortName", as);
	StrCopy(TNC.StrPort, as.c_str(), 31);
	TNC.Type = pIniFile->ReadInteger("TNC", "Type", TNC.Type);
	TNC.BaudRate = pIniFile->ReadInteger("TNC", "BaudRate", TNC.BaudRate);
	TNC.BitLen = pIniFile->ReadInteger("TNC", "BitLen", TNC.BitLen);
	TNC.Stop = pIniFile->ReadInteger("TNC", "Stop", TNC.Stop);
	TNC.Parity = pIniFile->ReadInteger("TNC", "Parity", TNC.Parity);
	TNC.flwXON = pIniFile->ReadInteger("TNC", "flwXON", TNC.flwXON);
	TNC.flwCTS = pIniFile->ReadInteger("TNC", "flwCTS", TNC.flwCTS);
	TNC.Echo = pIniFile->ReadInteger("TNC", "Echo", TNC.Echo);
}
void SaveTNCSetup(TMemIniFile *pIniFile)
{
	pIniFile->WriteString("TNC", "PortName", TNC.StrPort);
	pIniFile->WriteInteger("TNC", "Type", TNC.Type);
	pIniFile->WriteInteger("TNC", "BaudRate", TNC.BaudRate);
	pIniFile->WriteInteger("TNC", "BitLen", TNC.BitLen);
	pIniFile->WriteInteger("TNC", "Stop", TNC.Stop);
	pIniFile->WriteInteger("TNC", "Parity", TNC.Parity);
	pIniFile->WriteInteger("TNC", "flwXON", TNC.flwXON);
	pIniFile->WriteInteger("TNC", "flwCTS", TNC.flwCTS);
	pIniFile->WriteInteger("TNC", "Echo", TNC.Echo);
}
//---------------------------------------------------------------------------
//   注意: VCL オブジェクトのメソッドとプロパティを使用するには, Synchronize
//         を使ったメソッド呼び出しでなければなりません。次に例を示します。
//
//      Synchronize(UpdateCaption);
//
//   ここで, UpdateCaption は次のように記述できます。
//
//      void __fastcall CCtnc::UpdateCaption()
//      {
//        Form1->Caption = "スレッドから書き換えました";
//      }
//---------------------------------------------------------------------------
__fastcall CCtnc::CCtnc(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	m_CreateON = FALSE;	// クリエイトフラグ
	m_fHnd = NULL;		// ファイルハンドル
	m_wHnd = NULL;		// 親のウインドウハンドル
	m_ID = 0;			// メッセージのＩＤ番号
	m_Command = 0;		// スレッドへのコマンド
	m_TxAbort = 0;		// 送信中止フラグ
	ClearTxFifo();
    ClearRxFifo();
	m_timeout = 200;
    m_pMMT = NULL;
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::ClearRxFifo(void)
{
	m_rxwp = m_rxrp = m_rxcnt = 0;
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::ClearTxFifo(void)
{
	m_txwp = m_txrp = m_txcnt = 0;
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
BOOL __fastcall CCtnc::PutRxFifo(BYTE c)
{
	if( m_rxcnt >= TNC_RXBUFSIZE ) return FALSE;
	if( (TNC.Type == 2) && (cm.BitLen <= 6) ){
		if( m_pMMT && (c >= 0x20) ){
			m_rxbuf[m_rxwp] = c;
		}
		else {
			m_rxbuf[m_rxwp] = tbl[c&0x1f];
		}
	}
	else {
		m_rxbuf[m_rxwp] = c;
	}
	m_rxwp++;
	if( m_rxwp >= (int)sizeof(m_rxbuf) ) m_rxwp = 0;	//JA7UDE 0428
	m_rxcnt++;
    return TRUE;
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::Execute()
{
	//---- スレッドのコードをここに記述 ----
//	Priority = tpLower;
	if( TNC.Type == 2 ){
		m_PTT = GetPTT();
	}
	while(1){
		if( Terminated == TRUE ){
			return;
		}
		if( m_Command == CTNC_CLOSE ){
			m_Command = 0;
			return;
		}
		if( m_CreateON == TRUE ){
			if( m_txcnt ){
				if( !TxBusy() ){
					BYTE	d = m_txbuf[m_txrp];
					if( TNC.Type == 2 && (cm.BitLen <= 6) ){
						d = tbl[d & 0x1f];
					}
					if( IOPutChar(d) ){
						m_txrp++;
						if( m_txrp >= TNC_TXBUFSIZE ){
							m_txrp = 0;
						}
						m_txcnt--;
					}
				}
				::Sleep(1);
			}
			else if( m_rxcnt < (TNC_RXBUFSIZE/2) ){
				int len = RecvLen();
				if( len ){
					BYTE bf[256];
					if( len >= (int)sizeof(bf) ) len = sizeof(bf);
					if( len && (m_rxcnt < int(sizeof(m_rxbuf) - len)) ){
						int size = IORead(bf, len);
						for( int i = 0; i < size; i++ ){
							PutRxFifo(bf[i]);
						}
						if( m_wHnd != NULL ){
							PostMessage( m_wHnd, m_ID, MMTMSG_NULL, 0);
						}
					}
				}
				::Sleep(((TNC.Type != 2) || m_pMMT) ? 10 : 1);
			}
            else {
				::Sleep(10);
            }
			if( (TNC.Type == 2) && !m_pMMT ){
				BOOL tx = GetPTT();
				if( (tx != m_PTT) && (tx || !m_rxcnt) ){
					m_PTT = tx;
					PostMessage(m_wHnd, m_ID, MMTMSG_NULL, 0);
				}
				::Sleep(1);
			}
		}
		else {
			::Sleep(20);
		}
	}
}

//---------------------------------------------------------------------------
/*#$%
==============================================================
	通信回線をオープンしスレッドをアクティブにする
--------------------------------------------------------------
PortName : 回線の名前
pCP		 : COMMPARAのポインタ（ヌルの時はデフォルトで初期化）
pWnd     : メッセージ送信先のウインドウクラスのポインタ（ヌルの時はﾒｲﾝﾌﾚｰﾑｳｲﾝﾄﾞｳ）
nID		 : データ受信時のメッセージＩＤ
RBufSize : 受信バッファのサイズ(default=2048)
TBufSize : 送信バッファのサイズ(default=2048)
--------------------------------------------------------------
TRUE/FALSE
--------------------------------------------------------------
==============================================================
*/
BOOL __fastcall CCtnc::Open(CTNCPARA *cp, HWND hwnd, UINT nID, COMMPARA *pp)
{
	if( m_CreateON == TRUE ) Close();
	m_TxAbort = FALSE;
	m_wHnd = hwnd;
	m_ID = nID;
	m_fHnd = ::CreateFile(cp->StrPort, GENERIC_READ | GENERIC_WRITE,
						0, NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL
	);
	if( m_fHnd == INVALID_HANDLE_VALUE ) goto _mmt;
	// setup device buffers
	if( ::SetupComm( m_fHnd, DWORD(TNC_COMBUFSIZE), DWORD(TNC_COMBUFSIZE) ) == FALSE ){
		::CloseHandle(m_fHnd);
_mmt:;
		m_pMMT = new CMMTnc;
		if( m_pMMT->Open(cp->StrPort, m_wHnd, m_ID) ){
			m_CreateON = TRUE;
			Priority = tpLower;
			Resume();			// スレッドの実行
			return TRUE;
		}
		else {
			delete m_pMMT;
			m_pMMT = NULL;
			return FALSE;
		}
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
	m_dcb.fBinary = TRUE;
	const UCHAR _tp[]={NOPARITY, EVENPARITY, ODDPARITY};
	const UCHAR _ts[]={ONESTOPBIT,TWOSTOPBITS};
	if( TNC.Type == 2 ){
		memcpy(&cm, pp, sizeof(cm));
		m_dcb.BaudRate = pp->Baud;
		m_dcb.ByteSize = WORD(pp->BitLen);
		m_dcb.Parity = WORD(pp->Parity);
		m_dcb.StopBits = WORD(pp->Stop);
		m_dcb.fOutxCtsFlow = FALSE;
		m_dcb.fInX = m_dcb.fOutX = FALSE;
	}
	else {
		m_dcb.BaudRate = cp->BaudRate;
		m_dcb.ByteSize = USHORT(cp->BitLen ? 8 : 7);
		m_dcb.Parity = _tp[cp->Parity];
		m_dcb.StopBits = _ts[cp->Stop];
		m_dcb.fOutxCtsFlow = cp->flwCTS ? TRUE : FALSE;
		m_dcb.fInX = m_dcb.fOutX = cp->flwXON ? TRUE : FALSE;
	}
	m_dcb.fRtsControl = RTS_CONTROL_ENABLE;
	m_dcb.XonChar = 0x11;
	m_dcb.XoffChar = 0x13;
	m_dcb.fParity = FALSE;
	m_dcb.EvtChar = 0x00;	// dummy setting
//	m_dcb.fTXContinueOnXoff = TRUE;
	m_dcb.XonLim = USHORT(TNC_COMBUFSIZE/4);		// 1/4 of RBufSize
	m_dcb.XoffLim = USHORT(TNC_COMBUFSIZE*3/4);		// 3/4 of RBufSize
	m_dcb.DCBlength = sizeof( DCB );
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
	Priority = tpLower;
	Resume();		// スレッドの実行
	return TRUE;
}
/*#$%
==============================================================
	通信回線をクローズする
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
	スレッドが終了するまで待つ
==============================================================
*/
void __fastcall CCtnc::Close(void)
{
	if( m_CreateON == TRUE ){
		if( m_ID ){
			m_Command = CTNC_CLOSE;		// スレッド終了コマンド
			Priority = tpNormal;		//スレッドは通常の優先度である
			WaitFor();
		}
		if( m_pMMT ){
        	delete m_pMMT;
            m_pMMT = NULL;
        }
        else {
			::CloseHandle(m_fHnd);
        }
		m_CreateON = FALSE;
	}
	m_TxAbort = TRUE;
}
void __fastcall CCtnc::ReqClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_ID ){
			m_Command = CTNC_CLOSE;		// スレッド終了コマンド
			Priority = tpNormal;		//スレッドは通常の優先度である
		}
	}
}
void __fastcall CCtnc::WaitClose(void)
{
	if( m_CreateON == TRUE ){
		if( m_ID && m_Command ){
			WaitFor();
		}
		if( m_pMMT ){
        	delete m_pMMT;
            m_pMMT = NULL;
        }
		else {
			::CloseHandle(m_fHnd);
        }
		m_CreateON = FALSE;
	}
	m_TxAbort = TRUE;
}
//----------------------------------------------------------------------
BOOL __fastcall CCtnc::GetPTT(void)
{
	DWORD d = 0;
	BOOL tx = FALSE;
	if( !m_pMMT ){
		if( ::GetCommModemStatus(m_fHnd, &d) ){
			tx = (d & MS_CTS_ON) != 0;
        }
	}
	return tx;
}
/*#$%
==============================================================
	受信バッファ内の格納データ長を得る
--------------------------------------------------------------
--------------------------------------------------------------
	データの長さ
--------------------------------------------------------------
==============================================================
*/
DWORD __fastcall CCtnc::RecvLen(void)
{
	if( m_pMMT ){
		return m_pMMT->GetRxLen();
    }
    else {
		COMSTAT ComStat;
		DWORD	dwErrorFlags;

		::ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
		return ComStat.cbInQue;
    }
}

/*#$%
==============================================================
	送信ビジーかどうか調べる
--------------------------------------------------------------
--------------------------------------------------------------
TRUE : 送信ビジー状態
--------------------------------------------------------------
==============================================================
*/
int __fastcall CCtnc::TxBusy(void)
{
	if( m_pMMT ){
		return m_pMMT->IsTxBusy();
    }
    else {
		COMSTAT ComStat;
		DWORD	dwErrorFlags;

		::ClearCommError( m_fHnd, &dwErrorFlags, &ComStat );
		return ComStat.cbOutQue;
    }
}

/*#$%
==============================================================
	通信回線からデータを取り出す
--------------------------------------------------------------
p	: バッファのポインタ
len : バッファのサイズ
--------------------------------------------------------------
実際に受信したサイズ
--------------------------------------------------------------
==============================================================
*/
int __fastcall CCtnc::IORead(BYTE *p, DWORD len)
{
	DWORD	size=0;

	if( m_CreateON == TRUE ){
		if( m_pMMT ){
			for( ; len; len--, size++ ){
				*p++ = m_pMMT->GetChar();
            }
        }
        else {
			::ReadFile( m_fHnd, p, len, &size, NULL );
        }
	}
	return size;
}
int __fastcall CCtnc::IOPutChar(BYTE c)
{
	DWORD	size=0;

	if( m_CreateON == TRUE ){
		if( m_pMMT ){
			m_pMMT->PutChar(c);
            size = 1;
        }
        else {
			::WriteFile( m_fHnd, &c, 1, &size, NULL );
        }
	}
    return size;
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
==============================================================
*/
void __fastcall CCtnc::PutChar(char c)
{
	if( m_CreateON == TRUE ){
		if( m_txcnt < TNC_TXBUFSIZE ){
			m_txbuf[m_txwp] = c;
			m_txwp++;
			if( m_txwp >= TNC_TXBUFSIZE ) m_txwp = 0;
			m_txcnt++;
		}
	}
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
p	: バッファのポインタ
len : 送信するサイズ
--------------------------------------------------------------
実際に送信したサイズ
--------------------------------------------------------------
==============================================================
*/
void __fastcall CCtnc::Write(void *s, DWORD len)
{
	if( m_CreateON == TRUE ){
		char	*p;
		for( p = (char *)s; len; len--, p++ ){
			PutChar(*p);
		}
	}
}

/*#$%
==============================================================
	通信回線にデータを送信する
--------------------------------------------------------------
p	: バッファのポインタ
len : 送信するサイズ
--------------------------------------------------------------
実際に送信したサイズ
--------------------------------------------------------------
==============================================================
*/
void CCtnc::OutStr(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);
	Write(bf, strlen(bf));
}

void CCtnc::OutLine(LPCSTR fmt, ...)
{
	va_list	pp;
	char	bf[1024];

	va_start(pp, fmt);
	vsprintf( bf, fmt, pp );
	va_end(pp);
	Write(bf, strlen(bf));
	char r[] = "\r";
	Write( r, 1 );
	//Write("\r", 1);
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::SetPTT(int tx)
{
	if( m_pMMT ) m_pMMT->SetPTT(tx);
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::NotifyFreq(LONG mark, LONG space)
{
	if( m_pMMT ){
    	m_pMMT->m_NMMT.m_markfreq = mark;
		m_pMMT->m_NMMT.m_spacefreq = space;
    }
}
//---------------------------------------------------------------------------
void __fastcall CCtnc::NotifyLevel(LONG sq, LONG lvl)
{
	if( m_pMMT ){
		m_pMMT->m_NMMT.m_sqlevel = sq;
		m_pMMT->m_NMMT.m_siglevel = lvl;
    }
}
//==============================================================================
// CMMTnc カスタムTNCエミュレーション
//==============================================================================
//---------------------------------------------------------------------------
__fastcall CMMTnc::CMMTnc(void)
{
	m_hLib = NULL;
	memset(&m_NMMT, 0, sizeof(m_NMMT));
	m_NMMT.m_sampfreq = SampFreq;
	m_NMMT.m_demfreq = DemSamp;
    m_QueryResult = 0;
}
//---------------------------------------------------------------------------
__fastcall CMMTnc::~CMMTnc()
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::FreeLib(void)
{
	if( m_hLib ){
		FreeLibrary(m_hLib);
		m_hLib = NULL;
	}
}
//---------------------------------------------------------------------------
FARPROC __fastcall CMMTnc::GetProc(LPCSTR pName)
{
	if( !m_hLib ) return NULL;

	FARPROC fn = ::GetProcAddress(m_hLib, pName+1);
	if( fn == NULL ){
		fn = ::GetProcAddress(m_hLib, pName);
		if( fn == NULL ) FreeLib();
	}
	return fn;
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::Close(void)
{
	if( IsLib() ){
		fmmtClose();
		FreeLib();
	}
}
//---------------------------------------------------------------------------
BOOL __fastcall CMMTnc::Open(LPCSTR pItemName, HWND hWnd, UINT uMsg)
{
	Close();

	m_ItemName = pItemName;
	char LibName[MAX_PATH];
	if( !*GetEXT(pItemName) ){
		sprintf(LibName, "%s.mmt", pItemName);
		pItemName = LibName;
	}
	m_hLib = ::LoadLibrary(pItemName);
	if( m_hLib ){

		fmmtQueryTnc = PROC(mmtQueryTnc);
		fmmtOpen = PROC(mmtOpen);
		fmmtClose = PROC(mmtClose);
		fmmtSetPTT = PROC(mmtSetPTT);
		fmmtPutChar = PROC(mmtPutChar);
		fmmtGetChar = PROC(mmtGetChar);
		fmmtIsTxBusy = PROC(mmtIsTxBusy);
		fmmtGetRxLen = PROC(mmtGetRxLen);
		fmmtNotify = PROC(mmtNotify);
		fmmtNotifyFFT = PROC(mmtNotifyFFT);
		fmmtNotifyXY = PROC(mmtNotifyXY);

		if( m_hLib ){
			m_QueryResult = fmmtQueryTnc((LPLONG)&TNC.Type, (LPLONG)&TNC.Echo);
			if( !fmmtOpen(hWnd, uMsg, TNC.Type) ){
				FreeLib();
			}
		}
	}
	return IsLib();
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::SetPTT(int ptt)
{
	if( !IsLib() ) return;
	fmmtSetPTT(ptt);
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::PutChar(BYTE c)
{
	if( !IsLib() ) return;
	fmmtPutChar(c);
}
//---------------------------------------------------------------------------
BYTE __fastcall CMMTnc::GetChar(void)
{
	if( !IsLib() ) return 0;
	return fmmtGetChar();
}
//---------------------------------------------------------------------------
BOOL __fastcall CMMTnc::IsTxBusy(void)
{
	if( !IsLib() ) return FALSE;
	return fmmtIsTxBusy();
}
//---------------------------------------------------------------------------
LONG __fastcall CMMTnc::GetRxLen(void)
{
	if( !IsLib() ) return 0;
	return fmmtGetRxLen();
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::Notify(void)
{
	if( !IsLib() ) return;
    if( m_QueryResult & ntNOTIFY ) fmmtNotify(&m_NMMT);
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::NotifyFFT(const int *pFFT)
{
	if( !IsLib() ) return;
	if( m_QueryResult & ntNOTIFYFFT ) fmmtNotifyFFT(pFFT, FFT_SIZE, SampFreq);
}
//---------------------------------------------------------------------------
void __fastcall CMMTnc::NotifyXY(const double *pX, const double *pY)
{
	if( !IsLib() ) return;
    if( !(m_QueryResult & ntNOTIFYXY) ) return;

	LPLONG px = m_XY;
    LPLONG py = &m_XY[XYCOLLECT];
    for( int i = 0; i < XYCOLLECT; i++ ){
		*px++ = LONG(*pX++);
        *py++ = LONG(*pY++);
    }
	fmmtNotifyXY(m_XY);
}
//---------------------------------------------------------------------------
