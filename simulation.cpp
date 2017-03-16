#ifndef SIMULATION_H
#define SIMULATION_H
#include "pqueue.hpp"
#include "graph.hpp"
#include "sysadmin.cpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

enum ACTION {
	EXECUTE_ATTACK = 5,
	DEPLOY_ATTACK = 4,
	EXECUTE_FIX = 3,
	DEPLOY_FIX = 2,
	EXECUTE_REBUILD = 1,
	DEPLOY_REBUILD = 0
};

struct Event {
	ACTION action;
	GraphNode* source;
	GraphNode* target;
};

bool tiebreaker(Event& x1, int p1, Event& x2, int p2) {
	return x1.action > x2.action;
}

class Simulator {
	private:
		//input values
		int numAttackers;
		int numSysadmins;
		int numComputers;
		int seed;

		//time and number of attack
		int t;
		int numAttack;
		

		//Agent and queue
		Graph* computerNetwork;
		SysAdmin* sysAdminsQueue; 
		PriorityQueue<Event, tiebreaker> pq;
		bool checkRebuild = false;

		//Randocm number generation
		std::mt19937 mt;
		std::uniform_int_distribution<int> comp_distribution;
		std::uniform_int_distribution<int> attack_distribution{100,1000};
		std::uniform_int_distribution<int> fix_distribution{1000,2000};
		//Fetch-Execute cycle
		Event fetch();
		void process(Event& e);

		//Schedule methods
		void scheduleDeployAttack();
		void scheduleExecuteAttack(GraphNode* target);
		void scheduleDeployFix();
		void scheduleExecuteFix(GraphNode* target);
		void scheduleDeployRebuild();
		void scheduleExecuteRebuild();

		//Process methods
		void processDeployAttack(Event& e);
		void processExecuteAttack(Event& e);
		void processDeployFix(Event& e);
		void processExecuteFix(Event& e);
		void processDeployRebuild(Event& e);
		void processExecuteRebuild(Event& e);

		int randomComputer(int computer) {
			int randComp = this->comp_distribution(this->mt);
			return (randComp != computer) ? randComp : this->randomComputer(computer);
		}

	public:
		Simulator(int numAttackers, int numSysadmins, int numComputers, int seed);
		Simulator operator=(Simulator& rhs);
		Simulator(Simulator& rhs);
		~Simulator() {
			delete computerNetwork;
			delete sysAdminsQueue;
		}
		
		void run();
};
		
//Constructor
Simulator::Simulator(int numAttackers, int numSysadmins, int numComputers, int seed) {
	this->numAttackers = numAttackers;
	this->numSysadmins = numSysadmins;
	this->numComputers = numComputers;
	this->seed = seed;

	this->t= 0;
	this->numAttack = 0;

	computerNetwork = new Graph(numComputers, seed);
	sysAdminsQueue = new SysAdmin(numComputers);
	this->mt = std::mt19937(seed);
	this->comp_distribution = std::uniform_int_distribution<int>(0,numComputers - 1);
}

//Copy constructor
Simulator::Simulator(Simulator& s) {
	Simulator(s.numAttackers, s.numSysadmins, s.numComputers, s.seed);
	this->t = s.t;
	this->numAttack = s.numAttack;

	delete this->computerNetwork;
	delete this->sysAdminsQueue;
	this->computerNetwork = s.computerNetwork;
	this->sysAdminsQueue = s.sysAdminsQueue;
	this->pq = s.pq;
	this->mt = s.mt;
	this->comp_distribution = s.comp_distribution;
}

//Overloaded assignment operator
Simulator Simulator::operator=(Simulator& s) {
	delete this->computerNetwork;
	delete this->sysAdminsQueue;
	new (this) Simulator(s);
	return *this;
}

//Starts the simulation
void Simulator::run() {
	Event fetched;

	std::cout << "STARTING SIMULATION" << std::endl;
	for(int i = 0; i < numAttackers;i++) 
		this->scheduleDeployAttack();
	while(numAttack < 2000) {
		fetched = this->fetch();
		this->process(fetched);
	}
	std::cout << "ATTACK FINISHED" << std::endl;
}

//The fetch part of the fetch-execute cycle
Event Simulator::fetch() {
	auto next = pq.pop();
	this->t = next.priority;
	return next.content;
}

