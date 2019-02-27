#include <unordered_map>
#include <queue>
#include <algorithm> 
#include <iostream>
#include <thread>

int mapWidth;
int mapHeight;
int targetX;
int targetY;

struct Node
{
	int x = -1;
	int y = -1;
	unsigned int traveled = UINT32_MAX; // distance traveled to get to node
	unsigned int distance = UINT32_MAX; // Manhattan distance from target
	unsigned int fcost = UINT32_MAX; // Total cost of node (= traveled + distance)
	char value = 0; // 0 impassable, 1 traversable
	Node * parent = nullptr;
};

// Compares nodes based on fcost with the lowest moving to the front of the priority_queue
struct node_compare {
	bool operator()(const Node * n1, const Node * n2) const
	{
		return n1->fcost > n2->fcost;
	}
};

// Distance from start + distance from target
unsigned int FindDistance(const int& nTargetX, const int& nTargetY,
	const Node * currNode)
{
	return std::abs(currNode->x - nTargetX) + std::abs(currNode->y - nTargetY);
}

// Find the smallest element, move it to explored, and return it
Node * SetNewCurrent(std::priority_queue<Node*, std::vector<Node*>, node_compare>& unexplored, std::unordered_map<int, Node*>& explored)
{
	Node * foundNode = unexplored.top();
	explored[foundNode->x + foundNode->y * mapWidth] = foundNode;
	unexplored.pop();
	return foundNode;
}

// Find your way home!
int RetracePath(Node * current, int* pOutBuffer, const int& nOutBufferSize)
{
	// Shortest path found, now walk back and take note. Also compensating for smaller buffer
	int steps = current->traveled;
	int walkback = current->traveled;
	for (walkback; walkback > nOutBufferSize; --walkback)
	{
		current = current->parent;
	}

	while (walkback)
	{
		pOutBuffer[--walkback] = current->x + current->y * mapWidth;
		current = current->parent;
	}

	return steps;
}

// Check if the neighbour is viable for traversal
bool neighbourViability(Node * neighbour, const unsigned char* tMap, const std::unordered_map<int, Node*>& explored)
{
	// Is the neighbour outside the map?
	if (neighbour->x < 0 || neighbour->x >= mapWidth || neighbour->y < 0 || neighbour->y >= mapHeight)
	{
		return false;
	}

	// If not travesable or already explored, skip the neighbour
	neighbour->value = tMap[neighbour->x + neighbour->y * mapWidth];
	auto exploredNeighbour = explored.find(neighbour->x + neighbour->y * mapWidth);
	if (!neighbour->value || exploredNeighbour != explored.end())
	{
		return false;
	}
	else 
	{
		return true;
	}
}

void discoverNeighbours(Node * current, Node * neighbour, 
	const int neighbourDirection[2], const unsigned char* tMap,
	const std::unordered_map<int, Node*>& explored,
	std::unordered_map<int, Node*>& costmap,
	std::priority_queue<Node*, std::vector<Node*>, node_compare>& unexplored)
{
	// Set neighbour coordinates
	neighbour->x = current->x + neighbourDirection[0];
	neighbour->y = current->y + neighbourDirection[1];

	// Keep going if neighbour is viable
	if (neighbourViability(neighbour, tMap, explored))
	{
		int tmpTraveled = current->traveled + 1;
		auto foundNode = costmap.find(neighbour->x + neighbour->y * mapWidth);
		// If neighbour wasn't found in unexplored, create it, else update the neighbour if new path is shorter.
		if (foundNode == costmap.end() || (*foundNode).second->traveled > tmpTraveled)
		{
			neighbour->traveled = current->traveled + 1;
			neighbour->distance = FindDistance(targetX, targetY, neighbour);
			neighbour->fcost = tmpTraveled + neighbour->distance;
			neighbour->parent = current;
			costmap[neighbour->x + neighbour->y * mapWidth] = neighbour;
			unexplored.push(neighbour);
		}
	}
}

int FindPath(const int nStartX, const int nStartY,
	const int nTargetX, const int nTargetY,
	const unsigned char* pMap, const int nMapWidth, const int nMapHeight,
	int* pOutBuffer, const int nOutBufferSize)
{
	// Init variables, some global
	bool pathFound = false;
	int steps = -1;
	mapWidth = nMapWidth;
	mapHeight = nMapHeight;
	targetX = nTargetX;
	targetY = nTargetY;

	// Are we starting on the target? ez check
	if (nStartX == targetX && nStartY == targetY)
	{
		if (nOutBufferSize > 0)
		{
			pOutBuffer[0] = targetX + targetY * mapWidth;
		}
		return 0; // steps taken
	}

	// Might change during runtime because the pointer itself isn't constant
	unsigned char * tMap = new unsigned char[mapWidth * mapHeight];
	int size = mapWidth * mapHeight;
	std::copy(pMap, pMap + size, tMap);

	// Storage for nodes
	std::priority_queue<Node*, std::vector<Node*>, node_compare> unexplored;
	std::unordered_map<int, Node*> costmap;
	std::unordered_map<int, Node*> explored;


	// Adding start postition to be explored
	Node * start = new Node();
	start->x = nStartX;
	start->y = nStartY;
	start->traveled = 0;
	start->distance = FindDistance(targetX, targetY, start);
	start->fcost = start->traveled + start->distance;
	start->value = 1;
	unexplored.push(start);

	// Keep going till we can't explore more!
	while (!unexplored.empty())
	{
		// Update where we are exploring from
		Node * current = SetNewCurrent(unexplored, explored);

		// Is the newly explored node the target?
		if (current->x == targetX && current->y == targetY)
		{
			pathFound = true;
			steps = RetracePath(current, pOutBuffer, nOutBufferSize);
			break;
		}

		// Its dangerous to go alone, take this
		const int neighbourDirections[4][2] = { { -1,0 },{ 0,-1 },{ 1,0 },{ 0,1 } };
		Node * newNeighbours[4] = {};
		// Check the neighbours, no diagonals
		for (int i = 0; i < 4; ++i)
		{
			newNeighbours[i] = new Node;
			discoverNeighbours(current, newNeighbours[i], neighbourDirections[i], tMap, explored, costmap, unexplored);
		}
	}

	// Cleanup 
	for (auto& node : explored)
	{
		delete node.second;
	}
	while (!unexplored.empty())
	{
		delete unexplored.top(); 
		unexplored.pop();
	}
	delete[] tMap;

	return steps;	
}

int main()
{
	{
		unsigned char map[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 1, 0, 1, 1, 1, 1, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1, 0, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
		int pOutBuffer[100];
		int outBufferSize = 100;
		int steps = FindPath(0, 0, 2, 2, map, 10, 100, pOutBuffer, outBufferSize);

		std::cout << steps << std::endl;
		for (int i = 0; i < steps && i < outBufferSize; i++)
		{
			std::cout << pOutBuffer[i] << " ";
		}
		std::cout << std::endl;
	}
	{
		unsigned char map[] = { 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1 };
		int pOutBuffer[12];		
		int outBufferSize = 12;
		int steps = FindPath(0, 0, 1, 2, map, 4 , 3, pOutBuffer, outBufferSize);

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