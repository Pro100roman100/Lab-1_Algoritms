#pragma once
#include <raylib.h>
#include <string>
#include <list> 

struct LogData {
	std::string string;
	float remainingTime;
};

class Logger {
private:
	inline static float logShowTime = 5.0f;
	inline static const int logFontSize = 18;

	inline static std::list<LogData> logs;

public:
	static void Log(std::string string);
	static void DrawLogs();
	static void BeginFrame(float deltaTime);
};