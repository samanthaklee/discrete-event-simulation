//Code taken from www.github.com/nilocunger

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>

#include "graph.hpp"

char *lpad(char *text, int length, char padWith) {
  char *padded = new char[length+1];
  int textLength = strlen(text);
  for (int i=0; i < length; i++) { 
    if (i >= length - textLength) {
      padded[i] = text[i - (length - textLength)];
    } else {
      padded[i] = padWith;
    }
  }
  padded[length] = '\0';
  return padded;
}

int main(int argc, char **argv) {
  int seed;
  if (argc == 2) {
    seed = (std::random_device())();
  } else if (argc == 3) {
    seed = atoi(argv[2]);
  } else {
    std::cout << "Usage: generate <num_nodes> [<seed>]" << std::endl;
    exit(1);
  }

  int numNodes = atoi(argv[1]);
  Graph g(numNodes, seed);
  const int * const* adjMatrix = g.getAdjMatrix();
	const int * const* spanningTree = g.getSpanningTree();
	const std::vector<Edge> edges = g.getEdges();

	//Print AdjMatrix
  for (int i = 0; i < numNodes; i++) {
    for (int j = 0; j < numNodes; j++) {
      char element[4];
      sprintf(element, "%d", adjMatrix[i][j]);
      char *padded = lpad(element, 3, ' ');
      std::cout << padded << " "; 
      delete[] padded;
    }
    std::cout << std::endl;
  }
	

	//Print Edges
	for (unsigned int i = 0; i < edges.size(); i++) {
    std::cout << "Cost is " << edges[i].cost << std::endl;
	}

	std::cout << "\nSpanning Tree begins " << std::endl;

	//Print Spanning Trees
	for (int i = 0; i < numNodes; i++) 
		for (int j = 0; j < numNodes; j++) {
			if(spanningTree[i][j] > 0) {
			   std::cout << i << " node and " <<  j << " node has " << spanningTree[i][j] << " cost." << std::endl;
			}
		}
  return 0;
}
