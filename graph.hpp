//implementation of union find data structure and kruskal's algorithim on the computer network 
//part of the graph creation taken from www.github.com/nilocunger 

#ifndef GRAPH_H
#define GRAPH_H
#include <random>
#include <vector>
#include <queue> //priority_queue and queue
#include <stack>
#include <iostream> //cout

struct GraphNode {
	bool compromised;
	bool affected;
	std::stack<int> namePathStack;
	std::vector<GraphNode*> adjNodes;
	int currentName;
	int originalName;
};

struct Edge {
	GraphNode* leftNode;
	GraphNode* rightNode;
	int cost;

	bool operator ==(Edge e) const {
		return leftNode->originalName == e.leftNode->originalName && rightNode->originalName == e.rightNode->originalName && cost == e.cost;
	}
};

//To use priority queue
struct LessThanCost {
	bool operator()(const Edge* lhs, const Edge* rhs) {
		return lhs->cost > rhs->cost;
	}
};

//Define Graph class
using mt1337 = std::mt19937; 
class Graph {
  private:
		int numNodes;
		std::vector<Edge> costEdges;
		std::vector<Edge> fakeEdges;
		GraphNode* fakeNodes;
		int** fakeTree;
		int** spanningTree;
    int** adjMatrix;

	//Control the randomness 
    mt1337 mt;
    std::uniform_int_distribution<int> uniform;
    std::uniform_int_distribution<int> cost;
    int getRandCost() {
			return this->cost(this->mt);
		}		
		//If all cost values are negative, use random uniform funciton
    int getRandUniform() {
      return this->uniform(this->mt);
    }

		//build spanning tree with union find 
		std::priority_queue<Edge*,std::vector<Edge*>,LessThanCost> pq;
		void build();
		void unionSet(GraphNode* leftNode, GraphNode* rightNode);

		//methods if the tree has been affected 
		void affected(GraphNode* target);
		void rename(GraphNode* target);
		void removeFromTree(GraphNode* target);

		//fake MST
		void fakeBuild();
		void fakeReset();

  public:
	GraphNode* nodes;
    Graph(int numNodes, int seed);
    const int* const* getAdjMatrix() const { return this->adjMatrix; }
	const int* const* getSpanningTree() const { return this->spanningTree; }
	const std::vector<Edge> getEdges() const { return this->costEdges; }
	const int getNumNodes() const { return this->numNodes; }
    void changeNode(int i, int j, int newValue) {
      this->adjMatrix[i][j] = this->adjMatrix[j][i] = newValue;
    }

		//Rebuild spanning tree
		void rebuild();

		//Attacked and fixed
		void attacked(GraphNode* target);
		void fixed(GraphNode* target);
		bool partitioned();
};

Graph::Graph(int numNodes, int seed) : uniform(1, 100), cost(-120, 100) {
	//initialize nodes;
	this->numNodes = numNodes;
	nodes = new GraphNode[numNodes];
	for (int i = 0; i < numNodes; i++) {
		nodes[i].originalName = nodes[i].currentName = i;
		nodes[i].namePathStack.push(i);
	}
	
	//initialize cost matrix
  this->mt.seed(seed);
  this->adjMatrix = new int*[numNodes];
	this->spanningTree = new int*[numNodes];
	this->fakeTree = new int*[numNodes];
  for (int i = 0; i < numNodes; i++) {
    this->adjMatrix[i] = new int[numNodes];
		this->spanningTree[i] = new int[numNodes];
		this->fakeTree[i] = new int[numNodes];
  }

  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < i; j++) {
      this->adjMatrix[j][i] = this->adjMatrix[i][j] = getRandCost();
			this->spanningTree[j][i] = this->spanningTree[i][j] = 0;
			this->fakeTree[j][i] = this->fakeTree[i][j] = 0;
    }
  }

  for (int i = 0; i < numNodes; i++) {
    int max = -1337;
    int maxIndex = -1337;
    for (int j = 0; j < numNodes; j++) {
      if (this->adjMatrix[i][j] > max) {
        max = this->adjMatrix[i][j];
        maxIndex = j;
      }
      if (this->adjMatrix[i][j] <= 0) {
        this->adjMatrix[i][j] = -1337;
      }
    }
    if (max <= 0) {
      this->adjMatrix[i][maxIndex] = this->getRandUniform();
    }
  }

  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < numNodes; j++) {
      if (this->adjMatrix[i][j] == -1337 || i == j) {
        this->adjMatrix[i][j] = 0;
      }
    }
  }

	for (int i = 0; i < numNodes; i++) {
		for (int j = 0; j < numNodes; j++) {
			if (this->adjMatrix[i][j] != 0) {
				Edge edge;
				edge.leftNode = &nodes[i];
				edge.rightNode = &nodes[j];
				edge.cost = adjMatrix[i][j];
				costEdges.push_back(edge);
				
				Edge fakeEdge;
				fakeEdge.leftNode = &fakeNodes[i];
				fakeEdge.rightNode = &fakeNodes[j];
				fakeEdge.cost = adjMatrix[i][j];
				fakeEdges.push_back(fakeEdge);
			}
		}
	}

	build();
}

