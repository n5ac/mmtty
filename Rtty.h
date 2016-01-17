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
#ifndef RttyH
#define RttyH
//---------------------------------------------------------------------------
#include "Fir.h"
#include "CLX.h"

#define	VERFFTDEM	1		// CPhaseX Version
#define	VERAA6YQ	2		// CAA6YQ Version

#define	BITDEBUG	0		// キャラクタ長の測定
#define	TXDDEBUG	0		// FSKのキャラクタ長の測定
#define	FSKDEBUG	0		// FSKのデバッグモード

#ifndef SampFreq
extern	double	SampFreq;
#endif

//---------------------------------------------------------------------------
// Added by JE3HHT on Aug.2010
class CFAVG
{
private:
    double	m_Sum;
    double	m_Avg;
    double	m_Mul;
	int		m_Max;
    int		m_Cnt;
public:
	__fastcall CFAVG();
	void __fastcall Reset(void);
	void __fastcall Reset(double d);
    void __fastcall Create(int max);
	double __fastcall DoZ(double d);
	double __fastcall Do(double d);
    inline double __fastcall GetAvg(void){return m_Avg;};
    inline BOOL __fastcall IsFull(void){return m_Max == m_Cnt;};
    inline BOOL __fastcall IsHalf(void){return m_Max <= (m_Cnt*2);};
    inline int __fastcall GetCount(void){return m_Cnt;};
};
//--------------------------------------------------------
// CAGCクラス
// Added by JE3HHT on Aug.2010
class CAGCX
{
private:
	double  m_Max;
	double  m_Min;
	double  m_d;
	double  m_agc;

	double	m_fc;
	double	m_Gain;
    CIIR	m_LPF;
//    CIIR	m_Level;
    int		m_Count;
    int		m_TLimit;
    CFAVG	m_AvgOver;
public:
    double	m_LimitGain;
	double	m_MonitorFreq;
	double	m_CarrierFreq;
    double	m_SampleFreq;
public:
	__fastcall CAGCX();
    void __fastcall Create(void);
	inline void __fastcall SetSampleFreq(double f){m_SampleFreq = f; Create();};
	void __fastcall SetCarrierFreq(double f);
    void __fastcall SetFC(double fc);
    void __fastcall Reset(void);
    double __fastcall Do(double d);
	inline void __fastcall ResetOver(void){m_AvgOver.Reset(1.0);};
    BOOL __fastcall GetOver(void);
};

//---------------------------------------------------------------------------
class CSinTable
{
public:
	double	*m_tSin;
	int		m_Size;
public:
	__fastcall CSinTable();
    __fastcall ~CSinTable();
};

//---------------------------------------------------------------------------
class CAMPCONT
{
private:
	double	m_Max;
    double	m_Cnt;
    double	m_ADD;
    int		m_S;
    int		m_iMax;
public:
	__fastcall CAMPCONT();
	void __fastcall SetMax(int max);
    void __fastcall Reset(void);
    double __fastcall Do(int d);

    inline __fastcall IsMax(void){return m_Cnt >= m_Max;};
};

//---------------------------------------------------------------------------
class CTICK
{
public:
	int		*ptbl[2];
	int		m_wsel;
	int		*m_wp;
	int		m_wcnt;
	int		m_Trig;
	int		m_Samp;
public:
	CTICK();
	~CTICK();
	void Init(void);
	void Write(double d);
	inline int GetTrig(void){return m_Trig;};
	int *GetData(void);
};


class CVCO
{
private:
	double	m_c1;	// VCOの利得
	double	m_c2;	// フリーランニング周波数
	int 	m_z;

	int		m_TableSize;
	double	*pSinTbl;
	double	m_FreeFreq;
	double	m_SampleFreq;
	int     m_vlock;
public:
	CVCO();
	~CVCO();
	void VirtualLock(void);
	void InitPhase(void);
	void SetGain(double gain);
	void SetSampleFreq(double f);
	void SetFreeFreq(double f);
	double Do(double d);
};

//---------------------------------------------------------------------------
// Added by JE3HHT on Aug.2010
class CVCOX
{
private:
	double	m_c1;	// VCOの利得
	double	m_c2;	// フリーランニング周波数
	double	m_z;

