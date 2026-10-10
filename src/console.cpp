#include "console.h"

#include "config_loader.h"
#include "game_world.h"

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
		AddOutput("reload_config - load config from file");
		AddOutput("frustum_update - flip frustum update");
		AddOutput("spawn_interval <value> - change spawn interval");
		AddOutput("min_enemy_speed <value> - change min enemy speed");
		AddOutput("max_enemy_speed <value> - change max enemy speed");
		AddOutput("max_enemies <value> - change max enemies");
		AddOutput("log_show_time <value> - change log show time");
		AddOutput("max_fps <value> - change max fps");
		return;
	}
	if (name == "reload_config") {
		AddOutput(ConfigLoader::Load("config.txt")
			? "Configuration reloaded"
			: "Failed to reload configuration");
		return;
	}
	if (name == "frustum_update") {
		GameWorld::GetInstance().updateFrustum = !GameWorld::GetInstance().updateFrustum;
		AddOutput(GameWorld::GetInstance().updateFrustum
			? "frustum update on"
			: "frustum update off");
		return;
	}

	std::string value;
	stream >> value;
	ParseCode code = ConfigLoader::SetParameter(name, value);
	if (code == ParseCode::no_err)
		AddOutput(name + " changed successfully");
	else
		AddOutput(ConfigLoader::GetErrorString(code, name, value));
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