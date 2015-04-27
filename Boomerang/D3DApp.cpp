#include "AbsD3D.h"
#include "FX.h"
#include "TexManager.h"
#include "InputLayout.h"
#include "CollisionDetection.h"
#include "Camera.h"
#include "Sky.h"
#include "ParticleSystems.h"
#include "RenderToTex.h"
#include "World.h"
#include "AIManager.h"
#include <iomanip> 

//Vertex Structures
struct GridVertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Size;
	int Orientation;
};

//Game-Related Structures
struct Wall
{
	Wall()
	{
		ZeroOut();
	}
	void ZeroOut()
	{
		i = 0;
		j = 0;
		IsVerticalWall = false;
		IsHorizontalWall = false;
	}

	int i;
	int j;
	AABB Box;
	bool IsVerticalWall;
	bool IsHorizontalWall;
};

struct Player
{
	Player()
	{
		Speed = 0.0f;
		Height = 0.0f;
		Health = 0.0f;
	}

	D3DXVECTOR3 Pos;
	Mesh	MeshData;
	BS		Sphere;
	float	Speed;
	float	Height;
	double	Health;
};

//D3D Class
class D3DApp : public AbsD3D
{
public:

	D3DApp(HINSTANCE Hins, int Width, int Height);
	virtual ~D3DApp();

	virtual void InitApp();
	virtual void UpdateScene();
	virtual void RenderScene();
	virtual void OnResize();
	virtual LRESULT MsgProc(UINT msg, WPARAM wparam, LPARAM lparam);

protected:
	void GetFXHandles();
	
	void InitDataFields();
	void InitParticleSystems();
	void InitRTS();
	void InitWorld();
	void InitCubeMapping();
	void InitAI();
	void InitLighting();
	void InitPlayer();
	void InitExit();
	void InitLaserTurrets();

	void RestartLevel();
	void WonGame();
	void LostGame();

	void StartDamaging();
	void StopDamaging();

	void CheckPlayerCollision();
	void CheckLaserCollisions();
	void GenerateWorldAABBs();

	void CheckDeadLasers();
	void CreateAutomaticLaser();
	void CreateTrapLaser(const D3DXVECTOR3& Target);
	void CreateManualLaser(float dt);
	void ResetLaserInsts();
	void ResetPillarInst();

	void CheckWinLoss();
	void UpdateCamera();
	void UpdateAI();
	void UpdateWorld();
	void UpdateParticleSystems();
	void UpdateCollisions();
	void UpdatePlayer();
	void CheckForTrapTiles();

	void CheckShortJumping(float dt);
	void CheckLongJumping(float dt);

	void RenderAI();
	void RenderWorld();
	void RenderParticleSystems();
	void RenderSky();
	void RenderPlayer();
	void RenderExit();

	void RenderToFull();
	void RenderToScreen();
protected:

	//Win-loss
	bool	bWonGame;
	bool	bLostGame;

	//World
	World			mWorld;
	ID3D10Buffer*	mGridPtsVB;
	Light			mDirLight;
	ID3D10RasterizerState*	mRSNoCull;
	ID3D10DepthStencilState* mDDSAlways;
	std::vector<D3DXVECTOR3> mUnitCube;
	UINT			mLevel;
	UINT			mNumOfRows;
	UINT			mNumOfCols;
	float			mTileSpacing;

	//AI
	AIManager			mAIManager;
	std::vector<AI>		mLaserAIs;

	//Cube Mapping
	Sky	mSky;

	//Player
	Player		mPlayer;
	D3DXMATRIX	mPlayerWorld;
	bool		bTurningRight;	
	bool		bTurningLeft;
	float		mTurningCounter;
	float		mTurningDx;
	bool		bDidShortJump;
	bool		bDidLongJump;
	float		mAirTimer;
	float		mPlayerDy;
	bool		bPlayerFalling;
	int			mLongJumpsLeft;

	//Particle Systems
	PsPillarInst*	mPsExitPillar;

	std::vector<PsLaserInst*>	mPsLaserInsts;

	//RTS's
	RenderToTex RTS_Full;
	ID3D10Buffer* mQuadVB;

	//Blood Screen
	float mDamageCounter;
	double mMinDamageAmount;
	float mStartDamageLimit;

	bool  bIsPlayerDead;

	//>>>>>>>>>>>FX Handles	

	//Grid Tech
	ID3D10EffectTechnique*				mfxGridTech;
	ID3D10EffectMatrixVariable*			mfxGridWVP;
	ID3D10EffectMatrixVariable*			mfxGridW;
	ID3D10EffectShaderResourceVariable*	mfxGridTex;
	ID3D10EffectShaderResourceVariable*	mfxGridRandomTex;
	ID3D10EffectVectorVariable*			mfxGridEyePos;
	ID3D10EffectVariable*				mfxGridDirLight;

	//Ball Tech
	ID3D10EffectTechnique*				mfxBallTech;
	ID3D10EffectMatrixVariable*			mfxBallMatWVP;
	ID3D10EffectMatrixVariable*			mfxBallMatW;
	ID3D10EffectShaderResourceVariable*	mfxBallTex;
	ID3D10EffectVectorVariable*			mfxBallEyePos;
	ID3D10EffectVectorVariable*			mfxBallDiffMtrl;
	ID3D10EffectVariable*				mfxBallLight;

