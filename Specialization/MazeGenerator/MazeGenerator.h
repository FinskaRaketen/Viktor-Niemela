#pragma once
#include "MazeNode.h"
class MazeGenerator
{
public:
	MazeGenerator(int Width, int Height, tgCV3D StartPos, tgCV2D GridPos, tgCV4D _WallRemoverModifier);
	~MazeGenerator();

	
	void MazeGeneration();
	int Indexof(std::vector<MazeNode*> Vector1, std::vector<MazeNode*> Vector2, int Index);
	bool Find(std::vector<MazeNode*> Vector1, MazeNode* element);
	void Render();
	void Update();
	float GetWallLength() { return m_NodeLength; }
	tgCV3D GetMiddlePosition() { return m_MiddlePosition; }
	float GetWidth() { return m_Width; }
	float GetHeight() { return m_Height; }
	std::vector<MazeNode*> GetWalls() { return m_Nodes; }
	tgCV2D GetGridPos() { return m_MazePos; }
	tgCV3D GetStartPos() { return m_StartPos; }
	tgCAABox3D GetAABox() { return m_MazeAABox; }
	tgCV4D GetWallModifier() { return m_WallRemoverModifier; }
	bool GetGeneratedNeighbourMazes() { return m_HasGeneratedNeigbourMazes; }
	void SetGeneratedNeighbourMazes(bool NewValue) { m_HasGeneratedNeigbourMazes = NewValue; }
	bool DoesMultipleSameWallsExist(Wall* CurrentWall);

	struct SMeshInstanceData							
	{
		tgCMatrix	Matrix;
	};
	struct SInstance									
	{
		ID3D11Buffer*				pInstanceBuffer;	
		D3D11_MAPPED_SUBRESOURCE	SubResource;		
		tgUInt32					NumMeshes;			

	};
private:
	std::vector<MazeNode*> m_Nodes;
	float m_NodeLength;
	int m_Height;
	int m_Width;
	SInstance m_Instance;
	void MazeInitialization(tgCV3D StartPos);
	tgCModel* m_pWallModel;
	tgCV3D m_MiddlePosition;
	tgCV2D m_MazePos;
	tgCV4D m_WallRemoverModifier;
	tgCV3D m_StartPos;
	tgCAABox3D m_MazeAABox;
	bool m_HasGeneratedNeigbourMazes;
};

