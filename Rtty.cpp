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

#include "Rtty.h"

#define	LOGFFT	FALSE

CSinTable	g_SinTable;

//---------------------------------------------------------------------------
// CSinTableクラス
// Added by JE3HHT on Aug.2010
__fastcall CSinTable::CSinTable()
{
	m_Size = 48000;
    m_tSin = new double[m_Size];
	double pi2t = 2 * PI / double(m_Size);
	for( int i = 0; i < m_Size; i++ ){
		m_tSin[i] = sin(double(i) * pi2t);
	}
}
//---------------------------------------------------------------------------
__fastcall CSinTable::~CSinTable()
{
	delete[] m_tSin;
}
//---------------------------------------------------------------------------
CTICK::CTICK()
{
	ptbl[0] = new int[12500];
	ptbl[1] = new int[12500];
	memset(ptbl[0], 0, sizeof(int[12500]));
	memset(ptbl[1], 0, sizeof(int[12500]));
}
CTICK::~CTICK()
{
	delete[] ptbl[0];	//JA7UDE 0428
	delete[] ptbl[1];	//JA7UDE 0428
}
void CTICK::Init(void)
{
	m_wsel = 0;
	m_wp = ptbl[0];
	m_wcnt = 0;
	m_Trig = 0;
}
void CTICK::Write(double d)
{
	*m_wp = int(d);
	m_wp++;
	m_wcnt++;
	if( m_wcnt >= m_Samp ){
		m_wcnt = 0;
		m_wsel++;
		m_wsel &= 1;
		m_wp = ptbl[m_wsel];
		m_Trig = 1;
	}
}
int *CTICK::GetData(void)
{
	if( !m_Trig ) return NULL;
	m_Trig = 0;
	int sel = m_wsel + 1;
	sel &= 1;
	return ptbl[sel];
}
//---------------------------------------------------------------------------
// VCOクラス
CVCO::CVCO()
{
	m_vlock = 0;
	m_SampleFreq = SampFreq;
	m_FreeFreq = 1900.0;
	m_TableSize = int(SampFreq*2);
	pSinTbl = new double[m_TableSize];
	m_c1 = m_TableSize/16.0;
	m_c2 = int( double(m_TableSize) * m_FreeFreq / m_SampleFreq );
	m_z = 0;
	double pi2t = 2 * PI / double(m_TableSize);
	for( int i = 0; i < m_TableSize; i++ ){
		pSinTbl[i] = sin(double(i) * pi2t);
	}
}

CVCO::~CVCO()
{
	if( m_vlock ) ::VirtualUnlock(pSinTbl, sizeof(double)*m_TableSize);
	delete[] pSinTbl;
}

void CVCO::SetGain(double gain)
{
	m_c1 = double(m_TableSize) * gain / m_SampleFreq;
}

void CVCO::VirtualLock(void)
{
	if( !m_vlock ){
		::VirtualLock(pSinTbl, sizeof(double)*m_TableSize);
		m_vlock = 1;
	}
}

void CVCO::SetSampleFreq(double f)
{
	m_SampleFreq = f;
	int size = int(m_SampleFreq*2);
	if( m_TableSize != size ){
		if( pSinTbl != NULL ){
			if( m_vlock ) ::VirtualUnlock(pSinTbl, sizeof(double)*m_TableSize);
			delete[] pSinTbl;
		}
		m_TableSize = size;
		pSinTbl = new double[m_TableSize];
		if( m_vlock ) ::VirtualLock(pSinTbl, sizeof(double)*m_TableSize);
		double pi2t = 2 * PI / double(m_TableSize);
		for( int i = 0; i < m_TableSize; i++ ){
			pSinTbl[i] = sin(double(i) * pi2t);
		}
	}
	SetFreeFreq(m_FreeFreq);
}

void CVCO::SetFreeFreq(double f)
{
	m_FreeFreq = f;
	m_c2 = double(m_TableSize) * m_FreeFreq / m_SampleFreq;
}

void CVCO::InitPhase(void)
{
	m_z = 0;
}

double CVCO::Do(double d)
{		// -1 to 1
	m_z += (d * m_c1 + m_c2) + 0.5;
	while( m_z >= m_TableSize ){
		m_z -= m_TableSize;
	}
	while( m_z < 0 ){
		m_z += m_TableSize;
	}
	return pSinTbl[int(m_z)];
}

//---------------------------------------------------------------------------
// VCOXクラス
__fastcall CVCOX::CVCOX()
{
	m_SampleFreq = SampFreq;
	m_FreeFreq = 2000.0;
	m_TableSize = g_SinTable.m_Size;
    m_TableCOS = m_TableSize / 4;
	m_c1 = 0.0;
	m_c2 = double(m_TableSize) * m_FreeFreq / m_SampleFreq;
	m_z = 0.0;
}

__fastcall CVCOX::~CVCOX()
{
}

void __fastcall CVCOX::SetGain(double gain)
{
	m_c1 = double(m_TableSize) * gain / m_SampleFreq;
}

void __fastcall CVCOX::SetSampleFreq(double f)
{
	m_SampleFreq = f;
	SetFreeFreq(m_FreeFreq);
}

void __fastcall CVCOX::SetFreeFreq(double f)
{
	m_FreeFreq = f;
	m_c2 = double(m_TableSize) * m_FreeFreq / m_SampleFreq;
	if( f < 1.0 ) m_z = 0;
}

void __fastcall CVCOX::InitPhase(void)
{
	m_z = 0.0;
}
//------------------------------------------------------------------
double __fastcall CVCOX::Do(void)
{
	m_z += m_c2;
	if( m_z >= m_TableSize ){
		m_z -= m_TableSize;
	}
	if( m_z < 0 ){
		m_z += m_TableSize;
	}
	return g_SinTable.m_tSin[int(m_z)];
}
//------------------------------------------------------------------
double __fastcall CVCOX::Do(double d)
{
	// -1 to 1
	m_z += (d * m_c1 + m_c2);
	while( m_z >= m_TableSize ){
		m_z -= m_TableSize;
	}
	while( m_z < 0 ){
		m_z += m_TableSize;
	}
	return g_SinTable.m_tSin[int(m_z)];
}
//------------------------------------------------------------------
double __fastcall CVCOX::DoCos(void)
{
	double z = m_z + m_TableCOS;
    if(z >= m_TableSize ) z -= m_TableSize;
	return g_SinTable.m_tSin[int(z)];
}

//---------------------------------------------------------------------------
// CFSKMODクラス
CFSKMOD::CFSKMOD()
{
	m_SampFreq = sys.m_SampFreq + sys.m_TxOffset;
	vco.SetSampleFreq(m_SampFreq);

	memset(ZBPF, 0, sizeof(ZBPF));
	memset(HBPF, 0, sizeof(HBPF));
	m_bpftap = 48;

	m_rp = 0;
	m_wp = 0;
	m_cnt = 0;

	m_BitLen = 5;
	m_StopLen = 4;		// 0-1bit, 1-1.5bit, 2-2bit, 3-1.5bit, 4-1.5bit
	m_Parity = 0;
	m_SumParity = 0;

	m_Data = 0;
	m_DataCount = 0;
	m_mode = 0;
	m_inv = 0;
	m_diddle = 2;
	m_fig = 0;
	m_figout = 0;

	SetBaudRate(45.45);
	SetOutputGain(24576.0);

	m_MarkFreq = 2125;
	m_SpaceFreq = 2295;
	SetMarkFreq(2125);
	SetSpaceFreq(2295);

	m_out = 1;
	m_bpf = 1;
	SetLPFFreq(100.0);
	m_lpf = 0;

	m_CharWaitCount = 0;
	m_CharWait = 0;
	m_DiddleWait = 0;
	m_CharWaitDiddle = 0;

	m_idle = 1;
	pDem = NULL;
	m_RandomDiddle = 0;
	m_WaitTimer = 0;
	m_WaitTimerCount = 0;

	m_DisDiddle = 0;

	m_AmpVal = 0;
}

void CFSKMOD::CalcBPF(void)
{
	MakeFilter(HBPF, m_bpftap, ffBPF, SampFreq, m_MarkFreq - 150, m_SpaceFreq + 150, 60, 1.0);
}

void CFSKMOD::SetMarkFreq(double d)
{
	m_MarkFreq = d;
	vco.SetFreeFreq(m_SpaceFreq);
	vco.SetGain(m_MarkFreq - m_SpaceFreq);
	CalcBPF();
}

void CFSKMOD::SetSpaceFreq(double d)
{
	m_SpaceFreq = d;
	vco.SetFreeFreq(m_SpaceFreq);
	vco.SetGain(m_MarkFreq - m_SpaceFreq);
	CalcBPF();
}

void CFSKMOD::SetSampFreq(double f)
{
	m_SampFreq = sys.m_SampFreq + sys.m_TxOffset;
	vco.SetSampleFreq(m_SampFreq);
	SetBaudRate(m_BaudRate);
	SetLPFFreq(m_LPFFreq);
}

void CFSKMOD::SetBaudRate(double b)
{
	if( b >= 1.0 ){
		m_BaudRate = b;
		m_Count = m_ReCount = int(m_SampFreq/b + 0.5);
		m_Amp.SetMax(m_Count);
	}
}

void CFSKMOD::SetLPFFreq(double f)
{
	m_LPFFreq = f;
	avgLPF.SetCount(int(m_SampFreq/f + 0.5));
}

void CFSKMOD::PutData(int d)
{
	if( m_cnt < MODBUFMAX ){
		m_idle = 0;
		m_Buff[m_wp] = BYTE(d);
		m_wp++;
		if( m_wp >= MODBUFMAX ) m_wp = 0;
		m_cnt++;
	}
}

void CFSKMOD::OutTone(int sw, int bsize)
{
	if( sw ){
		m_out = 1;
		m_Count = m_ReCount/2;
	}
	else {
		m_out = -1;
		m_Count = m_ReCount * (1 + ((220 * bsize)/(m_SampFreq)));
	}
}

