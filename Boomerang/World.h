#ifndef WORLD_H
#define WORLD_H

#include "D3DUtil.h"


struct GridPt
{
	GridPt()
	{
		ZeroOut();
	}
	void ZeroOut()
	{
		IsVerticalWall	 = false;
		IsHorizontalWall = false;
		IsPlayerSpawnPt    = false;
		IsAISpawnPt		   = false;
		IsExitPt		   = false;
		Enabled				= false;
		IsTrapPt			= false;
		p = D3DXVECTOR3(0.0f,0.0f,0.0f);
		i = 0;
		j = 0;
		index = 0;
	}

	int i;
	int j;
	int index;
	D3DXVECTOR3 p;
	D3DXVECTOR2 Vel;
	bool IsVerticalWall;
	bool IsHorizontalWall;
	bool IsPlayerSpawnPt;
	bool IsAISpawnPt;
	bool IsExitPt;
	bool Enabled;
	bool IsTrapPt;
	AABB Box;
};

class World
{
public:
	World();
	~World();
	
	//Map Creation
	void GenerateRandomMap(UINT NumOfRows, UINT NumOfCols, float TileSpacing);
	void FilterMap();

	void Init(std::wstring FileName);
	void GenerateVelocities();
	void GenerateTrapPts();

	const GridPt& GetGridPt(UINT i, UINT j);
	const GridPt& GetGridPt(UINT i);
	const GridPt& GetTile(UINT i);
	const GridPt& GetPlayerSpawnPt();
	const GridPt& GetExitPt();
	const GridPt& GetAISpawnPt(UINT Num);
	void ActivateTrapPt(UINT i);

	UINT  GetNumRows();
	UINT  GetNumCols();
	float GetTileSpacing();
	UINT  GetNumElements();
	UINT  GetNumTiles();
	UINT  GetFullSize();
	UINT  GetNumAI();
	void FilterTiles();

	void SetAABB(UINT i, AABB Box);

private:
	bool GetWorldFromEditor();
	void FilterAIGridPts();

private:
	std::wstring mFileName;
	std::vector<GridPt> mGridPts;
	std::vector<GridPt> mAITiles;
	std::vector<GridPt> mTiles;
	UINT mFullSize;
	UINT mNumElements;
	UINT mNumTiles;
	UINT mNumCols;
	UINT mNumRows;
	float mTileSpacing;
	int mSpawnPtIndex;
	int mExitPtIndex;
};

#endif