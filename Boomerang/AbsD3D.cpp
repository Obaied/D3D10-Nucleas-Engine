#include "AbsD3D.h"
#include <sstream>

ID3D10Device* gd3dDev = 0;

AbsD3D::AbsD3D(HINSTANCE Hins, int Width, int Height)
	:mHins(Hins), mClientWidth(Width), mClientHeight(Height),
	mSwapChain(0), mRenderTargetView(0), mDepthStencilView(0),
	mAppPaused(false), mMinimized(false), mMaximized(false), mResizing(false)
{
	srand(time(0));
	mFrameStats = L"";
	mMainWndCaption = L"Boomerang";
}

AbsD3D::~AbsD3D()
{
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mHUDFont);
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static AbsD3D* app = 0;

	switch(msg)
	{

	case WM_CREATE:
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
			app = (AbsD3D*)cs->lpCreateParams;
			return 0;
		}
	}

	if(app)
		return app->MsgProc(msg,wparam,lparam);
	else
		return DefWindowProc(hwnd, msg, wparam, lparam);
}

void AbsD3D::InitApp()
{
	InitWindow();
	InitD3D();
	InitResources();
}

void AbsD3D::InitWindow()
{
	//initilize the window
	WNDCLASS wc;
	wc.style		= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	= MainWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= mHins;
	wc.hIcon		= LoadIcon(0, IDI_APPLICATION);
	wc.hCursor		= LoadCursor(0, IDC_ARROW);
	wc.hbrBackground= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= 0;
	wc.lpszClassName= L"D3DClass";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"Register Class FAILED",0,0);
		PostQuitMessage(0);
	}

	RECT R = {0,0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int Width = R.right - R.left;
	int Height = R.bottom - R.top;

	mMainWnd = CreateWindow(L"D3DClass", mMainWndCaption.c_str(), 
		WS_OVERLAPPEDWINDOW, 0, 0, Width, Height,
		0, 0, mHins, this);

	if( !mMainWnd)
	{
		MessageBox(0, L"CreateWindow() FAILED", 0,0);
		PostQuitMessage(0);
	}

	ShowWindow(mMainWnd, SW_SHOW);
	UpdateWindow(mMainWnd);
}

void AbsD3D::InitD3D()
{
	//Initialize the D3D
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator	  = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;

	sd.SampleDesc.Count   = 1;
	sd.SampleDesc.Quality = 0;

	sd.BufferCount  = 1;
	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mMainWnd;
	sd.SwapEffect	= DXGI_SWAP_EFFECT_DISCARD;
	sd.Windowed		= true;
	sd.Flags		= 0;

	UINT CreateDeviceFlags = 0;
#if defined(DEBUG) | defined(_DEBUG)
	CreateDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	HR( D3D10CreateDeviceAndSwapChain(
		0,		//default Adapter
		D3D10_DRIVER_TYPE_HARDWARE,
		0,		//not a software device
		CreateDeviceFlags,
		D3D10_SDK_VERSION,		//always
		&sd,
		&mSwapChain,
		&gd3dDev) );
	
	OnResize();
}

void AbsD3D::InitResources()
{
	//Create the font
	D3DX10_FONT_DESC FontDesc;
	FontDesc.CharSet	 = DEFAULT_CHARSET;
	wcscpy(FontDesc.FaceName, L"Times New Roman");
	FontDesc.Height		 = 18;
	FontDesc.Italic		 = true;
	FontDesc.MipLevels	 = 1;
	FontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	FontDesc.PitchAndFamily	 = DEFAULT_PITCH | FF_DONTCARE;
	FontDesc.Quality		 = DEFAULT_QUALITY;
	FontDesc.Weight			 = 3;
	FontDesc.Width			 = 0;

	D3DX10CreateFont(gd3dDev, FontDesc.Height, FontDesc.Width, FontDesc.Weight, FontDesc.MipLevels,
		FontDesc.Italic, FontDesc.CharSet, FontDesc.OutputPrecision, FontDesc.Quality, FontDesc.PitchAndFamily, FontDesc.FaceName, &mHUDFont);

	//Create the announcement font

	FontDesc.CharSet	 = DEFAULT_CHARSET;
	wcscpy(FontDesc.FaceName, L"Arial");
	FontDesc.Height		 = 30;
	FontDesc.Italic		 = false;
	FontDesc.MipLevels	 = 1;
	FontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	FontDesc.PitchAndFamily	 = DEFAULT_PITCH | FF_DONTCARE;
	FontDesc.Quality		 = DEFAULT_QUALITY;
	FontDesc.Weight			 = 6;
	FontDesc.Width			 = 0;

	D3DX10CreateFont(gd3dDev, FontDesc.Height, FontDesc.Width, FontDesc.Weight, FontDesc.MipLevels,
		FontDesc.Italic, FontDesc.CharSet, FontDesc.OutputPrecision, FontDesc.Quality, FontDesc.PitchAndFamily, FontDesc.FaceName, &mAnnounceFont);
}

