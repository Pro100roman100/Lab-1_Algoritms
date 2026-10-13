#include "map_save.h"

#include <sstream>
#include <vector>

bool MapSave::LoadFromFile(std::string filename) {
	std::ifstream mapFile(filename + ".map");
	if (!mapFile.is_open()) {
		LOG("Failed to open map file " + filename + ".map when loading");
		return false;
	}

	struct MapObject {
		Objects type;
		Vector3 position;
	};

	std::vector<MapObject> objects;
	std::string line;
	while (std::getline(mapFile, line)) {
		if (line.empty()) {
			continue;
		}

		std::istringstream record(line);
		int type;
		Vector3 position;
		std::string extra;
		if (!(record >> type >> position.x >> position.y >> position.z) ||
			record >> extra ||
			type < Objects::Cube1x1Pos || type > Objects::CameraPos) {
			LOG("Invalid record in map file " + filename + ".map");
			return false;
		}

		objects.push_back({ static_cast<Objects>(type), position });
	}

	GameWorld& world = GameWorld::GetInstance();
	world.ClearWorld();
	bool targetLoaded = false;
	bool cameraLoaded = false;
	for (const MapObject& object : objects) {
		switch (object.type) {
		case Objects::Cube1x1Pos:
			world.AddObject(new StaticCube(object.position, { 1.0f, 1.0f, 1.0f }));
			break;
		case Objects::Cube10x10Pos:
			world.AddObject(new StaticCube(object.position, { 10.0f, 1.0f, 10.0f }));
			break;
		case Objects::TargetPos:
			if (!targetLoaded) {
				world.AddObject(new Target(object.position));
				targetLoaded = true;
			}
			break;
		case Objects::CameraPos:
			if (!cameraLoaded) {
				world.AddObject(new CameraMarker(object.position));
				PlayerCamera::GetCamera().SetPosition(object.position);
				cameraLoaded = true;
			}
			break;
		}
	}

	LOG("Map loaded from " + filename + ".map");
	return true;
}

bool MapSave::SaveToFile(std::string filename) {
	std::ofstream mapFile(filename + ".map");
	if (!mapFile.is_open()) {
		LOG("Failed to open map file " + filename + ".map when saving");
		return false;
	}

	bool targetSaved = false;
	bool cameraSaved = false;
	for (GameObject* object : GameWorld::GetInstance().GetObjects()) {
		if (auto cube = dynamic_cast<StaticCube*>(object)) {
			if(abs(cube->size.x - 10.0f) < 0.001f)
				mapFile << Objects::Cube10x10Pos << " " << cube->position.x << " " << cube->position.y << " " << cube->position.z << std::endl;
			else
				mapFile << Objects::Cube1x1Pos << " " << cube->position.x << " " << cube->position.y << " " << cube->position.z << std::endl;
		}
		else if (auto target = dynamic_cast<Target*>(object)) {
			if (targetSaved) {
				LOG("Duplicate targets, only one is saved");
				continue;
			}
			mapFile << Objects::TargetPos << " " << target->position.x << " " << target->position.y << " " << target->position.z<< std::endl;
			targetSaved = true;
		}
		else if (auto camera = dynamic_cast<CameraMarker*>(object)) {
			if (cameraSaved) {
				LOG("Duplicate cameras, only one is saved");
				continue;
			}
			mapFile << Objects::CameraPos << " " << camera->position.x << " " << camera->position.y << " " << camera->position.z << std::endl;
			cameraSaved = true;
		}
	}

	LOG("Map saved to " + filename + ".map");
	return true;
}