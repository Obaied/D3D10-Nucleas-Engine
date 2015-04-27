#include "World.h"
#include <fstream>
#include <algorithm>

//Operator Overloadings
//==================================================================
std::wistream& operator>>(std::wistream& in, D3DXVECTOR3& v)
{
	in >> v.x >> v.y >> v.z;
	return in;
}

std::wistream& operator>>(std::wistream& in, D3DXVECTOR2& v)
{
	in >> v.x >> v.y;
	return in;
}

World::World()
{
}

World::~World()
{
}

void World::Init(std::wstring FileName)
{
	mFileName = FileName;
	if(!GetWorldFromEditor() )
	{
		MessageBox(0, 
			L"Couldn't process world. Check \"Tiles.txt\" ",
			L"Game Project 0",
			MB_ICONERROR|MB_OK);
	}
}

bool World::GetWorldFromEditor()
{
	std::wifstream in(L"Tiles.txt");
	if(!in.fail() )
	{
		std::wstring ignore;

		////Full_Size//
		in >> ignore;
		in >> mFullSize;
		mGridPts.resize(mFullSize);

		////Num Of Elements//
		in >> ignore;
		in >> mNumElements;

		////Num Of Tiles//
		in >> ignore;
		in >> mNumTiles;

		////Tile_Spacing//
		in >> ignore;
		in >> mTileSpacing;

		////Number_of_Rows//
		in >> ignore;
		in >> mNumRows;

		////Number_of_Cols//
		in >> ignore;
		in >> mNumCols;

		////Elements//
		in >> ignore;
		////i_j|X_Y_Z|IsPlayerSpawnPt|IsAISpawnPt|IsExitPt|IsHorizontalWall|IsVerticalWall//
		in >> ignore;

		bool Enabled = false;
		static bool PlayerSpawnPtOnce = true;
		static bool ExitPtOnce = true;
		for(UINT i = 0; i < mFullSize; i++)
		{
			in >> Enabled;
			if(Enabled)
			{
				in >> mGridPts[i].i;
				in >> mGridPts[i].j;
				in >> mGridPts[i].p;
				in >> mGridPts[i].IsPlayerSpawnPt;
				in >> mGridPts[i].IsAISpawnPt;
				in >> mGridPts[i].IsExitPt;
				in >> mGridPts[i].IsHorizontalWall;
				in >> mGridPts[i].IsVerticalWall;
				mGridPts[i].Enabled = true;
				mGridPts[i].index = i;
				if(mGridPts[i].IsPlayerSpawnPt && PlayerSpawnPtOnce)
				{
					PlayerSpawnPtOnce = false;
					mSpawnPtIndex = i;
				}
				else if(mGridPts[i].IsExitPt && ExitPtOnce)
				{
					ExitPtOnce = false;
					mExitPtIndex = i;
				}
			}
			else
				mGridPts[i].Enabled = false;
		}

		//Get the AI GridPts from the regular GridPts and put them in a seperate vector
		FilterAIGridPts();

		//Get the tile GridPts from the regular GridPts and put them in a seperate vector
		FilterTiles();
	}

	else
		return false;
	
	in.close();
	return true;
}

void World::FilterAIGridPts()
{
	//get the AI Spawn Points and put them in a seperate vector
	for(UINT i = 0; i < mFullSize; i++)
	{
		if(mGridPts[i].IsAISpawnPt)
			mAITiles.push_back(mGridPts[i]);
	}

	GenerateVelocities();
}

