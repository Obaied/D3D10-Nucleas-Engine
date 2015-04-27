#ifndef _ABSD3D_
#define _ABSD3D_

#include "D3DUtil.h"
#include <ctime>
#include "Timer.h"


class AbsD3D
{
public:
	AbsD3D(HINSTANCE Hins, int Width, int Height);
	~AbsD3D();

	virtual void InitApp();
	void InitWindow();
	void InitD3D();
	void InitResources();
	int Run();

	virtual LRESULT MsgProc(UINT Msg, WPARAM wparam, LPARAM lparam);
	
protected:

	virtual void UpdateScene();
	virtual void RenderScene();
	virtual void OnResize();
	void		 ResetOMViews();

protected:

	HINSTANCE mHins;
	HWND mMainWnd;
	UINT mClientWidth;
	UINT mClientHeight;
	std::wstring mMainWndCaption;

	IDXGISwapChain*			mSwapChain;
	ID3D10RenderTargetView* mRenderTargetView;
	ID3D10DepthStencilView* mDepthStencilView;

	ID3DX10Font* mHUDFont;
	ID3DX10Font* mAnnounceFont;

	std::wstring mFrameStats;
	
	bool mAppPaused;
	bool mMinimized;
	bool mMaximized;
	bool mResizing;
};

#endif