	//Quad Tech
	ID3D10EffectTechnique*				mfxQuadTech;
	ID3D10EffectShaderResourceVariable*	mfxQuadTex;

	//Opacity Tech
	ID3D10EffectTechnique*				mfxOpacityTech;
	ID3D10EffectScalarVariable*			mfxOpacityValue;
	ID3D10EffectShaderResourceVariable*	mfxOpacityOriginalTex;
	ID3D10EffectShaderResourceVariable*	mfxOpacityScreenTex;
};

int WINAPI WinMain(HINSTANCE Hins, HINSTANCE pHins, PSTR CmdLine, int ShowCmd)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	D3DApp App(Hins, 1000, 600);

	App.InitApp();

	return App.Run();
}

D3DApp::D3DApp(HINSTANCE Hins, int Width, int Height)
	:AbsD3D(Hins, Width, Height)
{
}

D3DApp::~D3DApp()
{
	if(!mPsLaserInsts.empty() )
	{
		for(UINT i = 0; i < mPsLaserInsts.size(); i++)
			delete mPsLaserInsts[i];

		mPsLaserInsts.clear();
	}

	if(mPsExitPillar != NULL)
	{
		delete mPsExitPillar;
	}

	ReleaseCOM(mRSNoCull);
	ReleaseCOM(mGridPtsVB);
	ReleaseCOM(mQuadVB);

	if(gd3dDev)
		gd3dDev->ClearState();

	FX::DestroyAll();
	InputLayout::DestroyAll();
}

LRESULT D3DApp::MsgProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
	return AbsD3D::MsgProc(msg, wparam, lparam);
}

void D3DApp::GetFXHandles()
{
	mfxGridTech				= FX::GridPtFX->GetTechniqueByName("GridTech");
	mfxGridWVP				= FX::GridPtFX->GetVariableByName("gMatWVP")->AsMatrix();
	mfxGridW				= FX::GridPtFX->GetVariableByName("gMatW")->AsMatrix();
	mfxGridTex				= FX::GridPtFX->GetVariableByName("gGridTex")->AsShaderResource();
	mfxGridRandomTex		= FX::GridPtFX->GetVariableByName("gRandomTex")->AsShaderResource();
	mfxGridEyePos			= FX::GridPtFX->GetVariableByName("gEyePos")->AsVector();
	mfxGridDirLight			= FX::GridPtFX->GetVariableByName("gDirLight");

	mfxBallTech				= FX::BallFX->GetTechniqueByName("AITech");
	mfxBallMatWVP			= FX::BallFX->GetVariableByName("gMatWVP")->AsMatrix();
	mfxBallMatW				= FX::BallFX->GetVariableByName("gMatW")->AsMatrix();
	mfxBallTex				= FX::BallFX->GetVariableByName("gTex")->AsShaderResource();
	mfxBallEyePos			= FX::BallFX->GetVariableByName("gEyePos")->AsVector();
	mfxBallDiffMtrl			= FX::BallFX->GetVariableByName("gDiffMtrl")->AsVector();
	mfxBallLight			= FX::BallFX->GetVariableByName("gLight");

	mfxQuadTech				= FX::PP_FX->GetTechniqueByName("QuadTech");
	mfxQuadTex				= FX::PP_FX->GetVariableByName("gQuadTex")->AsShaderResource();

	mfxOpacityTech			= FX::PP_FX->GetTechniqueByName("OpacityTech");
	mfxOpacityValue			= FX::PP_FX->GetVariableByName("gOpacity")->AsScalar();
	mfxOpacityOriginalTex	= FX::PP_FX->GetVariableByName("gOriginalTex")->AsShaderResource();
	mfxOpacityScreenTex		= FX::PP_FX->GetVariableByName("gScreenTex")->AsShaderResource();
}

void D3DApp::OnResize()
{
	AbsD3D::OnResize();
	float aspect = (float)mClientWidth/mClientHeight;
	GetCamera().SetLens(D3DX_PI/4.0f, aspect, 0.1f, 1000.0f);
}


void D3DApp::InitApp()
{
	//AbsD3D
	AbsD3D::InitApp();

	MessageBox(0, L"Welcome to Boomerang!", L"Boomerang", MB_OK);
	MessageBox(0, L"WASD : move the ball\nArrow Keys: Rotate the camera\nSpace to jump\nTab to long jump", L"Boomerang", MB_OK);
	MessageBox(0, L"Reach the end point to win.\nIf you fall you lose the game", L"Boomerang", MB_OK);
	MessageBox(0, L"Careful not to activate the traps\n Enjoy!!", L"Boomerang", MB_OK);
	//Texture Manager
	GetTexManager().Init();

	//Initilize FX
	FX::InitAll();
	GetFXHandles();
	
	//Initialize The Input Layout
	InputLayout::InitAll();

	//Initialize member variables(data fields)
	InitDataFields();

	//Game-related initializations
	InitLighting();
	InitParticleSystems();
	InitWorld();
	InitCubeMapping();
	InitRTS();
	InitAI();
	InitPlayer();
	InitLaserTurrets();
}

