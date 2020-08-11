#include "..\..\Source Files\Utility\Logger.h"
#include <iostream>
#include <cassert>

int main()
{
	namespace pt = boost::posix_time;
	std::string line;
	std::string time;
	time = pt::to_iso_string(pt::second_clock::universal_time());
	Logger::WriteFile();
	std::ifstream file("Engine.log");
	std::getline(file, line);
	assert(line.compare(time) == 0);
	std::getline(file, line);
	assert(line.compare("File: Engine.log Line: 7 | ") == 0);
	file.close();
	return 0;
}