	double	m_FreeFreq;
	double	m_SampleFreq;
	int		m_TableSize;
    double	m_TableCOS;
public:
	__fastcall CVCOX();
	__fastcall ~CVCOX();
	void __fastcall InitPhase(void);
	void __fastcall SetGain(double gain);
	void __fastcall SetSampleFreq(double f);
	void __fastcall SetFreeFreq(double f);
	double __fastcall Do(void);
	double __fastcall Do(double d);
	double __fastcall DoCos(void);
};

class CSmooz{
private:
	double	*bp;
	int	Wp;
	int	Max;
	int	Cnt;
	inline void IncWp(void){
		Wp++;
		if( Wp >= Max ) Wp = 0;
	};
	inline double Avg(void){
		double	d = 0.0;
		int		i;
		for( i = 0; i < Cnt; i++ ){
			d += bp[i];
		}
		if( Cnt ){
			return d/double(Cnt);
		}
		else {
			return 0;
		}
	};
public:
	inline CSmooz(int max = 60){
		Max = max;
		bp = new double[max];
		Cnt = 0;
		Wp = 0;
	};
	inline ~CSmooz(void){
		delete[] bp;
	};
	inline void SetCount(int n){
		double *np = new double[n];
		Max = n;
		Cnt = Wp = 0;
		double *op = bp;
		bp = np;
		delete[] op;	//JA7UDE 0428
		//delete[] np;	//JA7UDE 0428
	};
	inline double Avg(double d){
		bp[Wp] = d;
		IncWp();
		if( Cnt < Max ){
			Cnt++;
		}
		return Avg();
	};
};

//---------------------------------------------------------------------------
// Added by JE3HHT on Aug.2010
class CFIR
{
private:
	int		m_Tap;
	double	*m_pZ;
    double	*m_pH;
public:
	__fastcall CFIR();
    __fastcall ~CFIR();
	void __fastcall Create(int tap, int type, double fs, double fcl, double fch, double att, double gain);
	double __fastcall Do(double d);
    void __fastcall SaveCoef(LPCSTR pName);
    inline double __fastcall GetHD(int n){return m_pH[n];};
    inline double *__fastcall GetHP(void){return m_pH;};
    inline int __fastcall GetTap(void){return m_Tap;};
};

//---------------------------------------------------------------------------
// ダブルバッファによるFIRフィルタ
// Added by JE3HHT on Aug.2010
class CFIR2
{
private:
	int		m_Tap;
    int		m_TapHalf;
	double	*m_pZ;
    double	*m_pH;
	double	*m_pZP;

    int		m_W;
    double	m_fs;
public:
	__fastcall CFIR2();
    __fastcall ~CFIR2();
	void __fastcall Delete(void);
	void __fastcall Clear(void);
	void __fastcall Create(int tap, int type, double fs, double fcl, double fch, double att, double gain);
	void __fastcall Create(int tap, double fs, double fcl, double fch);
	void __fastcall CreateSamp(int tap, double fs, const double *pSmpFQ);
	double __fastcall Do(double d);
	double __fastcall Do(double *hp);
	void __fastcall Do(CLX &z, double d);

    inline double __fastcall GetHD(int n){return m_pH[n];};
    inline double *__fastcall GetHP(void){return m_pH;};
    inline int __fastcall GetTap(void){return m_Tap;};
    inline double __fastcall GetSampleFreq(void){return m_fs;};
};
//---------------------------------------------------------------------------
// ダブルバッファによるFIRフィルタ(複素数用)
// Added by JE3HHT on Aug.2010
class CFIRX
{
private:
	int		m_Tap;
    int		m_TapHalf;
	CLX		*m_pZ;
    double	*m_pH;
	CLX		*m_pZP;

    int		m_W;
    double	m_fs;
public:
	__fastcall CFIRX();
    __fastcall ~CFIRX();
	void __fastcall Clear(void);
	void __fastcall Create(int tap, int type, double fs, double fcl, double fch, double att, double gain);
	void __fastcall Do(CLX &d);

    inline double __fastcall GetHD(int n){return m_pH[n];};
    inline double *__fastcall GetHP(void){return m_pH;};
    inline int __fastcall GetTap(void){return m_Tap;};
    inline double __fastcall GetSampleFreq(void){return m_fs;};
};
//---------------------------------------------------------------------------
// スライディングFFT
// Added by JE3HHT on Aug.2010
class CSlideFFT
{
private:
	int 	m_Length;
	int 	m_Base;
    int		m_Tones;

