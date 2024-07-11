#pragma once
#include "../common/bounding_volumes.h"
#include <qopengl.h>
#include <cstdint>
#include <string>

struct RendererMesh {
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	GLuint indexCount = 0;
	std::string diffuseTexturePath = "defaultDiffuseTexture";
	std::string emissiveTexturePath = "defaultEmissiveTexture";
	float alphaCutoff = 0.0f;

	AABB aabb;
	OBB obb;
};