void Graph::build() {
	//sort first
	for (unsigned int i = 0; i < costEdges.size(); i++) {
		pq.push(&costEdges[i]);
	}

	Edge* tempEdge;
	int leftIndex;
	int rightIndex;
	int leftNodeName;
	int rightNodeName;
	int queueSize = pq.size();
	for (int i = 0; i < queueSize; i++) {
		tempEdge = pq.top();
		leftIndex = tempEdge->leftNode->originalName;
		rightIndex = tempEdge->rightNode->originalName;
		leftNodeName = tempEdge->leftNode->currentName;
		rightNodeName = tempEdge->rightNode->currentName;
		pq.pop();
		
		//add condition that Node is not compromised
		if((leftNodeName != rightNodeName) &&
			!tempEdge->leftNode->compromised &&
			!tempEdge->leftNode->affected &&
			!tempEdge->rightNode->compromised &&
			!tempEdge->rightNode->affected) {
			unionSet(tempEdge->leftNode, tempEdge->rightNode);
			spanningTree[leftIndex][rightIndex] = tempEdge->cost;
		}
	}
}

void Graph::fakeBuild() {
	for (unsigned int i = 0; i < fakeEdges.size(); i++) {
		pq.push(&fakeEdges[i]);
	}

	Edge* tempEdge;
	int leftIndex;
	int rightIndex;
	int leftNodeName;
	int rightNodeName;
	int queueSize = pq.size();
	for(int i = 0; i < queueSize; i++) {
		tempEdge = pq.top();
		leftIndex = tempEdge->leftNode->originalName;
		rightIndex = tempEdge->rightNode->originalName;
		leftNodeName = tempEdge->leftNode->currentName;
		rightNodeName = tempEdge->rightNode->currentName;
		pq.pop();
		
		//add condition that Node is not compromised
		if((leftNodeName != rightNodeName) &&
			!tempEdge->leftNode->compromised &&
			!tempEdge->leftNode->affected &&
			!tempEdge->rightNode->compromised &&
			!tempEdge->rightNode->affected) {
			unionSet(tempEdge->leftNode, tempEdge->rightNode);
			fakeTree[leftIndex][rightIndex] = tempEdge->cost;
		}
	}
}

void Graph::fakeReset() {
	for(int i = 0; i < numNodes; i++) {
		fakeNodes[i].currentName = fakeNodes[i].originalName;
	}

	for(int i = 0; i < numNodes; i++) 
		for(int j = 0; j < numNodes; j++) 
			spanningTree[i][j] = 0;
}

