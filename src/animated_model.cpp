#include "animated_model.h"

AnimatedModel::AnimatedModel(const char* filename) {
	model = LoadModel(filename);
	anims = LoadModelAnimations(filename, &animCount);
}

AnimatedModel::~AnimatedModel() {
	UnloadModelAnimations(anims, animCount);
	UnloadModel(model);
}

void AnimatedModel::UpdateAnimation(float deltaTime, bool defaultImplementation) {
	if (frameTime <= 0.0f || anims[currentAnimation].keyframeCount <= 0)
		return;

	frameAcumulator += deltaTime;
	while (frameAcumulator >= frameTime) {
		frameAcumulator -= frameTime;
		currentFrame = (currentFrame + 1) % anims[currentAnimation].keyframeCount;
	}

	if (defaultImplementation) {
		UpdateModelAnimation(model, anims[currentAnimation], (frameAcumulator / frameTime) + currentFrame);
	}
	else {
		UpdateAnimationPose();
	}
}

Model AnimatedModel::GetModel() const {
	return model;
}

void AnimatedModel::UpdateAnimationPose() {
	if (model.boneMatrices == nullptr) 
		return;

	if (anims[currentAnimation].keyframeCount > 0 &&
		model.skeleton.bones != nullptr &&
		anims[currentAnimation].keyframePoses != nullptr) {

		unsigned int nextFrame = (currentFrame + 1) % anims[currentAnimation].keyframeCount;
		float blend = Clamp(frameAcumulator / frameTime, 0.f, 1.f);

		ModelAnimPose& currentPose = anims[currentAnimation].keyframePoses[currentFrame];
		ModelAnimPose& nextPose = anims[currentAnimation].keyframePoses[nextFrame];

		Matrix bindPoseMatrix = { 0 };
		Matrix currentPoseMatrix = { 0 };
		for (unsigned int boneIndex = 0; boneIndex < model.skeleton.boneCount; boneIndex++) {
			model.currentPose[boneIndex].translation = Vector3Lerp(
				currentPose[boneIndex].translation,
				nextPose[boneIndex].translation, 
				blend);
			model.currentPose[boneIndex].rotation = QuaternionSlerp(
				currentPose[boneIndex].rotation,
				nextPose[boneIndex].rotation, 
				blend);
			model.currentPose[boneIndex].scale = Vector3Lerp(
				currentPose[boneIndex].scale,
				nextPose[boneIndex].scale, 
				blend);

			Transform* bindPoseTransform = &model.skeleton.bindPose[boneIndex];
			bindPoseMatrix = BuildTransformMatrix(bindPoseTransform);

			Transform* currentPoseTransform = &model.currentPose[boneIndex];
			currentPoseMatrix = BuildTransformMatrix(currentPoseTransform);

			model.boneMatrices[boneIndex] = MatrixMultiply(MatrixInvert(bindPoseMatrix), currentPoseMatrix);
			//model.boneMatrices[boneIndex] = currentPoseMatrix;
		}
	}

	// CPU-скінінг
    for (int m = 0; m < model.meshCount; m++) {
        Mesh& mesh = model.meshes[m];
        Vector3 animVertex = { 0 };
        Vector3 animNormal = { 0 };
        const int vertexValuesCount = mesh.vertexCount * 3;

        int boneIndex = 0;
        int boneCounter = 0;
        float boneWeight = 0.0f;
        bool bufferUpdateRequired = false;

        if (mesh.boneWeights == nullptr || mesh.boneIndices == nullptr ||
            mesh.animVertices == nullptr) 
            continue;

        for (int vCounter = 0; vCounter < vertexValuesCount; vCounter += 3)
        {
            mesh.animVertices[vCounter] = 0;
            mesh.animVertices[vCounter + 1] = 0;
            mesh.animVertices[vCounter + 2] = 0;

            for (int j = 0; j < 4; j++, boneCounter++)
            {
                boneWeight = mesh.boneWeights[boneCounter];
                boneIndex = mesh.boneIndices[boneCounter];

                if (boneWeight == 0.0f) 
                    continue;
                animVertex = Vector3{ mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                animVertex = Vector3Transform(animVertex, model.boneMatrices[boneIndex]);
                mesh.animVertices[vCounter] += animVertex.x * boneWeight;
                mesh.animVertices[vCounter + 1] += animVertex.y * boneWeight;
                mesh.animVertices[vCounter + 2] += animVertex.z * boneWeight;
                bufferUpdateRequired = true;
            }
        }

        if (bufferUpdateRequired)
            rlUpdateVertexBuffer(mesh.vboId[SHADER_LOC_VERTEX_POSITION], mesh.animVertices, sizeof(float) * mesh.vertexCount * 3, 0);
    }
}

inline Matrix AnimatedModel::BuildTransformMatrix(Transform* transform) {
	return MatrixMultiply(
		MatrixMultiply(MatrixScale(transform->scale.x, transform->scale.y, transform->scale.z),
			QuaternionToMatrix(transform->rotation)),
		MatrixTranslate(transform->translation.x, transform->translation.y, transform->translation.z));
}