void World::GenerateVelocities()
{
	//Velocities
	//=========================================================
	std::vector<D3DXVECTOR2> PossibleVels;

	for(UINT i = 0; i < mAITiles.size(); i++)
	{
		GridPt& a = mAITiles[i];

		const int Current_i = a.i;
		const int Current_j = a.j;

		//DIR_DOWN
		int Next_i = Current_i;
		int Next_j = Current_j;

		const GridPt& Next_a_Down = GetGridPt(++Next_i, Next_j);
		if(Next_a_Down.Enabled && !Next_a_Down.IsVerticalWall && !Next_a_Down.IsHorizontalWall)
			PossibleVels.push_back(DIR_DOWN);

		//DIR_UP
		Next_i = Current_i;
		Next_j = Current_j;

		const GridPt& Next_a_Up = GetGridPt(--Next_i, Next_j);
		if(Next_a_Up.Enabled && !Next_a_Up.IsVerticalWall && !Next_a_Up.IsHorizontalWall)
			PossibleVels.push_back(DIR_UP);

		//DIR_RIGHT
		Next_i = Current_i;
		Next_j = Current_j;

		const GridPt& Next_a_Right = GetGridPt(Next_i, ++Next_j);
		if(Next_a_Right.Enabled && !Next_a_Right.IsVerticalWall && !Next_a_Right.IsHorizontalWall)
			PossibleVels.push_back(DIR_RIGHT);
		//DIR_LEFT
		Next_i = Current_i;
		Next_j = Current_j;

		const GridPt& Next_a_Left = GetGridPt(Next_i, --Next_j);
		if(Next_a_Left.Enabled && !Next_a_Left.IsVerticalWall && !Next_a_Left.IsHorizontalWall)
			PossibleVels.push_back(DIR_LEFT);

		if(PossibleVels.empty() )
		{
			a.Vel = D3DXVECTOR2(0.0f,0.0f);
		}
		else
		{
			UINT Rand = (UINT)RandF(0.0f, (float)PossibleVels.size() );
			a.Vel = PossibleVels[Rand];
		}

		PossibleVels.clear();
	}

}

void World::FilterTiles()
{
	//Generate the trap tiles
	GenerateTrapPts();
	//then get the tile Points and put them in a seperate vector
	for(UINT i = 0; i < mNumElements; i++)
	{
		if(mGridPts[i].Enabled
			&& !mGridPts[i].IsHorizontalWall
			&& !mGridPts[i].IsVerticalWall)
		{
			mTiles.push_back(mGridPts[i]);
		}
	}

}

void World::GenerateTrapPts()
{
	//Simply, make a random percentage of the maps grid points as traps that have a bool turned on in them
	int NumOfTrapPts;
	std::vector<int> TrapPtIndices;
	NumOfTrapPts = (UINT)(GetNumTiles() * 0.9f);
	if(mNumTiles <= 0 || NumOfTrapPts <= 0)
		return;

	for(;;)
	{
		int Rand = (UINT)RandF(0.0f, (float)GetNumTiles() );
		if(Rand == GetPlayerSpawnPt().index || Rand == GetExitPt().index
			|| mGridPts[Rand].IsHorizontalWall || mGridPts[Rand].IsVerticalWall || !mGridPts[Rand].Enabled)
		{
			continue;
		}
		for(int j = 0; j < TrapPtIndices.size(); j++)
		{
			if(Rand == TrapPtIndices[j])
				continue;
		}

		//So that random index is not a player spawn point, an exit point, or an already taken trap point index
		// We can assign it to be a trap point

		mGridPts[Rand].IsTrapPt = true;
		TrapPtIndices.push_back(Rand);

		//exit the loop when we have gotten all our trap points
		if(TrapPtIndices.size() == NumOfTrapPts)
			break;
	}
}

void World::ActivateTrapPt(UINT i)
{
	if(i >= mNumTiles)
		return;

	mTiles[i].IsTrapPt = false;
}

const GridPt& World::GetGridPt(UINT i, UINT j)
{
	if(i >= mNumRows){
		Error("GetGridPt(UINT i, UINT j):Error trying to access mGridPts over its size with a value of \
			'i' greater than the maximum number of rows\n"); 
	}
	if(j >= mNumCols){
		Error("GetGridPt(UINT i, UINT j):Error trying to access mGridPts over its size with a value of \
			'j' greater than the maximum number of columns\n"); 
	}
	return mGridPts[i*mNumCols +j];
}

