#include <tgSystem.h>
#include "ProceduralMazeTileGen.h"
#include "tgCMutex.h"
#include "tgCThread.h"
#include "CApplication.h"
#include "Managers/CModelManager.h"
#include "Camera/CCamera.h"
#include "tgCDebugManager.h"
#include "tgMemoryDisable.h"
#include <functional>
#include "tgMemoryEnable.h"

ProceduralMazeTileGen::ProceduralMazeTileGen()
	:m_Workers{ }
	,m_pMazesMutex(new tgCMutex("MazeMutex"))
{
	Start();
    m_Wallmodel = CModelManager::GetInstance().LoadModel("models/wall", "Temp", false);
	m_WallLength = (m_Wallmodel->GetMeshArray()->GetAABBox().GetMax().x - m_Wallmodel->GetMeshArray()->GetAABBox().GetMin().x);
	SWorker* pWorker = m_Workers[std::rand() % m_Workers.size()];
	std::hash<int> Hash;
	std::hash<unsigned long long> HashX;
	tgCMutexScopeLock Lock(*pWorker->pMutex);
	MazeChunk chunk{ tgCV2D(0,0),tgCV4D(100, 100, 0, 0), tgCV3D(0,0,0), HashX(Hash(0.0f) + Hash(0.0f) * Hash(0.0f))};
	pWorker->DelegatedChunks.push_back(chunk);
	m_GridPositions.push_back(chunk);
	m_pCurrentMaze = nullptr;
	m_pPlayerModel = CModelManager::GetInstance().GetModel("Player");

	m_NextMazePositions.emplace_back(1, 0);
	m_NextMazePositions.emplace_back(-1, 0);
	m_NextMazePositions.emplace_back(0, 1);
	m_NextMazePositions.emplace_back(0, -1);
	m_NextMazePositions.emplace_back(1, 1);
	m_NextMazePositions.emplace_back(-1, 1);
	m_NextMazePositions.emplace_back(1, -1);
	m_NextMazePositions.emplace_back(-1, -1);
}

ProceduralMazeTileGen::~ProceduralMazeTileGen()
{
	for(SWorker* pWorker: m_Workers)
	{
		pWorker->Running = false;
		delete pWorker->pMutex;
		delete pWorker->pThread;
		delete pWorker;
	}
	m_Workers.clear();

	for(MazeGenerator* Maze: m_Mazes)
	{
		delete Maze;
	}
	if(m_Wallmodel)
	{
		CModelManager::GetInstance().DestroyModel(m_Wallmodel);
	}
	if(m_pMazesMutex)
	{
		delete m_pMazesMutex;
	}
}

