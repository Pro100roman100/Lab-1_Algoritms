#pragma once

#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <vector>

#include "log.h"
#include "game_object.h"

struct CellCoord {
	int x, y, z;
	CellCoord(int x, int y, int z) : x(x), y(y), z(z) {};
};

template <std::derived_from<GameObject> T>
struct GridCell {
	CellCoord pos{ 0, 0, 0 };
	BoundingBox bounds;
	std::vector<T*> objects;
};

template <std::derived_from<GameObject> T>
class UniformGrid {
public:
	void Clear() {
		for (GridCell<T>& cell : cells) {
			cell.objects.clear();
		}
		totalObjects = 0;
	}
	void GenerateGrid(Vector3 position, CellCoord size, Vector3 cellSize = { 1.f,1.f,1.f }) {
		this->position = position;
		this->size = size;
		this->cellSize = cellSize;
		GenGrid();
	}
	void GenerateGrid(BoundingBox bounds, Vector3 cellSize = { 1.f,1.f,1.f }) {
		this->cellSize = cellSize;

		size = {
			static_cast<int>(std::ceil((bounds.max.x - bounds.min.x) / cellSize.x)) + padding * 2,
			static_cast<int>(std::ceil((bounds.max.y - bounds.min.y) / cellSize.y)) + padding * 2,
			static_cast<int>(std::ceil((bounds.max.z - bounds.min.z) / cellSize.z)) + padding * 2
		};
		position = bounds.min - padding * cellSize;

		GenGrid();
	}
	BoundingBox CalculateBoundingBox(std::vector<GameObject*>& objects) {
		if (objects.empty())
			return { Vector3{0,0,0}, Vector3{0,0,0} };

		BoundingBox bounds = objects[0]->GetBounds();
		for (GameObject* object : objects) {
			BoundingBox objectBounds = object->GetBounds();
			bounds.min.x = std::min(bounds.min.x, objectBounds.min.x);
			bounds.min.y = std::min(bounds.min.y, objectBounds.min.y);
			bounds.min.z = std::min(bounds.min.z, objectBounds.min.z);
			bounds.max.x = std::max(bounds.max.x, objectBounds.max.x);
			bounds.max.y = std::max(bounds.max.y, objectBounds.max.y);
			bounds.max.z = std::max(bounds.max.z, objectBounds.max.z);
		}
		bounds.min += Vector3(.01f, .01f, .01f);
		bounds.max -= Vector3(.01f, .01f, .01f);
		return bounds;
	}

	void AddObject(T* object) {
		if (object == nullptr) {
			return;
		}

		BoundingBox objBounds = object->GetBounds();
		CellCoord min = WorldToCell(objBounds.min + Vector3(.01f, .01f, .01f));
		CellCoord max = WorldToCell(objBounds.max - Vector3(.01f, .01f, .01f));

		if (min.x < 0 || min.y < 0 || min.z < 0 || max.x > size.x - 1 || max.y > size.y - 1 || max.z > size.z - 1)
			cells[0].objects.push_back(object);

		min.x = std::max(min.x, 0);
		min.y = std::max(min.y, 0);
		min.z = std::max(min.z, 0);

		max.x = std::min(max.x, size.x - 1);
		max.y = std::min(max.y, size.y - 1);
		max.z = std::min(max.z, size.z - 1);

		for (int x = min.x; x <= max.x; x++) {
			for (int y = min.y; y <= max.y; y++) {
				for (int z = min.z; z <= max.z; z++) {
					GetCell(CellCoord{ x,y,z }).objects.push_back(object);
				}
			}
		}

		totalObjects++;
	}
	void RemoveObject(T* object) {
		if (object == nullptr) {
			return;
		}

		BoundingBox objBounds = object->GetBounds();
		CellCoord min = WorldToCell(objBounds.min + Vector3(.01f, .01f, .01f));
		CellCoord max = WorldToCell(objBounds.max - Vector3(.01f, .01f, .01f));

		auto& cellObj = cells[0].objects;
		if (min.x < 0 || min.y < 0 || min.z < 0 || max.x > size.x - 1 || max.y > size.y - 1 || max.z > size.z - 1) {
			for (size_t i = 0; i < cellObj.size(); i++) {
				if (cellObj[i] == object) {
					cellObj[i] = cellObj.back();
					cellObj.pop_back();
					break;
				}
			}
		}

		min.x = std::max(min.x, 0);
		min.y = std::max(min.y, 0);
		min.z = std::max(min.z, 0);

		max.x = std::min(max.x, size.x - 1);
		max.y = std::min(max.y, size.y - 1);
		max.z = std::min(max.z, size.z - 1);

		for (int x = min.x; x <= max.x; x++) {
			for (int y = min.y; y <= max.y; y++) {
				for (int z = min.z; z <= max.z; z++) {
					auto& cellObj = GetCell(CellCoord{ x,y,z }).objects;
					for (size_t i = 0; i < cellObj.size(); i++) {
						if (cellObj[i] == object) {
							cellObj[i] = cellObj.back();
							cellObj.pop_back();
							break;
						}
					}
				}
			}
		}

		totalObjects--;
	}