const GridPt& World::GetGridPt(UINT i)
{
	if(i >= mFullSize){
		Error("GetGridPt(UINT i):Error trying to access mGridPts over its size");
	}
	return mGridPts[i];
}

const GridPt& World::GetTile(UINT i)
{
	if(i >= mNumTiles){
		Error("GetTile(UINT i):Error trying to access mTiles over its size");
	}
	return mTiles[i];
}

const GridPt& World::GetPlayerSpawnPt()
{
	return GetGridPt(mSpawnPtIndex);
}

const GridPt& World::GetExitPt()
{
	return GetGridPt(mExitPtIndex);
}

const GridPt& World::GetAISpawnPt(UINT Num)
{
	if(Num > mAITiles.size() ){
		Error("Error trying to access mAITiles over its size\n");
	}
	return mAITiles[Num];
}

UINT World::GetNumRows()
{	return mNumRows;	}

UINT World::GetNumCols()
{	return mNumCols;	}
 
float World::GetTileSpacing()
{	return mTileSpacing;	}

UINT World::GetNumElements()
{	return mNumElements;	}

UINT World::GetNumTiles()
{	return mNumTiles;	}

UINT World::GetFullSize()
{	return mFullSize;	}

UINT World::GetNumAI()
{
	return mAITiles.size();
}

void World::SetAABB(UINT i, AABB Box)
{
	mGridPts[i].Box = Box;
}

void World::GenerateRandomMap(UINT NumOfRows, UINT NumOfCols, float TileSpacing)
{
	mNumRows = NumOfRows;
	mNumCols = NumOfCols;
	mTileSpacing = TileSpacing;
	mNumTiles = 0;
	mNumElements = 0;
	mFullSize = mNumRows * mNumCols;

	//Hardcoded 30% of the map's area would be converted to tiles
	int NumOfRandomTiles = (int)( (float)(mFullSize) * 0.3f);
	int TilesAssigned  = 0;
	if(NumOfRandomTiles <= 0)
		return;

	//Fill the Grid vector with 0, and then randomly fill it with tiles
	std::vector<int> RandomTileIndices;
	std::vector<char> Grid(mNumRows*mNumCols);
	std::fill(Grid.begin(), Grid.end(), '0');
	mGridPts.clear();

	while(TilesAssigned < NumOfRandomTiles)
	{
		int Rand = (UINT)RandF(0.0f, (float)(mFullSize) );
		if(Grid[Rand] == '0')
		{
			Grid[Rand] = 'x';
			TilesAssigned++;
		}

	}

	//Exit and spawn points are also hardcoded
	Grid[0] = 'S';
	Grid[Grid.size() - 1] = 'E';

	//Filtering process
	//Get the real coordinates and put it in mGridPts
	GridPt Pt;

	for(UINT i = 0; i < mNumRows; i++)
		for(UINT j = 0; j < mNumCols; j++)
	{
		UINT k = i*mNumCols+j;
		if(Grid[k] == 'S' || Grid[k] == 'x' || Grid[k] == 'E')
		{
			float x = (float)j*mTileSpacing;
			float y = 0.0f;
			float z = (float)((mNumRows-1)-i)*mTileSpacing;

			Pt.ZeroOut();
			Pt.Enabled = true;
			Pt.i = i;
			Pt.j = j;
			Pt.p = D3DXVECTOR3(x,0.0f,z);
			mNumTiles++;
			if(Grid[k] == 'S')
			{
				Pt.IsPlayerSpawnPt = true;
				mSpawnPtIndex = mGridPts.size();
			}
			else if(Grid[k] == 'E')
			{
				Pt.IsExitPt = true;
				mExitPtIndex = mGridPts.size();
			}
			mGridPts.push_back(Pt);
		}
	}

	mNumElements = mGridPts.size();
}