	CLX		m_tData[16];
	CLX 	m_tWindow[16];

    CLX		*m_pCur;
    CLX		*m_pEnd;
	CLX		*m_pBase;

	double	m_kWindow;
private:
public:
	__fastcall CSlideFFT();
	__fastcall ~CSlideFFT();

	void __fastcall Create(int len, int base, int tones);
	CLX* __fastcall Do(const CLX &zIn);
};

/*=============================================================================
  CPHASEクラス
=============================================================================*/
#define	CPHASE_BASEFREQ	0.0
class CPHASE
{
private:
//	CFIRX	m_LPF;
	CVCOX	m_VCO;
    CFIR2	m_Hilbert;
    CAGCX	m_AGC;

    FILE	*m_fp;
public:
    CLX		m_sig;
	CSlideFFT	m_SlideFFT;

    int			m_TONES;
	double		m_SHIFT;
    double		m_fftSHIFT;
	int			m_BASEPOINT;

	double	m_MixerFreq;
    double	m_SymbolLen;
	double	m_CarrierFreq;
	double	m_SampleFreq;

    double	m_dm;
    double	m_ds;
private:
public:
	__fastcall CPHASE();
    void __fastcall Create(void);
    void __fastcall SetSampleFreq(double f);
	void __fastcall SetCarrierFreq(double f);
	inline void __fastcall AddCarrierFreq(double f){
		SetCarrierFreq(m_CarrierFreq + f);
    };
	CLX* __fastcall Do(double d);
    inline CFIR2* __fastcall GetFIR(void){return &m_Hilbert;};

	void __fastcall DoFSK(double d);
    void __fastcall SetShift(double f);
	void __fastcall ShowPara(void);

    inline void SetTones(int n){
		m_TONES = n;
        Create();
    }
};

class CPLL
{
private:
	CIIR	loopLPF;
	CIIR	outLPF;

	double	m_err;
	double	m_out;
	double	m_vcoout;
	double	m_SampleFreq;
	double	m_FreeFreq;
	double	m_Shift;
public:
	int		m_loopOrder;
	double	m_loopFC;
	int		m_outOrder;
	double	m_outFC;
	double	m_vcogain;
public:
	CVCO	vco;
	inline CPLL(){
		m_err = 0;
		m_out = 0;
		m_vcoout = 0;
		m_vcogain = 3.0;
		m_SampleFreq = SampFreq * 0.5;
		m_Shift = 170.0;
		m_FreeFreq = (2125 + 2295)/2;
		m_loopOrder = 2;
		m_outOrder = 4;
		m_loopFC = 250.0;
		m_outFC = 200.0;
		SetFreeFreq(2125, 2295);
		SetSampleFreq(m_SampleFreq);
	};
	inline void SetVcoGain(double g){
		m_vcogain = g;
		vco.SetGain(-m_Shift * g);
	};
	inline void MakeLoopLPF(void){
		loopLPF.MakeIIR(m_loopFC, m_SampleFreq, m_loopOrder, 0, 0);
	}
	inline void MakeOutLPF(void){
		outLPF.MakeIIR(m_outFC, m_SampleFreq, m_outOrder, 0, 0);
	}
	inline void SetFreeFreq(double f1, double f2){
		m_FreeFreq = (f1 + f2)/2.0;
		m_Shift = f2 - f1;
		vco.SetFreeFreq(m_FreeFreq);
		vco.SetGain(-m_Shift * m_vcogain);
	}
	void SetSampleFreq(double f){
		m_SampleFreq = f;
		vco.SetSampleFreq(f);
		vco.SetFreeFreq(m_FreeFreq);
		SetVcoGain(3.0);
		MakeLoopLPF();
		MakeOutLPF();
	};
	double Do(double d){
		d /= 32768.0;
		// Loop Filter
		m_out = loopLPF.Do(m_err);
		// VCO
		m_vcoout = vco.Do(m_out);
		// 位相比較
		m_err = m_vcoout * d;
		return outLPF.Do(m_out) * 32768.0;
	};
	inline double GetErr(void){return m_err*32768;};	// Phase Det
	inline double GetOut(void){return m_out*32768;};	// Phase Det
	inline double GetVco(void){return m_vcoout;};
};


