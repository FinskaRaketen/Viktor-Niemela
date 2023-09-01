#pragma once
#include "tgCModel.h"
#include "tgCAABox3D.h"




struct Wall
{

	tgCMatrix Matrix = tgCMatrix::Identity;
	bool ShouldBeRendered = true;
	tgCAABox3D WallAABox;
};

class MazeNode
{
public:
	MazeNode(tgCV3D Offset, tgCV3D _Length);

	void RemoveWall(int Index);

	tgCV3D GetMiddle() { return m_Walls[2].Matrix.Pos; }

	std::vector<Wall>& GetWalls() { return m_Walls; }
	tgCMatrix GetFloorMatrix() { return m_FloorMatrix; }
	

private:
	std::vector<Wall> m_Walls;
	tgCMatrix m_FloorMatrix;
};


