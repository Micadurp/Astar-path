#include <vector>
#include <stdlib.h>
#include <algorithm> 
#include <iostream>

struct Node
{
	int x = -1;
	int y = -1;
	int cost = INT_MAX;
	char value = 0;
	int parent = 0; // Can't have a pointer to a thingy in a vector because the vector might resize and then the pointer will point wrong
};

// Finds nodes based on x and y
struct find_node {
	int x;
	int y;
	find_node(int x, int y) : x(x), y(y) {}
	bool operator()(Node const& n) const 
	{
		return n.x == x && n.y == y;
	}
};

// Distance from start + distance from target
int FindCost(const int& nStartX, const int& nStartY,
	const int& nTargetX, const int& nTargetY,
	const int& nCurrX, const int& nCurrY)
{
	int distStart = abs(nCurrX - nStartX) + abs(nCurrY - nStartY);
	int distTarget = abs(nCurrX - nTargetX) + abs(nCurrY - nTargetY);
	return distStart + distTarget;
}

int FindPath(const int nStartX, const int nStartY,
	const int nTargetX, const int nTargetY,
	const unsigned char* pMap, const int nMapWidth, const int nMapHeight,
	int* pOutBuffer, const int nOutBufferSize)
{
	std::vector<Node> unexplored;
	std::vector<Node> explored;
	bool pathFound = false;
	int steps = 0;

	// Adding start postition to be explored
	Node start;
	start.x = nStartX;
	start.y = nStartY;
	start.cost = FindCost(nStartX, nStartY, nTargetX, nTargetY, nStartX, nStartY);
	start.value = 1;
	unexplored.push_back(start);

	while (!pathFound)
	{
		Node current;
		// Intentional scope
		{
			// Get lowest cost
			auto expl = unexplored.cbegin();
			for (auto unex = unexplored.cbegin(); unex != unexplored.cend(); unex++) {

				if (expl->cost > unex->cost)
				{
					expl = unex;
				}
			}
			// Found lowest cost, move to explored from unexplored
			explored.push_back(*expl);
			current = explored.back();
			unexplored.erase(expl);
		}
		
		// Is the newly explored node the target?
		if (current.x == nTargetX && current.y == nTargetY)
		{
			pathFound = true;
			while (current.parent != -1 || nOutBufferSize - ++steps <= 0)
			{
				pOutBuffer[nOutBufferSize - steps] = current.x + current.y * nMapWidth;
				current = current.parent;
			}
			break;
		}
		
		// Its dangerous to go alone, take this
		const int neighbourDirections[4][2] {{ -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 }};

		// four neighbors because we aint counting diagonals
		for (auto neDi : neighbourDirections)
		{
			Node neighbour;

			// Is the neighbour outside the map?
			if (current.x + neDi[0] < 0 || current.x + neDi[0] >= nMapWidth || current.y + neDi[1] < 0 || current.y + neDi[1] >=nMapHeight)
			{
				continue;
			} 

			neighbour.x = current.x + neDi[0];
			neighbour.y = current.y + neDi[1];
			neighbour.value = pMap[neighbour.x + neighbour.y * nMapWidth];
			bool eeeh = std::find_if(explored.begin(), explored.end(), find_node(neighbour.x, neighbour.y)) != explored.end();
			// Travesable or explored skip the neighbour
			if (!neighbour.value || std::find_if(explored.begin(), explored.end(), find_node(neighbour.x, neighbour.y)) != explored.end())
			{
				continue;
			}
				
			// Get the "new" cost of the neighbour
			int tmpCost = FindCost(nStartX, nStartY, nTargetX, nTargetY, neighbour.x, neighbour.y);
			auto foundNode = std::find_if(unexplored.begin(), unexplored.end(), find_node(neighbour.x, neighbour.y));
			if (foundNode == unexplored.end())
			{
				neighbour.cost = tmpCost;
				neighbour.parent = current;
				unexplored.push_back(neighbour);
			}
			else if (neighbour.cost < tmpCost)
			{
				foundNode->cost = tmpCost;
				foundNode->parent = current;
			}
		}

		if (unexplored.empty())
		{
			pathFound = false;
			break;
		}
	}
	
	// Return steps if path is found, else -1
	return pathFound ? steps : -1;
}

int main()
{
	unsigned char pMap[] = { 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1 };
	int pOutBuffer[12];
	std::cout << FindPath(0, 0, 1, 2, pMap, 4, 3, pOutBuffer, 12) << std::endl;
	
	for (auto index : pOutBuffer)
	{
		std::cout << index << ", ";
	}
	std::cout << std::endl;
	//unsigned char pMap2[] = { 0, 0, 1, 0, 1, 1, 1, 0, 1 };
	//int pOutBuffer2[7];
	//std::cout << FindPath(2, 0, 0, 2, pMap2, 3, 3, pOutBuffer2, 7) << std::endl;
	
	std::cout << "Press enter to exit" << std::endl;
	std::cin.get();
	return 0;
}