#define	SCOPESIZE	8192
class CScope
{
private:
	int		m_DataFlag;

	int		m_wp;

public:
	int		m_ScopeSize;
	double	*pScopeData;

public:
	CScope();
	~CScope();
	void WriteData(double d);
	void UpdateData(double d);
	inline int GetFlag(void){return m_DataFlag;};	//JA7UDE 0428, edited by JA7UDE (Jan 17, 2016)
	void Collect(int size);
};

#define	SAMPLEPEAKMAX	16
class CSamplePeak
{
private:
	double	Strage[SAMPLEPEAKMAX];
	double	m_CurPeak;
	double	m_Peak;

	int		m_Strage;
	int		m_Count;
	int		m_ReCount;
public:
	CSamplePeak();

	inline int GetStrage(void){ return m_Strage + 1;};
	inline void SetStrage(int n){
		if( n > SAMPLEPEAKMAX ) n = SAMPLEPEAKMAX;
		m_Strage = n ? (n - 1) : 0;
	};
	double GetPeak(void){return m_Peak;};
	void SetBaudRate(double b);
	void Sync(int Delay);
	int Sample(double d);
};

class CAGC
{
private:
	double	m_MarkGain;
	double	m_SpaceGain;
public:
	int		m_Sync;
	double	m_StepGain;
	double	m_MaxGain;
	double	m_DeffGain;
	CSamplePeak	Mark;
	CSamplePeak	Space;
public:
	CAGC();
	void Sync(int Delay);
	double SampleMark(double d);
	double SampleSpace(double d);

};

#define	ATCMAX	17
#define ATCC	(8192.0)
#define	ATCW	(1024.0)
class CATC
{
private:
	double	m_Low;
	double	m_High;
	double	m_CurLow;
	double	m_CurHigh;
	double	m_LowList[ATCMAX];
	double	m_HighList[ATCMAX];
	int		m_Cnt;
	CIIR	m_iir;
public:
	int		m_Max;
public:
	CATC();
	double Do(double d);
};

/********************************************************
CAA6YQ class by JE3HHT on Sep.2010
*********************************************************/
class CAA6YQ {		// Added by JE3HHT on Sep.2010
public:
	BOOL	m_fEnabled;
    int		m_bpfTaps;
    int		m_befTaps;
	double	m_bpfFW;
    double	m_befFW;
	double	m_afcERR;

    double	m_dblMark;
    double	m_dblSpace;
	double	m_dblMarkAFC;
    double	m_dblSpaceAFC;

	CFIR2	m_BPF;
    CFIR2	m_BEF;
private:
public:
	__fastcall CAA6YQ(void);
	void __fastcall Create(void);
	void __fastcall SetMarkFreq(double f);
	void __fastcall SetSpaceFreq(double f);
	void __fastcall SetMarkFreqByAFC(double f);
	void __fastcall SetSpaceFreqByAFC(double f);
	double __fastcall Do(double d);
};

#define	DEMBUFMAX	512
#define	DEMBPFTAP	36
class CFSKDEM
{
private:
	double	HMark[TAPMAX+1];
	double	ZMark[TAPMAX+1];
	double	HSpace[TAPMAX+1];
	double	ZSpace[TAPMAX+1];

	int		Count;		// デモジュレータディスパッチ
	double	i2;			// デモジュレータ2ndInput

	CDECM2	DECM2;		// デモジュレータ

	int		m_Smooz;
	CSmooz	avgMark;
	CSmooz	avgSpace;

	double	m_BaudRate;
	int		m_Count;
	int		m_ReCount;

	int		m_mode;
	int		m_inv;
	int		m_sq;
	double	m_SQLevel;

	int		m_Tap;
	double	m_FilWidth;
	double	m_MarkFreq;
	double	m_SpaceFreq;
	double	m_SetMarkFreq;
	double	m_SetSpaceFreq;
	double	m_AFCMarkFreq;
	double	m_AFCSpaceFreq;

	int		m_DataCount;
	BYTE	m_Data;

