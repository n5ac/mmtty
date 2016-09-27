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

#include <io.h>

#include "Sound.h"
#include "Main.h"

WAVEINCAPS InCaps;
WAVEOUTCAPS OutCaps;

//---------------------------------------------------------------------------
//   注意: VCL オブジェクトのメソッドとプロパティを使用するには, Synchronize
//         を使ったメソッド呼び出しでなければなりません。次に例を示します。
//
//      Synchronize(UpdateCaption);
//
//   ここで, UpdateCaption は次のように記述できます。
//
//      void __fastcall TSound::UpdateCaption()
//      {
//        Form1->Caption = "スレッドから書き換えました";
//      }
//---------------------------------------------------------------------------
__fastcall TSound::TSound(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	::VirtualLock(this, sizeof(TSound));

	if( SampType == 2 ){
		m_FFTWINDOW = (3000 * FFT_SIZE / SampFreq);
	}
	else {
		m_FFTWINDOW = (4000 * FFT_SIZE / SampFreq);
	}
	m_BuffSize = SampSize;
	m_Stop = TRUE;
	memset(ZBPF, 0, sizeof(ZBPF));
	memset(HBPF, 0, sizeof(HBPF));

	m_ReqFifoSize = 0;
    m_IDDevice = WAVE_MAPPER;
	m_IDOutDevice = WAVE_MAPPER; //AA6YQ 1.66
	m_playmode = 0;
	m_susp = 0;
	m_suspack = 0;
	m_bpf = 0;
	m_lmsbpf = 0;
	m_bpffw = 100.0;
	m_bpftap = 56;
	m_bpfafc = 1;
	CalcBPF();

	InitWFX();
	m_Test = 0;
	m_Noise = 0;
	m_FFTSW = 1;
	m_FFTFW = 0;
	m_Tx = m_ReqTx = 0;
	FSKMOD.SetDem(&FSKDEM);

	m_FFTMax = 128;
	m_FFTSumMax = 128*2;

	m_WaterMax = 128;
	m_WaterSumMax = 128*8;
	m_WaterMin = 0;

    m_ReqSpeedTest = 0;
}

__fastcall TSound::~TSound()
{
	::VirtualUnlock(this, sizeof(TSound));
}

void __fastcall TSound::InitWFX(void)
{
	m_WFX.wFormatTag = WAVE_FORMAT_PCM;
	m_WFX.nChannels = WORD(sys.m_SoundStereo ? 2 : 1);
	m_WFX.wBitsPerSample = 16;
	m_WFX.nSamplesPerSec = int(SampBase);
	m_WFX.nBlockAlign = WORD(m_WFX.nChannels *(m_WFX.wBitsPerSample/8));
	m_WFX.nAvgBytesPerSec = m_WFX.nBlockAlign * m_WFX.nSamplesPerSec;
	m_WFX.cbSize = 0;
	Wave.m_SoundStereo = sys.m_SoundStereo;
}

void __fastcall TSound::CalcBPF(void)
{
	MakeFilter(HBPF, m_bpftap, ffBPF, SampFreq, FSKDEM.GetMarkFreq() - m_bpffw, FSKDEM.GetSpaceFreq() + m_bpffw, 60, 1.0);
	m_lms.SetWindow(FSKDEM.GetMarkFreq(), FSKDEM.GetSpaceFreq());
}

void __fastcall TSound::CalcBPF(double fl, double fh, double fw)
{
	MakeFilter(HBPF, m_bpftap, ffBPF, SampFreq, fl - fw, fh + fw, 60, 1.0);
	m_lms.SetWindow(fl, fh);
}

void __fastcall TSound::Stop(void)
{
	if( m_Stop == FALSE ){
		Priority = tpNormal;
		Terminate();
		WaitFor();
		Wave.InClose();
		Wave.OutAbort();
	}
}

void __fastcall TSound::ReqStop(void)
{
	if( m_Stop == FALSE ){
		Priority = tpNormal;
		Terminate();
	}
}

void __fastcall TSound::WaitStop(void)
{
	WaitFor();
	Wave.InClose();
	Wave.OutAbort();
}

