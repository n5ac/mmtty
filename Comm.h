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
///  ÇqÇrÇQÇRÇQÇbí êMÉNÉâÉX
///
///  (C) JE3HHT Makoto.Mori
///
//---------------------------------------------------------------------------
#ifndef CommH
#define CommH
#include "rtty.h"
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
#define	CR	0x0d
#define	LF	0x0a
#define	COMM_CLOSE	1

typedef struct {
	int	change;

	int Baud;
	int	BitLen;
	int	Stop;
	int	Parity;
}COMMPARA;
extern	COMMPARA	COMM;

typedef long (__stdcall *extfskOpen)(long para);
typedef void (__stdcall *extfskClose)(void);
typedef long (__stdcall *extfskIsTxBusy)(void);
typedef void (__stdcall *extfskPutChar)(BYTE c);
typedef void (__stdcall *extfskSetPTT)(long tx);

class CEXTFSK
{
private:
	//HANDLE			m_hLib;
	HINSTANCE		m_hLib;    //JA7UDE 0427
	extfskOpen		fextfskOpen;
	extfskClose		fextfskClose;
	extfskIsTxBusy	fextfskIsTxBusy;
	extfskPutChar	fextfskPutChar;
	extfskSetPTT	fextfskSetPTT;
private:
	FARPROC GetProc(LPCSTR pName);

public:
	__fastcall CEXTFSK(LPCSTR pName);
	__fastcall ~CEXTFSK();
	long __fastcall IsLib(void){return m_hLib != NULL;};
	long __fastcall Open(long para);
	void __fastcall Close(void);
	long __fastcall IsTxBusy(void);
	void __fastcall PutChar(BYTE c);
	void __fastcall SetPTT(long tx);
};

#define	COMM_TXBUFSIZE		MODBUFMAX
class CComm : public TThread
{
public:
	BOOL	m_CreateON;		// ÉNÉäÉGÉCÉgÉtÉâÉO
	volatile	int	m_Command;
	int		m_Execute;
	DCB		m_dcb;			// ÇcÇbÇa
	HANDLE	m_fHnd;			// ÉtÉ@ÉCÉãÉnÉìÉhÉã
	int		m_inv;

	int		m_TxEnb;
	char	m_txbuf[COMM_TXBUFSIZE];
	int		m_txcnt;
	int		m_txwp;
	int		m_txrp;

	int		m_idle;

	BYTE	m_Data;
	BYTE	m_fig;
	int		m_DisDiddle;
	int		m_FirstGaurd;

	int		m_WaitTimerCount;

	int		m_figout;
	CFSKMOD	*pMod;

	DWORD	m_nextcount;
	DWORD	m_addcount;
#if BITDEBUG
	int		m_bitCount;
	int		m_bitCountA;
#endif

	CEXTFSK	*m_pEXT;
protected:
	void virtual __fastcall Execute();
private:
	void __fastcall NextBuf(void);
	void __fastcall WaitTXD(int w);
	void __fastcall OutData(BYTE d);
public:
	__fastcall CComm(bool CreateSuspended);
	__fastcall ~CComm(){
		Close();
	};
	inline BOOL __fastcall IsOpen(void){
		return m_CreateON;
	};
	inline void __fastcall SetInv(int inv){m_inv = inv;};
	BOOL __fastcall Open(LPCTSTR PortName, int inv, COMMPARA *cp);
	void __fastcall Close(void);
	void __fastcall ReqClose(void);
	void __fastcall WaitClose(void);
	void __fastcall SetTXRX(int sw);
//	void __fastcall SetDTR(int sw);
	int __fastcall TxBusy(void);
//	void Out(BYTE d);
	void __fastcall PutChar(BYTE c);
	void __fastcall EnbTX(int sw);
	void __fastcall Timer(void);
	inline __fastcall GetBufCount(void){
		return m_txcnt;
	};
	void __fastcall DisDiddle(int sw){
		m_DisDiddle = sw;
	};
	void __fastcall InitTxBuf(void){
		m_txwp = m_txrp = m_txcnt = 0;
		FSKCount1 = 0;
		FSKCount2 = 0;
		FSKCount = 0;
		FSKDeff = 0;
	};
//	DWORD RecvLen(void);
};

void InitCOMMPara(void);
#endif