void D3DApp::InitDataFields()
{
	mLevel			= 1;
	mGridPtsVB		= 0;
	mQuadVB			= 0;
	mRSNoCull		= 0;
	mDDSAlways		= 0;
	mNumOfRows		= 17;
	mNumOfCols		= 17;
	mMinDamageAmount	= 2.75f;
	mStartDamageLimit	= 0.05f;
	mDamageCounter	= 0.0f;
	mTurningCounter = 0.0f;
	mTurningDx		= 0.0f;
	mAirTimer		= 0.0f;
	mTileSpacing	= 10.0f;
	bIsPlayerDead	= false;
	bTurningRight	= false;
	bTurningLeft	= false;
	bDidShortJump	= false;
	bDidLongJump	= false;
	bWonGame		= false;
	bLostGame		= false;
	bPlayerFalling	= false;
	mPsExitPillar	= 0;
}

void D3DApp::InitAI()
{
	mAIManager.Init();
	//Const FX Sets
	ID3D10ShaderResourceView* TexSRV  = GetTexManager().CreateTexture2D(L"WhiteTex.dds");
	HR(mfxBallTex->SetResource(TexSRV) );
	HR(mfxBallLight->SetRawValue((void*)&mDirLight, 0, sizeof(Light) ) );
}

void D3DApp::InitCubeMapping()
{
	//Sky
	mSky.Init(L"CubeMap0.dds", 10000.0f);
}

void D3DApp::InitParticleSystems()
{
	//RFE: Why use Tex arrays???

	//Pillars
	std::vector<std::wstring> SrcNames;
	SrcNames.push_back(L"smoke.dds");
	ID3D10ShaderResourceView* TexArraySRV = GetTexManager().CreateTextureArray(L"PsPillarTextures", SrcNames);
	UINT MaxParticles = 5000;
	GetPsPillar().Init(MaxParticles, L"Pillar.fx", TexArraySRV);

	//Lasers
	MaxParticles = 100000;
	ID3D10ShaderResourceView* Tex0SRV = GetTexManager().CreateTexture2D(L"bolt.dds");
	ID3D10ShaderResourceView* Tex1SRV = GetTexManager().CreateTexture2D(L"smoke.dds");
	GetPsLaser().Init(MaxParticles, L"Laser.fx", 5.0f, Tex0SRV, Tex1SRV);
}

void D3DApp::InitRTS()
{
	RTS_Full.Init(mClientWidth, mClientHeight, true, DXGI_FORMAT_R8G8B8A8_UNORM);

	PTvertex v[] = {
		{	D3DXVECTOR3(-1.0f,-1.0f,0.0f), D3DXVECTOR2(0.0f,1.0f)	},
		{	D3DXVECTOR3(-1.0f, 1.0f,0.0f), D3DXVECTOR2(0.0f,0.0f)	},
		{	D3DXVECTOR3( 1.0f,-1.0f,0.0f), D3DXVECTOR2(1.0f,1.0f)	},
		{	D3DXVECTOR3( 1.0f, 1.0f,0.0f), D3DXVECTOR2(1.0f,0.0f)	}
	};

	D3D10_BUFFER_DESC vbd;
	vbd.BindFlags		 = D3D10_BIND_VERTEX_BUFFER;
	vbd.ByteWidth		 = sizeof(PTvertex)*4;
	vbd.CPUAccessFlags	 = 0;
	vbd.MiscFlags		 = 0;
	vbd.Usage			 = D3D10_USAGE_IMMUTABLE;

	D3D10_SUBRESOURCE_DATA Data;
	Data.pSysMem = v;

	HR(gd3dDev->CreateBuffer(&vbd, &Data, &mQuadVB) );
}

