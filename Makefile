CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

all: network_sim

network_sim: src/main.cpp
	$(CXX) $(CXXFLAGS) -o network_sim src/main.cpp

clean:
	@cmake -E remove -f network_sim network_sim.exe *.o