	int		m_BufCount;
	BYTE	m_Buff[DEMBUFMAX];
	int		m_wp;
	int		m_rp;

	double	m_dMark;
	double	m_dSpace;
	int		m_mark;
	int		m_space;

	double	m_SmoozFreq;

	int		m_SumParity;

	CIIRTANK	m_iirm;
	CIIRTANK	m_iirs;

	int		m_sqcount;
	CSmooz	SmoozSQ;

	COVERLIMIT	OverLimit;
public:
	int		m_type;
	double	m_iirfw;

	int 	m_OverFlow;
//	int		m_Bit;
	int		m_Scope;
	int		m_XYScope;

	int		m_BitLen;
	int		m_StopLen;
	int		m_Parity;

	int		m_majority;		// 多数決ロジック
	int		m_ignoreFream;	// フレーミングエラーの無視

	int		m_Limit;
	int		m_LimitAGC;
	double	m_LimitGain;

	double	m_limitMax;
	double	m_limitMin;
	double	m_d;
	double	m_limitagc;

	double	m_deff;
	double	m_avgdeff;

	CScope	m_XYScopeMark;
	CScope	m_XYScopeSpace;
	CScope	m_ScopeMark[4];
	CScope	m_ScopeSpace[4];
	CScope	m_ScopeSync;
	CScope	m_ScopeBit;

	CPLL	m_pll;

	int		m_lpf;
	int		m_lpfOrder;
	double	m_lpffreq;
	CIIR	LpfMark;
	CIIR	LpfSpace;

	int		m_LimitOverSampling;
	int		m_Tick;
	CTICK	Tick;
	int		m_atc;
	CATC	m_atcMark;
	CATC	m_atcSpace;
	CATC	m_atcPLL;

    CPHASE	m_Phase;
	CAA6YQ	m_AA6YQ;
#if BITDEBUG
	int		m_bitCount;
	int		m_bitCountA;
#endif
private:
	void DoFSK(void);


public:
	CFSKDEM();
	void Do(double d);

	inline void ClearMode(void){m_mode = 0;};
	double GetFilWidth(int tap);

	void SetIIR(double b);

	void AFCMarkFreq(double d);
	void AFCSpaceFreq(double d);
	void SetMarkFreq(double d);
	void SetSpaceFreq(double d);
	inline double GetMarkFreq(void){return m_MarkFreq;};
	inline double GetSpaceFreq(void){return m_SpaceFreq;};
	inline double GetSetMarkFreq(void){return m_SetMarkFreq;};
	inline double GetSetSpaceFreq(void){return m_SetSpaceFreq;};
	inline double GetAFCMarkFreq(void){return m_AFCMarkFreq;};
	inline double GetAFCSpaceFreq(void){return m_AFCSpaceFreq;};
	inline int GetRev(void){ return m_inv; };
	inline void SetRev(int inv){ m_inv = inv;};
	void SetBaudRate(double b);
	inline double GetBaudRate(void){return m_BaudRate;};
	inline double GetSmoozFreq(void){return m_SmoozFreq;};
	inline int GetSmoozCount(void){return m_Smooz;};
	void SetSmoozCount(int n);
	void SetSmoozFreq(double f);
	void SetLPFFreq(double f);
	inline int GetSQ(void){return m_sq;};
	inline void SetSQ(int sq){m_sq = sq;};
	inline double GetSQLevel(void){return m_SQLevel;};
	inline void SetSQLevel(double lvl){m_SQLevel = lvl;};

	inline int GetFilterTap(void){return m_Tap;};
	void SetFilterTap(int tap);
	inline int GetCount(void){return m_BufCount;};
	int GetData();
	inline void ClearRXBuf(void){
		m_BufCount = 0;
		m_rp = m_wp = 0;
		m_mode = 0;
	};
	void WriteData(BYTE d);
};

#define	MODBUFMAX	2048
class CFSKMOD
{
private:
	double	HBPF[TAPMAX+1];
	double	ZBPF[TAPMAX+1];

	CSmooz	avgLPF;
	double	m_LPFFreq;
	double	m_SampFreq;

	CVCO	vco;

	BYTE	m_Buff[MODBUFMAX];
	int		m_rp;
	int		m_wp;
	int		m_cnt;

	int		m_Count;
	int		m_out;

	int		m_inv;

