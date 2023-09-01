#include <tgSystem.h>
#include "MazeGenerator.h"
#include "Managers/CModelManager.h"
#include "Managers/CWorldManager.h"
#include <tgMemoryDisable.h>
#include <map>
#include <tgMemoryEnable.h>
#include <tgError.h>
#include <tgFrustum.h>
#include "tgCCameraManager.h"
#include "tgCLightManager.h"
#include "tgCDebugManager.h"
#include "Renderer/CRenderCallbacks.h"


MazeGenerator::MazeGenerator(int _Width, int _Height, tgCV3D _StartPos, tgCV2D GridPos, tgCV4D _WallRemoverModifier)
{
	m_pWallModel = CModelManager::GetInstance().GetModel("Temp");
	m_Instance.NumMeshes = 0;
	m_Instance.pInstanceBuffer = nullptr;
	m_NodeLength = (m_pWallModel->GetMeshArray()->GetAABBox().GetMax().x - m_pWallModel->GetMeshArray()->GetAABBox().GetMin().x);
	m_Height = _Height;
	m_Width = _Width;
	m_MazePos = GridPos;
	m_StartPos = _StartPos;
	m_WallRemoverModifier = _WallRemoverModifier;
	m_HasGeneratedNeigbourMazes = false;
	m_MazeAABox = tgCAABox3D(tgCV3D(m_StartPos.x - (m_NodeLength) /2 , m_StartPos.y, m_StartPos.z), tgCV3D(m_StartPos.x + (m_NodeLength * ((float)_Width)) - (m_NodeLength) / 2, m_StartPos.y + (m_NodeLength), m_StartPos.z  + (m_NodeLength * ((float)_Height))));
	MazeInitialization(m_StartPos);
	MazeGeneration();
	m_MiddlePosition = tgCV3D(m_StartPos.x + (m_NodeLength * ((float) _Width / 2)), 0, m_StartPos.z + (m_NodeLength * ((float)_Height / 2)));

	tgCD3D11& rD3D11 = tgCD3D11::GetInstance();
	ID3D11Device* pDevice = rD3D11.LockDevice();

	D3D11_BUFFER_DESC Desc{};
	Desc.ByteWidth = sizeof(SMeshInstanceData) * m_Nodes.size() * 5;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.StructureByteStride = 0;

	TG_D3D11_CHECK_ERROR(pDevice->CreateBuffer(&Desc, NULL, &m_Instance.pInstanceBuffer));

	rD3D11.UnlockDevice();
}

MazeGenerator::~MazeGenerator()
{
	for(MazeNode* Node: m_Nodes)
	{
		delete Node;
	}
}

void MazeGenerator::MazeGeneration()
{
	std::vector<MazeNode*> CurrentPath;
	std::vector<MazeNode*> CompletedNodes;

	CurrentPath.push_back(m_Nodes[tgMathRandom(0, m_Nodes.size() - 1)]);

	while (CompletedNodes.size() < m_Nodes.size())
	{
		std::vector<int> PossibleDirections;
		std::vector<int> PossibleNextNodes;

		int CurrentIndex = Indexof(m_Nodes, CurrentPath, CurrentPath.size() - 1);
		int CurrentNodeX = CurrentIndex / m_Height;
		int CurrentNodeY = CurrentIndex % m_Height;
		if (CurrentNodeX < m_Width - 1)
		{
			if (!Find(CompletedNodes, m_Nodes[CurrentIndex + m_Height]) && !Find(CurrentPath, m_Nodes[CurrentIndex + m_Height]))
			{
				PossibleDirections.push_back(1);
				PossibleNextNodes.push_back(CurrentIndex + m_Height);
			}

		}
		if (CurrentNodeX > 0)
		{

			if (!Find(CompletedNodes, m_Nodes[CurrentIndex - m_Height]) && !Find(CurrentPath, m_Nodes[CurrentIndex - m_Height]))
			{
				PossibleDirections.push_back(2);
				PossibleNextNodes.push_back(CurrentIndex - m_Height);
			}
		}
		if (CurrentNodeY < m_Height - 1)
		{
			if (!Find(CompletedNodes, m_Nodes[CurrentIndex + 1]) && !Find(CurrentPath, m_Nodes[CurrentIndex + 1]))
			{
				PossibleDirections.push_back(3);
				PossibleNextNodes.push_back(CurrentIndex + 1);
			}
		}
		if (CurrentNodeY > 0)
		{
			if (!Find(CompletedNodes, m_Nodes[CurrentIndex - 1]) && !Find(CurrentPath, m_Nodes[CurrentIndex - 1]))
			{
				PossibleDirections.push_back(4);
				PossibleNextNodes.push_back(CurrentIndex - 1);
			}
		}

		if (PossibleDirections.size() > 0)
		{
			int chosenDirection = tgMathRandom(0, PossibleDirections.size() - 1);
			MazeNode* chosenNode = m_Nodes[PossibleNextNodes[chosenDirection]];

			CurrentPath.push_back(chosenNode);

			switch (PossibleDirections[chosenDirection])
			{
			case 1:
				chosenNode->RemoveWall(1);
				CurrentPath[CurrentPath.size() - 2]->RemoveWall(0);
				break;

			case 2:
				chosenNode->RemoveWall(0);
				CurrentPath[CurrentPath.size() - 2]->RemoveWall(1);
				break;
			case 3:
				chosenNode->RemoveWall(3);
				CurrentPath[CurrentPath.size() - 2]->RemoveWall(2);
				break;
			case 4:
				chosenNode->RemoveWall(2);
				CurrentPath[CurrentPath.size() - 2]->RemoveWall(3);
				break;
			}
		}
		else
		{
			CompletedNodes.push_back(CurrentPath[CurrentPath.size() - 1]);
			CurrentPath.erase(CurrentPath.begin() + (CurrentPath.size() - 1));
		}
	}

	for(int i = 0; i < m_Nodes.size(); i++)
	{
		for(int j = 0; j < 4; j++)
		{
			
			if(m_Nodes[i]->GetWalls()[j].ShouldBeRendered && DoesMultipleSameWallsExist(&m_Nodes[i]->GetWalls()[j]))
			{
				m_Nodes[i]->RemoveWall(j);
			}
			
		}
	}
	
}

