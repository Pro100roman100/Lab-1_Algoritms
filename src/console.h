#pragma once

#include <raylib.h>
#include <string>
#include <vector>

class Console {
private:
	bool open = false;
	std::string input;
	std::vector<std::string> output;

	void ExecuteCommand(std::string& command);
	void AddOutput(const std::string& text);

public:
	void Update();
	void Draw();
	bool IsOpen() const;
};