double CFSKMOD::Do(int echo){
	if( m_CharWaitDiddle && m_CharWaitCount ){
		m_CharWaitCount--;
	}
	if( m_DisDiddle > 0 ) m_DisDiddle--;
	if( sys.m_TxPort && m_FSKCount ) m_FSKCount--;
	if( !m_Count ){
		m_Count = m_ReCount;
		switch(m_mode){
			case 0:		// スタートビット出力
				if( sys.m_TxPort == txTXD ) FSKDeff = m_cnt - FSKCount;
				if( !m_CharWaitDiddle ){
					if( m_CharWaitCount ){
						m_CharWaitCount--;
						m_out = 1;
						m_Count = 1;
						break;
					}
					else if( m_CharWait && m_cnt ){
						m_CharWaitCount = m_CharWait * m_ReCount/3;
					}
				}
_try:;
				if( m_cnt &&
					((!m_CharWaitDiddle) || (!m_CharWaitCount)) &&
					((sys.m_TxPort != txTXD) || (FSKCount <= m_cnt) || (m_FSKCount) || (m_Buff[m_rp]==0xff) || (m_Buff[m_rp]==0xfe) )
					 ){
					m_WaitTimerCount = 4;
					m_idle = 0;
					m_out = 0;
					if( m_figout && (m_fig == 0x1b) && (m_Buff[m_rp]!=0x1f) ){
						m_Data = 0x1b;
						if( !echo ){
							pDem->WriteData(m_Data);
						}
					}
					else {
						if( sys.m_TxPort == txTXDOnly ){		// FSK Only時の同期
#if 0
							while( FSKCount1 == FSKCount2 ){
								::Sleep(10);
							}
#endif
							FSKCount2++;
						}
						m_Data = m_Buff[m_rp];
						if( m_Data == 0xff ){		// マーク信号送信
							m_out = 1;
							m_Count = m_ReCount * 3;
							m_cnt--;
							m_rp++;
							if( m_rp >= MODBUFMAX ) m_rp = 0;
							m_CharWaitCount = 0;
							break;
						}
						else if( m_Data == 0xfe ){	// キャリア切断
							m_out = -1;
							m_Count = m_ReCount * 3;
							m_cnt--;
							m_rp++;
							if( m_rp >= MODBUFMAX ) m_rp = 0;
							m_CharWaitCount = 0;
							break;
						}
						else if( m_Data == 0xfd ){	// Dis Diddle
							m_DisDiddle = -1;
							m_cnt--;
							m_rp++;
							if( m_rp >= MODBUFMAX ) m_rp = 0;
							goto _try;
						}
						else if( m_Data == 0xfc ){	// Enb Diddle
							m_DisDiddle = 0;
							m_cnt--;
							m_rp++;
							if( m_rp >= MODBUFMAX ) m_rp = 0;
							goto _try;
						}
						else if( (m_Data == 0x1b)||(m_Data == 0x1f) ){
							m_fig = m_Data;
						}
						m_cnt--;
						m_rp++;
						if( m_rp >= MODBUFMAX ) m_rp = 0;
						if( !echo ){
							pDem->WriteData(m_Data);
						}
					}
					m_figout = 0;
					m_DataCount = m_BitLen;
					m_SumParity = 0;
					m_mode++;
				}
				else if( m_Data == 0x00fe ){
					m_idle = 1;
					m_out = -1;
					m_Count = m_ReCount * 50;
					m_Data = 0;
					break;
				}
				else if( (m_BitLen < 6) && ((m_diddle && !m_DisDiddle) || ((sys.m_TxPort == txTXD)&& m_cnt)) ){
//				else if( m_diddle && (!m_DisDiddle || ((sys.m_TxPort == txTXD)&& m_cnt) ) ){
//				else if( m_diddle && !m_DisDiddle ){
#if 0
					if( sys.m_TxPort == txTXDOnly ){
						::Sleep((m_ReCount * m_BitLen)*1000/m_SampFreq);
					}
#endif
					if( !m_cnt ) m_idle = 1;
					m_out = 0;
					switch(m_diddle){
						case 1:		// BLK
							if( m_RandomDiddle && !(rand() & 3) ){
								m_Data = 0x1f;
								if( m_BitLen <= 6 ) m_figout = 1;
							}
							else {
								m_Data = 0x00;
							}
							break;
						default:	// LTR
							if( m_RandomDiddle && !(rand() & 3) ){
								m_Data = 0x00;
							}
							else {
								m_Data = 0x1f;
								if( m_BitLen <= 6 ) m_figout = 1;
							}
							break;
					}
					m_DataCount = m_BitLen;
					m_SumParity = 0;
					m_mode++;
					if( !m_CharWaitDiddle ){
						if( (sys.m_LWait == 2) || m_cnt ){
							m_CharWaitCount = m_CharWait * m_ReCount/3;
						}
						else if( m_DiddleWait ){
							m_CharWaitCount = m_DiddleWait * m_ReCount/3;
						}
						else {
							m_CharWaitCount = 0;
						}
						if( sys.m_LWait != 2 ){
							if( m_WaitTimer ){
								if( !m_WaitTimerCount ){
									m_CharWaitCount = 0;
								}
								else {
									m_WaitTimerCount--;
								}
							}
						}
					}
				}
				else {
					if( !m_cnt ) m_idle = 1;
					m_out = 1;
					m_Count = 1;
#if 0
					if( sys.m_TxPort == txTXDOnly ){
						::Sleep(11);
						m_Count = m_ReCount;
					}
#endif
				}
				if( m_CharWaitDiddle && (!m_CharWaitCount) ){
					if( m_Data != 0x1b ){
						m_CharWaitCount = m_CharWait * m_ReCount/3;
					}
				}
				break;
			case 1:		// 符号出力
				if( m_DataCount ){
					switch(m_BitLen){
						case 6:
							m_out = (m_Data & 0x20) ? 1 : 0;
							break;
						case 7:
							m_out = (m_Data & 0x40) ? 1 : 0;
							break;
						case 8:
							m_out = (m_Data & 0x80) ? 1 : 0;
							break;
						default:
							m_out = (m_Data & 0x10) ? 1 : 0;
							break;
					}
					m_Data = BYTE(m_Data << 1);
					m_DataCount--;
					if( m_out ) m_SumParity++;
				}
				else {	// ストップビット
					if( m_Parity ){
						m_mode++;
						switch(m_Parity){
							case 1:		// Even
								m_out = m_SumParity & 1 ? 0 : 1;
								break;
							case 2:		// Odd
								m_out = m_SumParity & 1 ? 1 : 0;
								break;
							case 3:
								m_out = 1;
								break;
							case 4:
								m_out = 0;
								break;
						}
					}
					else {
						m_mode++;
						goto _nx;
					}
				}
				break;
			case 2:
_nx:;
				m_out = 1;
				m_mode++;
				switch(m_StopLen){
					case 0:		// 1bit
						m_Count = 1;
						break;
					case 1:		// 1.5bit
					case 3:
					case 4:
						m_Count = (m_ReCount/2) - 1;
						break;
					case 2:		// 2bit
						m_Count = m_ReCount - 1;
						break;
				}
				break;
			case 3:		// ストップビット終了
				m_mode = 0;
				break;
		}
	}
	m_Count--;
	double d;
	if( m_lpf ){	// GMSK
		if( m_inv ){
			d = m_out ? 0 : 1;
		}
		else {
			d = m_out;
		}
		d = avgLPF.Avg(d);
		d = vco.Do(d) * m_OutputGain;
	}
	else {			// AFSK
		if( m_inv ){
			d = vco.Do(m_out ? 0 : 1) * m_OutputGain;
		}
		else {
			d = vco.Do(m_out) * m_OutputGain;
		}
	}
	if( m_out < 0 ) d = 0;
	if( m_bpf ){
		d = DoFIR(HBPF, ZBPF, d, m_bpftap);
	}
	return d * m_Amp.Do(m_AmpVal);
}


//---------------------------------------------------------------------------
// CFSKDEMクラス
CFSKDEM::CFSKDEM()
{
	m_OverFlow = 0;
	m_lpf = 0;
	m_lpfOrder = 5;
	SetLPFFreq(40.0);

	m_pll.SetSampleFreq(DemSamp);
    m_Phase.SetSampleFreq(DemSamp);

	memset(ZMark, 0, sizeof(ZMark));
	memset(ZSpace, 0, sizeof(ZSpace));
	memset(HMark, 0, sizeof(HMark));
	memset(HSpace, 0, sizeof(HSpace));
	i2 = 0;
//	o1 = o2 = 0;
	Count = 0;

	SetSmoozFreq(70.0);
	avgMark.SetCount(m_Smooz);
	avgSpace.SetCount(m_Smooz);

	m_BitLen = 5;
	m_StopLen = 4;		// 0-1bit, 1-1.5bit, 2-2bit, 3-1bit, 4-1.42bit
	m_Parity = 0;
	m_SumParity = 0;

	m_mode = 0;
	m_DataCount = 0;

	m_inv = 0;
	m_sq = 0;
	m_SQLevel = 600.0;
	m_Tap = 72;
	m_FilWidth = GetFilWidth(m_Tap);

	m_wp = 0;
	m_rp = 0;
	m_BufCount = 0;

	SetBaudRate(45.45);

	m_MarkFreq = 2125;
	m_SpaceFreq = 2295;
	SetMarkFreq(2125);
	SetSpaceFreq(2295);

	m_dMark = 0;
	m_dSpace = 0;

	m_Scope = 0;
	m_majority = 1;
	m_ignoreFream = 0;
	m_XYScope = 0;
	SetIIR(60.0);
	m_Limit = 1;
	m_LimitGain = 10.0;

	m_deff = 0;
	m_avgdeff = 0;

	m_sqcount = 0;
//	SmoozSQ.SetCount(4);
	SmoozSQ.SetCount(8);

	m_LimitOverSampling = 0;

	m_Tick = 0;
	m_atcPLL.m_Max = 12;

	m_LimitAGC = 1;
	m_limitMax = 1;
	m_limitMin = -1;
	m_d = 0;
}

