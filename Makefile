CXX      = g++
CXXFLAGS = -std=c++23

all: build/2025/good-friday build/2025/holy-saturday build/2025/easter-sunday

build/2025/%: src/2025/%.cpp build/2025
	$(CXX) $(CXXFLAGS) $< -o $@

build/2025:
	mkdir -p build/2025