void __fastcall TSound::InitSound(void)
{
	Suspend();
	FSKMOD.SetSampFreq(SampFreq + sys.m_TxOffset);
	InitWFX();
	m_ReqFifoSize = TRUE;
	Resume();
}
//---------------------------------------------------------------------------
void __fastcall TSound::ErrorMsg(void)
{
	if( m_IDDevice == -2 ){
		InfoMB("Sound I/O failed in the MMW (%s)", sys.m_SoundMMW.c_str());
    }
    else {
		ErrorMB((sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"Can't open Sound card (%d)":"サウンドカードがオープンできません.", m_IDDevice);
    }
}
//---------------------------------------------------------------------------
BOOL __fastcall TSound::ReInitSound(void)
{
	if( Terminated == TRUE ) return FALSE;
	Wave.OutAbort();
	Wave.InClose();
	InitWFX();
	if( m_Err >= 3 ){
		m_Err = 5;
		::Sleep(100);
	}
	else {
		m_Err++;
	}
	if( m_Tx || (m_playmode == 1) ){
		if( (sys.m_TxPort != txTXDOnly) || (m_playmode == 1) ){
			if( Wave.OutOpen(&m_WFX, m_IDOutDevice, m_BuffSize) ) return TRUE;    //AA6YQ 1.66
		}
	}
	else {
		if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) ) return TRUE;
	}
    return FALSE;
}
//---------------------------------------------------------------------------
int __fastcall TSound::UpdateFifoSize(void)
{
	int r = 2;
	if( !m_ReqFifoSize ) return 1;

	int fi = Wave.IsInOpen();
    int fo = Wave.IsOutOpen();
    if( fi ) Wave.InClose();
    if( fo ) Wave.OutAbort();
	Wave.m_InFifoSize = sys.m_SoundFifoRX;
	Wave.m_OutFifoSize = sys.m_SoundFifoTX;
    m_IDDevice = sys.m_SoundDevice;
    m_IDOutDevice = sys.m_SoundOutDevice;   //AA6YQ 1.66
    Wave.UpdateDevice(m_IDDevice);
    if( fi ){
        if( !Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) ) r = FALSE;
    }
    if( fo ){
        if( !Wave.OutOpen(&m_WFX, m_IDOutDevice, m_BuffSize) ) r = FALSE;  //AA6YQ 1.66
    }
	TaskPriority();
	m_ReqFifoSize = 0;
    return r;
}
//---------------------------------------------------------------------------
void __fastcall TSound::TaskPriority(void)
{
	switch(sys.m_SoundPriority){
		case 0:
			if( Priority != tpNormal ){
				Priority = tpNormal;		//スレッドは通常の優先度である
			}
			break;
		case 1:
			if( Priority != tpHigher ){
				Priority = tpHigher;		//スレッドの優先度は通常よりも 1 ポイント高い
			}
			break;
		case 2:
			if( Priority != tpHighest ){
				Priority = tpHighest;		//スレッドの優先度は通常よりも 2 ポイント高い
			}
			break;
		default:
			if( Priority != tpTimeCritical ){
				Priority = tpTimeCritical;	//スレッドはもっとも高い優先度を取得する
			}
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TSound::Execute()
{
	static double Buff[8192];
	double *lp;
	int i;
	int Len;

	m_Stop = FALSE;
	memset(Buff, 0, sizeof(Buff));
	TaskPriority();
	::Sleep(200);		// 200ms
	Wave.m_InFifoSize = sys.m_SoundFifoRX;
	Wave.m_OutFifoSize = sys.m_SoundFifoTX;
    m_IDDevice = sys.m_SoundDevice;
    m_IDOutDevice = sys.m_SoundOutDevice;   //AA6YQ 1.66
    Wave.UpdateDevice(m_IDDevice);
_init:;
	Wave.InClose();
	if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) != TRUE ){
		int timeout = Wave.GetTimeout() / 200;
		for( int i = 0; i < timeout; i++ ){
			::Sleep(200);
			if( Terminated == TRUE ) goto _ex;
			if( UpdateFifoSize() == 2 ) goto _init;
			if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
		}
		if( Wave.IsInOpen() != TRUE ){
			if( timeout ) ErrorMsg();
			if( Terminated == TRUE ) goto _ex;
            int Count = 3000/50;
			while(1){							// 擬似実行
				if( Terminated == TRUE ) goto _ex;
				::Sleep(50);
                Wave.PumpMessages();
				if( (m_Tx != m_ReqTx) || m_ReqFifoSize || !Count ){
					Count = 3000/50;
					if( UpdateFifoSize() == 2 ) goto _init;
					m_Tx = m_ReqTx;
					if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
				}
                Count--;
			}
			m_Tx = 0;
		}
	}
	m_Tx = 0;
	Wave.OutAbort();
	Len = m_BuffSize;
	while(1){
		if( Terminated == TRUE ) break;
		if( Wave.IsInOpen() ){
			if( Wave.InRead(Buff, m_BuffSize) == FALSE ){
				if( !ReInitSound() ) goto _init;
			}
		}
		if( m_playmode && (!m_Tx || (m_playmode != 1))) WaveFile.ReadWrite(Buff, m_BuffSize);
		if( Terminated == TRUE ) break;
		if( m_Test && !m_Tx ){
			for( i = 0, lp = Buff; i < Len; i++, lp++ ){
				if( m_Test ) *lp += FSKMOD.Do(1);
			}
		}
		if( m_bpf || m_lmsbpf ){
			for( i = 0, lp = Buff; i < Len; i++, lp++ ){
				if( m_bpf ){
					*lp = DoFIR(HBPF, ZBPF, *lp, m_bpftap);
				}
				if( m_lmsbpf ){
					*lp = m_lms.Do(*lp);
				}
			}
		}
		if( m_FFTSW ) fftIN.CollectFFT(Buff, Len);
		if( !m_Tx || sys.m_echo ){
			for( i = 0, lp = Buff; i < Len; i++, lp++ ){
				FSKDEM.Do(*lp);
			}
		}
_skip1:;
		if( m_Tx ){
			if( (sys.m_echo == 2) && Wave.IsInBufCritical() && Wave.IsInOpen() ) goto _skip2;
			for( i = 0, lp = Buff; i < Len; i++, lp++ ){
				*lp = FSKMOD.Do(sys.m_echo);
				if( (m_ReqTx != m_Tx) && !m_ReqTx && (!FSKMOD.GetMode()) ){
					for( ; i < Len; i++, lp++ ) *lp = 0;
					break;
				}
			}
			if( (sys.m_TxPort == txTXDOnly) && !Wave.IsInOpen() && (m_ReqTx == m_Tx)){
				if( FSKCount1 <= FSKCount2 ){
					::Sleep(Len * 1000 / SampFreq);
				}
				else {
					::Sleep(1);
				}
			}
		}
		if( Wave.IsOutOpen() == TRUE ){
			if( Wave.OutWrite(Buff, Len) != TRUE ){
				ReInitSound();
				if( Wave.OutWrite(Buff, Len) != TRUE ){
					if( !ReInitSound() ) goto _init;
				}
			}
			else {
				if( (sys.m_echo == 2) && m_Tx && Wave.IsInBufNull() && !Wave.IsOutBufFull() ) goto _skip1;
//	            if( (sys.m_echo == 2) && m_Tx && !Wave.IsInBufCritical() && !Wave.IsOutBufFull() ) goto _skip1;
			}
		}
_skip2:;
		if( m_Tx != m_ReqTx ){
			if( !m_ReqTx ){	// To RX
				Wave.OutAbort();
				fftIN.ClearBuf();
				if( !Wave.IsInOpen() ){
					Wave.m_InFifoSize = sys.m_SoundFifoRX;
					Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize);
					FSKDEM.m_OverFlow = 0;
					FSKDEM.ClearMode();
				}
				m_Tx = m_ReqTx = 0;
				Wave.SetPTT(m_Tx);
				TaskPriority();
			}
			else {								// To TX
				if( sys.m_echo != 2 ){
					Wave.InClose();
					fftIN.ClearBuf();
				}
				FSKMOD.InitPhase();
                FSKMOD.SetCount(m_BuffSize * 3);
				if((sys.m_TxPort != txTXDOnly) && (m_playmode != 1) ){
					Wave.m_OutFifoSize = sys.m_SoundFifoTX;
                    m_IDOutDevice = sys.m_SoundOutDevice;   //AA6YQ 1.66
					Wave.OutOpen(&m_WFX, m_IDOutDevice, m_BuffSize);     //AA6YQ 1.66
					if( sys.m_echo == 2 ){
						memset(Buff, 0, sizeof(Buff));
						for( i = 0; (i < 4) && !Wave.IsOutBufFull(); i++ ){
							if( Wave.OutWrite(Buff, Len) != TRUE ) break;
                        }
                    }
				}
				if( sys.m_echo != 2 ){
					FSKDEM.m_OverFlow = 0;
					FSKDEM.ClearMode();
				}
				m_Tx = m_ReqTx;
				Wave.SetPTT(m_Tx);
				TaskPriority();
			}
			if( m_playmode == 1 ) m_playmode = 0;
		}
		if( m_ReqFifoSize ){
			if( !UpdateFifoSize() ) goto _init;
        }
		if( (!m_Tx) && (WaveFile.m_mode != m_playmode) ){
			if( WaveFile.m_mode == 1 ){
				Wave.InClose();
				Wave.m_OutFifoSize = sys.m_SoundFifoTX;
                m_IDOutDevice = sys.m_SoundOutDevice;   //AA6YQ 1.66
				Wave.OutOpen(&m_WFX, m_IDOutDevice, m_BuffSize);      //AA6YQ 1.66
				FSKDEM.m_OverFlow = 0;
			}
			else if( m_playmode == 1 ){
				Wave.OutClose();
				Wave.m_InFifoSize = sys.m_SoundFifoRX;
				Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize);
				FSKDEM.m_OverFlow = 0;
			}
			m_playmode = WaveFile.m_mode;
		}
		if( m_susp ){
			JobSuspend();
		}
        if( m_ReqSpeedTest ){
			JobSpeedTest();
        }
	}