void CFSKDEM::SetIIR(double bw)
{
	m_iirfw = bw;
	m_iirm.SetFreq(m_AFCMarkFreq, DemSamp, m_iirfw);
	m_iirs.SetFreq(m_AFCSpaceFreq, DemSamp, m_iirfw);
	m_pll.SetFreeFreq(m_AFCMarkFreq, m_AFCSpaceFreq);
}

void CFSKDEM::SetBaudRate(double b)
{
	if( b >= 1.0 ){
		m_BaudRate = b;
		m_Count = m_ReCount = int(DemSamp/b + 0.5);
	}
}

void CFSKDEM::SetSmoozFreq(double f)
{
	m_SmoozFreq = f;
	m_Smooz = int(DemSamp / f + 0.5);

	avgMark.SetCount(m_Smooz);
	avgSpace.SetCount(m_Smooz);
}

void CFSKDEM::SetLPFFreq(double f)
{
	m_lpffreq = f;
	LpfMark.MakeIIR(f, DemSamp, m_lpfOrder, 0, 1.0);
	LpfSpace.MakeIIR(f, DemSamp, m_lpfOrder, 0, 1.0);
}

void CFSKDEM::SetMarkFreq(double d)
{
	if ((m_AFCSpaceFreq - d) < 4000) {    //1.70D prevent divide by zero in M_SlideFFT.Create
		m_SetMarkFreq = m_AFCMarkFreq = m_MarkFreq = d;
		MakeFilter(HMark, m_Tap, ffBPF, DemSamp, m_MarkFreq-m_FilWidth, m_MarkFreq+m_FilWidth, 60, 1.0);
		SetIIR(m_iirfw);
		m_Phase.SetCarrierFreq(m_AFCMarkFreq);
		if( m_AFCSpaceFreq > m_AFCMarkFreq ) m_Phase.SetShift(m_AFCSpaceFreq - m_AFCMarkFreq);
		m_AA6YQ.SetMarkFreq(m_AFCMarkFreq);
	}
}

void CFSKDEM::SetSpaceFreq(double d)
{
	if ((d - m_AFCMarkFreq) < 4000) {    //1.70D prevent divide by zero in M_SlideFFT.Create
		m_SetSpaceFreq = m_AFCSpaceFreq = m_SpaceFreq = d;
		MakeFilter(HSpace, m_Tap, ffBPF, DemSamp, m_SpaceFreq-m_FilWidth, m_SpaceFreq+m_FilWidth, 60, 1.0);
		SetIIR(m_iirfw);
		m_Phase.SetCarrierFreq(m_AFCMarkFreq);
		if( m_AFCSpaceFreq > m_AFCMarkFreq ) m_Phase.SetShift(m_AFCSpaceFreq - m_AFCMarkFreq);
		m_AA6YQ.SetSpaceFreq(m_AFCSpaceFreq);
	}
}

void CFSKDEM::AFCMarkFreq(double d)
{
	if( d != m_MarkFreq ){
		m_MarkFreq = d;
		if( fabs(d - m_AFCMarkFreq) >= (m_type ? 10.0 : 5.0) ){
			m_AFCMarkFreq = d;
			MakeFilter(HMark, m_Tap, ffBPF, DemSamp, m_MarkFreq-m_FilWidth, m_MarkFreq+m_FilWidth, 60, 1.0);
			SetIIR(m_iirfw);
		    m_Phase.SetCarrierFreq(m_AFCMarkFreq);
		}
	    m_AA6YQ.SetMarkFreqByAFC(d);
	}
}

void CFSKDEM::AFCSpaceFreq(double d)
{
	if( d != m_SpaceFreq ){
		m_SpaceFreq = d;
		if( fabs(d - m_AFCSpaceFreq) >= (m_type ? 10.0 : 5.0) ){
			m_AFCSpaceFreq = d;
			MakeFilter(HSpace, m_Tap, ffBPF, DemSamp, m_SpaceFreq-m_FilWidth, m_SpaceFreq+m_FilWidth, 60, 1.0);
			SetIIR(m_iirfw);
		}
	    m_AA6YQ.SetSpaceFreqByAFC(d);
	}
}

double CFSKDEM::GetFilWidth(int tap)
{
	double Width;

	if( tap >= 256 ){
		Width = 20;
	}
	else if( tap >= 192 ){
		Width = 30;
	}
	else {
		Width = 40;
	}
	return Width;
}

void CFSKDEM::SetFilterTap(int tap)
{
	if( tap != m_Tap ){
		m_Tap = tap;
		m_FilWidth = GetFilWidth(tap);
		MakeFilter(HSpace, m_Tap, ffBPF, DemSamp, m_SpaceFreq-m_FilWidth, m_SpaceFreq+m_FilWidth, 60, 1.0);
		MakeFilter(HMark, m_Tap, ffBPF, DemSamp, m_MarkFreq-m_FilWidth, m_MarkFreq+m_FilWidth, 60, 1.0);
	}
}

void CFSKDEM::WriteData(BYTE d)
{
	if( m_BufCount < DEMBUFMAX ){
		m_Buff[m_wp] = d;
		m_wp++;
		m_BufCount++;
		if( m_wp >= DEMBUFMAX ) m_wp = 0;
	}
}