	void MoveObject(T* object, Vector3 transform) {
		RemoveObject(object);
		object->position += transform;
		AddObject(object);
	}

	const std::vector<GridCell<T>>& GetCells() const {
		return cells;
	}
	GridCell<T>& GetCell(Vector3 position) {
		return GetCell(WorldToCell(position));
	}
	GridCell<T>& GetCell(CellCoord cellCoord) {
		if (cellCoord.x < 0 || cellCoord.x >= size.x ||
			cellCoord.y < 0 || cellCoord.y >= size.y ||
			cellCoord.z < 0 || cellCoord.z >= size.z)
			return cells[0];

		return cells[static_cast<size_t>(cellCoord.x) +
			static_cast<size_t>(cellCoord.y) * size.x +
			static_cast<size_t>(cellCoord.z) * size.x * size.y + 1];
	}

	CellCoord WorldToCell(Vector3 cellPos) const {
		return { static_cast<int>(std::floor((cellPos.x - position.x) / cellSize.x)),
			static_cast<int>(std::floor((cellPos.y - position.y) / cellSize.y)),
			static_cast<int>(std::floor((cellPos.z - position.z) / cellSize.z)) };
	}
	Vector3 CellToWorld(CellCoord cellCoord) const {
		return { position.x + (cellCoord.x * cellSize.x) + (cellSize.x / 2.0f),
			position.y + (cellCoord.y * cellSize.y) + (cellSize.y / 2.0f),
			position.z + (cellCoord.z * cellSize.z) + (cellSize.z / 2.0f)
		};
	}

	int getTotalObjects() {
		return totalObjects;
	}
private:
	void GenGrid() {
		cells.clear();
		cells.resize(static_cast<size_t>(size.x) * size.y * size.z + 1);
		for (int x = 0; x < size.x; x++) {
			for (int y = 0; y < size.y; y++) {
				for (int z = 0; z < size.z; z++) {
					CellCoord coord = CellCoord{ x,y,z };
					GridCell<T>& cell = GetCell(coord);
					cell.pos = coord;
					cell.bounds.min = { position.x + cellSize.x * x,
						position.y + cellSize.y * y,
						position.z + cellSize.z * z };
					cell.bounds.max = { position.x + cellSize.x * (x + 1),
						position.y + cellSize.y * (y + 1),
						position.z + cellSize.z * (z + 1) };
				}
			}
		}
	}

	int padding = 1;

	int totalObjects = 0;

	Vector3 position{ 0.f,0.f,0.f };
	CellCoord size{ 0,0,0 };
	Vector3 cellSize{ 1, 1, 1 };

	std::vector<GridCell<T>> cells; // 1st cell is objects outside of grid
};
