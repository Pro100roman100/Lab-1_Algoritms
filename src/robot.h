#pragma once

#include "game_object.h"
#include "animated_model.h"

class Robot : public UpdatableObject, public DrawableObject {
public:
	Robot(Vector3 position) : model("resources/robot.glb") {
		this->position = position;
		robotCount++;
		defaultRealisation = robotCount % 2 == 0;
	}
	~Robot() {
		robotCount--;
	}

	virtual BoundingBox GetBounds() const override {
		return { position - Vector3{2.f, 2.f, 2.f}, position + Vector3{2.f, 2.f, 2.f} };
	}

	virtual void Update(float deltaTime) override {
		model.UpdateAnimation(deltaTime, defaultRealisation);
	}
	virtual void Draw() const override {
		DrawModel(model.GetModel(), position - Vector3{ 0.f, 2.f, 0.f }, 1.f, 
			defaultRealisation ? WHITE : LIGHTGRAY);
	}
private:
	inline static int robotCount = 0;
	bool defaultRealisation = false;

	AnimatedModel model;
};