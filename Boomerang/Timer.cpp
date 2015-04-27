#include "Timer.h"

Timer& GetTimer()
{
	static Timer mTimer;
	return mTimer;
}

Timer::Timer()
{
	mDT			 = 0.0;
	mSecsPerTick = 0.0;
	mPausedTime  = 0;
	mStoppedTime = 0;
	mBaseTime    = 0;
	mCurrTime	 = 0;
	mPrevTime	 = 0;
	mStopped	 = false;
}

Timer::~Timer()
{
}

void Timer::StartTimer()
{
	__int64 CurrTime;
	__int64 TicksPerSec;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime);
	QueryPerformanceFrequency((LARGE_INTEGER*)&TicksPerSec);

	mStopped  = false;
	mBaseTime = CurrTime;
	mPrevTime = CurrTime;
	mStoppedTime = 0;
	mSecsPerTick = 1.0/(double)TicksPerSec;
}

void Timer::Tick()
{
	if(mStopped)
	{
		mDT = 0.0f;
		return;
	}

	__int64 CurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime);

	mCurrTime = CurrTime;
	mDT = (CurrTime-mPrevTime) * mSecsPerTick;
	mPrevTime = CurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if(mDT < 0.0)
	{
		mDT = 0.0;
	}
}

void Timer::Stop()
{
	if(!mStopped)
	{
	__int64 CurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime);

	mStoppedTime = CurrTime;
	mStopped = true;
	}
}

void Timer::Start()
{
	if(mStopped)
	{
	__int64 CurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime);

	mPausedTime += CurrTime - mStoppedTime;
	mStoppedTime = 0;
	mPrevTime = CurrTime;
	mStopped = false;
	}
}

float Timer::GetGameTime()
{
	if(mStopped)
		return (float)(( mStoppedTime-mPausedTime - mBaseTime)*mSecsPerTick);

	return (float)(( mCurrTime-mPausedTime - mBaseTime)*mSecsPerTick);
}

float Timer::GetDT()
{
	return (float)mDT;;
}