int AbsD3D::Run()
{
	MSG msg = {0};
	GetTimer().StartTimer();

	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GetTimer().Tick();

			if(mAppPaused)
			{
				Sleep(50);
			}
			else
			{
				UpdateScene();
				RenderScene();
			}
		}
	}

	return (int)msg.wParam;
}

void AbsD3D::OnResize()
{
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);

	//Resize the swap chain
	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	//Create a new texture to get the render target view of the backbuffer
	ID3D10Texture2D* BB = 0;
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), 
		reinterpret_cast<void**>(&BB)));
	HR(gd3dDev->CreateRenderTargetView(BB, 0, &mRenderTargetView));
	ReleaseCOM(BB);

	//Create the depth/stencil buffer
	D3D10_TEXTURE2D_DESC ds;
	ID3D10Texture2D* DepthStencilBuffer = 0;

	ds.ArraySize			= 1;
	ds.BindFlags			= D3D10_BIND_DEPTH_STENCIL;
	ds.CPUAccessFlags		= 0;
	ds.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	ds.Height				= mClientHeight;
	ds.Width				= mClientWidth;
	ds.MipLevels			= 1;
	ds.MiscFlags			= 0;
	ds.Usage				= D3D10_USAGE_DEFAULT;

	ds.SampleDesc.Count		= 1;
	ds.SampleDesc.Quality	= 0;
	
	HR(gd3dDev->CreateTexture2D(&ds, 0, &DepthStencilBuffer));
	HR(gd3dDev->CreateDepthStencilView(DepthStencilBuffer, 0, &mDepthStencilView));
	ReleaseCOM(DepthStencilBuffer);

	//Bind the render target and the depth/stencil buffer VIEWS to the 
	//Output merger stage

	gd3dDev->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	//Set the viewport
	D3D10_VIEWPORT vp = {0,0, mClientWidth,mClientHeight, 0.0f,1.0f};
	gd3dDev->RSSetViewports(1, &vp);
}

void AbsD3D::UpdateScene()
{
	static float t_base = 0.0f;
	static UINT	 NumFramesPassed = 0;

	NumFramesPassed++;

	if( (GetTimer().GetGameTime() - t_base) >= 1.0f)
	{
		float FPS  = (float)NumFramesPassed;
		float mspf = 1000.0f/NumFramesPassed;
		
		std::wostringstream outs;
		outs.precision(6);
		outs << L"FPS: " << FPS << L"\n"
			<< "Milliseconds per Frame: " << mspf << L"\n";
		mFrameStats = outs.str();

		NumFramesPassed = 0;
		t_base += 1.0f;

	}

}

void AbsD3D::RenderScene()
{
	gd3dDev->ClearRenderTargetView(mRenderTargetView, D3DCOLOR_BLACK );
	gd3dDev->ClearDepthStencilView(mDepthStencilView, D3D10_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0f, 0);
}

LRESULT AbsD3D::MsgProc(UINT Msg, WPARAM wparam, LPARAM lparam)
{
	switch( Msg )
	{

	case WM_ACTIVATE:
		if(LOWORD(wparam) == WA_INACTIVE)
		{
			mAppPaused = true;
			GetTimer().Stop();
		}
		else
		{
			mAppPaused = false;
			GetTimer().Start();
		}
		return 0;


	case WM_SIZE:
		mClientWidth  = LOWORD(lparam);
		mClientHeight = HIWORD(lparam);

		if( gd3dDev )
		{
			if( wparam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if( wparam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if( wparam == SIZE_RESTORED)
			{
				if(mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}
				else if(mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else
					// API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					OnResize();
			}
		}
		return 0;


	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing  = true;
		GetTimer().Stop();
		return 0;


	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing  = false;
		GetTimer().Start();
		OnResize();
		return 0;


	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);


	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lparam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lparam)->ptMinTrackSize.y = 200;
		return 0;

	}

	return DefWindowProc(mMainWnd, Msg, wparam, lparam);
}

void AbsD3D::ResetOMViews()
{
	gd3dDev->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	
	D3D10_VIEWPORT vp;
	vp.Width		= mClientWidth;
	vp.Height		= mClientHeight;
	vp.TopLeftX		= 0;
	vp.TopLeftY		= 0;
	vp.MinDepth		= 0.0f;
	vp.MaxDepth		= 1.0f;

	gd3dDev->RSSetViewports(1, &vp);
}