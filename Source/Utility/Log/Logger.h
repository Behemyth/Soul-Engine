#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <mutex>
#include <iostream>

/* Values that represent log severities. */
enum class LogSeverity {
	S_TRACE,
	S_WARNING,
	S_ERROR,
	S_FATAL
};


class Logger {

	//User: Do Not Touch

public:

	template <typename T>

	/*
	 *    Logs a help.
	 *    @param [in,out]	o	A std::ostream to process.
	 *    @param 		 	t	A T to process.
	 */

	void LogHelp(std::ostream& o, T t)
	{
		o << t;
	}

	template<typename T, typename... Args>

	/*
	 *    Logs a help.
	 *    @param [in,out]	o   	A std::ostream to process.
	 *    @param 		 	t   	A T to process.
	 *    @param 		 	args	Variable arguments providing the arguments.
	 */

	void LogHelp(std::ostream& o, T t, Args... args)
	{
		LogHelp(o, t);
		LogHelp(o, args...);
	}

	/* The log severity strings[ 4] */
	std::string LogSeverityStrings[4];
	/* The log mut */
	std::mutex logMut;


	/* A log i. */
	typedef struct LogI {

		/* The message */
		std::string msg;
		/* The severity */
		LogSeverity severity;
		/* The lune number */
		int luneNumber;
		/* Filename of the file */
		const char* filename;

		// .
	}LogI;

	/* The storage */
	std::deque<LogI> storage;

	/*
	 *    Setters for logging messages. Interfaced by macros.
	 *    @param [in,out]	oss 	The oss.
	 *    @param 		 	file	The file.
	 *    @param 		 	line	The line.
	 */

	void WriteInfo(std::ostream& oss, const char* file, int line);


	//Logs a message of the specified type
	template<typename... Args>

	/*
	 *    Logs.
	 *    @param	logType	Type of the log.
	 *    @param	file   	The file.
	 *    @param	line   	The line.
	 *    @param	args   	Variable arguments providing the arguments.
	 */

	void Log(LogSeverity logType, const char* file, int line, Args... args)
	{
		std::ostringstream oss;
		WriteInfo(oss, file, line);
		Logger::LogHelp(oss, args...);
		Logger::logMut.lock();
		Logger::storage.push_back({ oss.str() ,logType ,line,file });
		Logger::logMut.unlock();
		std::cout << oss.str() << std::endl; //temporary
	}


	/*
	 *    Get the next available string to write. Empty string if none written.
	 *    @return	A std::string.
	 */

	std::string Get();

	void WriteFile();
};