void CFSKDEM::DoFSK(void)
{
	int b;

	if( m_Tick ){
		Tick.Write(m_dMark);
		return;
	}
	if( m_dMark >= m_dSpace ){	// マークの時
		b = m_inv ? 0 : 1;
	}
	else {						// スペースの時
		b = m_inv ? 1 : 0;
	}
	double deff = fabs(m_dMark - m_dSpace);

// Adjustment for the level meter, added by JE3HHT (Ver 1.68A)
	if( m_AA6YQ.m_fEnabled && (m_type != 3) ){
		deff *= 4;
       	deff -= 30000;
    }
// till end

	if( m_deff < deff ) m_deff = deff;
	if( !m_sqcount ){
		m_avgdeff = SmoozSQ.Avg(m_deff);
		m_deff = 0;
		m_sqcount = DemSamp / 10;
	}
	m_sqcount--;

	if( m_sq ){
		if( !m_mode ){
			if( m_Limit  ){		// 受信時
				if( (m_SQLevel * 10.0) > m_avgdeff ){
					b = 1;
				}
			}
			else {				// 送信時
				if( m_SQLevel > m_avgdeff ){
					b = 1;
				}
			}
		}
	}
//	m_Bit = b;
	if( m_Scope ){
		m_ScopeSync.WriteData(m_Count ? 0 : 8192.0);
		m_ScopeBit.WriteData(b ? 8192.0 : 0);
	}
	switch(m_mode){
		case 0:			// スタートビット検出待ち
			if( !b ){
				m_Count = m_ReCount/2;
				if( m_majority ){
					m_mark = m_space = 0;
					m_mode = 256;	// 多数決ロジック
				}
				else {
					m_mode++;
				}
#if BITDEBUG
				m_bitCountA = m_bitCount;
				m_bitCount = 0;
#endif
			}
			else {
				m_Count = m_ReCount;
			}
			break;
// 通常ロジックによる判定
		case 1:			// スタートビット検出待ち(Half)
			if( b ){
				m_mode = 0;
			}
			else if( !m_Count ){
				if( m_Scope ){
					m_ScopeSync.UpdateData(-8192.0);	// スタートビット位置
				}
				m_Count = m_ReCount;
				m_Data = 0;
				m_DataCount = m_BitLen;
				m_SumParity = 0;
				m_mode++;
			}
			break;
		case 2:			// 符号記録中
			if( !m_Count ){
				m_Count = m_ReCount;
				m_Data = BYTE(m_Data << 1);
				m_Data |= BYTE(b);
				m_DataCount--;
				if( !m_DataCount ){
					if( m_Parity ){
						m_mode++;
					}
					else {
						m_mode += 2;
					}
				}
				if( m_Parity ){
					m_SumParity += b;
				}
			}
			break;
		case 3:			// パリティビット
			if( !m_Count ){
				m_Count = m_ReCount;
				m_mode++;
				switch(m_Parity){
					case 1:			// Even
						if( (!(m_SumParity & 1) ^ b) & 1 ) m_mode = 0;		// パリティエラー
						break;
					case 2:			// Odd
						if( ((m_SumParity & 1) ^ b) & 1 ) m_mode = 0;		// パリティエラー
						break;
					case 3:
						if( !b ) m_mode = 0;
						break;
					case 4:
						if( b ) m_mode = 0;
						break;
					default:
						break;
				}
				if( !m_mode ){	// パリティエラー
					m_Count = m_ReCount;
					m_mode = 7;
				}
			}
			break;
		case 4:			// ストップビットの確認
			if( !m_Count ){
				if( m_Scope ){
					m_ScopeSync.UpdateData(-4096.0);	// Stopbit位置
				}
				if( b  || m_ignoreFream ){
					if( m_BufCount < DEMBUFMAX ){
						m_Buff[m_wp] = m_Data;
						m_wp++;
						m_BufCount++;
						if( m_wp >= DEMBUFMAX ) m_wp = 0;
					}
					switch(m_StopLen){
						case 2:		// 2bit
							m_Count = (m_ReCount * 11/8);		// 23/16bit時間のウエイト
							break;
						case 1:		// 1.5bit
							m_Count = (m_ReCount * 7/8);		// 15/16bit時間のウエイト
							break;
						case 4:		// 1.42bit
							m_Count = (m_ReCount * 4/5);		// 0.82bit時間のウエイト
							break;
						case 3:
						default:	// 1bit
							m_Count = (m_ReCount * 3/8);		// 7/16bit時間のウエイト
							break;
					}
					m_mode++;
				}
				if( !b ){			// フレーミングエラー
					m_mode = 8;
				}
			}
			break;
		case 5:			// ストップビット終了待ちタイマー
			if( !m_Count ){
				if( m_Scope ){
					m_ScopeSync.UpdateData(-4096.0);	// Stopbits終了位置
				}
				m_mode++;
			}
			break;
		case 6:			// ストップ終了待ち
			if( b ){
				m_Count = m_ReCount;
			}
			else {		// 次のスタートビット
				m_Count = m_ReCount/2;
				m_mode = 0;
			}
			break;
		case 7:			// パリティエラー時
			if( !m_Count ){
				m_ScopeSync.UpdateData(-4096.0);	// Stopbit位置
				m_mode--;
			}
			break;
		case 8:			// フレーミングエラー時
			m_Count = m_ReCount;
			if( b ){
				m_mode = 0;
			}
			break;
// 多数決ロジックによるデータ取りこみ
		case 256:		// スタートビット検出待ち(Half)
			if( b ){m_mark++;} else {m_space++;}
			if( !m_Count ){
				b = (m_mark >= m_space) ? 1 : 0;
				if( b ){
					m_mode = 0;
				}
				else {
					if( m_Scope ){
						m_ScopeSync.UpdateData(-8192.0);	// スタートビット位置
					}
					m_Count = m_ReCount/2;
					m_mode++;
				}
			}
			break;
		case 257:		// スタートビット終了待ち
			if( !m_Count ){
				if( m_Scope ){
					m_ScopeSync.UpdateData(-8192.0);	// スタートビット位置
				}
				m_Count = m_ReCount;
				m_mark = m_space = 0;
				m_Data = 0;
				m_DataCount = m_BitLen;
				m_SumParity = 0;
				m_mode++;
			}
			break;
		case 258:			// 符号記録中
			if( b ){m_mark++;} else {m_space++;}
			if( !m_Count ){
				b = (m_mark >= m_space) ? 1 : 0;
				m_mark = m_space = 0;
				m_Count = m_ReCount;
				m_Data = BYTE(m_Data << 1);
				m_Data |= BYTE(b);
				m_DataCount--;
				if( !m_DataCount ){
					if( m_Parity ){
						m_mode++;
					}
					else {
						switch(m_StopLen){
							case 2:		// 2bit
							case 1:		// 1.5bit
							case 4:		// 1.42bit
								break;
							case 3:
							default:	// 1bit
								m_Count = m_ReCount * 7 / 8;
								break;
						}
						m_mode += 2;
					}
				}
				if( m_Parity ){
					m_SumParity += b;
				}
			}
			break;
		case 259:			// パリティビット
			if( b ){
				m_mark++;
			}
			else {
				m_space++;
			}
			if( !m_Count ){
				b = (m_mark >= m_space) ? 1 : 0;
				m_mark = m_space = 0;
				m_Count = m_ReCount;
				m_mode++;
				switch(m_Parity){
					case 1:			// Even
						if( (!(m_SumParity & 1) ^ b) & 1 ) m_mode = 0;		// パリティエラー
						break;
					case 2:			// Odd
						if( ((m_SumParity & 1) ^ b) & 1 ) m_mode = 0;		// パリティエラー
						break;
					case 3:
						if( !b ) m_mode = 0;
						break;
					case 4:
						if( b ) m_mode = 0;
						break;
					default:
						break;
				}
				if( !m_mode ){	// パリティエラー
					m_Count = m_ReCount;
					m_mode = 7;
				}
				else {
					switch(m_StopLen){
						case 2:		// 2bit
						case 1:		// 1.5bit
						case 4:		// 1.42bit
							break;
						case 3:
						default:	// 1bit
							m_Count = m_ReCount * 7 / 8;
							break;
					}
				}
			}
			break;
		case 260:			// ストップビットの確認
			if( b ){
				m_mark++;
			}
			else {
				m_space++;
			}
			if( !m_Count ){
				b = (m_mark >= m_space) ? 1 : 0;
				m_mark = m_space = 0;
				if( m_Scope ){
					m_ScopeSync.UpdateData(-4096.0);	// Stopbit位置
				}
				if( b || m_ignoreFream ){
					if( m_BufCount < DEMBUFMAX ){
						m_Buff[m_wp] = m_Data;
						m_wp++;
						m_BufCount++;
						if( m_wp >= DEMBUFMAX ) m_wp = 0;
					}
					switch(m_StopLen){
						case 2:		// 2bit
							m_Count = (m_ReCount * 7/8);
							break;
						case 1:		// 1.5bit
							m_Count = (m_ReCount * 3/8);
							break;
						case 4:		// 1.42bit
							m_Count = (m_ReCount * 2/5);
							break;
						case 3:
						default:	// 1bit
							m_Count = m_ReCount;
							if( m_Scope ){
								m_ScopeSync.UpdateData(-4096.0);	// Stopbits終了位置
							}
							m_mode = -1;
							break;
					}
					m_mode++;
				}
				if( !b ){		// フレーミングエラー
					m_mode = 8;
				}
			}
			break;
		case 261:			// ストップビット終了待ちタイマー
			if( !m_Count ){
				if( m_Scope ){
					m_ScopeSync.UpdateData(-4096.0);	// Stopbits終了位置
				}
				m_mode++;
			}
			break;
		case 262:			// ストップ終了待ち
			if( b ){
				m_Count = m_ReCount;
			}
			else {		// 次のスタートビット
				m_Count = m_ReCount/2;
				m_mode = 0;
			}
			break;
	};
	m_Count--;
#if BITDEBUG
	m_bitCount++;
#endif
}

void CFSKDEM::Do(double d)
{
	if( m_AA6YQ.m_fEnabled ) d = m_AA6YQ.Do(d);
	double ds = d;

	if( (d > 24578.0) || (d < -24578.0) ){
		m_OverFlow = 1;
	}
	if( m_Limit ){
		if( m_LimitAGC ){
			if( m_limitMax < d ) m_limitMax = d;
			if( m_limitMin > d ) m_limitMin = d;
			if( (d >= 0) && (m_d < 0) ){
				m_limitagc = (m_limitMax - m_limitMin);
				if( m_limitagc ){
					m_limitagc = (64.0 * 16384.0)/ m_limitagc;
					if( m_limitagc >= 4096.0 ) m_limitagc = 4096;
				}
				else {
					m_limitagc = 200;
				}
				m_limitMax = 1;
				m_limitMin = -1;
			}
			m_d = d;
			if( m_LimitOverSampling ){
				d = OverLimit.Do(d, m_limitagc);
			}
			else {
				d *= m_limitagc;
				if( d > 16384.0 ) d = 16384.0;
				if( d < -16384.0 ) d = -16384.0;
			}
		}
		else {
			if( m_LimitOverSampling ){
				d = OverLimit.Do(d, m_LimitGain);
			}
			else {
				d *= m_LimitGain;
				if( d > 16384.0 ) d = 16384.0;
				if( d < -16384.0 ) d = -16384.0;
			}
		}
//		if( m_AA6YQ.m_fEnabled ) d *= 0.3333;	// Delete by JE3HHT (Ver1.68A)
	}
	if( (Count & 1) || (!DemOver) ){
		if( DemOver ) d = DECM2.Do(d, i2);		// 1/2デシメータ

		switch(m_type){
			case 2:			// PLL
			{
				double dm = m_iirm.Do(d);
				double ds = m_iirs.Do(d);
				d = m_pll.Do(d);
				if( m_XYScope ){	// For XY-Scope
					m_XYScopeMark.WriteData(dm);
					m_XYScopeSpace.WriteData(ds);
				}
				if( m_Scope  ){	// For Scope
					m_ScopeMark[0].WriteData(dm);
					m_ScopeSpace[0].WriteData(ds);
				}
				if( m_Scope ){
					m_ScopeMark[1].WriteData(m_pll.GetOut());
					m_ScopeSpace[1].WriteData(d);
				}
				d = m_atcPLL.Do(d + 8192.0) - 8192.0;
				if( d >= 0 ){
					m_dMark = d;
					m_dSpace = 0;
				}
				else {
					m_dMark = 0;
					m_dSpace = -d;
				}
				// 平滑
				if( m_lpf ){
					m_dMark = LpfMark.Do(m_dMark);
					m_dSpace = LpfSpace.Do(m_dSpace);
				}
				else {
					m_dMark = avgMark.Avg(m_dMark);
					m_dSpace = avgSpace.Avg(m_dSpace);
				}
				if( m_Scope ){
					m_ScopeMark[2].WriteData(m_dMark);
					m_ScopeSpace[2].WriteData(m_dSpace);
				}
				if( m_atc ){
					m_dMark = m_atcMark.Do(m_dMark);
					m_dSpace = m_atcSpace.Do(m_dSpace);
					if( m_Scope ){
						m_ScopeMark[3].WriteData(m_dMark);
						m_ScopeSpace[3].WriteData(m_dSpace);
					}
				}
				break;
            }
			case 3:			// FFT
            {
				m_Phase.DoFSK(ds);
                m_dMark = m_Phase.m_dm;
                m_dSpace = m_Phase.m_ds;

				if( m_XYScope || m_Scope ){
					double dm = m_iirm.Do(d);
					double ds = m_iirs.Do(d);

					if( m_XYScope ){	// For XY-Scope
						m_XYScopeMark.WriteData(dm);
						m_XYScopeSpace.WriteData(ds);
					}
					if( m_Scope  ){		// For Scope
						m_ScopeMark[0].WriteData(dm);
						m_ScopeSpace[0].WriteData(ds);
					}
                }
				// 検波
//				m_dMark = fabs(m_dMark);
//				m_dSpace = fabs(m_dSpace);
				if( m_dMark < 0.0 ) m_dMark = -m_dMark;
				if( m_dSpace < 0.0 ) m_dSpace = -m_dSpace;
				if( m_Scope ){
					m_ScopeMark[1].WriteData(m_dMark);
					m_ScopeSpace[1].WriteData(m_dSpace);
				}
				// 平滑
				if( m_lpf ){
					m_dMark = LpfMark.Do(m_dMark);
					m_dSpace = LpfSpace.Do(m_dSpace);
				}
				else {
					m_dMark = avgMark.Avg(m_dMark);
					m_dSpace = avgSpace.Avg(m_dSpace);
				}
				if( m_Scope ){
					m_ScopeMark[2].WriteData(m_dMark);
					m_ScopeSpace[2].WriteData(m_dSpace);
				}
				if( m_atc ){
					m_dMark = m_atcMark.Do(m_dMark);
					m_dSpace = m_atcSpace.Do(m_dSpace);
					if( m_Scope ){
						m_ScopeMark[3].WriteData(m_dMark);
						m_ScopeSpace[3].WriteData(m_dSpace);
					}
				}
#if 1			// 高速計算
				m_dMark /= (64*32768.0);
                m_dSpace /= (64*32768.0);
#else
				m_dMark = m_dMark > 0.0 ? sqrt(m_dMark) : 0.0;
                m_dSpace = m_dSpace > 0.0 ? sqrt(m_dSpace) : 0.0;
				m_dMark *= (1.0/16.0);
                m_dSpace *= (1.0/16.0);
#endif
				break;
            }
			default:
            {
				if( m_type ){			// FIR
					m_dMark = DoFIR(HMark, ZMark, d, m_Tap);
					m_dSpace = DoFIR(HSpace, ZSpace, d, m_Tap);
				}
				else {					// IIR
					m_dMark = m_iirm.Do(d);
					m_dSpace = m_iirs.Do(d);
				}
				if( m_XYScope ){	// For XY-Scope
					m_XYScopeMark.WriteData(m_dMark);
					m_XYScopeSpace.WriteData(m_dSpace);
				}
				if( m_Scope  ){		// For Scope
					m_ScopeMark[0].WriteData(m_dMark);
					m_ScopeSpace[0].WriteData(m_dSpace);
				}
				// 検波
//				m_dMark = fabs(m_dMark);
//				m_dSpace = fabs(m_dSpace);
				if( m_dMark < 0 ) m_dMark = -m_dMark;
				if( m_dSpace < 0 ) m_dSpace = -m_dSpace;
				if( m_Scope ){
					m_ScopeMark[1].WriteData(m_dMark);
					m_ScopeSpace[1].WriteData(m_dSpace);
				}
				// 平滑
				if( m_lpf ){
					m_dMark = LpfMark.Do(m_dMark);
					m_dSpace = LpfSpace.Do(m_dSpace);
				}
				else {
					m_dMark = avgMark.Avg(m_dMark);
					m_dSpace = avgSpace.Avg(m_dSpace);
				}
				if( m_Scope ){
					m_ScopeMark[2].WriteData(m_dMark);
					m_ScopeSpace[2].WriteData(m_dSpace);
				}
				if( m_atc ){
					m_dMark = m_atcMark.Do(m_dMark);
					m_dSpace = m_atcSpace.Do(m_dSpace);
					if( m_Scope ){
						m_ScopeMark[3].WriteData(m_dMark);
						m_ScopeSpace[3].WriteData(m_dSpace);
					}
				}
				break;
            }
        }
		DoFSK();
	}
	else {
		i2 = d;
	}
	Count++;
}