_ex:;
	Wave.InClose();
	Wave.OutAbort();
    Wave.UpdateDevice(-1);
	m_Stop = TRUE;
}
//---------------------------------------------------------------------------
void __fastcall TSound::JobSuspend(void)
{
	Wave.InClose();
	Wave.OutAbort();
	m_suspack = 1;
	Priority = tpNormal;
	while(1){
		if( Terminated == TRUE ){
			m_Stop = TRUE;
			return;
		}
		if( m_Tx != m_ReqTx ){
			m_Tx = m_ReqTx;
		}
		Sleep(100);
        Wave.PumpMessages();
		if( !m_susp ){
			for( int i = 0; i < 5; i++ ){
				if( Wave.InOpen(&m_WFX, m_IDDevice, m_BuffSize) == TRUE ) break;
				::Sleep(100);
                Wave.PumpMessages();
			}
			if( Wave.IsInOpen() ){
				m_suspack = 1;
				break;
			}
			else {
				m_susp = 1;
				m_suspack = 1;
				ErrorMsg();
			}
		}
	}
	TaskPriority();
	m_Tx = 0;
	m_playmode = 0;
}
//---------------------------------------------------------------------------
void __fastcall TSound::JobSpeedTest()
{
	DWORD tc = ::GetTickCount();
//	RTTYMOD.InitTXBuf();
//    RTTYDEM.Stop();
    for( int i = 0; i < 1000000; i++ ){
		FSKMOD.Do(0);
		FSKDEM.Do(0);
    }
	m_SpeedValue = ::GetTickCount() - tc;
	m_ReqSpeedTest = 0;
}
//---------------------------------------------------------------------------
// スレッド内でコールしてはいけない
#define	AFC_PEAKDOWN	128
int __fastcall TSound::DoAFC(void)
{
	if( !sys.m_AFC ) return 0;
	if( sys.m_echo != 2 ){
		if( m_Tx ) return 0;	// 送信中は実行しない
	}

	double mfq = FSKDEM.GetMarkFreq();
	double sfq = FSKDEM.GetSpaceFreq();
	double sft = sfq - mfq;
	double sk = sft * 0.5;

	int	nb = (sft < 50.0) ? 1 : 0;
	int xm = mfq * (FFT_SIZE / SampFreq);
	int xs = sfq * (FFT_SIZE / SampFreq);
	int xc = int((xs - xm) * sys.m_AFCSweep);
	int	xe = int(xc * 1.2);
	if( nb ) xe = 80 * (FFT_SIZE / SampFreq);
	int n = 0;
	int avg = 0;

	int x, xx;
	int d;
	int max1H = -MAXINT;
	int m1H = 0;
	for( x = 0; x < xc; x++ ){		// Mark Peak +
		xx = xm + x;
		if( (xx >= 0) && (xx < m_FFTWINDOW) ){
			d = fftIN.m_fft[xx];
			if( max1H < d ){
				max1H = d;
				m1H = xx;
			}
			else if( (d + AFC_PEAKDOWN) < max1H ){
				break;
			}
		}
	}
	int max1L = -MAXINT;
	int m1L = 0;
	for( x = 0; x < xe; x++ ){		// Mark Peak -
		xx = xm - x;
		if( (xx >= 0) && (xx < m_FFTWINDOW) ){
			d = fftIN.m_fft[xx];
			avg += d;
			n++;
			if( max1L < d ){
				max1L = d;
				m1L = xx;
			}
			else if( (d + AFC_PEAKDOWN) < max1L ){
				break;
			}
		}
	}

	int max2H = -MAXINT;
	int m2H = 0;
	for( x = 0; x < xe; x++ ){	// Space Peak +
		xx = xs + x;
		if( (xx >= 0) && (xx < m_FFTWINDOW) ){
			d = fftIN.m_fft[xx];
			avg += d;
			n++;
			if( max2H < d ){
				max2H = d;
				m2H = xx;
			}
			else if( (d + AFC_PEAKDOWN) < max2H ){
				break;
			}
		}
	}

	int max2L = -MAXINT;
	int m2L = 0;
	for( x = 0; x < xc; x++ ){	// Space Peak -
		xx = xs - x;
		if( (xx >= 0) && (xx < m_FFTWINDOW) ){
			d = fftIN.m_fft[xx];
			if( max2L < d ){
				max2L = d;
				m2L = xx;
			}
			else if( (d + AFC_PEAKDOWN) < max2L ){
				break;
			}
		}
	}
	if( n ) avg /= n;

//   m1l - m1h  m2l - m2h
#if 0
	FILE *fp = fopen("f:\\test.txt", "at");
	fprintf(fp, "%5u %5u %5u %5u\n", m1L, m1H, m2L, m2H);
	fclose(fp);
#endif

	if( nb ){
		if( max1L < max1H ){ max1L = max1H; m1L = m1H; }
		if( max1L < max2L ){ max1L = max2L; m1L = m2L; }
		if( max1L < max2H ){ max1L = max2H; m1L = m2H; }
		max2L = max1L;
		m2L = m1L;
	}
	else if( m1H == m2L ){
		if( max2H > max1L ){
			max1L = max1H;
			m1L = m1H;
			max2L = max2H;
			m2L = m2H;
		}
	}
	else {
		if( max1H > max1L ){
			m1L = m1H;
			max1L = max1H;
		}
		if( max2H > max2L ){
			m2L = m2H;
			max2L = max2H;
		}
	}
	if( !nb ){
		switch(sys.m_FFTGain){
			case 0:
				if( ((max1L - avg) < sys.m_AFCSQ) || ((max2L - avg) < sys.m_AFCSQ) ) return 0;
				break;
			case 1:
				if( ((max1L - avg) < sys.m_AFCSQ*1.2) || ((max2L - avg) < sys.m_AFCSQ*1.2) ) return 0;
				break;
			case 2:
				if( ((max1L - avg) < sys.m_AFCSQ*1.5) || ((max2L - avg) < sys.m_AFCSQ*1.5) ) return 0;
				break;
			case 3:
				if( ((max1L - avg) < sys.m_AFCSQ*1.8) || ((max2L - avg) < sys.m_AFCSQ*1.8) ) return 0;
				break;
			default:
				if( ((max1L - avg) < sys.m_AFCSQ*0.5) || ((max2L - avg) < sys.m_AFCSQ*0.5) ) return 0;
				break;
		}
	}
	else {
		if( (max1L - avg) < sys.m_AFCSQ*0.5 ) return 0;
	}
	int ns = m2L - m1L;						// 検出したシフト幅
	if( !nb ){
		if( ns < int(140.0 * (FFT_SIZE / SampFreq)) ) return 0;
	}
	if( ns > int(1500.0 * (FFT_SIZE / SampFreq)) ) return 0;

	int os = (sft * (FFT_SIZE / SampFreq));	// 現在のシフト幅
	double nmfq = m1L * (SampFreq / FFT_SIZE);
	double nsfq = m2L * (SampFreq / FFT_SIZE);
	int ds = ABS(ns-os);					// シフト幅の差
	switch(sys.m_FixShift){
		case 0:		// Free
			if( nb ) goto _fixed;
			if( ((ds <= (os * 1.2)) || nb) && ((nsfq - nmfq) >= 15.0) ){
				if( fabs(nmfq - mfq) >= 2.0 ){
					mfq += (nmfq - mfq)/sys.m_AFCTime;
				}
				if( fabs(nsfq - sfq) >= 2.0 ){
					sfq += (nsfq - sfq)/sys.m_AFCTime;
				}
				mfq = double(int(mfq+0.5));
				sfq = double(int(sfq+0.5));
				if( mfq < MARKL ) mfq = MARKL;
				if( sfq > SPACEH ) sfq = SPACEH;
				Suspend();
				FSKDEM.AFCMarkFreq(mfq);
				FSKDEM.AFCSpaceFreq(sfq);
				if( m_bpfafc && (FSKDEM.GetAFCMarkFreq() == mfq) && (FSKDEM.GetAFCSpaceFreq() == sfq) ){
					CalcBPF();
				}
				Resume();
				return 1;
			}
			break;
		case 1:		// Fixed Shift
_fixed:;
			if( nb ){
				double fq = (nmfq + nsfq) / 2;
				nmfq = fq - sft/2;
				nsfq = fq + sft/2;
			}
			if( ((ds <= (os * 0.25)) || nb) && ((nsfq - nmfq) >= 15.0) ){
				double cfq = (nmfq + nsfq)/2.0;
				nmfq = cfq - sk;
				if( fabs(nmfq - mfq) >= 2.0 ){
					if( nb && (fabs(nmfq - mfq) < 10.0) ){
						mfq += (nmfq - mfq)/(sys.m_AFCTime * 4.0);
					}
					else {
						mfq += (nmfq - mfq)/sys.m_AFCTime;
					}
					mfq = double(int(mfq+0.5));
					if( mfq < MARKL ) mfq = MARKL;
					if( (mfq + sft) > SPACEH ) mfq = SPACEH - sft;
					sfq = mfq + sft;
					Suspend();
					FSKDEM.AFCMarkFreq(mfq);
					FSKDEM.AFCSpaceFreq(sfq);
					if( m_bpfafc && (FSKDEM.GetAFCMarkFreq() == mfq) && (FSKDEM.GetAFCSpaceFreq() == sfq) ){
						CalcBPF();
					}
					Resume();
					return 1;
				}
			}
			break;
		case 2:		// HAM
			if( nb ) goto _fixed;
			if( (ns >= 140.0*FFT_SIZE/SampFreq) && (ns <= 260.0*FFT_SIZE/SampFreq) ){
				sft = nsfq - nmfq;
				if( sft > 230.0 ){
					sft = 240.0;
				}
				else if( sft > 210.0 ){
					sft = 220.0;
				}
				else if( sft > 185 ){
					sft = 200.0;
				}
				else {
					sft = 170.0;
				}
				nsfq = nmfq + sft;
				if( fabs(nmfq - mfq) >= 2.0 ){
					mfq += (nmfq - mfq)/sys.m_AFCTime;
				}
				if( fabs(nsfq - sfq) >= 2.0 ){
					sfq += (nsfq - sfq)/sys.m_AFCTime;
				}
				mfq = double(int(mfq+0.5));
				sfq = double(int(sfq+0.5));
				if( mfq < MARKL ) mfq = MARKL;
				if( sfq > SPACEH ) sfq = SPACEH;
				Suspend();
				FSKDEM.AFCMarkFreq(mfq);
				FSKDEM.AFCSpaceFreq(sfq);
				if( m_bpfafc && (FSKDEM.GetAFCMarkFreq() == mfq) && (FSKDEM.GetAFCSpaceFreq() == sfq) ){
					CalcBPF();
				}
				Resume();
				return 1;
			}
			break;
		case 3:		// FSK
			if( (ns >= 140.0*FFT_SIZE/SampFreq) && (ns <= 260.0*FFT_SIZE/SampFreq) ){
				sft = nsfq - nmfq;
				if( sft > 230.0 ){
					sft = 240.0;
				}
				else if( sft > 210.0 ){
					sft = 220.0;
				}
				else if( sft > 185 ){
					sft = 200.0;
				}
				else {
					sft = 170.0;
				}
				nsfq = nmfq + sft;
				if( fabs(nmfq - mfq) >= 2.0 ){
					nmfq = mfq + (nmfq - mfq)/sys.m_AFCTime;
				}
				if( fabs(nsfq - sfq) >= 2.0 ){
					nsfq = sfq + (nsfq - sfq)/sys.m_AFCTime;
				}
				sft = nsfq - nmfq;
				if( sft < 175 ) sft = 170;
				if( sft > 195 ) sft = 200;
				sft = double(int((sft*0.5)+0.5));
				nmfq = ((sfq + mfq)*0.5) - sft;
				nsfq = ((sfq + mfq)*0.5) + sft;
				mfq = double(int(nmfq+0.5));
				sfq = double(int(nsfq+0.5));
				if( mfq < MARKL ) mfq = MARKL;
				if( sfq > SPACEH ) sfq = SPACEH;
				Suspend();
				FSKDEM.AFCMarkFreq(mfq);
				FSKDEM.AFCSpaceFreq(sfq);
				if( m_bpfafc && (FSKDEM.GetAFCMarkFreq() == mfq) && (FSKDEM.GetAFCSpaceFreq() == sfq) ){
					CalcBPF();
				}
				Resume();
				return 1;
			}
			break;
	}
	return 0;
}
//---------------------------------------------------------------------------
double __fastcall TSound::GetScopeRange(double &low, double center, double shift)
{
	double FM;

	switch(m_FFTFW ){
		case 0:			// Auto
			if( shift >= 800 ){
				FM = 3000 * FFT_SIZE / SampFreq;
				low = center - 1500;
			}
			else if( shift >= 400 ){
				FM = 2000 * FFT_SIZE / SampFreq;
				low = center - 1000;
			}
			else {
				FM = 1000 * FFT_SIZE / SampFreq;
				low = center - 500;
			}
			break;
		case 1:			// 500
			FM = 500 * FFT_SIZE / SampFreq;
			low = center - 250;
			break;
		case 2:			// 1000
			FM = 1000 * FFT_SIZE / SampFreq;
			low = center - 500;
			break;
		case 3:			// 1500
			FM = 1500 * FFT_SIZE / SampFreq;
			low = center - 750;
			break;
		case 4:			// 2000
			FM = 2000 * FFT_SIZE / SampFreq;
			low = center - 1000;
			break;
		default:   		// 3000
			FM = 3000 * FFT_SIZE / SampFreq;
			low = center - 1500;
			break;
	}
	return FM;
}
//---------------------------------------------------------------------------
// スレッド内でコールしてはいけない
int __fastcall TSound::DrawFFT(Graphics::TBitmap *pBitmap, int sw, int XRD)
{
	if( m_FFTSW ){
		if( fftIN.m_CollectFFT ){
			switch(sys.m_FFTGain){
				case 0:
					fftIN.CalcFFT(m_FFTWINDOW, 30.0, sys.m_FFTResp);
					break;
				case 1:
					fftIN.CalcFFT(m_FFTWINDOW, 34.0, sys.m_FFTResp);
					break;
				case 2:
					fftIN.CalcFFT(m_FFTWINDOW, 42.0, sys.m_FFTResp);
					break;
				case 3:
					fftIN.CalcFFT(m_FFTWINDOW, 54.0, sys.m_FFTResp);
					break;
				default:
					fftIN.CalcFFT(m_FFTWINDOW, (sys.m_echo != 2 && m_Tx) ? 0.02 : 0.1, sys.m_FFTResp);
					break;
			}
			fftIN.TrigFFT();
		}
		else if( !sw ){
			return 0;
		}
	}
	if( Remote & REMSHOWOFF ) return 2;

	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	int XL = 0;
	int XR = pBitmap->Width - 1;
	int YT = 0;
	int YB = pBitmap->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	tp->Brush->Color = clBlack;
	tp->FillRect(rc);
	XR -= XRD;

	int i, x;
	if( m_Err || FSKDEM.m_OverFlow ){
		if( m_Err || (!m_Tx && !m_Test) ){
			tp->Font->Color = clWhite;
			tp->Font->Size = 10;
			char bf[32];
			strcpy(bf, m_Err ? "Lost sound" : "Overflow");
			x = tp->TextWidth(bf);
			tp->TextOut(XR - x - 3, 0, bf);
		}
		FSKDEM.m_OverFlow = 0;
		if( m_Err ) m_Err--;
	}
	if( WaveFile.m_mode ){
		tp->Font->Color = clWhite;
		tp->Font->Size = 10;
		if( WaveFile.m_pause ){
			tp->TextOut(3, 0, "Pause");
		}
		else {
			switch(WaveFile.m_mode){
				case 1:
					tp->TextOut(3, 0, "Play");
					break;
				case 2:
					tp->TextOut(3, 0, "Rec.");
					break;
			}
		}
	}
	double space = FSKDEM.GetSpaceFreq();
	double mark = FSKDEM.GetMarkFreq();
	double shift = fabs(mark - space);
	double center = (mark + space)/2.0;
	double low;
	int FM = GetScopeRange(low, center, shift) + 0.5;
	tp->Pen->Color = clYellow;
	tp->Pen->Style = psSolid;
	x = int(((mark-low)*FFT_SIZE*double(XR)/double(SampFreq*FM)) + 0.5);
	tp->MoveTo(x, YT);
	tp->LineTo(x, YB);
	x = int(((space-low)*FFT_SIZE*double(XR)/double(SampFreq*FM)) + 0.5);
	tp->MoveTo(x, YT);
	tp->LineTo(x, YB);

	if( sw || (!m_FFTSW) ) return 1;

	int offx = int((low * FFT_SIZE / SampFreq) + ((FM/XR)/2.0) + 0.5);
	tp->Pen->Color = clWhite;

	int d;
	int max = 0;
	double k;
	if( sys.m_FFTGain >= 4 ){
		switch(sys.m_FFTGain){
			case 4:
				k = double(YB) * 0.85 / m_FFTMax;
				break;
			case 5:
				k = double(YB) * 1.0 / m_FFTMax;
				break;
			case 6:
				k = double(YB) * 1.2 / m_FFTMax;
				break;
			case 7:
				k = double(YB) * 1.5 / m_FFTMax;
				break;
		}
	}
	else {
		k = double(YB)/256.0;
	}
	for( i = 0; i < XR; i++ ){
		x = (i * FM)/XR + offx;
		if( (x >= 0) && (x < m_FFTWINDOW) ){
			d = fftIN.m_fft[x];
			if( d > max ) max = d;
			d *= k;
		}
		else {
			d = 0;
		}
		if( d >= (YB-YT) ) d = YB - YT - 1;
		if( i ){
			tp->LineTo(i, YB-d);
		}
		else {
			tp->MoveTo(i, YB-d);
		}
	}
	if( sys.m_FFTGain >= 4 ){
		m_FFTSumMax -= m_FFTMax;
		m_FFTSumMax += max;
		if( m_FFTSumMax < 64 ) m_FFTSumMax = 64;
		m_FFTMax = m_FFTSumMax / 2;
	}

	if( m_lmsbpf && m_lms.m_Type ){
		if( m_lms.m_lmsNotch ){
			tp->Pen->Color = clYellow;
			tp->Pen->Style = psSolid;
			x = int(((m_lms.m_lmsNotch-low)*FFT_SIZE*double(XR)/double(SampFreq*FM)) + 0.5);
			//POINT	ary[3];	//JA7UDE 0428
			TPoint	ary[3];	//JA7UDE 0428
			ary[0].x = x, ary[0].y = YB;
			ary[1].x = x-3, ary[1].y = YB-5;
			ary[2].x = x+3, ary[2].y = YB-5;
			tp->Brush->Color = clRed;
			tp->Polygon(ary, 2);
			if( m_lms.m_twoNotch && m_lms.m_lmsNotch2 ){
				x = int(((m_lms.m_lmsNotch2-low)*FFT_SIZE*double(XR)/double(SampFreq*FM)) + 0.5);
				ary[0].x = x, ary[0].y = YB;
				ary[1].x = x-3, ary[1].y = YB-5;
				ary[2].x = x+3, ary[2].y = YB-5;
				tp->Brush->Color = clBlue;
				tp->Polygon(ary, 2);
			}
		}
	}
	return 2;
}

