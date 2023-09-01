#pragma once
#include <tgMemoryDisable.h>
#include <vector>
#include <thread>
#include <tgMemoryEnable.h>
#include "MazeGenerator.h"
#include "tgCV3D.h"
class ProceduralMazeTileGen
{
public:
	struct MazeChunk
	{
		tgCV2D GridPos;
		tgCV4D WallRemoverModiefier;
		tgCV3D StartPos;
		int ChunkID;

		bool operator<(const MazeChunk& Chunk) const
		{
			return ChunkID < Chunk.ChunkID;
		}
	};

	ProceduralMazeTileGen();
	~ProceduralMazeTileGen();
	void Update();
	void Start();
	
	std::vector<MazeGenerator*> GetMazes() { return m_Mazes; }
	tgCV4D DoesBorderMazesExist(tgCV2D GridPos);
	auto DoesChunkExist(MazeChunk GridPos);
	
	void Stop();
	MazeGenerator* FindMaze(tgCV2D GridPos);
	void GenererateSurroundingMazes();
	void GenerateMaze(tgCV2D AddedGridPos);
	MazeGenerator* GetCurrentMaze() { return m_pCurrentMaze; }
	
private:
	
	

	typedef std::vector < MazeChunk >	ChunkVector;
	struct SWorker
	{
		ChunkVector	 DelegatedChunks;
		ProceduralMazeTileGen* MazeTilGen;
		tgCThread* pThread;
		tgBool Running;
		tgCMutex* pMutex;
	};

	static void	Work(tgCThread* pThread);

	std::vector<SWorker*> m_Workers;
	std::vector<MazeGenerator*> m_Mazes;
	ChunkVector m_GridPositions;
	MazeGenerator* m_pCurrentMaze;
	tgCModel* m_pPlayerModel;

	tgCMutex* m_pMazesMutex;
	float m_WallLength;
	std::vector<tgCV2D> m_NextMazePositions;
	tgCModel* m_Wallmodel;
};