void D3DApp::InitWorld()
{
	RestartLevel();

	//Fill the vertex buffer
	UINT NumOfElements = mWorld.GetNumElements();
	float TileSpacing = mWorld.GetTileSpacing();
	D3DXVECTOR2 s = D3DXVECTOR2(TileSpacing,TileSpacing);
	Array1D<GridVertex> v(NumOfElements);
	
	UINT k = 0;
	for(UINT i = 0; i < NumOfElements; i++)
	{
		const GridPt& Pt = mWorld.GetGridPt(i);
		if(Pt.Enabled)
		{
			v[k].Pos		  = Pt.p;
			v[k].Size		  = s;
			v[k].Orientation  = 0;
			if(Pt.IsVerticalWall)			v[k].Orientation = 1;
			else if(Pt.IsHorizontalWall)	v[k].Orientation = 2;
			k++;
		}
	}

	D3D10_BUFFER_DESC vbd;
	vbd.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
	vbd.ByteWidth		= sizeof(GridVertex)*NumOfElements;
	vbd.CPUAccessFlags	= 0;
	vbd.MiscFlags		= 0;
	vbd.Usage			= D3D10_USAGE_IMMUTABLE;

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = v.GetDataPointer();
	HR(gd3dDev->CreateBuffer(&vbd, &InitData, &mGridPtsVB) );

	//Lighting & Texturing
	D3DXVECTOR3 LightPos(5.0f, 5.0f, 0.0f);
	mDirLight.Dir = D3DXVECTOR3(0.0f,0.0f,0.0f) - LightPos;
	D3DXVec3Normalize(&mDirLight.Dir,&mDirLight.Dir);

	mDirLight.Diffuse	= D3DCOLOR_WHITE;
	mDirLight.Ambient	= D3DCOLOR_WHITE;
	mDirLight.Specular	= D3DCOLOR_BLACK;
	mDirLight.Atten		= D3DXVECTOR3(1.0f,0.0f,0.0f);
	
	HR(mfxGridDirLight->SetRawValue((void*)&mDirLight, 0, sizeof(Light) ) );
	ID3D10ShaderResourceView* RandomTex = GetTexManager().GetRandomTexture();
	ID3D10ShaderResourceView* GridTex = GetTexManager().CreateTexture2D(L"GridTex3.dds");
	HR(mfxGridRandomTex->SetResource(RandomTex) );
	HR(mfxGridTex->SetResource(GridTex) );

	//No cull RS
	D3D10_RASTERIZER_DESC RzDesc;
	ZeroMemory(&RzDesc, sizeof(RzDesc) );
	RzDesc.CullMode = D3D10_CULL_NONE;
	RzDesc.FillMode = D3D10_FILL_SOLID;

	HR(gd3dDev->CreateRasterizerState(&RzDesc, &mRSNoCull) );

	//DDS Always
	D3D10_DEPTH_STENCIL_DESC DepthDesc;
	ZeroMemory(&DepthDesc, sizeof(DepthDesc) );
	DepthDesc.DepthEnable		= true;
	DepthDesc.DepthFunc			= D3D10_COMPARISON_ALWAYS;
	DepthDesc.DepthWriteMask	= D3D10_DEPTH_WRITE_MASK_ALL;

	HR(gd3dDev->CreateDepthStencilState(&DepthDesc, &mDDSAlways) );
}

void D3DApp::InitLighting()
{
	//Lighting
	D3DXVECTOR3 LightPos(5.0f, 5.0f, 0.0f);
	mDirLight.Dir = D3DXVECTOR3(0.0f,0.0f,0.0f) - LightPos;
	D3DXVec3Normalize(&mDirLight.Dir,&mDirLight.Dir);

	mDirLight.Diffuse	= D3DCOLOR_WHITE;
	mDirLight.Ambient	= D3DCOLOR_WHITE;
	mDirLight.Specular	= D3DCOLOR_BLACK;
	mDirLight.Atten		= D3DXVECTOR3(1.0f,0.0f,0.0f);
}

void D3DApp::InitPlayer()
{
	mPlayer.Height			= 5.0f;
	mPlayer.Speed			= 15.0f;
	mPlayer.Health			= 100.0;
	mLongJumpsLeft			= 5;
	mPlayer.Pos				= mWorld.GetPlayerSpawnPt().p;
	mPlayer.Pos.y			= mPlayer.Height;
	mPlayer.Sphere.Center	= mPlayer.Pos;
	mPlayer.Sphere.Radius	= 2.0f;
	mPlayer.MeshData		= GetMeshManager().CreateSphere();
	D3DXMatrixScaling(&mPlayer.MeshData.ScalingMatrix, mPlayer.Sphere.Radius, mPlayer.Sphere.Radius, mPlayer.Sphere.Radius);
}

void D3DApp::InitLaserTurrets()
{
	AI LaserAI;
	D3DXVECTOR3 Pos(170.0f, 10.0f, 100.0f);
	D3DXVECTOR2 Vel(0.0f, 0.0f);
	LaserAI = mAIManager.CreateAI(Pos, Vel, 1.5f, false, true);
	mLaserAIs.push_back(LaserAI);

	Pos = D3DXVECTOR3(85.0f, 10.0f, 170.0f);
	LaserAI = mAIManager.CreateAI(Pos, Vel, 1.5f, false, true);
	mLaserAIs.push_back(LaserAI);

	Pos = D3DXVECTOR3(85.0f, 10.0f, 0.0f);
	LaserAI = mAIManager.CreateAI(Pos, Vel, 1.5f, false, true);
	mLaserAIs.push_back(LaserAI);
}

void D3DApp::InitExit()
{
	D3DXVECTOR2 ParticleSize(1.0f,1.0f);
	D3DXVECTOR3 InitVec(0.0f,0.0f,0.0f);
	float Radius = 4.0f;
	float ThetaStep = 0.5f;
	float VelocityStep = 0.1f;
	D3DXVECTOR3 ExitPos = mWorld.GetExitPt().p;

	D3DXMATRIX T;
	D3DXMatrixTranslation(&T, ExitPos.x, ExitPos.y, ExitPos.z);

	mPsExitPillar = new PsPillarInst();
	mPsExitPillar->Init(T, InitVec, false, ThetaStep, VelocityStep,
		Radius, ParticleSize);
}