int CFSKDEM::GetData(void)
{
	int r;
	if( m_BufCount ){
		r = m_Buff[m_rp];
		m_BufCount--;
		m_rp++;
		if( m_rp >= DEMBUFMAX ) m_rp = 0;
	}
	else {
		r = -1;
	}
	return r;
}


BCODETBL _TTY[]={			// S-BELL
		// 0x20 - 0x7f
		{0x04, 2}, {0x16, 1}, {0x11, 1}, {0x00, 2}, //  !"#
		{0x12, 1}, {0x00, 2}, {0x0b, 1}, {0x1a, 1}, // $%&'
		{0x1e, 1}, {0x09, 1}, {0x00, 2}, {0x00, 2}, // ()*+
		{0x06, 1}, {0x18, 1}, {0x07, 1}, {0x17, 1}, // ,-./
		{0x0d, 1}, {0x1d, 1}, {0x19, 1}, {0x10, 1},	// 0123	30-33
		{0x0a, 1}, {0x01, 1}, {0x15, 1}, {0x1c, 1},	// 4567	34-37
		{0x0c, 1}, {0x03, 1}, {0x0e, 1}, {0x0f, 1},	// 89	38-3b
		{0x00, 2}, {0x00, 2}, {0x00, 2}, {0x13, 1},	// <=>?	3c-3f
		{0x00, 2}, {0x18, 0}, {0x13, 0}, {0x0e, 0},	// @ABC	40-43
		{0x12, 0}, {0x10, 0}, {0x16, 0}, {0x0b, 0},	// DEFG
		{0x05, 0}, {0x0c, 0}, {0x1a, 0}, {0x1e, 0},	// HIJK
		{0x09, 0}, {0x07, 0}, {0x06, 0}, {0x03, 0},	// LMNO
		{0x0d, 0}, {0x1d, 0}, {0x0a, 0}, {0x14, 0},	// PQRS
		{0x01, 0}, {0x1c, 0}, {0x0f, 0}, {0x19, 0},	// TUVW
		{0x17, 0}, {0x15, 0}, {0x11, 0}, {0x00, 2},	// XYZ[ 58-5b
		{0x00, 2}, {0x00, 2}, {0x00, 2}, {0x00, 2}, // \]^_
		{0x00, 2}, {0x18, 0}, {0x13, 0}, {0x0e, 0},	// @ABC	60-63
		{0x12, 0}, {0x10, 0}, {0x16, 0}, {0x0b, 0},	// DEFG
		{0x05, 0}, {0x0c, 0}, {0x1a, 0}, {0x1e, 0},	// HIJK
		{0x09, 0}, {0x07, 0}, {0x06, 0}, {0x03, 0},	// LMNO
		{0x0d, 0}, {0x1d, 0}, {0x0a, 0}, {0x14, 0},	// PQRS
		{0x01, 0}, {0x1c, 0}, {0x0f, 0}, {0x19, 0},	// TUVW
		{0x17, 0}, {0x15, 0}, {0x11, 0}, {0x00, 2},	// XYZ[ 78-7b
		{0x00, 2}, {0x00, 2}, {0x00, 2}, {0x00, 2},	// |{~
};

const char _LTR[32]={
	0x00, 'T', 0x0d, 'O', ' ', 'H', 'N', 'M',
	0x0a, 'L', 'R', 'G', 'I', 'P', 'C', 'V',
	'E', 'Z', 'D', 'B', 'S', 'Y', 'F', 'X',
	'A', 'W', 'J', 0x00, 'U', 'Q', 'K', 0x00,
};
const char _FIG[32]={
	0x00, '5', 0x0d, '9', ' ', 'h', ',', '.',
	0x0a, ')', '4', '&', '8', '0', ':', ';',
	'3', '"', '$', '?', 's', '6', '!', '/',
	'-', '2', 0x27, 0x00, '7', '1', '(', 0x00,
};

CRTTY::CRTTY()
{
	m_outfig = 3;

	m_fig = 0;
	m_uos = 0;

	m_txuos = 1;
	SetCodeSet();
}

void CRTTY::SetCodeSet(void)
{
	memcpy(m_TBL, _TTY, sizeof(m_TBL));
	if( sys.m_CodeSet ){		// J-BELL
		m_TBL[7].Code = 0x14;
	}
}

// 11011 FIG
// 11111 LTR
char CRTTY::ConvAscii(int d)
{
	char c = 0;
	d &= 0x1f;
	if( d == 0x1b ){		// FIG
		m_fig = 1;
	}
	else if( d == 0x1f ){	// LTR
		m_fig = 0;
	}
	else if( m_fig ){
		c = _FIG[d];
		if( sys.m_CodeSet ){
			switch(c){
				case 's':
					c = 0x27;
					break;
				case 0x27:
					c = 'j';
					break;
			}
		}
		if( m_uos ){
			switch(c){
				case ' ':
//                case 0x0d:
//                case 0x0a:
					m_fig = 0;
					break;
				default:
					break;
			}
		}
	}
	else {
		c = _LTR[d];
	}
	return c;
}

int CRTTY::ConvRTTY(char d)
{
	int fig = 2;
	int r = 0;
	switch(d){
		case '_':
			r = 0x00ff;
			break;
		case '~':
			r = 0x00fe;
			break;
		case '[':
			r = 0x00fd;
			break;
		case ']':
			r = 0x00fc;
			break;
		case 0x0a:
			r = 0x08;
			break;
		case 0x0d:
			r = 0x02;
			break;
		case 0x1b:
			r = 0x1b;
			fig = 1;
			break;
		case 0x1f:
			r = 0x1f;
			fig = 0;
			break;
		default:
			if( d >= 0x20 ){
				d -= char(0x20);
				r = m_TBL[d].Code;
				fig = m_TBL[d].Fig;
			}
			break;
	}
	if( fig != 2 ){
		if( fig != m_outfig ){
			r |= (fig ? 0x1b00 : 0x1f00);
			m_outfig = fig;
		}
	}
	else if( r == 0x04 ){	// スペースの時
		if( sys.m_txuos && (m_outfig == 1) ) m_outfig = 2;
	}
	return r;
}

int CRTTY::ConvRTTY(BYTE *t, LPCSTR p)
{
	int n;
	int d;
	for( n = 0;*p; p++ ){
		d = ConvRTTY(*p);
		if( d & 0x0000ff00 ){
			*t++ = BYTE(d >> 8);
			n++;
			if( sys.m_dblsft ){
				*t++ = BYTE(d >> 8);
				n++;
			}
		}
		*t++ = BYTE(d);
		n++;
	}
	return n;
}

int CRTTY::GetShift(char d)
{
	int fig = 2;
	switch(d){
		default:
			if( d >= 0x20 ){
				d -= char(0x20);
				fig = m_TBL[d].Fig;
				if( !m_TBL[d].Code ){
					fig = 2;
				}
			}
			break;
	}
	return fig;
}