void ProceduralMazeTileGen::Update()
{
	if(!m_pCurrentMaze)
	{
		m_pCurrentMaze = m_Mazes[0];
		m_Mazes[0]->SetGeneratedNeighbourMazes(true);
		GenererateSurroundingMazes();
	}

	if(!m_Workers.empty() && m_pCurrentMaze)
	{
		tgCDebugManager::GetInstance().AddText2D(tgCV2D(0,200), tgCColor::Green, tgCString("%.d",m_Mazes.size()));
		
		
		if(m_pPlayerModel->GetTransform().GetMatrixWorld().Pos.z < (m_pCurrentMaze->GetMiddlePosition().z - m_WallLength * (m_pCurrentMaze->GetHeight() / 2)))
		{
			MazeGenerator* Temp = FindMaze(tgCV2D(m_pCurrentMaze->GetGridPos().x, m_pCurrentMaze->GetGridPos().y - 1));
			if (Temp != nullptr) 
			{
				m_pCurrentMaze = Temp;
				if(!Temp->GetGeneratedNeighbourMazes())
				GenererateSurroundingMazes();
				Temp->SetGeneratedNeighbourMazes(true);
				
			}
			
		}
			
		else if (m_pPlayerModel->GetTransform().GetMatrixWorld().Pos.z > (m_pCurrentMaze->GetMiddlePosition().z + m_WallLength * (m_pCurrentMaze->GetHeight() / 2)))
		{
			MazeGenerator* Temp = FindMaze(tgCV2D(m_pCurrentMaze->GetGridPos().x, m_pCurrentMaze->GetGridPos().y + 1));
			if (Temp != nullptr)
			{
				m_pCurrentMaze = Temp;
				if (!Temp->GetGeneratedNeighbourMazes())
				GenererateSurroundingMazes();
				Temp->SetGeneratedNeighbourMazes(true);
			}
			
		}
		else if(m_pPlayerModel->GetTransform().GetMatrixWorld().Pos.x < (m_pCurrentMaze->GetMiddlePosition().x - m_WallLength * (m_pCurrentMaze->GetWidth() / 2)))
		{
			MazeGenerator* Temp = FindMaze(tgCV2D(m_pCurrentMaze->GetGridPos().x - 1, m_pCurrentMaze->GetGridPos().y));
			if (Temp != nullptr)
			{
				m_pCurrentMaze = Temp;
				if(!Temp->GetGeneratedNeighbourMazes())
				GenererateSurroundingMazes();
				Temp->SetGeneratedNeighbourMazes(true);
			}
		}
		else if(m_pPlayerModel->GetTransform().GetMatrixWorld().Pos.x > (m_pCurrentMaze->GetMiddlePosition().x + m_WallLength * (m_pCurrentMaze->GetWidth() / 2)))
		{
			MazeGenerator* Temp = FindMaze(tgCV2D(m_pCurrentMaze->GetGridPos().x + 1, m_pCurrentMaze->GetGridPos().y));
			if (Temp != nullptr)
			{
				m_pCurrentMaze = Temp;
				if (!Temp->GetGeneratedNeighbourMazes())
					GenererateSurroundingMazes();
				Temp->SetGeneratedNeighbourMazes(true);

			}
			
		}
		
	}
	tgCMutexScopeLock Lock(*m_pMazesMutex);
	for(MazeGenerator* Maze : m_Mazes)
	{
		Maze->Update();
		if(Maze->GetMiddlePosition() ==	m_pCurrentMaze->GetMiddlePosition())
			tgCDebugManager::GetInstance().AddLineSphere(tgCSphere(Maze->GetMiddlePosition(), 2.0f), tgCColor::Green);
		else
		{
			tgCDebugManager::GetInstance().AddLineSphere(tgCSphere(Maze->GetMiddlePosition(), 2.0f), tgCColor::Blue);
		}
	}
	
}

void ProceduralMazeTileGen::Start()
{
	Stop();
	for(int WorkerIndex = 0; WorkerIndex < 3; WorkerIndex++)
	{
		SWorker* Worker = new SWorker();
		Worker->MazeTilGen = this;
		Worker->pThread = new tgCThread(tgCString("Thread: Worker %d", WorkerIndex), Work, tgCThread::PRIORITY_NORMAL, 64 * 1024, Worker);
		Worker->Running = true;
		Worker->pMutex = new tgCMutex("Worker Mutex");
		m_Workers.push_back(Worker);
	}
}

void ProceduralMazeTileGen::Work(tgCThread* pThread)
{
	
	SWorker* pWorker = (SWorker*)pThread->GetUserData();
	while (pWorker->Running)
	{
		if (pWorker->DelegatedChunks.empty())
		{
			tgSleep(1);
			continue;
		}
		pWorker->pMutex->Lock();
		int rand = 100;
		MazeChunk Chunk;
		{
			rand = tgMathRandom(0, pWorker->DelegatedChunks.size() - 1);
			Chunk = pWorker->DelegatedChunks[rand];
		}
		pWorker->DelegatedChunks.erase(pWorker->DelegatedChunks.begin() + rand);

		

		pWorker->pMutex->Unlock();

		std::vector<MazeGenerator*> NewMazes;
		MazeGenerator* NewMaze = new MazeGenerator(6, 6, Chunk.StartPos, Chunk.GridPos, Chunk.WallRemoverModiefier);
		
		pWorker->MazeTilGen->m_pMazesMutex->Lock();

		pWorker->MazeTilGen->m_Mazes.push_back(NewMaze);

		pWorker->MazeTilGen->m_pMazesMutex->Unlock();

	}
	
}