void D3DApp::StartDamaging()
{
	float dt = GetTimer().GetDT();
	mDamageCounter += dt;

	if(mDamageCounter > mStartDamageLimit)
	{
		mDamageCounter = 0.0f;
		mPlayer.Health -= mMinDamageAmount;
	}

	if(mPlayer.Health <= 0.0f)
	{
		bIsPlayerDead = true;
		LostGame();
	}
}

void D3DApp::StopDamaging()
{
	mDamageCounter = 0.0f;
}

void D3DApp::CheckLaserCollisions()
{
	//check the Laser-player collisions
	bool IsAttacking = false;
	const BS& SphereBS = mPlayer.Sphere;
	for(UINT i = 0; i < mPsLaserInsts.size(); i++)
	{
		if(mPsLaserInsts[i]->IsAccelerating() )
		{
			const D3DXVECTOR3& RayPos = mPsLaserInsts[i]->GetInitPos();
			const D3DXVECTOR3& RayDir = mPsLaserInsts[i]->GetRayDir(); 
		
			if(IT_RayBS(RayPos, RayDir, SphereBS) )
			{
				IsAttacking = true;
				break;
			}
		}
	}

	if(IsAttacking)
	{
		StartDamaging();
	}
	else
	{
		StopDamaging();
	}
}

void D3DApp::GenerateWorldAABBs()
{
	float HalfWidth  = mWorld.GetTileSpacing()*0.5f;
	float HalfHeight = mWorld.GetTileSpacing()*0.5f;
	float HalfDepth  = mWorld.GetTileSpacing()*0.5f;

	AABB MainBox;
	MainBox.Min = D3DXVECTOR3(-HalfWidth, -HalfHeight, -HalfDepth);
	MainBox.Max = D3DXVECTOR3(HalfWidth, HalfHeight, HalfDepth);

	for(UINT i = 0; i < mWorld.GetNumElements(); i++)
	{
		const GridPt& Pt = mWorld.GetGridPt(i);

		if(Pt.Enabled)
		{
			D3DXMATRIX T;
			D3DXMatrixTranslation(&T, Pt.p.x, Pt.p.y + HalfHeight, Pt.p.z);

			AABB Box;
			Box.Min = MainBox.Min;
			Box.Max = MainBox.Max;

			Box.ApplyMatrix(T);
			mWorld.SetAABB(i, Box);
		}
	}

}


void D3DApp::CheckDeadLasers()
{
	for(UINT i = 0; i < mPsLaserInsts.size(); i++)
		if(mPsLaserInsts[i]->IsDestroyed() ||
			mPsLaserInsts[i]->GetSystemAge() >= GetPsLaser().GetDeathTime() + 0.5f )
		{
			delete mPsLaserInsts[i];
			mPsLaserInsts.erase(mPsLaserInsts.begin()+i);
			i--;
		}
}
	
void D3DApp::CreateTrapLaser(const D3DXVECTOR3& Target)
{
	//UINT rand = (UINT)RandF(0.0f, ((float)mLaserAIs.size() - 1.0f) );
	for(int i = 0; i < mLaserAIs.size(); i++)
	{
		D3DXVECTOR3 Dir = Target - mLaserAIs[i].Pos;
		D3DXVec3Normalize(&Dir, &Dir);
		PsLaserInst* ps = new PsLaserInst();
		ps->Init(mLaserAIs[i].Pos, mLaserAIs[i].World, Dir, 300, 0.6f);
		ps->Enable();
		mPsLaserInsts.push_back(ps);
	}
}

void D3DApp::CreateManualLaser(float dt)
{
	/*static bool once = true;
	if(GetAsyncKeyState('T') & 0x8000)
	{
		UINT rand = (UINT)RandF(0.0f, ((float)mLaserAIs.size() - 1.0f) );
	
		D3DXVECTOR3 Dir = mPlayer.Pos - mLaserAIs[rand].Pos;
		D3DXVec3Normalize(&Dir, &Dir);

		PsLaserInst* ps = new PsLaserInst();
		ps->Init(mLaserAIs[rand].Pos, mLaserAIs[rand].World, Dir, 200, 1.0f);
		ps->Enable();
		mPsLaserInsts.push_back(ps);
	}*/
}

void D3DApp::UpdateScene()
{
	AbsD3D::UpdateScene();

	if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) PostQuitMessage(0);

	//Check Collisions
	UpdateCollisions();
	
	//Update the camera controls
	UpdateCamera();
	UpdatePlayer();

	//Update the Particle Systems
	UpdateParticleSystems();

	//Check for any activated trap tiles
	CheckForTrapTiles();

	CheckWinLoss();
}

void D3DApp::CheckWinLoss()
{
	float dt = GetTimer().GetDT();
	//Check win loss situations
	static float RestartTimer = 0.0f;
	if(bWonGame || bLostGame)
	{
		RestartTimer += dt;
		if(RestartTimer >= 3.0f)
		{
			if(bLostGame)
			{
				mLongJumpsLeft = 5;
				mPlayer.Health = 100.0f;
				mLevel = 1;
			}
			RestartTimer = 0.0f;
			RestartLevel();
		}
	}
}

void D3DApp::UpdateParticleSystems()
{
	float dt = GetTimer().GetDT();

	CheckDeadLasers();
	CreateManualLaser(dt);
}