double __fastcall TSound::GetScreenFreq(int x, int XW, int XRD)
{
	double space = FSKDEM.GetSpaceFreq();
	double mark = FSKDEM.GetMarkFreq();
	double shift = fabs(mark - space);
	double center = (mark + space)/2.0;
	double low;
	double FW = GetScopeRange(low, center, shift) * SampFreq / FFT_SIZE;
	return ((x * FW) / (XW-XRD)) + low;
}

//---------------------------------------------------------------------------
// スレッド内でコールしてはいけない
int __fastcall TSound::DrawFFTWater(Graphics::TBitmap *pBitmap, int sw, int XRD)
{
	if( Remote & REMSHOWOFF ) return 2;
	TCanvas *tp = pBitmap->Canvas;
	TRect rc;
	int XL = 0;
	int XR = pBitmap->Width - 1;
	int YT = 0;
	int YB = pBitmap->Height - 1;
	rc.Left = XL;
	rc.Right = XR;
	rc.Top = YT;
	rc.Bottom = YB+1;
	if( sw ){
		tp->Brush->Color = clBlack;
		tp->FillRect(rc);
		return 1;
	}
	int i, x;
	int XRR = XR - XRD;

	double space = FSKDEM.GetSpaceFreq();
	double mark = FSKDEM.GetMarkFreq();
	double shift = fabs(mark - space);
	double center = (mark + space)/2.0;
	double low;
	int FM = GetScopeRange(low, center, shift) + 0.5;

	TRect src(rc);
	src.Bottom--;
	rc.Top++;
	tp->CopyRect(rc, tp, src);

	int offx = int((low * FFT_SIZE / SampFreq) + ((FM/XRR)/2.0) + 0.5);
	int d;
	int mx = -MAXINT;
	int mi = 0;
	int micnt = 0;
	double k1 = 128.0/double(m_WaterMax - m_WaterMin);
	for( i = 0; i < XR; i++ ){
		x = (i * FM)/XRR + offx;
		if( ((x >= 0) && (x < m_FFTWINDOW)) ){
			d = fftIN.m_fft[x] / 2;
			if( mx < d ) mx = d;
			mi += d;
			micnt++;

			d -= m_WaterMin;
			d = double(d) * k1;

			if( d >= 128 ) d = 127;
			if( d < 0 ) d = 0;
		}
		else {
			d = 0;
		}
		tp->Pixels[i][0] = TColor(ColorTable[127-d] | sys.d_PaletteMask);
	}
	m_WaterSumMax -= m_WaterMax;
	m_WaterSumMax += mx;
	if( sys.m_FFTGain >= 4 ){
		if( m_WaterSumMax < (16*2) ) m_WaterSumMax = (16*2);
		if( m_WaterSumMax > 512 ) m_WaterSumMax = 512;
	}
	else {
		if( m_WaterSumMax < (128*2) ) m_WaterSumMax = (128*2);
	}
	m_WaterMax = m_WaterSumMax / 8;
	if( micnt ){
		m_WaterMin = mi/micnt;
	}
	if( sys.m_FFTGain >= 4 ){
		if( m_WaterMin >= (m_WaterMax - 4) ) m_WaterMax = m_WaterMin + 4;
	}
	else {
		if( m_WaterMin >= (m_WaterMax - 32) ) m_WaterMax = m_WaterMin + 32;
	}
	return 2;
}


