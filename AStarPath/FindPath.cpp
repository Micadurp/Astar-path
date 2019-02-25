#include <vector>
#include <stack>
#include <algorithm> 
#include <iostream>

struct Node
{
	int x = -1;
	int y = -1;
	unsigned int cost = UINT32_MAX;
	char value = 0; // 0 impassable, 1 traversable
	Node * parent = nullptr; 
};

// Finds nodes based on x and y
struct find_node {
	int x;
	int y;
	find_node(int x, int y) : x(x), y(y) {}
	bool operator()(Node const* n) const 
	{
		return n->x == x && n->y == y;
	}
};

// Distance from start + distance from target
unsigned int FindCost(const int& nStartX, const int& nStartY,
	const int& nTargetX, const int& nTargetY,
	const int& nCurrX, const int& nCurrY)
{
	int distStart = std::abs(nCurrX - nStartX) + std::abs(nCurrY - nStartY);
	int distTarget = std::abs(nCurrX - nTargetX) + std::abs(nCurrY - nTargetY);
	return distStart + distTarget;
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
	std::vector<Node*> unexplored;
	std::vector<Node*> explored;


	// Adding start postition to be explored
	Node * start = new Node();
	start->x = nStartX;
	start->y = nStartY;
	start->cost = FindCost(nStartX, nStartY, nTargetX, nTargetY, nStartX, nStartY);
	start->value = 1;
	unexplored.push_back(start);

	while (!pathFound)
	{
		Node * current = nullptr;
		
		// Intentional scope
		{
			// Get lowest cost within unexplored
			auto expl = std::min_element(unexplored.begin(), unexplored.end(),
				[](const Node* x, const Node* y) { return x->cost < y->cost; });
			// Found lowest cost, move to explored from unexplored
			explored.push_back(*expl);
			current = explored.back();
			unexplored.erase(expl);
		}
		
		// Is the newly explored node the target?
		if (current->x == nTargetX && current->y == nTargetY)
		{
			pathFound = true;
			std::stack<int> tmp;

			// Shortest path found, now to take note of what it was
			while (current->parent != nullptr)
			{
				tmp.push(current->x + current->y * nMapWidth);
				current = current->parent;
				steps++;
			}

			// Path is reversed so lets quickly change that, don't go bigger than the buffer
			for (int i = 0; i < steps && i < nOutBufferSize; ++i)
			{
				pOutBuffer[i] = tmp.top();
				tmp.pop();
			}

			// Test because I'm unsure what to return when path is bigger than outbuffer
			/*if ( steps > nOutBufferSize )
			{
				steps = -1;
			}*/

			break;
		}
		
		// Its dangerous to go alone, take this
		const int neighbourDirections[4][2] {{ -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 }};

		// Check the neighbours, no diagonals
		for (auto neDi : neighbourDirections)
		{
			// Set neighbour values
			Node * neighbour = new Node();
			neighbour->x = current->x + neDi[0];
			neighbour->y = current->y + neDi[1];
			neighbour->value = tMap[neighbour->x + neighbour->y * nMapWidth];

			// Is the neighbour outside the map?
			if (neighbour->x < 0 || neighbour->x >= nMapWidth || neighbour->y < 0 || neighbour->y >=nMapHeight)
			{
				continue;
			} 

			// If not travesable or already explored, skip the neighbour
			auto exploredNeighbour = std::find_if(explored.begin(), explored.end(), find_node(neighbour->x, neighbour->y));
			if (!neighbour->value || exploredNeighbour != explored.end())
			{
				continue;
			}
				
			// Get the cost of the neighbour
			unsigned int tmpCost = FindCost(nStartX, nStartY, nTargetX, nTargetY, neighbour->x, neighbour->y);
			auto foundNode = std::find_if(unexplored.begin(), unexplored.end(), find_node(neighbour->x, neighbour->y));
			// If neighbour wasn't found in unexplored, create it, else update the neighbour if new path is shorter.
			if (foundNode == unexplored.end())
			{
				neighbour->cost = tmpCost;
				neighbour->parent = current;
				unexplored.push_back(neighbour);
			}
			else if (neighbour->cost < tmpCost)
			{
				(*foundNode)->cost = tmpCost;
				(*foundNode)->parent = current;
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
	for (auto node : explored)
	{
		delete node;
	}
	for (auto node : unexplored)
	{
		delete node;
	}
	delete[] tMap;

	// Return steps if path is found, else -1
	return pathFound ? steps : -1;
}

int main()
{
	{
		unsigned char map[] = { 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1 };
		int pOutBuffer[12];
		int outBufferSize = 12;
		int steps = FindPath(0, 0, 0, 0, map, 4, 3, pOutBuffer, outBufferSize);

		std::cout << steps << std::endl;
		std::cout << pOutBuffer[0];
		for (int i = 1; i < steps && i < outBufferSize; i++)
		{
			std::cout << ", " << pOutBuffer[i];
		}
		std::cout << std::endl;
	}

	{
		unsigned char map[] = { 0, 0, 1, 0, 1, 1, 1, 0, 1 };
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