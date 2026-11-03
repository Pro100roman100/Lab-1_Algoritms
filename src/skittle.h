#pragma once

#include "game_object.h"
#include "physics_object.h"

class Skittle : public DrawableObject, public PhysicsObject {
public:
	Skittle(Vector3 position);

	void Draw() const override;
	BoundingBox GetBounds() const override;

private:

	static std::unique_ptr<Model> model;
	static std::unique_ptr<Texture2D> texture;
};