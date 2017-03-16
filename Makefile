CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra 

.PHONY: clean 

simulation: simulation.o
	$(CXX) $(CXXFLAGS) $< -o $@

graph: command.o
	$(CXX) $(CXXFLAGS) $< -o $@

command.o : command.cpp graph.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

simulation.o : simulation.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:: 
	rm graph simulation command.o simulation.o
