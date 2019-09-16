#include "Logger.h"

std::string LogSeverityStrings[4]{
	"TRACE",
	"WARNING",
	"ERROR",
	"FATAL"
};

/* The log mut */
std::mutex logMut;
/* The storage */
std::deque<Logger::LogI> storage;

/*
 *    Writes an information.
 *    @param [in,out]	oss 	The oss.
 *    @param 		 	file	The file.
 *    @param 		 	line	The line.
 */

void WriteInfo(std::ostream& oss, const char* file, int line) {
	oss << "File: TODO" << " Line: " << line << " | ";
}


/*
 *    Gets the get.
 *    @return	A std::string.
 */

std::string Get() {
	logMut.lock();
	if (storage.empty() > 0) {

		Logger::LogI temp = storage.front();
		storage.pop_front();
		logMut.unlock();

		return ("[" + LogSeverityStrings[static_cast<int>(temp.severity)] + "] " + temp.msg + "/n");
	}
	else {
		logMut.unlock();
		return std::string();
	}
}

void WriteFile()
{
	//TODO
}
