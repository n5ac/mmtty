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
#ifndef FilterH
#define FilterH
//---------------------------------------------------------------------------
#include <Classes.hpp>

#include "Wave.h"
#include "fft.h"
#include "rtty.h"
//---------------------------------------------------------------------------
class CWaveFile
{
private:
	HMMIO	m_Handle;
	int		m_dis;
	BYTE	m_Head[4];
public:
	long	m_length;
	long	m_pos;
	int		m_mode;
	int		m_pause;
	int		m_autopause;
	AnsiString	m_FileName;

	__fastcall CWaveFile();
	__fastcall ~CWaveFile();
	void __fastcall FileClose(void);
	void __fastcall Rec(LPCSTR pName);
	BOOL __fastcall Play(LPCSTR pName);
	void __fastcall Rewind(void);
	void __fastcall ReadWrite(double *s, int size);
	long __fastcall GetPos(void);
	void __fastcall Seek(int n);
	int __fastcall ChangeSampFreq(LPCSTR tName, LPCSTR pName, int sSamp);
};


class TSound : public TThread
{
private:
	double			ZBPF[TAPMAX+1];

	BOOL			m_Stop;
	WAVEFORMATEX	m_WFX;

	int		m_FFTMax;
	int	    m_FFTSumMax;

	int		m_WaterMax;
	int	    m_WaterSumMax;
	int		m_WaterMin;

	CWave	Wave;
	CNoise	Noise;

	int		m_ReqFifoSize;

	void __fastcall ErrorMsg(void);
	double __fastcall GetScopeRange(double &low, double center, double shift);
	void __fastcall ReadWrite(double *s, int size);
	int __fastcall UpdateFifoSize(void);
	BOOL __fastcall ReInitSound(void);
	void __fastcall JobSuspend(void);

protected:
	void __fastcall Execute();


public:
	__fastcall TSound(bool CreateSuspended);
	__fastcall ~TSound();

	void __fastcall Stop(void);
	void __fastcall ReqStop(void);
	void __fastcall WaitStop(void);
	void __fastcall TimingWait(void);

	void __fastcall InitSound(void);
	double __fastcall GetScreenFreq(int x, int XW, int XRD);
	int __fastcall DrawFFT(Graphics::TBitmap *pBitmap, int sw, int XRD);
	int __fastcall DrawFFTWater(Graphics::TBitmap *pBitmap, int sw, int XRD);
	void __fastcall SetTXRX(int sw);
	int __fastcall DoAFC(void);

	void __fastcall CalcBPF(void);
	void __fastcall CalcBPF(double fl, double fh, double fw);
	void __fastcall TaskPriority(void);

    LPCSTR __fastcall GetInputSoundcard(unsigned int ID);
    LPCSTR __fastcall GetOutputSoundcard(unsigned int ID);

	CFSKDEM	FSKDEM;
	CFSKMOD	FSKMOD;

	CWaveFile	WaveFile;

	int		m_Test;

	void __fastcall FileClose(void);
	void __fastcall Rec(LPCSTR pName);
	void __fastcall Play(LPCSTR pName);
	void __fastcall Rewind(void);

	void __fastcall InitWFX(void);

	UINT	m_IDDevice;
    UINT    m_IDOutDevice; //AA6YQ 1.6.6

	int		m_playmode;
	int		m_susp;
	int		m_suspack;

	int		m_bpf;
	int		m_bpftap;
	int		m_bpfafc;
	double	m_bpffw;
	int		m_lmsbpf;
	CLMS	m_lms;
	double	HBPF[TAPMAX+1];

	int		m_Tx;
	int		m_ReqTx;

	CFFT	fftIN;

	int		m_Err;
	int		m_BuffSize;

	int		m_Noise;

	int		m_FFTSW;
	int		m_FFTFW;

	int		m_FFTWINDOW;

    int		m_ReqSpeedTest;
    DWORD	m_SpeedValue;
	void __fastcall JobSpeedTest();

	inline int __fastcall GetOutCount(int sw){
		return (sw ? Wave.GetOutBCC() : Wave.GetOutBC()) * m_BuffSize;
	}
	inline void __fastcall TrigBCC(void){
		Wave.SetOutBCC(Wave.GetOutBC());
	}
	inline int __fastcall GetBCC(void){
		return Wave.GetOutBCC();
	}
};
//---------------------------------------------------------------------------
#endif