//---------------------------------------------------------------------------
//AA6YQ 1.66
LPCSTR __fastcall TSound::GetInputSoundcard(unsigned int ID)
{

    if (MMSYSERR_NOERROR==(waveInGetDevCaps(ID,&InCaps,sizeof(InCaps)))) {
        return InCaps.szPname;
    }
    else {
        return "";
    }
}

//---------------------------------------------------------------------------
//AA6YQ 1.6.6
LPCSTR __fastcall TSound::GetOutputSoundcard(unsigned int ID)
{

    if (MMSYSERR_NOERROR==(waveOutGetDevCaps(ID,&OutCaps,sizeof(OutCaps)))) {
        return OutCaps.szPname;
    }
    else {
        return "";
    }
}
//---------------------------------------------------------------------------

void __fastcall CWaveFile::ReadWrite(double *s, int size)
{
	SHORT	d;

	if( m_Handle != NULL ){
		if( m_mode == 2 ){		// 書きこみ
			if( !m_pause ){
				for( ; size; s++, size-- ){
					d = SHORT(*s);
					if( mmioWrite(m_Handle, (const char*)&d, 2) != 2 ){
						mmioClose(m_Handle, 0);
						m_Handle = 0;
						m_mode = 0;
						break;
					}
					else {
						m_pos += 2;
					}
				}
			}
		}
		else {						// 読み出し
			if( m_pause || m_dis ){
				memset(s, 0, sizeof(double)*size);
			}
			else {
				for( ; size; s++, size-- ){
					if( mmioRead(m_Handle, (char *)&d, 2) == 2 ){
						*s = d;
						m_pos += 2;
					}
					else if( m_autopause ){
						m_pause = 1;
						break;
					}
					else {
						mmioClose(m_Handle, 0);
						m_Handle = 0;
						m_mode = 0;
						break;
					}
				}
				for( ; size; s++, size-- ){
					*s = 0;
				}
			}
		}
	}
}

