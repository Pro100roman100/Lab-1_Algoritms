#pragma once

#include <raylib.h>
#include <fstream>

#include "game_world.h"
#include "log.h"
#include "player_camera.h"

#include "camera_marker.h"

enum Objects {
	Cube1x1Pos,
	Cube10x10Pos,
	TargetPos,
	CameraPos,
	DynamicSpherePos,
	SkittlePos
};

class MapSave {
public:
	static bool LoadFromFile(std::string filename = "level");
	static bool SaveToFile(std::string filename = "level");
};