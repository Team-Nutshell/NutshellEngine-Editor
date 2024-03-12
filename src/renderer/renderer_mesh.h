#pragma once
#include <qopengl.h>
#include <cstdint>
#include <string>

struct RendererMesh {
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	GLuint indexCount = 0;
	std::string diffuseTexturePath = "defaultDiffuseTexture";
	std::string emissiveTexturePath = "defaultEmissiveTexture";
};