__fastcall CWaveFile::CWaveFile()
{
	m_mode = 0;
	m_pause = 0;
	m_Handle = NULL;
	m_dis = 0;
	m_autopause = 0;
}

__fastcall CWaveFile::~CWaveFile()
{
	FileClose();
}

void __fastcall CWaveFile::FileClose(void)
{
	m_mode = 0;
	m_pause = 0;
	if( m_Handle != NULL ){
		mmioClose(m_Handle, 0);
		m_Handle = 0;
	}
}

void __fastcall CWaveFile::Rec(LPCSTR pName)
{
	FileClose();
	m_FileName = pName;
	m_Handle = mmioOpen(m_FileName.c_str(), NULL, MMIO_CREATE|MMIO_WRITE|MMIO_ALLOCBUF);
	if( m_Handle == NULL ){
		ErrorMB( (sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"Can't open '%s'":"'%s'を作成できません.", pName);
		return;
	}
	m_Head[0] = 0x55;
	m_Head[1] = 0xaa;
	m_Head[2] = char(SampType);
	m_Head[3] = 0;
	mmioWrite(m_Handle, (const char *)m_Head, 4);
	m_pos = 4;
	m_mode = 2;
	m_pause = 0;
	m_dis = 0;
}

int     SampTable[]={11025, 8000, 6000, 12000, 16000, 18000, 22050, 24000, 44100};
BOOL __fastcall CWaveFile::Play(LPCSTR pName)
{
	FileClose();
	FILE *fp = fopen(pName, "rb");
	if( fp == NULL ){
		ErrorMB( (sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"Can't open '%s'":"'%s'をオープンできません.", pName);
		return FALSE;
	}
	m_length = filelength(fileno(fp));
	m_FileName = pName;
	fclose(fp);
	m_Handle = mmioOpen(m_FileName.c_str(), NULL, MMIO_READ|MMIO_ALLOCBUF);
	if( m_Handle == NULL ){
		ErrorMB( (sys.m_WinFontCharset != SHIFTJIS_CHARSET)?"Can't open '%s'":"'%s'をオープンできません.", pName);
		return FALSE;
	}
	m_pos = 0;
	if( mmioRead(m_Handle, (char *)m_Head, 4) == 4 ){
		int type = 0;
		if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
			type = m_Head[2];
			m_pos = 4;
			m_length -= 4;
		}
		if( type > 8 ) type = 0;
		if( type != SampType ){
			if( YesNoMB(
				(sys.m_WinFontCharset != SHIFTJIS_CHARSET)
				? "%s\r\n\r\nThis file has been recorded based on %uHz, play it with sampling conversion?"
				: "%s\r\n\r\nこのファイルは %uHz ベースで記録されています. 周波数変換して再生しますか？",
				m_FileName.c_str(), SampTable[type] ) == IDNO ){
				mmioClose(m_Handle, 0);
				m_Handle = 0;
				return TRUE;
			}
			else {
				mmioClose(m_Handle, 0);
				m_Handle = 0;
				char bf[1024];
				strcpy(bf, pName);
				SetEXT(bf, "");
				char wName[1024];
				sprintf(wName, "%s_%u.MMV", bf, int(SampBase));
				if( ChangeSampFreq(wName, pName, SampTable[type]) == TRUE ){
					Play(wName);
				}
			}
		}
	}
	m_mode = 1;
	m_pause = 0;
	m_dis = 0;
	return TRUE;
}

void __fastcall CWaveFile::Rewind(void)
{
	if( m_Handle != NULL ){
		m_dis++;
		if( m_mode == 2 ){
			mmioSeek(m_Handle, 4, SEEK_SET);
			m_pos = 4;
		}
		else {
			mmioSeek(m_Handle, 0, SEEK_SET);
			m_pos = 0;
		}
		m_dis--;
	}
}

void __fastcall CWaveFile::Seek(int n)
{
	if( m_Handle != NULL ){
		m_dis++;
		if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
			mmioSeek(m_Handle, n + 4, SEEK_SET);
			m_pos = n + 4;
		}
		else {
			mmioSeek(m_Handle, n, SEEK_SET);
			m_pos = n;
		}
		m_dis--;
	}
}

