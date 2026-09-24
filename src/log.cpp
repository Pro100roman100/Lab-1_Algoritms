#include "log.h"

std::list<LogData> Logger::logs;

void Logger::Log(std::string string) {
    const GameSettings& settings = GameSettings::GetInstance();
    logs.push_front({ string, settings.logShowTime });
}

void Logger::DrawLogs() {
	int screenWidth = GetScreenWidth();
	int offset = 5;
    bool isFirstLog = true;

	for (LogData logData : logs) {
        if (isFirstLog) {
            int firstFontSize = logFontSize + (int)((float)logFontSize * 0.2f);
            int textSize = MeasureText(logData.string.c_str(), firstFontSize);
            DrawText(logData.string.c_str(), screenWidth - textSize - 5, offset, firstFontSize, BLACK);
            offset += firstFontSize;
        }
        else {
            int textSize = MeasureText(logData.string.c_str(), logFontSize);
            DrawText(logData.string.c_str(), screenWidth - textSize - 5, offset, logFontSize, {20, 20, 20, 255});
            offset += logFontSize;
        }

        isFirstLog = false;
	}
}

void Logger::BeginFrame(float deltaTime) {
    for (auto it = logs.begin(); it != logs.end(); ) {
        it->remainingTime -= deltaTime;

        if (it->remainingTime <= 0.0f) {
            it = logs.erase(it);
        }
        else {
            ++it;
        }
    }
}