tgCV4D ProceduralMazeTileGen::DoesBorderMazesExist(tgCV2D _GridPos)
{
	tgCV4D _WallRemoverModifer(tgCV4D(0,0,0,0));
	for(MazeChunk GridPos: m_GridPositions  )
	{
		if (GridPos.GridPos == tgCV2D(_GridPos.x + 1, _GridPos.y))
		{
			_WallRemoverModifer.y = 1;
		}
		else if (GridPos.GridPos == tgCV2D(_GridPos.x - 1, _GridPos.y))
		{
			_WallRemoverModifer.w = 1;
		}
		else if(GridPos.GridPos == tgCV2D(_GridPos.x, _GridPos.y + 1))
		{
			_WallRemoverModifer.x = 1;
		}
		else if (GridPos.GridPos == tgCV2D(_GridPos.x, _GridPos.y -1))
		{
			_WallRemoverModifer.z = 1;
		}
		
		if(_WallRemoverModifer.x == 1 && _WallRemoverModifer.y == 1 && _WallRemoverModifer.z == 1 && _WallRemoverModifer.w == 1)
		{
			return _WallRemoverModifer;
		}
	}
	return _WallRemoverModifer;
}

auto ProceduralMazeTileGen::DoesChunkExist(MazeChunk GridPos)
{
	return  std::lower_bound(m_GridPositions.begin(), m_GridPositions.end(), GridPos);
}

void ProceduralMazeTileGen::Stop()
{
	for (SWorker* pWorker : m_Workers)
	{
		pWorker->Running = false;

		delete pWorker->pMutex;
		delete pWorker->pThread;
		delete pWorker;
	}

	m_Workers.clear();
}

MazeGenerator* ProceduralMazeTileGen::FindMaze(tgCV2D GridPos)
{
	tgCMutexScopeLock Lock(*m_pMazesMutex);
	for (MazeGenerator* Maze : m_Mazes)
	{
		if (Maze->GetGridPos() == GridPos)
		{
			return Maze;
		}
	}
	return nullptr;
}

void ProceduralMazeTileGen::GenererateSurroundingMazes()
{
	for(tgCV2D& NextMazePosition: m_NextMazePositions)
	{
		GenerateMaze(NextMazePosition);
	}
}

void ProceduralMazeTileGen::GenerateMaze(tgCV2D AddedGridPos)
{
	std::hash<int> Hash;
	std::hash<unsigned long long> HashX;
	tgCV2D Temp(m_pCurrentMaze->GetGridPos() + AddedGridPos);
	tgCV3D StartPos = tgCV3D(m_pCurrentMaze->GetStartPos().x + (m_WallLength * m_pCurrentMaze->GetWidth() * AddedGridPos.x), 0, m_pCurrentMaze->GetStartPos().z + (m_WallLength * m_pCurrentMaze->GetHeight() * AddedGridPos.y));
	MazeChunk Chunk{ Temp, DoesBorderMazesExist(Temp) , StartPos, HashX(Hash(Temp.x) + Hash(Temp.y) * Hash(Temp.x)) };
	auto it = DoesChunkExist(Chunk);
	if (it == m_GridPositions.end() || it._Ptr->ChunkID != Chunk.ChunkID)
	{
		SWorker* pWorker = m_Workers[std::rand() % m_Workers.size()];
		tgCMutexScopeLock Lock(*pWorker->pMutex);
		m_GridPositions.insert(it, Chunk);
		pWorker->DelegatedChunks.push_back(Chunk);
	}
}