char CRTTY::InvShift(char c)
{
	int fs = GetShift(c);
	if( (c == 'h') || (c == 's') || (c == 'j') ) fs = 1;
	int d = ConvRTTY(c) & 0x001f;
	switch(fs){
		case 0:
			if( _FIG[d] ){
				c = _FIG[d];
				if( sys.m_CodeSet ){
					switch(c){
						case 's':
							c = 0x27;
							break;
						case 0x27:
							c = 'j';
							break;
					}
				}
			}
			return c;
		case 1:
			return _LTR[d] ? _LTR[d] : c;
		default:
			return c;
	}
}

//--------------------------------------------------------
// CScopeクラス
CScope::CScope()
{
	m_ScopeSize = SCOPESIZE;
	m_DataFlag = 0;

	pScopeData = new double[m_ScopeSize];
	memset(pScopeData, 0, sizeof(double)*m_ScopeSize);
}

CScope::~CScope()
{
	delete[] pScopeData;
}

void CScope::WriteData(double d)
{
	if( !m_DataFlag ){
		if( m_wp < m_ScopeSize ){
			pScopeData[m_wp] = d;
			m_wp++;
			if( m_wp >= m_ScopeSize ){
				m_DataFlag = 1;
			}
		}
	}
}

void CScope::UpdateData(double d)
{
	if( !m_DataFlag ){
		if( m_wp ){
			pScopeData[m_wp-1] = d;
		}
	}
}

void CScope::Collect(int size)
{
	m_DataFlag = 1;
	m_ScopeSize = size;
	m_wp = 0;
	m_DataFlag = 0;
}



//--------------------------------------------------------
// CNoiseクラス
CNoise::CNoise()
{
	reg = 0x12345;

	memset(Z, 0, sizeof(Z));
	MakeFilter(H, NOISEBPFTAP, ffLPF, SampFreq, 3000.0, 3000.0, 60, 1.0);
};

double CNoise::GetNoise(void)
{
	DWORD r = reg >> 1;
	if( (reg ^ r) & 1 ){
		r |= 0xffe00000;
	}
	else {
		r &= 0x001fffff;
	}
	reg = r;
	double d = double(reg) / 500000.0;
//    return d;
	return DoFIR(H, Z, d, NOISEBPFTAP);		// 帯域制限
}

//--------------------------------------------------------
// CSamplePeakクラス
CSamplePeak::CSamplePeak()
{
	memset(Strage, 0, sizeof(Strage));
	m_CurPeak = 0.0;
	m_Peak = 0.0;
	m_Strage = 8 - 1;
	SetBaudRate(45.45);
}

void CSamplePeak::SetBaudRate(double b)
{
	if( b >= 1.0 ){
		m_ReCount = m_Count = int(DemSamp/b + 0.5);
	}
}

void CSamplePeak::Sync(int Delay)
{
	m_Count = m_ReCount - Delay;
	while( m_Count < 0 ) m_Count += m_ReCount;
	memcpy(Strage, &Strage[1], sizeof(double)*m_Strage);
	Strage[m_Strage] = m_CurPeak;
	m_Peak = 0.0;
	int i;
	for( i = 0; i <= m_Strage; i++ ){
		if( m_Peak < Strage[i] ) m_Peak = Strage[i];
	}
	m_CurPeak = 0.0;
}

int CSamplePeak::Sample(double d)
{
	int r = 0;

	if( m_CurPeak < d ){
		m_CurPeak = d;
		if( m_Peak < d ){
			m_Peak = d;
			r = 1;
		}
	}
	if( !m_Count ){
		Sync(0);
		r = 1;
	}
	m_Count--;
	return r;
}

//--------------------------------------------------------
// CAGCクラス
CAGC::CAGC()
{
	m_MaxGain = 2048.0;
	m_StepGain = 3.0;
	m_MarkGain = 1.0;
	m_SpaceGain = 1.0;
	m_DeffGain = 12.0;
	m_Sync = 1;
}

void CAGC::Sync(int Delay)
{
	if( m_Sync ){
		Mark.Sync(Delay);
		Space.Sync(Delay);
	}
}

double CAGC::SampleMark(double d)
{
	if( Mark.Sample(d) ){
		double gain = Mark.GetPeak();
		if( gain ){
			gain = 8192.0 / gain;
			double ugain = m_MarkGain * m_StepGain;
			double dgain = m_MarkGain / m_StepGain;
			if( ugain < gain ){
				m_MarkGain = ugain;
			}
			else if( dgain > gain ){
				m_MarkGain = dgain;
			}
			else {
				m_MarkGain = gain;
			}
			if( m_MarkGain > m_MaxGain ) m_MarkGain = m_MaxGain;
		}
		else {
			m_MarkGain = 1.0;
		}
	}
	return d * m_MarkGain;
}

double CAGC::SampleSpace(double d)
{
	if( Space.Sample(d) ){
		double gain = Space.GetPeak();
		if( gain ){
			gain = 8192.0 / gain;
			double ugain = m_SpaceGain * m_StepGain;
			double dgain = m_SpaceGain / m_StepGain;
			if( ugain < gain ){
				m_SpaceGain = ugain;
			}
			else if( dgain > gain ){
				m_SpaceGain = dgain;
			}
			else {
				m_SpaceGain = gain;
			}
			if( m_SpaceGain > m_MaxGain ) m_SpaceGain = m_MaxGain;
			if( m_SpaceGain > m_MarkGain ){
				if( m_SpaceGain > m_MarkGain * m_DeffGain ){
					m_SpaceGain = m_MarkGain * m_DeffGain;
				}
			}
			else {
				if( m_SpaceGain * m_DeffGain < m_MarkGain ){
					m_SpaceGain = m_MarkGain / m_DeffGain;
				}
			}
		}
		else {
			m_SpaceGain = 1.0;
		}
	}
	return d * m_SpaceGain;
}

//--------------------------------------------------------
// CATCクラス
CATC::CATC()
{
	m_Low = 0;
	m_High = 16384.0;
	m_CurLow = MAXDOUBLE;
	m_CurHigh = -MAXDOUBLE;
	m_Max = 4;
	m_Cnt = 0;
	int i;
	for( i = 0; i < ATCMAX; i++ ){
		m_LowList[i] = m_Low;
		m_HighList[i] = m_High;
	}
	m_iir.MakeIIR(100, DemSamp, 3, 0, 0);
}

double CATC::Do(double d)
{
	if( m_CurLow > d ) m_CurLow = d;
	if( m_CurHigh < d ) m_CurHigh = d;
//		if( m_Low > d ) m_Low = d;
//        if( m_High < d ) m_High = d;
	if( !m_Cnt ){
		m_Cnt = 64;
		if( m_CurLow > (ATCC-ATCW) ) m_CurLow = (ATCC-ATCW);
		if( m_CurHigh < (ATCC+ATCW) ) m_CurHigh = (ATCC+ATCW);
		if( m_Max ){
			memcpy(m_LowList, &m_LowList[1], (m_Max)*sizeof(double));
			memcpy(m_HighList, &m_HighList[1], (m_Max)*sizeof(double));
		}
		m_LowList[m_Max] = m_CurLow;
		m_HighList[m_Max] = m_CurHigh;
		int i;
		m_Low = m_LowList[0];
		m_High = m_HighList[0];
		for( i = 1; i <= m_Max; i++ ){
			if( m_Low > m_LowList[i] ) m_Low = m_LowList[i];
			if( m_High < m_HighList[i] ) m_High = m_HighList[i];
		}
		m_CurLow = MAXDOUBLE;
		m_CurHigh = -MAXDOUBLE;
	}
	m_Cnt--;
	double th = ((m_High + m_Low)*0.5);
	th = m_iir.Do(th);
	if( m_High > m_Low ){
		d = (d - th) * 1.1 + th;
	}
	d += (ATCC - th);
	return d;
}

//---------------------------------------------------------------------------
// CFIRクラス
__fastcall CFIR::CFIR()
{
	m_pZ = NULL;
    m_pH = NULL;
    m_Tap = 0;
}

//---------------------------------------------------------------------------
__fastcall CFIR::~CFIR()
{
	if( m_pZ ) delete[] m_pZ;
	if( m_pH ) delete[] m_pH;
}
//---------------------------------------------------------------------------
void __fastcall CFIR::Create(int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	m_Tap = tap;
	if( m_pZ ) delete[] m_pZ;
    if( m_pH ) delete[] m_pH;
	m_pZ = new double[tap+1];
	m_pH = new double[tap+1];
    memset(m_pZ, 0, sizeof(double)*(tap+1));
	::MakeFilter(m_pH, tap, type, fs, fcl, fch, att, gain);
}
//---------------------------------------------------------------------------
double __fastcall CFIR::Do(double d)
{
	return DoFIR(m_pH, m_pZ, d, m_Tap);
}
//---------------------------------------------------------------------------
void __fastcall CFIR::SaveCoef(LPCSTR pName)
{
	FILE *fp;
    if( (fp = fopen(pName, "wt")) != NULL ){
		int i;
		for( i = 0; i <= m_Tap; i++ ){
			fprintf(fp, "H[%u]=%lf\n", i, m_pH[i]);
        }
		fclose(fp);
    }
}

//---------------------------------------------------------------------------
// CFIR2クラス
__fastcall CFIR2::CFIR2()
{
	m_pZ = NULL;
    m_pH = NULL;
    m_pZP = NULL;
    m_W = 0;
    m_Tap = 0;
    m_fs = 0;
}

