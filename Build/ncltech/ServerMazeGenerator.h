#pragma once
#include "ServerSearchAlgorithm.h"
#include <ncltech\GameObject.h>
#include <ncltech\Scene.h>




class ServerMazeGenerator
{
public:
	ServerMazeGenerator(); //Maze_density goes from 1 (single path to exit) to 0 (no walls at all)
	virtual ~ServerMazeGenerator();

	void Generate(int size, float maze_density);

	//All points on the maze grid are connected in some shape or form
	// so any two nodes picked randomly /will/ have a path between them
	GraphNode* GetStartNode() const { return start; }
	GraphNode* GetGoalNode()  const { return end; }
	uint GetSize() const { return size; }


	//Used as a hack for the MazeRenderer to generate the walls more effeciently
	GraphNode* GetAllNodesArr() { return allNodes; }

	int getSize() { return size; };


protected:
	void GetRandomStartEndNodes();

	void Initiate_Arrays();

	void Generate_Prims();
	void Generate_Sparse(float density);



public:
	uint size;
	GraphNode *start, *end;

	GraphNode* allNodes = nullptr;
	GraphEdge* allEdges = nullptr;
};