void D3DApp::UpdateCollisions()
{
	CheckLaserCollisions();
}

void D3DApp::CheckShortJumping(float dt)
{
	if( (GetAsyncKeyState(VK_SPACE) & 0x8000) && !bDidShortJump && !bDidLongJump)  
	{
		bDidShortJump = true;
	}
	
	mPlayerDy = 0.0f;
	if(bDidShortJump)
	{
		mAirTimer += dt * 0.5f;
		mPlayerDy = (1.7f*mAirTimer) + (0.5f * -9.81f * mAirTimer * mAirTimer);
		mPlayer.Pos.y += mPlayerDy;
	}
}

void D3DApp::CheckLongJumping(float dt)
{
	if(!bDidLongJump && mLongJumpsLeft <= 0)
		return;

	if( (GetAsyncKeyState(VK_TAB) & 0x8000) && !bDidShortJump && !bDidLongJump)  
	{
		bDidLongJump = true;
		mLongJumpsLeft--;
	}

	mPlayerDy = 0.0f;
	if(bDidLongJump)
	{
		mAirTimer += dt * 0.5f;
		mPlayerDy = (4.0f*mAirTimer) + (0.5f * -9.81f * mAirTimer * mAirTimer);
		mPlayer.Pos.y += mPlayerDy;
	}
}

void D3DApp::UpdatePlayer()
{
	float dt = GetTimer().GetDT();

	//Movement code
	D3DXVECTOR3 Vel(0.0f,0.0f,0.0f);
	float s = mPlayer.Speed;
	
	if(GetAsyncKeyState('A') & 0x8000) {Vel += -GetCamera().Right();}
	if(GetAsyncKeyState('D') & 0x8000) {Vel +=  GetCamera().Right();}
	if(GetAsyncKeyState('W') & 0x8000) {Vel +=  GetCamera().Look();}
	if(GetAsyncKeyState('S') & 0x8000) {Vel += -GetCamera().Look();}
	if(GetAsyncKeyState(VK_RIGHT) & 0x8000) bTurningRight = true;
	else									bTurningRight = false;
	if(GetAsyncKeyState(VK_LEFT) & 0x8000)	bTurningLeft = true;
	else									bTurningLeft = false;

	mPlayer.Pos += (Vel*s) * dt;
	mPlayer.Sphere.Center = mPlayer.Pos;

	D3DXMatrixTranslation(&mPlayerWorld, mPlayer.Pos.x, mPlayer.Pos.y, mPlayer.Pos.z);
	mPlayerWorld = mPlayer.MeshData.ScalingMatrix * mPlayerWorld;

	CheckPlayerCollision();

	if(bPlayerFalling)
	{
		LostGame();
		mPlayer.Pos.y -= s * s * dt;
	}
	//Jumping code
	if(!bWonGame && !bLostGame)
	{
		CheckShortJumping(dt);
		CheckLongJumping(dt);
	}
	
	//Smooth turning code
	if(bTurningRight || bTurningLeft)
	{
		if(mTurningCounter < 3.0f)
			mTurningCounter += dt * 6.0f;

		mTurningDx += mTurningCounter * (bTurningRight? 0.007f : -0.007f);
	}
	else
	{
		mTurningCounter = 0.0f;
	}
	if( (bDidShortJump || bDidLongJump) && mPlayer.Pos.y <= mPlayer.Height)
	{
		//Just landed. Reset the jump values
		bDidShortJump = false;
		bDidLongJump  = false;
		mAirTimer = 0.0f;
		mPlayerDy = 0.0f;
		mPlayer.Pos.y = mPlayer.Height;
	}
}

void D3DApp::UpdateCamera()
{
	float dt = GetTimer().GetDT();
	
	if(!bPlayerFalling)
	{
		//Position the camera to orbit around the player
		float x = -40.0f * cosf(mTurningDx);
		float z = 40.0f * sinf(mTurningDx);
		GetCamera().Pos().x = x;
		GetCamera().Pos().y = 60.0f;
		GetCamera().Pos().z = z;

		D3DXMATRIX T;
		D3DXMatrixTranslation(&T, mPlayer.Pos.x, mPlayer.Pos.y, mPlayer.Pos.z);
		D3DXVec3TransformCoord(&GetCamera().Pos(), &GetCamera().Pos(), &T);

		//Build View matrix
		D3DXVECTOR3 Target = mPlayer.Pos;
		Target.y += 7.5f;
		D3DXMatrixLookAtLH(&GetCamera().ViewMatrix(), &GetCamera().Pos(), &Target, &GetCamera().Up());

		GetCamera().Up() = D3DXVECTOR3(0.0f,1.0f,0.0f);

		D3DXVECTOR3 v0 = GetCamera().Pos();
		D3DXVECTOR3 v1 = mPlayer.Pos;
		v0.y = v1.y = 0.0f;
		GetCamera().Look() = v1-v0;
		D3DXVec3Normalize(&GetCamera().Look(), &GetCamera().Look());
	
		D3DXVec3Cross(&GetCamera().Right(), &GetCamera().Up(), &GetCamera().Look());
		D3DXVec3Normalize(&GetCamera().Right(),&GetCamera().Right());

		D3DXVec3Cross(&GetCamera().Up(), &GetCamera().Look(), &GetCamera().Right());
		D3DXVec3Normalize(&GetCamera().Up(), &GetCamera().Up());
	}
}