void Graph::unionSet(GraphNode* leftNode, GraphNode* rightNode) {
	//Print before union
	//std::cout << "Before : " << std::endl;
	//std::cout << "Left node's original Name is " << leftNode->originalName << " and " << "current name is " << leftNode->currentName << std::endl;
	//std::cout << "Right node's original Name is " << rightNode->originalName << " and " << "current name is " << rightNode->currentName << std::endl;

	leftNode->adjNodes.push_back(rightNode);
	rightNode->adjNodes.push_back(leftNode);

	//Union Set
	int leftNodeName = leftNode->currentName;
	int rightNodeName = rightNode->currentName;
	
	if(leftNodeName < rightNodeName) {
		for(int i = 0; i < numNodes; i++) {
			if(nodes[i].currentName == rightNodeName) {
				nodes[i].currentName = leftNodeName;
				nodes[i].namePathStack.push(leftNode->originalName);
			}
		}
	}	else {
		for(int i = 0; i < numNodes; i++) {
			if(nodes[i].currentName == leftNodeName) {
				nodes[i].currentName = rightNodeName;
				nodes[i].namePathStack.push(rightNode->originalName);
			}
		}
	}
}

void Graph::rebuild() {
	build();
	fakeBuild();
	fakeReset();
}

void Graph::fixed(GraphNode* target) {
	target->compromised = false;
	target->affected = false;
}

void Graph::attacked(GraphNode* target) {
	target->compromised = true; //compromised
	for(unsigned int i = 0; i < target->adjNodes.size();i++) {
		this->affected(target->adjNodes[i]);
	}

	//remove from spanning tree
	this->removeFromTree(target);  //remove from spanning tree

	//rename
	this->rename(target); //rename 
}

void Graph::affected(GraphNode* target) {
	//std::cout << "Affected" << std::endl;
	target->affected = true;

	this->removeFromTree(target);
	this->rename(target);
}

void Graph::removeFromTree(GraphNode* target) {
	//std::cout << "Remove From Tree" << std::endl;
	int index = target->originalName;
	for(int i = 0; i < numNodes; i++)
		for(int j = 0; j < numNodes; j++)
			if(i == index||j == index)
				spanningTree[i][j] = 0;
}

void Graph::rename(GraphNode* target) {
	//std::cout << "Renaming" << std::endl;
	std::vector<GraphNode*> tempNodes = target->adjNodes;

	if(target->compromised || target->affected) 
		target->currentName = target->originalName;
	
	unsigned int adjNodesSize = tempNodes.size();
	for(unsigned int i = 0; i < adjNodesSize;i++) {
		GraphNode* tempNode = tempNodes[i];

		//find uncompromised and unaffected node through name path stack
		while((tempNode->currentName != tempNode->originalName) && (nodes[tempNode->currentName].compromised || nodes[tempNode->currentName].affected)) {
			int index = tempNode->namePathStack.top();
			if(tempNode->namePathStack.size() == 1) {
				tempNode->currentName = tempNode->namePathStack.top();
				//std::cout << "NamePathStack(0) is " << tempNode->namePathStack.top() << std::endl;
				//std::cout << "Original Name is " << tempNode->originalName << std::endl;
			} else if(nodes[index].compromised||nodes[index].affected) {
				tempNode->namePathStack.pop();
			} else {
				tempNode->currentName = tempNode->namePathStack.top();
			}
		}
	}
	
}

//If spanning tree have a different value, it is a partitioned tree.
bool Graph::partitioned() {
	int tempIndex1 = -1;
	int tempIndex2 = -1;
	
	for(int i = 0; i < numNodes; i++) {
		for(int j = 0; j < numNodes; j++) {
			int tempCost = spanningTree[i][j];
			if(tempCost > 0) {
				//initialize at first time
				if(tempIndex1 == -1) {
					tempIndex1 = i;
					tempIndex2 = j;
				}

				tempIndex1 = i;
				
				int tempName1 = nodes[tempIndex1].currentName;
				int tempName2 = nodes[tempIndex2].currentName;
				//std::cout << "Current name1 is " << tempName1 << std::endl;
				//std::cout << "Current name2 is " << tempName2 << std::endl;

				if(tempName1 != tempName2) {
					std::cout << "The tree is partitioned." << std::endl;
					return true;
				}
			}

		}
	}

	std::cout << "The tree is complete." << std::endl;
	return false;
}
#endif 
