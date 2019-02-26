#include <unordered_map>
#include <algorithm> 
#include <iostream>

struct Node
{
	int x = -1;
	int y = -1;
	unsigned int cost = UINT32_MAX; // Cost to get to node
	unsigned int distance = UINT32_MAX; // Distance from target
	unsigned int fcost = UINT32_MAX;
	char value = 0; // 0 impassable, 1 traversable
	Node * parent = nullptr;
};

// Distance from start + distance from target
unsigned int FindDistance(const int& nTargetX, const int& nTargetY,
	const Node * currNode)
{
	return std::abs(currNode->x - nTargetX) + std::abs(currNode->y - nTargetY);
}

int FindPath(const int nStartX, const int nStartY,
	const int nTargetX, const int nTargetY,
	const unsigned char* pMap, const int nMapWidth, const int nMapHeight,
	int* pOutBuffer, const int nOutBufferSize)
{
	bool pathFound = false;
	int steps = 0;

	// Are we starting on the target
	if (nStartX == nTargetX && nStartY == nTargetY)
	{
		if (nOutBufferSize > 0)
		{
			pOutBuffer[0] = nTargetX + nTargetY * nMapWidth;
		}
		return steps;
	}

	// Might change during runtime because the pointer itself isn't constant
	unsigned char * tMap = new unsigned char[nMapWidth * nMapHeight];
	int size = nMapWidth * nMapHeight;
	std::copy(pMap, pMap + size, tMap);

	// Create variables
	std::unordered_map<int, Node*> unexplored;
	unexplored.reserve(64); // Rough guess on how many unexplored paths there might be at the same time
	std::unordered_map<int, Node*> explored;


	// Adding start postition to be explored
	Node * start = new Node();
	start->x = nStartX;
	start->y = nStartY;
	start->cost = 0;
	start->distance = FindDistance(nTargetX, nTargetY, start);
	start->fcost = start->cost + start->distance;
	start->value = 1;
	explored.reserve(start->cost * 2); // Rough guess on how far it is to get to the point
	unexplored[start->x + start->y * nMapWidth] = start; // The index should be good as unique key

	while (!pathFound)
	{
		Node * current = nullptr;

		// Intentional scope
		{
			// Get lowest fcost within unexplored
			auto expl = std::min_element(unexplored.begin(), unexplored.end(),
				[](const std::pair<int, Node*> x, const std::pair<int, Node*>  y) { return x.second->fcost < y.second->fcost; });
			// Found lowest fcost, move to explored from unexplored
			explored[(*expl).first] = (*expl).second;
			current = (*expl).second;
			unexplored.erase(expl);
		}

		// Is the newly explored node the target?
		if (current->x == nTargetX && current->y == nTargetY)
		{
			pathFound = true;
			steps = current->cost;

			// Shortest path found, now walk back and take note. Also compensating for smaller buffer
			int walkback = steps;
			for (walkback; walkback > nOutBufferSize; --walkback)
			{
				current = current->parent;
			}

			while (walkback)
			{
				pOutBuffer[--walkback] = current->x + current->y * nMapWidth;
				current = current->parent;
			}

			break;
		}

		// Its dangerous to go alone, take this
		const int neighbourDirections[4][2] = { { -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 } };

		// Check the neighbours, no diagonals
		for (auto& neDi : neighbourDirections)
		{
			// Set neighbour values
			Node * neighbour = new Node();
			neighbour->x = current->x + neDi[0];
			neighbour->y = current->y + neDi[1];

			// Is the neighbour outside the map?
			if (neighbour->x < 0 || neighbour->x >= nMapWidth || neighbour->y < 0 || neighbour->y >= nMapHeight)
			{
				continue;
			}

			// If not travesable or already explored, skip the neighbour
			neighbour->value = tMap[neighbour->x + neighbour->y * nMapWidth];
			auto exploredNeighbour = explored.find(neighbour->x + neighbour->y * nMapWidth);
			if (!neighbour->value || exploredNeighbour != explored.end())
			{
				continue;
			}

			// Get the cost of the neighbour
			unsigned int cost = current->cost + 1;
			unsigned int tmpDistance = FindDistance(nTargetX, nTargetY, neighbour);
			auto foundNode = unexplored.find(neighbour->x + neighbour->y * nMapWidth);
			// If neighbour wasn't found in unexplored, create it, else update the neighbour if new path is shorter.
			if (foundNode == unexplored.end())
			{
				neighbour->cost = cost;
				neighbour->distance = tmpDistance;
				neighbour->fcost = cost + tmpDistance;
				neighbour->parent = current;
				unexplored[neighbour->x + neighbour->y * nMapWidth] = neighbour;
			}
			else if ((*foundNode).second->cost > cost)
			{
				(*foundNode).second->cost = cost;
				(*foundNode).second->fcost = cost + (*foundNode).second->distance;
				(*foundNode).second->parent = current;
			}
		}

		// If unexplored is empty, that means no path could be found
		if (unexplored.empty())
		{
			pathFound = false;
			break;
		}
	}

	// Cleanup 
	for (auto& node : explored)
	{
		delete node.second;
	}
	for (auto& node : unexplored)
	{
		delete node.second;
	}
	delete[] tMap;

	// Return steps if path is found, else -1
	return pathFound ? steps : -1;	
}

int main()
{
	{
		unsigned char map[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
								1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
								1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
								1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
								1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
								1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
								1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
								1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
								1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
		int pOutBuffer[100];
		int outBufferSize = 100;
		int steps = FindPath(0, 0, 2, 2, map, 10, 10, pOutBuffer, outBufferSize);

		std::cout << steps << std::endl;
		for (int i = 0; i < steps && i < outBufferSize; i++)
		{
			std::cout << pOutBuffer[i] << " ";
		}
		std::cout << std::endl;
	}

	{
		unsigned char map[] = { 0, 0, 1, 
								0, 1, 1, 
								1, 0, 1 };
		int pOutBuffer[7];
		int steps = FindPath(2, 0, 0, 2, map, 3, 3, pOutBuffer, 7);

		std::cout << steps << std::endl;
		if (steps != -1)
		{
			std::cout << pOutBuffer[0];
			for (int i = 1; i < steps; i++)
			{
				std::cout << ", " << pOutBuffer[i];
			}
			std::cout << std::endl;
		}
	}
	
	std::cout << "Press enter to exit" << std::endl;
	std::cin.get();
	return 0;
}