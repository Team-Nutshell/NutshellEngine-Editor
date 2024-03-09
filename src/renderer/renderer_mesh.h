#pragma once
#include <qopengl.h>
#include <cstdint>
#include <string>

struct RendererMesh {
	int vertexBuffer = 0;
	int indexBuffer = 0;
	int indexCount = 0;
	std::string texturePath = "defaultTexture";
};