//sysadmin fix queue
//implementation of constant time enqueue, dequeue, and membership check 

#ifndef SYSADMIN_H
#define SYSADMIN_H
#include "graph.hpp" 
#include <queue> //queue STL

class SysAdmin {
	private:
		std::queue<GraphNode*> queue;
		bool* networkTable;
		int numComputers;
	public:
		SysAdmin(int numComputers) { //constructor 
			this->numComputers = numComputers;
			networkTable = new bool[numComputers];
			for(int i = 0; i < numComputers; i++) 
				 networkTable[i] = false;
		}
		void push(GraphNode* node) {
			queue.push(node);
			networkTable[node->originalName] = 1;
		}
		GraphNode* pop() {
			GraphNode* temp = queue.front();
			queue.pop();
			networkTable[temp->originalName] = 0;
			return temp;
		}
		bool check(GraphNode* node) {
			return networkTable[node->originalName];
		}
};

	
#endif 
