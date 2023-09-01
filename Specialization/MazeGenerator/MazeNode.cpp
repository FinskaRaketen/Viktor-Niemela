#include <tgSystem.h>
#include "MazeNode.h"
#include "Managers/CModelManager.h"
#include "Managers/CWorldManager.h"
#include "tgCDebugManager.h"
MazeNode::MazeNode(tgCV3D Offset, tgCV3D _Length)
{
	Offset.y = 0;
	for(int i = 0; i < 4; i++)
	{
		tgCString Temp("Fence%d" ,i);
		m_Walls.emplace_back();
		
	}
	
	m_Walls[1].Matrix.Pos.x +=  (_Length.x / 2);
	m_Walls[1].Matrix.Pos.z += (_Length.z / 2);
	m_Walls[1].Matrix.Pos.y += 0.0005;
	

	
	m_Walls[1].Matrix.RotateY(90, tgCMatrix::COMBINE_PRE_MULTIPLY);

	m_Walls[2].Matrix.Pos.z += (_Length.z);

	m_Walls[3].Matrix.Pos.z += (_Length.z/2);
	m_Walls[3].Matrix.Pos.x += (-_Length.x / 2);
	m_Walls[3].Matrix.RotateY(90, tgCMatrix::COMBINE_PRE_MULTIPLY);
	m_Walls[3].Matrix.Pos.y += 0.0005;
	
	for(Wall& Wall: m_Walls)
	{
		Wall.Matrix.Pos += Offset;
		tgCV3D MinPos(Wall.Matrix.Pos - Wall.Matrix.At * 0.2f - Wall.Matrix.Left * (_Length / 2));
		tgCV3D MaxPos(Wall.Matrix.Pos + Wall.Matrix.At * 0.2f + Wall.Matrix.Left * (_Length / 2));
		MaxPos.y += _Length.y;
		Wall.WallAABox = tgCAABox3D(tgCV3D(tgMathMin(MinPos.x, MaxPos.x), tgMathMin(MinPos.y, MaxPos.y), tgMathMin(MinPos.z, MaxPos.z)),tgCV3D(tgMathMax(MinPos.x, MaxPos.x), tgMathMax(MinPos.y, MaxPos.y), tgMathMax(MinPos.z, MaxPos.z)));
		
	}
	
	m_FloorMatrix = m_Walls[1].Matrix;
	m_FloorMatrix.RotateX(-90, tgCMatrix::COMBINE_PRE_MULTIPLY);
	m_FloorMatrix.Pos.y -= 0.1;

	std::vector<Wall>Correctorder;
	Correctorder.push_back(m_Walls[1]);
	Correctorder.push_back(m_Walls[3]);
	Correctorder.push_back(m_Walls[2]);
	Correctorder.push_back(m_Walls[0]);
	m_Walls.swap(Correctorder);
	
	
}
void MazeNode::RemoveWall(int Index)
{
	m_Walls[Index].ShouldBeRendered = false;
}

