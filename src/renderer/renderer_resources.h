#pragma once
#include "../common/bounding_volumes.h"
#include <qopengl.h>
#include <qopenglfunctions.h>
#include <vector>
#include <utility>
#include <string>
#include <cstdint>

struct RendererSampler {
	void bind(QOpenGLFunctions& gl) {
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		if (minFilter == GL_NEAREST) {
			if (mipmapFilter == GL_NEAREST) {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
			else {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			}
		}
		else {
			if (mipmapFilter == GL_NEAREST) {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			}
			else {
				gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
		}
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, anisotropyLevel);
	}

	GLint magFilter = GL_NEAREST;
	GLint minFilter = GL_NEAREST;
	GLint mipmapFilter = GL_NEAREST;
	GLint wrapS = GL_CLAMP_TO_EDGE;
	GLint wrapT = GL_CLAMP_TO_EDGE;
	GLfloat anisotropyLevel = 1.0f;
};

struct RendererMesh {
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	GLuint indexCount = 0;
};

struct RendererMaterial {
	std::string diffuseTextureName = "defaultDiffuseTexture";
	std::string diffuseTextureSamplerName = "defaultSampler";
	std::string normalTextureName = "defaultNormalTexture";
	std::string normalTextureSamplerName = "defaultSampler";
	std::string metalnessTextureName = "defaultORMTexture";
	std::string metalnessTextureSamplerName = "defaultSampler";
	std::string roughnessTextureName = "defaultORMTexture";
	std::string roughnessTextureSamplerName = "defaultSampler";
	std::string occlusionTextureName = "defaultORMTexture";
	std::string occlusionTextureSamplerName = "defaultSampler";
	std::string emissiveTextureName = "defaultEmissiveTexture";
	std::string emissiveTextureSamplerName = "defaultSampler";
	float emissiveFactor = 1.0f;
	float alphaCutoff = 0.0f;
	bool useTriplanarMapping = false;
	nml::vec2 scaleUV = nml::vec2(1.0f, 1.0f);
	nml::vec2 offsetUV = nml::vec2(0.0f, 0.0f);
};

struct RendererPrimitive {
	RendererMesh mesh;
	RendererMaterial material;
};

struct RendererModel {
	std::vector<RendererPrimitive> primitives;
};