int MazeGenerator::Indexof(std::vector<MazeNode*> Vector1, std::vector<MazeNode*> Vector2, int Index)
{
	MazeNode* Node = Vector2[Index];
	for(int i = 0; i < Vector1.size(); i++)
	{
		if(Node == Vector1[i])
		{
			return i;
		}
	}
	return 0;
}

bool MazeGenerator::Find(std::vector<MazeNode*> Vector1, MazeNode* element)
{
	for(MazeNode* Node: Vector1)
	{
		if(Node == element)
		{
			return true;
		}
	}
	return false;
}

void MazeGenerator::Render()
{
	tgCLightManager& LightManager = tgCLightManager::GetInstance();
	for (tgCLight* Light : LightManager.GetLightList())
	{
		LightManager.SetCurrentLight(*Light);
		
		for (int NumMeshes = 0; NumMeshes < m_pWallModel->GetNumMeshes(); NumMeshes++)
		{
			if (m_Instance.NumMeshes)
			{
				CRenderCallBacks::MeshInstanceCB(*m_pWallModel->GetMesh(NumMeshes), m_Instance.pInstanceBuffer, m_Instance.NumMeshes);
			}

		}
		
	}
}

void MazeGenerator::Update()
{
	tgCD3D11& rD3D11 = tgCD3D11::GetInstance();
	ID3D11DeviceContext* pDeviceContext = rD3D11.LockDeviceContext();
	const tgCCamera* pCamera = tgCCameraManager::GetInstance().GetCurrentCamera();
	const tgCPlane3D* pCameraFrustum = pCamera->GetFrustum();
	TG_D3D11_CHECK_ERROR(pDeviceContext->Map(m_Instance.pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_Instance.SubResource));
	m_Instance.NumMeshes = 0;
	rD3D11.UnlockDeviceContext();
	if (tgFrustumTestAABox(pCameraFrustum, 6, m_MazeAABox))
	{
		for (int i = 0; i < m_Nodes.size(); i++)
		{
			std::vector<Wall> CurrentNodeWalls = m_Nodes[i]->GetWalls();
			SMeshInstanceData* MeshData = (SMeshInstanceData*)m_Instance.SubResource.pData;
			for (int k = 0; k < CurrentNodeWalls.size(); k++)
			{
				if (CurrentNodeWalls[k].ShouldBeRendered)
				{
					MeshData[m_Instance.NumMeshes].Matrix = CurrentNodeWalls[k].Matrix;
					m_Instance.NumMeshes++;
				}
			}
			MeshData[m_Instance.NumMeshes].Matrix = m_Nodes[i]->GetFloorMatrix();
			m_Instance.NumMeshes++;
		}
	}
	pDeviceContext->Unmap(m_Instance.pInstanceBuffer, 0);
}

bool MazeGenerator::DoesMultipleSameWallsExist(Wall* CurrentWall)
{
	for(MazeNode* Node: m_Nodes)
	{
		for(Wall wall: Node->GetWalls())
		{
			if (wall.ShouldBeRendered)
			{
				if (CurrentWall->Matrix.Pos.Between(wall.Matrix.Pos - 0.01f, wall.Matrix.Pos + 0.01f) && CurrentWall->Matrix.Pos != wall.Matrix.Pos)
					return true;
			}
		}
	}
	return false;
}

void MazeGenerator::MazeInitialization(tgCV3D StartPos)
{
	int OpeningRandomMinX = tgMathRandom(0, m_Width - 1);
	int OpeningRandomMinZ = tgMathRandom (0, m_Height - 1);
	int OpeningRandomMaxX = tgMathRandom(0, m_Width - 1);
	int OpeningRandomMaxZ = tgMathRandom(0, m_Height - 1);
	for (int i = 0; i < m_Width; i++)
	{
		for (int k = 0; k < m_Height; k++)
		{
			tgCV3D CurrentNodePos = tgCV3D(StartPos.x + (i * m_NodeLength), 0, StartPos.z + (k * m_NodeLength));
			m_Nodes.push_back(new MazeNode(CurrentNodePos, m_NodeLength));
			MazeNode* CurrentNode = m_Nodes[m_Nodes.size() - 1];

			if(k == 0 && i == OpeningRandomMinX)
			{
				CurrentNode->RemoveWall(3);
			}
			if(k == m_Height - 1 && i == OpeningRandomMaxX)
			{
				CurrentNode->RemoveWall(2);
			}
			if(i == 0 && k == OpeningRandomMinZ)
			{
				CurrentNode->RemoveWall(1);
			}
			if(i == m_Width - 1 && k == OpeningRandomMaxZ)
			{
				CurrentNode->RemoveWall(0);
			}
			
			if(m_WallRemoverModifier.x == 1 && k == m_Height - 1 )
			{
				CurrentNode->RemoveWall(2);
			}
			if(m_WallRemoverModifier.z == 1 && k == 0)
			{
				CurrentNode->RemoveWall(3);
			}
			
			if(m_WallRemoverModifier.y == 1 && i == m_Width - 1)
			{
				CurrentNode->RemoveWall(0);
			}
			
			if(m_WallRemoverModifier.w == 1 && i == 0)
			{
				CurrentNode->RemoveWall(1);
			}
		}
	}
}