//The execute part of the fetch-execute cycle
void Simulator::process(Event& e) {

	switch(e.action) {
		case EXECUTE_ATTACK:
			this->processExecuteAttack(e);
			break;
		case DEPLOY_ATTACK:
			this->processDeployAttack(e);
			break;
		case EXECUTE_FIX:
			this->processExecuteFix(e);
			break;
		case DEPLOY_FIX:
			this->processDeployFix(e);
			break;
		case EXECUTE_REBUILD:
			this->processExecuteRebuild(e);
			break;
		case DEPLOY_REBUILD:
			this->processDeployRebuild(e);
			break;
	}
}

void Simulator::scheduleDeployAttack() {
	//std::cout << "is this working" << std::endl;
	Event e;
	e.action = DEPLOY_ATTACK;
	e.target = &(computerNetwork->nodes[this->comp_distribution(this->mt)]);
	int t = this->t + attack_distribution(this->mt);
	//std::cout << "current time attack " << time << std::endl;
	this->pq.push(e, t);
	std::cout << "Deploy_Attack(" << t << ", " << e.target->originalName << ")" << std::endl;
}

void Simulator::scheduleExecuteAttack(GraphNode* target) {
	//std::cout << "this is working" << std::endl;
	Event e;
	e.action = EXECUTE_ATTACK;
	e.target = target;
	this->pq.push(e, this->t);
	(this->numAttack)++;
	std::cout << "Execute_Attack(" << t << ", " << e.target->originalName << ")" << std::endl;
}

void Simulator::scheduleDeployFix() {
	//std::cout << "helloooo work please" << std::endl;
	Event e;
	e.action = DEPLOY_FIX;
	int t = this->t + fix_distribution(this->mt);
	this->pq.push(e, t);
	std::cout << "Deploy_Fix(" << t<< ", " << e.target->originalName << ")" << std::endl;
}

void Simulator::scheduleExecuteFix(GraphNode* target) {
	//std::cout << "fix scheduled" << std::endl;
	Event e;
	e.action = EXECUTE_FIX;
	e.target = target;
	this->pq.push(e, this->t);
	std::cout << "Execute_Repair(" << e.target->originalName << ")" << std::endl;
}

//Rebuild is executed when NOT on the queueu
//checked by bool variable checkRebuild

void Simulator::scheduleDeployRebuild() {
	//std::cout << "rebuild scheduled" << std::endl;
	if(!(this->checkRebuild)) {
		Event e;
		e.action = DEPLOY_REBUILD;
		int t = this->t + 20;
		this->pq.push(e, t);
		std::cout << "Deploy_Rebuild(" << e.target->originalName << ")" << std::endl;
	}
	this->checkRebuild=true;
}

void Simulator::scheduleExecuteRebuild() {
	//std::cout << "scheduleExecutebuild" << std::endl;
	Event e;
	e.action = EXECUTE_REBUILD;
	this->pq.push(e,t);
	std::cout << "Execute_Rebuild(" << e.target->originalName << ")" << std::endl;
}

//The processor method to handle the execution of the events
void Simulator::processDeployAttack(Event &e) {
	this->scheduleExecuteAttack(e.target);
}

void Simulator::processExecuteAttack(Event &e) {
	GraphNode* tempNode = e.target;
	std::vector<GraphNode*> adjNodes = tempNode->adjNodes;

	//queue compromised and affected node first before real attack
	sysAdminsQueue->push(tempNode); //push on sysadmin fix queue
	for(unsigned int i = 0; i < adjNodes.size(); i++)
		sysAdminsQueue->push(adjNodes[i]);
	
	computerNetwork->attacked(tempNode);

	//rebuild when the spanning tree is partitioned

	if(computerNetwork->partitioned())
		this->scheduleDeployRebuild();
	this->scheduleDeployAttack();
}

void Simulator::processDeployFix(Event &e) {	
	this->scheduleExecuteFix(e.target);
}

void Simulator::processExecuteFix(Event &e) {
	GraphNode* tempNode = sysAdminsQueue->pop();
	computerNetwork->fixed(tempNode);
	this->scheduleDeployRebuild();
	this->scheduleDeployFix();
}

void Simulator::processDeployRebuild(Event &e) {
	this->scheduleExecuteRebuild();
}

void Simulator::processExecuteRebuild(Event &e) {
	this->computerNetwork->rebuild();
	this->checkRebuild = false;
}

int main(int argc, char** argv) {
	if (argc != 5) {
		std::cout << "Usage: ./simulator <num_attackers> <num_sysadmins> <num_computers> <seed_number>" << std::endl;
		exit(1);
	}
	Simulator simulator(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
	simulator.run();

	return 0;
}

#endif