long __fastcall CWaveFile::GetPos(void)
{
	if( (m_Head[0] == 0x55)&&(m_Head[1] == 0xaa) ){
		long n = m_pos - 4;
		if( n < 0 ) n = 0;
		return n;
	}
	else {
		return m_pos;
	}
}

int __fastcall CWaveFile::ChangeSampFreq(LPCSTR tName, LPCSTR pName, int sSamp)
{
	int rr = FALSE;
	FILE *fp = fopen(pName, "rb");
	if( fp != NULL ){
		CWaitCursor w;
		BYTE    head[4];
		memset(head, 0, sizeof(head));
		head[0] = 0x55;
		head[1] = 0xaa;
		head[2] = char(SampType);
		head[3] = 0;
		FILE *wfp = fopen(tName, "wb");
		if( wfp != NULL ){
			fwrite(head, 1, 4, wfp);

			int rsize = 16384;
			int wsize = rsize * SampBase/double(sSamp);
			short *rp = new short[rsize];
			short *wp = new short[wsize];
			CIIR    riir;
			riir.MakeIIR(2800, sSamp, 8, 0, 0);
			CIIR    wiir;
			wiir.MakeIIR(2800, SampBase, 16, 0, 0);
			int rlen, wlen;
			short *tp;
			int i, r;
			while(1){
				rlen = fread(rp, 1, rsize * 2, fp);
				if( !rlen ) break;
				rlen /= 2;
				if( SampBase < sSamp ){             // デシメーション
					tp = rp;
					for( i = 0; i < rlen; i++, tp++ ){
						*tp = riir.Do(*tp);
					}
				}
				tp = wp;
				wlen = rlen * SampBase / sSamp;
				if( wlen > wsize ) wlen = wsize;
				for( i = 0; i < wlen; i++ ){
					r = int(double(i) * sSamp / SampBase);
					*tp++ = wiir.Do(rp[r]);
				}
				if( fwrite(wp, 1, wlen * 2, wfp) != size_t(wlen * 2) ) break;
			}
			if( !fclose(wfp) ) rr = TRUE;
			delete[] rp;
			delete[] wp;
		}
		fclose(fp);
	}
	return rr;
}