//---------------------------------------------------------------------------
__fastcall CFIR2::~CFIR2()
{
	if( m_pZ ) delete[] m_pZ;
	if( m_pH ) delete[] m_pH;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Delete(void)
{
	if( m_pZ ) delete[] m_pZ;
    if( m_pH ) delete[] m_pH;
	m_pZ = NULL;
    m_pH = NULL;
    m_pZP = NULL;
    m_W = 0;
    m_Tap = 0;
    m_fs = 0;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Create(int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	if( (m_Tap != tap) || !m_pZ || !m_pH ){
		if( m_pZ ) delete[] m_pZ;
		m_pZ = new double[(tap+1)*2];
		memset(m_pZ, 0, sizeof(double)*(tap+1)*2);
		if( m_pH ) delete[] m_pH;
		m_pH = new double[tap+1];
		m_W = 0;
    }
	m_Tap = tap;
    m_TapHalf = tap/2;
    m_fs = fs;
	::MakeFilter(m_pH, tap, type, fs, fcl, fch, att, gain);
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Create(int tap, double fs, double fcl, double fch)
{
	if( (m_Tap != tap) || !m_pZ || !m_pH ){
		if( m_pZ ) delete[] m_pZ;
		m_pZ = new double[(tap+1)*2];
		memset(m_pZ, 0, sizeof(double)*(tap+1)*2);
		if( m_pH ) delete[] m_pH;
		m_pH = new double[tap+1];
		m_W = 0;
    }
	m_Tap = tap;
    m_TapHalf = tap/2;
    m_fs = fs;
	::MakeHilbert(m_pH, tap, fs, fcl, fch);
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::CreateSamp(int tap, double fs, const double *pSmpFQ)
{
	if( (m_Tap != tap) || !m_pZ || !m_pH ){
		if( m_pZ ) delete[] m_pZ;
		m_pZ = new double[(tap+1)*2];
		memset(m_pZ, 0, sizeof(double)*(tap+1)*2);
		if( m_pH ) delete[] m_pH;
		m_pH = new double[tap+1];
        m_W = 0;
    }

    int htap = tap/2;
    int i, j;
    double *pSamp = new double[tap+1];
    memcpy(pSamp, pSmpFQ, sizeof(double)*(tap/2));
    for( i = 0; i < tap/2; i++ ){
		pSamp[tap-i] = pSamp[i];
    }
    pSamp[tap/2] = pSamp[tap/2 - 1];
    double *pH = new double[tap+1];
    double re, fm;

	for( i = 0; i <= htap; i++ ){
		re = 0.0;
		for( j = 0; j < tap; j++ ){
			fm = 2.0 * PI * double((i*j)%tap)/double(tap);
			re += pSamp[j] * cos(fm);
		}
		pH[i] = re / tap;
    }
#if 0
	fm = 0;
    for( i = 0; i <= htap; i++ ){
		fm += pH[i];
    }
    fm = 0.5 / fm;
    for( i = 0; i <= htap; i++ ){
		pH[i] *= fm;
    }
#endif
	for( i = 0; i <= htap; i++ ) m_pH[htap-i] = pH[i];
	for( i = 0; i < htap; i++ ) m_pH[tap-i] = m_pH[i];
	delete[] pH;
    delete[] pSamp;

	m_Tap = tap;
    m_TapHalf = htap;
    m_fs = fs;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Clear(void)
{
	if( m_pZ ) memset(m_pZ, 0, sizeof(double)*(m_Tap+1)*2);
    m_W = 0;
}
//---------------------------------------------------------------------------
double __fastcall CFIR2::Do(double d)
{
	double *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
    d = 0;
    double *hp = m_pH;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp1--) * (*hp++);
    }
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
    return d;
}
//---------------------------------------------------------------------------
double __fastcall CFIR2::Do(double *hp)
{
    double d = 0;
    double *dp = m_pZP;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp--) * (*hp++);
    }
    return d;
}
//---------------------------------------------------------------------------
void __fastcall CFIR2::Do(CLX &z, double d)
{
	double *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
    d = 0;
    double *hp = m_pH;
    for( int i = 0; i <= m_Tap; i++ ){
		d += (*dp1--) * (*hp++);
    }
    z.j = d;
    z.r = m_pZ[m_W+m_TapHalf+1];
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
}


//---------------------------------------------------------------------------
// CFIRXクラス
__fastcall CFIRX::CFIRX()
{
	m_pZ = NULL;
    m_pH = NULL;
    m_pZP = NULL;
    m_W = 0;
    m_Tap = 0;
    m_fs = 0;
}

//---------------------------------------------------------------------------
__fastcall CFIRX::~CFIRX()
{
	if( m_pZ ) delete[] m_pZ;
	if( m_pH ) delete[] m_pH;
}
//---------------------------------------------------------------------------
void __fastcall CFIRX::Create(int tap, int type, double fs, double fcl, double fch, double att, double gain)
{
	if( (m_Tap != tap) || !m_pZ || !m_pH ){
		if( m_pZ ) delete[] m_pZ;
		m_pZ = new CLX[(tap+1)*2];
		memset(m_pZ, 0, sizeof(CLX)*(tap+1)*2);
	    if( m_pH ) delete[] m_pH;
		m_pH = new double[tap+1];
		m_W = 0;
    }
	m_Tap = tap;
    m_TapHalf = tap/2;
    m_fs = fs;
	::MakeFilter(m_pH, tap, type, fs, fcl, fch, att, gain);
}
//---------------------------------------------------------------------------
void __fastcall CFIRX::Clear(void)
{
	if( m_pZ ) memset(m_pZ, 0, sizeof(CLX)*(m_Tap+1)*2);
    m_W = 0;
}
//---------------------------------------------------------------------------
void __fastcall CFIRX::Do(CLX &d)
{
	CLX *dp1 = &m_pZ[m_W+m_Tap+1];
	m_pZP = dp1;
	*dp1 = d;
    m_pZ[m_W] = d;
	CLX z = 0;
    double *hp = m_pH;
    for( int i = 0; i <= m_Tap; i++, dp1-- ){
		z.r += dp1->r * (*hp);
		z.j += dp1->j * (*hp++);
    }
    m_W++;
	if( m_W > m_Tap ) m_W = 0;
    d = z;
}

/*=============================================================================
  CSlideFFTクラス  スライディング　FFT
=============================================================================*/
#define	SLIDE_WINDOW_COEFF		0.9999
__fastcall CSlideFFT::CSlideFFT(void)
{
	m_Length = 0;
	m_Base = 0;
	m_Tones = 0;
	m_kWindow = 0;
	m_pBase = NULL;
    m_pCur = m_pEnd = NULL;
}

//--------------------------------------------------------------------------
__fastcall CSlideFFT::~CSlideFFT()
{
	if( m_pBase ) delete[] m_pBase;
}

//--------------------------------------------------------------------------
void __fastcall CSlideFFT::Create(int len, int base, int tones)
{
#if LOGFFT
	FILE *fp = fopen("SLIDERFFT.txt", "wt");
	fprintf(fp, "len=%d, base=%d, tones=%d\n", len, base, tones);
	fclose(fp);
#endif
	if( !m_pBase || (len != m_Length) ){
		if( m_pBase ) delete[] m_pBase;
		m_pBase = new CLX[len];
	}

    memset(m_pBase, 0, sizeof(CLX)*len);
    memset(m_tWindow, 0, sizeof(m_tWindow));
    memset(m_tData, 0, sizeof(m_tData));

	m_Length = len;
	m_Base = base;
    m_Tones = tones;

	double k = 2.0 * PI / double(len);
	for(int i = 0; i < tones; i++){
		m_tWindow[i].r = cos((i+base) * k) * SLIDE_WINDOW_COEFF;
		m_tWindow[i].j = sin((i+base) * k) * SLIDE_WINDOW_COEFF;
	}
	m_kWindow = pow(SLIDE_WINDOW_COEFF, len);
    m_pCur = m_pBase;
    m_pEnd = &m_pBase[m_Length];
}

//--------------------------------------------------------------------------
CLX* __fastcall CSlideFFT::Do(const CLX &zIn)
{
	CLX z;

	if( m_pCur >= m_pEnd ) m_pCur = m_pBase;
	z = *m_pCur;
	*m_pCur = zIn;
    m_pCur++;
	z *= m_kWindow;

	CLX *pData = m_tData;
	CLX *pWindow = m_tWindow;
	for( int i = 0; i < m_Tones; i++, pData++ ){
		*pData -= z;
		*pData += zIn;
		*pData *= *pWindow++;
	}
	return m_tData;
}

/*=============================================================================
  CPHASEクラス
=============================================================================*/
__fastcall CPHASE::CPHASE()
{
	m_TONES = 4;
	m_SHIFT = 170.0;
	m_SampleFreq = 11025.0*0.5;
    m_CarrierFreq = 1750;
    m_MixerFreq = 0;
    SetSampleFreq(m_SampleFreq);
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::ShowPara(void)
{
/*
	if( Application->MainForm ){
		char bf[256];
	    sprintf(bf, "Car=%.lf, Shift=%.lf", m_CarrierFreq, m_SHIFT);
		Application->MainForm->Caption = bf;
    }
*/
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::SetSampleFreq(double f)
{
	m_SampleFreq = f;
    Create();
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::Create(void)
{
//	m_fftSHIFT = m_SHIFT * (m_TONES + 1) / m_TONES;
	m_fftSHIFT = m_SHIFT * m_TONES / (m_TONES - 1);
	m_SymbolLen = m_TONES * m_SampleFreq / m_fftSHIFT;
    m_BASEPOINT = int(CPHASE_BASEFREQ * m_TONES / m_fftSHIFT);

	m_MixerFreq = double(m_BASEPOINT) * m_SampleFreq / m_SymbolLen;
	m_VCO.SetSampleFreq(m_SampleFreq);
	m_Hilbert.Create(20, m_SampleFreq, 25.0, m_SampleFreq*0.5 - 25.0);
    m_Hilbert.Clear();
    SetCarrierFreq(m_CarrierFreq);
    m_SlideFFT.Create(int(m_SymbolLen + 0.5), m_BASEPOINT, m_TONES);
	m_AGC.SetSampleFreq(m_SampleFreq);
	m_AGC.SetCarrierFreq(m_CarrierFreq);
//    m_LPF.Create(192, ffBEF, m_SampleFreq, 85.0 - 20, 85.0 + 20, 60, 1.0);
#if LOGFFT
    m_fp = fopen("FFT.txt", "wt");
#endif
//	ShowPara();
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::SetShift(double f)
{
	m_SHIFT = f;
    Create();
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::SetCarrierFreq(double f)
{
	m_CarrierFreq = f;
    m_VCO.SetFreeFreq(m_CarrierFreq - m_MixerFreq);
	m_AGC.SetCarrierFreq(m_CarrierFreq);
//	ShowPara();
}
//--------------------------------------------------------------------------
CLX* __fastcall CPHASE::Do(double d)
{
	m_Hilbert.Do(m_sig, d);					// 複素数化

    CLX z;
	z.r = m_VCO.Do();
    z.j = m_VCO.DoCos();
	z *= m_sig;								// 周波数変換

//	m_LPF.Do(z);
	return m_SlideFFT.Do(z);
}
//--------------------------------------------------------------------------
void __fastcall CPHASE::DoFSK(double d)
{
//	d = m_AGC.Do(d);
	CLX *pFFT = Do(d);
    m_dm = pFFT[0].vAbs();
    m_ds = pFFT[m_TONES-1].vAbs();

#if LOGFFT
    for( int i = 0; i < m_TONES; i++ ){
		if( i ) fprintf(m_fp, ",");
		fprintf(m_fp, "%.0lf", pFFT[i].vAbs());
    }
    fprintf(m_fp, "\n");
#endif
}
//--------------------------------------------------------------------------
void __fastcall DoAvg(double &av, double in, double factor)
{
	av = av * (1.0 - factor) + (in * factor);
}
//---------------------------------------------------------------------------
__fastcall CFAVG::CFAVG()
{
	Create(16);
}
//---------------------------------------------------------------------------
void __fastcall CFAVG::Reset(void)
{
	m_Cnt = 0;
    m_Sum = 0;
    m_Avg = 0;
}
//---------------------------------------------------------------------------
void __fastcall CFAVG::Reset(double d)
{
	m_Cnt = m_Max;
    m_Sum = d * m_Max;
    m_Avg = d;
}
//---------------------------------------------------------------------------
void __fastcall CFAVG::Create(int max)
{
	m_Max = max;
    m_Mul = 1.0 / m_Max;
    Reset();
}
//---------------------------------------------------------------------------
double __fastcall CFAVG::DoZ(double d)
{
	m_Sum += d;
    if( m_Cnt < m_Max ){
		m_Cnt++;
    }
    else {
		m_Sum -= m_Avg;
    }
    m_Avg = m_Sum * m_Mul;
    return m_Avg;
}
//---------------------------------------------------------------------------
double __fastcall CFAVG::Do(double d)
{
	m_Sum += d;
    if( m_Cnt < m_Max ){
		m_Cnt++;
	    m_Avg = m_Sum / m_Cnt;
    }
    else {
		m_Sum -= m_Avg;
	    m_Avg = m_Sum * m_Mul;
    }
    return m_Avg;
}

//--------------------------------------------------------
// CAGCクラス
__fastcall CAGCX::CAGCX()
{
	m_fc = 1000.0;
	m_MonitorFreq = SampFreq/2048;
    m_SampleFreq = SampFreq;
	m_CarrierFreq = 1750.0;
    m_LimitGain = 0.005;
    m_AvgOver.Create(4);
	Create();
}
//--------------------------------------------------------
void __fastcall CAGCX::Create(void)
{
	m_Count = 0;
	m_Max = -1.0;
	m_Min = 1.0;
	m_d = 0;
	m_agc = 1.0;
//    m_Level.Create(ffLPF, 0.3, m_MonitorFreq, 3, 0, 0);
	SetCarrierFreq(m_CarrierFreq);
}
//--------------------------------------------------------
void __fastcall CAGCX::SetCarrierFreq(double f)
{
	m_CarrierFreq = f;
	if( m_CarrierFreq >= 1000.0 ){
	    m_Gain = (m_CarrierFreq / 1000.0);

/*
500		0.003		333
1000	0.0035		285		1.0		1.0
1500	0.005		200		1.4		1.5
1800	0.006		166		1.71
2000	0.008		125     2.3		2.0
2100	0.01		100		2.85	2.1
2200	0.012       83      3.4		2.2
2300	0.015		66      4.3     2.3
2400	0.025       40      7.1		2.4
2500	0.028		36      7.9		2.5
2600	0.030       33      8.6		2.6
*/
        if( m_Gain >= 2.35 ){
			m_Gain *= 7.1 / 2.4;
        }
		else if( m_Gain >= 2.25 ){
			m_Gain *= 4.3 / 2.3;
        }
        else if( m_Gain >= 2.15 ){
        	m_Gain *= 3.4 / 2.2;
        }
		else if( m_Gain >= 2.05 ){
			m_Gain *= 2.9 / 2.1;
        }
        else if( m_Gain >= 1.8 ){
			m_Gain *= 2.3 / 2.2;
        }
    }
    else {
		m_Gain = 1.0;
    }
    SetFC(m_fc);
}
//--------------------------------------------------------
void __fastcall CAGCX::SetFC(double fc)
{
	m_fc = fc;
    if( fc > m_CarrierFreq*0.45 ) fc = m_CarrierFreq*0.45;
//    m_LPF.Create(ffLPF, fc, m_CarrierFreq, 1, 0, 0);
    m_LPF.MakeIIR(fc, m_CarrierFreq, 1, 0, 0);
    m_d = 0;
    m_TLimit = m_SampleFreq*0.8/m_CarrierFreq;
    m_AvgOver.Reset(1.0);
}
//--------------------------------------------------------
void __fastcall CAGCX::Reset(void)
{
	m_Max = -1.0;
	m_Min = 1.0;
	m_agc = 1.0;
    m_d = 0;
    m_Count = 0;
    for( int i = 0; i < 12; i++ ){
		m_LPF.Do(1.0);
//    	m_Level.Do(0);
    }
    m_AvgOver.Reset(1.0);
}
//--------------------------------------------------------
double __fastcall CAGCX::Do(double d)
{
	if( m_Max < d ) m_Max = d;
	if( m_Min > d ) m_Min = d;
	if( (d >= 0) && (m_d < 0) && (m_Count >= m_TLimit) ){
		double amp = m_Max - m_Min;
        if( amp > 0.1 ){
		    m_agc = m_LPF.Do(5.0/amp);
    	    if( m_agc >= 1.0 ) m_agc = 1.0;
			m_Max = -32768.0;
			m_Min = 32768.0;
        }
        m_Count = 0;
	}
    m_Count++;
	m_d = d;
	d *= m_agc;
    if( d > 2.5 ){ d = 2.5; } else if( d < -2.5 ){ d = -2.5; }
    return d;
}
//--------------------------------------------------------
BOOL __fastcall CAGCX::GetOver(void)
{
// 5.0 / (65536 * 0.666)
	return (m_AvgOver.Do(m_agc) < 0.0001146);
//	return (m_agc < 0.0001146);
}
/********************************************************
CAMPCONT class by JE3HHT on Sep.2010
*********************************************************/
__fastcall CAMPCONT::CAMPCONT()
{
    m_Max = g_SinTable.m_Size / 4;
    m_iMax = g_SinTable.m_Size / 4;
    m_Cnt = m_Max;
    m_S = 0;
    SetMax(16);
}
//---------------------------------------------------------------------------
void __fastcall CAMPCONT::SetMax(int max)
{
    m_ADD = m_Max / double(max);
}
//---------------------------------------------------------------------------
void __fastcall CAMPCONT::Reset(void)
{
	m_Cnt = m_Max;
    m_S = 0;
}
//---------------------------------------------------------------------------
double __fastcall CAMPCONT::Do(int s)
{
	if( s != m_S ){
		m_Cnt = 0.0;
	    m_S = s;
    }
	int r = m_Cnt;
    if( r >= m_Max ) return s;
    m_Cnt += m_ADD;
    if( s ){
		return g_SinTable.m_tSin[r];
    }
    else {
		return g_SinTable.m_tSin[r+m_iMax];
    }
}
/********************************************************
CAA6YQ class by JE3HHT on Sep.2010
*********************************************************/
__fastcall CAA6YQ::CAA6YQ(void)
{
	m_fEnabled = FALSE;
   	m_bpfTaps = 512;
	m_bpfFW = 35.0;
    m_befTaps = 256;
    m_befFW = 15.0;
    m_afcERR = 5.0;

	m_dblMark = 2125.0;
    m_dblSpace = 2295.0;
	m_dblMarkAFC = m_dblMark;
    m_dblSpaceAFC = m_dblSpace;
	Create();
}
//--------------------------------------------------------
void __fastcall CAA6YQ::Create(void)
{
    m_BPF.Create(m_bpfTaps, ffBPF, SampFreq, m_dblMark-m_bpfFW, m_dblSpace+m_bpfFW, 60.0, 1.0);
	double fc = (m_dblMark + m_dblSpace)/2.0;
    m_BEF.Create(m_befTaps, ffBEF, SampFreq, fc-m_befFW, fc+m_befFW, 10.0, 1.0);
}
//--------------------------------------------------------
void __fastcall CAA6YQ::SetMarkFreq(double f)
{
	m_dblMark = f;
	m_dblMarkAFC = f;
	if( m_fEnabled ) Create();
}
//--------------------------------------------------------
void __fastcall CAA6YQ::SetSpaceFreq(double f)
{
	m_dblSpace = f;
    m_dblSpaceAFC = f;
	if( m_fEnabled ) Create();
}
//--------------------------------------------------------
void __fastcall CAA6YQ::SetMarkFreqByAFC(double f)
{
	m_dblMark = f;
	double df = fabs(f - m_dblMarkAFC);
    if( df >= m_afcERR ){
		m_dblMarkAFC = f;
		if( m_fEnabled ) Create();
    }
}
//--------------------------------------------------------
void __fastcall CAA6YQ::SetSpaceFreqByAFC(double f)
{
	m_dblSpace = f;
	double df = fabs(f - m_dblSpaceAFC);
    if( df >= m_afcERR ){
		m_dblSpaceAFC = f;
		if( m_fEnabled ) Create();
    }
}
//--------------------------------------------------------
double __fastcall CAA6YQ::Do(double d)
{
	return m_BEF.Do(m_BPF.Do(d));
}
//--------------------------------------------------------


