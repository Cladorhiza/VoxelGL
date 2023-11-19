#pragma once
#include <chrono>
#include <iostream>
#include <string>

class Stopwatch {

public:

	Stopwatch();
	void Restart();
	std::string ToString() const;
	double GetElapsed() const;

	//overload cout operator << to print the time elapsed
	friend std::ostream& operator<<(std::ostream& os, const Stopwatch& s);
	
private:

	std::chrono::time_point<std::chrono::high_resolution_clock> start;
};