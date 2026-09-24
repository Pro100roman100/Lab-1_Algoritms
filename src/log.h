#pragma once
#include <raylib.h>
#include <string>
#include <list> 

#include "game_settings.h"

#define LOG(string) Logger::Log(string)

struct LogData {
	std::string string;
	float remainingTime;
};

class Logger {
private:
	static const int logFontSize = 18;
	static std::list<LogData> logs;

public:
	static void Log(std::string string);
	static void DrawLogs();
	static void BeginFrame(float deltaTime);
};