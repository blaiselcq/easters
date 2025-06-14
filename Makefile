CXX      = g++
CXXFLAGS = -std=c++23


build/2025/easter-sunday: src/2025/easter-sunday.cpp build/2025
	$(CXX) $(CXXFLAGS) $< -o $@


build/2025:
	mkdir -p build/2025
