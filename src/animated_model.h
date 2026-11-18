#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <rlgl.h>

class AnimatedModel {
public:
	AnimatedModel(const char* filename);
	~AnimatedModel();

	void UpdateAnimation(float deltaTime, bool defaultImplementation);
	Model GetModel() const;
private:

	void UpdateAnimationPose();
	inline Matrix BuildTransformMatrix(Transform* transform);

	Model model;
	
	float frameAcumulator = 0.f;
	float frameTime = 1.f / 60.f;

	unsigned int currentFrame = 0;
	unsigned int currentAnimation = 0;

	int animCount = 0;
	ModelAnimation* anims;
};