	int		m_mode;
	int		m_DataCount;

	double	m_BaudRate;
	double	m_MarkFreq;
	double	m_SpaceFreq;

	double	m_OutputGain;

	BYTE	m_Data;
	int		m_FSKCount;

	int		m_SumParity;
	int		m_fig;
	int		m_figout;

	int		m_idle;
	CFSKDEM	*pDem;

private:
public:
	int		m_BitLen;
	int		m_StopLen;
	int		m_Parity;
	int		m_diddle;
	int		m_bpf;
	int		m_bpftap;
	int		m_lpf;

	int		m_DiddleWait;
	int		m_CharWait;
	int		m_CharWaitDiddle;
	int		m_RandomDiddle;
	int		m_WaitTimer;
	int		m_DisDiddle;
	int		m_CharWaitCount;
	int		m_WaitTimerCount;

	int		m_ReCount;

	int		m_CoutCount;

	int			m_AmpVal;
    CAMPCONT	m_Amp;

	CFSKMOD();
	inline int IsIdle(void){return m_idle;};
	inline double GetBaudRate(void){return m_BaudRate;};
	void SetBaudRate(double b);
	void CalcBPF(void);
	void SetMarkFreq(double d);
	void SetSpaceFreq(double d);
	void SetSampFreq(double f);
	inline void InitPhase(void){
		vco.InitPhase();
		m_mode = 0;
		m_CharWaitCount = 0;
		m_WaitTimerCount = 4;
		m_Data = 0;
		memset(ZBPF, 0, (m_bpftap+1) * sizeof(double));
	};
	void OutTone(int sw, int bsize);
	inline int GetMode(void){
		return m_mode;
	};
	inline double GetMarkFreq(void){return m_MarkFreq;};
	inline double GetSpaceFreq(void){return m_SpaceFreq;};
	inline int GetRev(void){return m_inv;};
	inline void SetRev(int inv){m_inv = inv;};
	inline double GetOutputGain(void){return m_OutputGain;};
	inline void SetOutputGain(double gain){m_OutputGain = gain;};
	inline void SetBPF(int sw){m_bpf = sw;};

	inline double GetLPFFreq(void){return m_LPFFreq;};
	void SetLPFFreq(double f);

	inline void SetCount(int count){
		m_Count = count;
	};
	inline void ClearTXBuf(void){
		m_out = -1;
		m_Count = m_ReCount/2;
		m_cnt = 0;
		m_rp = m_wp = 0;
		m_figout = 0;
		m_DisDiddle = -1;
		m_Data = 0;
		m_FSKCount = m_SampFreq * 2;
	};
	inline void DeleteTXBuf(void){
		m_cnt = 0;
		m_rp = m_wp = 0;
		m_figout = 0;
	};
	inline int SetDiddleTimer(int n){
		m_DisDiddle = n;
		return 0;	//JA7UDE 0428
	};
	inline int GetBufCount(void){return m_cnt;};
	void PutData(int d);
	double Do(int echo);
	inline void SetDem(CFSKDEM *p){pDem = p;};
};

#pragma option -a-	// パックの指示
typedef struct {
	BYTE	Code;
	BYTE	Fig;
}BCODETBL;
#pragma option -a.	// パック解除の指示

class CRTTY
{
private:
	int m_outfig;
	BCODETBL	m_TBL[24*4];

public:
	int	m_fig;
	int	m_uos;

	int	m_txuos;
	CRTTY();
	char ConvAscii(int d);
	int ConvRTTY(char d);
	int ConvRTTY(BYTE *t, LPCSTR p);
	inline int IsChar(int d){
		d &= 0x000000ff;
		if( !d ) return 0;
		if( d == 0x000000ff ) return 0;
		return 1;
	};
	inline void ClearTX(void){m_outfig = 3;};
	inline void ClearRX(void){m_fig = 0;};

	int GetShift(char d);
	char InvShift(char c);
	void SetCodeSet(void);
};

#define	NOISEBPFTAP	12
class CNoise		// M系列ノイズ N=22 (Tap=1)
{
private:
	double	H[NOISEBPFTAP+1];
	double	Z[NOISEBPFTAP+1];

	DWORD	reg;

public:
	CNoise();
	double GetNoise(void);
};
#endif