void D3DApp::CheckForTrapTiles()
{
	for(int i = 0; i < mWorld.GetNumTiles(); i++)
	{
		const GridPt& Tile = mWorld.GetTile(i);

		if(Tile.IsTrapPt)
		{
			float TileSpacing = mWorld.GetTileSpacing();
			float Xa = Tile.p.x - TileSpacing/2.0f;
			float Xb = Tile.p.x + TileSpacing/2.0f;
			float Za = Tile.p.z - TileSpacing/2.0f;
			float Zb = Tile.p.z + TileSpacing/2.0f;

			if(mPlayer.Pos.x > Xa && mPlayer.Pos.x <= Xb
				&& mPlayer.Pos.z > Za && mPlayer.Pos.z <= Zb
				&& mPlayer.Pos.y <= mPlayer.Height + 1.0f)
			{
				mWorld.ActivateTrapPt(i);
				D3DXVECTOR3 Target = Tile.p;
				Target.y += mPlayer.Height;
				CreateTrapLaser(Target);
			}
		}
	}
}


void D3DApp::RenderScene()
{
	AbsD3D::RenderScene();
	
	RenderToFull();
	RenderToScreen();
	
	gd3dDev->RSSetState(0);

	//Frame Statistics
	std::wostringstream wss;
	RECT R = {5, 5, 0, 0};
	wss << "Health: " << mPlayer.Health;
	mHUDFont->DrawText(0, wss.str().c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_WHITE);

	R.top = 20;
	R.left = 5;
	R.bottom = 0;
	R.right = 0;
	wss.str(L"");
	wss << "Long Jumps: " << mLongJumpsLeft;
	mHUDFont->DrawText(0, wss.str().c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_WHITE);

	R.top = 37;
	R.left = 5;
	R.bottom = 0;
	R.right = 0;
	wss.str(L"");
	wss << "Level: " << mLevel;
	mHUDFont->DrawText(0, wss.str().c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_WHITE);

	if(bWonGame)
	{
		R.top = 50;
		R.left = 350;
		R.bottom = 0;
		R.right = 0;
		wss.str(L"");
		wss << "You Win!!!!";
		mAnnounceFont->DrawText(0, wss.str().c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_BLUE);
	}
	else if(bLostGame)
	{
		R.top = 50;
		R.left = 350;
		R.bottom = 0;
		R.right = 0;
		wss.str(L"");
		wss << "You Lose!!!!";
		mAnnounceFont->DrawText(0, wss.str().c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_RED);
	}

	HR(mSwapChain->Present(0,0));
}

void D3DApp::RenderAI()
{
	//AI Sphere
	float dt = GetTimer().GetDT();
	gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gd3dDev->IASetInputLayout(InputLayout::ILPos);

	D3D10_TECHNIQUE_DESC TechDesc;
	mfxBallTech->GetDesc(&TechDesc);

	HR(mfxBallEyePos->SetFloatVector((float*)&GetCamera().Pos() ) );
	D3DXVECTOR4 Red = D3DCOLOR_RED;
	Red *= 0.5f;

	HR(mfxBallDiffMtrl->SetFloatVector((float*)&Red) );
	
	for(UINT i = 0; i < mAIManager.GetNumAI(); i++)
	{
		const AI& a = mAIManager.GetConstAI(i);

		//D3DXMATRIX WVP = a.World*GetCamera().VP();
		D3DXMATRIX WVP = a.World*GetCamera().ViewMatrix()*GetCamera().ProjMatrix();
		HR(mfxBallMatWVP->SetMatrix((float*)&WVP) );
		HR(mfxBallMatW->SetMatrix((float*)&a.World) );

		for(UINT j = 0; j < TechDesc.Passes; j++)
		{
			HR(mfxBallTech->GetPassByIndex(j)->Apply(0) );
			mAIManager.DrawSphere(0);
		}
	}
}

void D3DApp::RenderWorld()
{
	//World
	gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
	gd3dDev->IASetInputLayout(InputLayout::ILPosSizeVH);
	gd3dDev->RSSetState(mRSNoCull);

	D3D10_TECHNIQUE_DESC TechDesc;
	mfxGridTech->GetDesc(&TechDesc);

	UINT Stride = sizeof(GridVertex);
	UINT Offset = 0;
	gd3dDev->IASetVertexBuffers(0, 1, &mGridPtsVB, &Stride, &Offset);
	D3DXMATRIX World;
	D3DXMatrixIdentity(&World);
	//D3DXMATRIX WVP = World * GetCamera().VP();
	D3DXMATRIX WVP = World * GetCamera().ViewMatrix()*GetCamera().ProjMatrix();
	HR(mfxGridWVP->SetMatrix((float*)&WVP) );
	HR(mfxGridW->SetMatrix((float*)&World) );
	HR(mfxGridEyePos->SetFloatVector((float*)&GetCamera().Pos() ) );
	for(UINT i = 0; i < TechDesc.Passes; i++)
	{
		HR(mfxGridTech->GetPassByIndex(i)->Apply(0) )
		gd3dDev->Draw(mWorld.GetNumElements() , 0);
	}
	
	gd3dDev->RSSetState(0);
}

