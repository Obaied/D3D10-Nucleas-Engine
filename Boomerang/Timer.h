#ifndef _GAMETIME_
#define _GAMETIME_

#include <Windows.h>

class Timer
{
public:
	friend Timer& GetTimer();
	Timer();
	~Timer();

	float GetDT();
	float GetGameTime();

	void StartTimer();
	void Start();
	void Stop();
	void Tick();
	
private:

	bool mStopped;

	double mDT;
	double mSecsPerTick;

	__int64 mPausedTime;
	__int64 mStoppedTime;
	__int64 mBaseTime;
	__int64 mCurrTime;
	__int64 mPrevTime;
	
};

#endif