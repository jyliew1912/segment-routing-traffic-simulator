CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall

all: network_sim

network_sim: src/main.cpp
	$(CXX) $(CXXFLAGS) -o network_sim src/main.cpp

clean:
	rm -f network_sim *.o