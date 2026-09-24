#include "console.h"

#include "config_loader.h"

#include <sstream>

void Console::AddOutput(const std::string& text) {
	output.push_back(text);
	if (output.size() > 12) {
		output.erase(output.begin());
	}
}

void Console::ExecuteCommand(std::string& command) {
	to_lower_case(command);
	if (command.empty()) {
		return;
	}

	std::istringstream stream(command);
	std::string name;
	stream >> name;


	if (name == "help" || name == "h") {
		AddOutput("help - help menu");
		AddOutput("reloadconfig - load config from file");
		AddOutput("spawn_interval <value> - change spawn interval");
		AddOutput("min_enemy_speed <value> - change min enemy speed");
		AddOutput("max_enemy_speed <value> - change max enemy speed");
		AddOutput("max_enemies <value> - change max enemies");
		AddOutput("log_show_time <value> - change log show time");
		return;
	}
	if (name == "reloadconfig") {
		AddOutput(ConfigLoader::Load("config.txt")
			? "Configuration reloaded"
			: "Failed to reload configuration");
		return;
	}

	std::string value;
	if (stream >> value && ConfigLoader::SetParameter(name, value)) {
		AddOutput("Set " + name + " = " + value);
	}
	else {
		AddOutput("Unknown or invalid command: " + command);
	}
}

void Console::Update() {
	if (IsKeyPressed(KEY_GRAVE)) {
		open = !open;
		if (!open) {
			input.clear();
		}
		return;
	}

	if (!open) {
		return;
	}

	if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) {
		input.pop_back();
	}

	if (IsKeyPressed(KEY_ENTER)) {
		AddOutput("> " + input);
		ExecuteCommand(input);
		input.clear();
	}

	int character = GetCharPressed();
	while (character > 0) {
		if (character >= 32 && character <= 126) {
			input += static_cast<char>(character);
		}
		character = GetCharPressed();
	}
}

void Console::Draw() {
	if (!open) {
		return;
	}

	const int fontSize = 18;
	const int lineHeight = 22;
	const int height = GetScreenHeight() / 2;

	DrawRectangle(0, 0, GetScreenWidth(), height, { 0, 0, 0, 220 });

	int y = 8;
	for (const std::string& line : output) {
		DrawText(line.c_str(), 8, y, fontSize, RAYWHITE);
		y += lineHeight;
	}

	DrawLine(8, height - lineHeight - 8, GetScreenWidth() - 8,
		height - lineHeight - 8, GRAY);
	DrawText(("> " + input).c_str(), 8, height - lineHeight, fontSize, PINK);
}

bool Console::IsOpen() const {
	return open;
}