void D3DApp::RenderParticleSystems()
{
	mPsExitPillar->Render();

	for(UINT i = 0; i < mPsLaserInsts.size(); i++)
		mPsLaserInsts[i]->Render();
}

void D3DApp::RenderSky()
{
	mSky.Render(GetCamera().ViewMatrix()*GetCamera().ProjMatrix());
}

void D3DApp::RenderPlayer()
{
	float dt = GetTimer().GetDT();
	gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gd3dDev->IASetInputLayout(InputLayout::ILPos);

	D3D10_TECHNIQUE_DESC TechDesc;
	mfxBallTech->GetDesc(&TechDesc);

	HR(mfxBallEyePos->SetFloatVector((float*)&GetCamera().Pos() ) );

	D3DXVECTOR4 Blue = D3DCOLOR_BLUE;
	Blue *= 0.5f;
	HR(mfxBallDiffMtrl->SetFloatVector((float*)&Blue) );
	
	//D3DXMATRIX WVP = mPlayerWorld*GetCamera().VP();
	D3DXMATRIX WVP = mPlayerWorld * GetCamera().ViewMatrix()*GetCamera().ProjMatrix();
	HR(mfxBallMatWVP->SetMatrix((float*)&WVP) );
	HR(mfxBallMatW->SetMatrix((float*)&mPlayerWorld) );

	for(UINT j = 0; j < TechDesc.Passes; j++)
	{
		HR(mfxBallTech->GetPassByIndex(j)->Apply(0) );
		mPlayer.MeshData.Data->DrawSubset(0);
	}
}

void D3DApp::RenderToFull()
{
	gd3dDev->OMSetDepthStencilState(0,0);
	float BlendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	gd3dDev->OMSetBlendState(0, BlendFactor, 0xffffffff);

	RTS_Full.Begin();

	//World
	RenderWorld();
	
	//AI
	RenderAI();

	//Player
	RenderPlayer();

	//Sky
	RenderSky();

	//Particle Systems
	RenderParticleSystems();

	RTS_Full.End();
}

void D3DApp::RenderToScreen()
{
	//Crucial part of this whole functions --
	AbsD3D::ResetOMViews();

	UINT Stride = sizeof(PTvertex);
	UINT Offset = 0;
	gd3dDev->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	gd3dDev->IASetInputLayout(InputLayout::ILPosTex);
	gd3dDev->IASetVertexBuffers(0, 1, &mQuadVB, &Stride, &Offset);

	D3D10_TECHNIQUE_DESC TechDesc;
	HR(mfxQuadTech->GetDesc(&TechDesc) );

	HR(mfxQuadTex->SetResource(RTS_Full.GetColorMap() ) );

	for(UINT i = 0; i < TechDesc.Passes; i++)
	{
		HR(mfxQuadTech->GetPassByIndex(i)->Apply(0) );
		gd3dDev->Draw(4, 0);
	}
}

void D3DApp::ResetLaserInsts()
{
	for(UINT i = 0; i < mPsLaserInsts.size(); i++)
	{
		delete mPsLaserInsts[i];
		mPsLaserInsts.erase(mPsLaserInsts.begin()+i);
		i--;
	}
}

void D3DApp::ResetPillarInst()
{
	if(mPsExitPillar != 0)
		delete mPsExitPillar;
}

void D3DApp::RestartLevel()
{
	//Reset win-loss flags
	bWonGame = false;
	bLostGame = false;
	bIsPlayerDead = false;
	bPlayerFalling = false;
	
	//Reset particle systems
	ResetLaserInsts();
	ResetPillarInst();

	//Generate random maps
	mWorld.GenerateRandomMap(mNumOfRows, mNumOfCols, mTileSpacing);

	//Generate World AABBs
	GenerateWorldAABBs();

	//Generate trap tiles
	mWorld.FilterTiles();
	InitExit();

	//Restart player
	mPlayer.Pos				= mWorld.GetPlayerSpawnPt().p;
	mPlayer.Pos.y			= mPlayer.Height;
}

void D3DApp::WonGame()
{
	bWonGame = true;
	mLevel++;
}

void D3DApp::LostGame()
{
	bLostGame = true;
}

void D3DApp::CheckPlayerCollision()
{
	if(bDidLongJump || bDidShortJump || bWonGame || bLostGame)
		return;

	const BS& SphereBS = mPlayer.Sphere;
	bPlayerFalling = true;
	for(int i = 0; i < mWorld.GetNumTiles(); i++)
	{
		const AABB& BoxAABB = mWorld.GetTile(i).Box;
		//If there is at least one interaction, then the player isn't falling
		if(IT_SphereAABB(SphereBS.Center, SphereBS.Radius, BoxAABB) )
		{
			if(mWorld.GetTile(i).IsExitPt)
			{
				WonGame();
				bPlayerFalling = false;
				return;
			}

			//Ball is not falling
			bPlayerFalling = false;
			break;
		}
	}

}