#include "light_mesh.h"

void LightMesh::update(GlobalInfo& globalInfo, EntityID entityID) {
	const Entity& entity = globalInfo.entities[entityID];

	if (!entity.light) {
		return;
	}

	const Light& light = entity.light.value();

	RendererResourceManager::Model lightModel;

	if (light.type == "Directional") {
		RendererResourceManager::ModelPrimitive modelPrimitive;

		RendererResourceManager::Mesh::Vertex baseVertex;
		baseVertex.position = nml::vec3(nml::vec3(0.0f, 0.0f, 0.0f));
		modelPrimitive.mesh.vertices.push_back(baseVertex);

		RendererResourceManager::Mesh::Vertex tipVertex;
		tipVertex.position = nml::vec3(nml::normalize(light.direction));
		modelPrimitive.mesh.vertices.push_back(tipVertex);

		modelPrimitive.mesh.indices.push_back(0);
		modelPrimitive.mesh.indices.push_back(1);

		lightModel.primitives.push_back(modelPrimitive);
	}
	else if (light.type == "Point") {
		RendererResourceManager::ModelPrimitive modelPrimitive;

		const size_t nbLongLat = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);
		const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongLat));

		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			for (float phi = 0.0f; phi < nml::PI; phi += phiStep) {
				if ((phi + phiStep) >= nml::PI) {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = nml::vec3(0.0f, -light.distance, 0.0f);
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
				else {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = (nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * light.distance);
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
			}
		}

		for (size_t i = 1; i < modelPrimitive.mesh.vertices.size(); i++) {
			if (i % (nbLongLat / 2 + 1) != 0) {
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i) - 1);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));
			}
		}

		lightModel.primitives.push_back(modelPrimitive);
	}
	else if (light.type == "Spot") {
		RendererResourceManager::ModelPrimitive modelPrimitive;

		RendererResourceManager::Mesh::Vertex baseVertex;
		baseVertex.position = nml::vec3(nml::vec3(0.0f, 0.0f, 0.0f));
		modelPrimitive.mesh.vertices.push_back(baseVertex);

		const nml::vec3 normalizedDirection = nml::normalize(light.direction);

		RendererResourceManager::Mesh::Vertex tipVertex;
		tipVertex.position = nml::vec3(normalizedDirection * light.distance);
		modelPrimitive.mesh.vertices.push_back(tipVertex);

		modelPrimitive.mesh.indices.push_back(0);
		modelPrimitive.mesh.indices.push_back(1);

		const size_t nbLongLat = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);

		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			RendererResourceManager::Mesh::Vertex vertex;
			vertex.position = (nml::vec3(std::cos(theta) * std::sin(nml::toRad(light.cutoff.x)), 0.0f, std::sin(theta) * std::sin(nml::toRad(light.cutoff.x))));
			modelPrimitive.mesh.vertices.push_back(vertex);
		}

		for (size_t i = 3; i < modelPrimitive.mesh.vertices.size(); i++) {
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i) - 1);
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));
		}
		uint32_t innerCircleLastIndex = static_cast<uint32_t>(modelPrimitive.mesh.vertices.size()) - 1;
		modelPrimitive.mesh.indices.push_back(innerCircleLastIndex);
		modelPrimitive.mesh.indices.push_back(3);

		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			RendererResourceManager::Mesh::Vertex vertex;
			vertex.position = (nml::vec3(std::cos(theta) * std::sin(nml::toRad(light.cutoff.y)), 0.0f, std::sin(theta) * std::sin(nml::toRad(light.cutoff.y))));
			modelPrimitive.mesh.vertices.push_back(vertex);
		}

		for (size_t i = innerCircleLastIndex + 2; i < modelPrimitive.mesh.vertices.size(); i++) {
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i) - 1);
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));
		}
		uint32_t outerCircleLastIndex = static_cast<uint32_t>(modelPrimitive.mesh.vertices.size()) - 1;
		modelPrimitive.mesh.indices.push_back(outerCircleLastIndex);
		modelPrimitive.mesh.indices.push_back(innerCircleLastIndex + 1);

		lightModel.primitives.push_back(modelPrimitive);
	}
	else if (light.type == "Ambient") {
		return;
	}

	const std::string modelName = "Light_" + std::to_string(entityID);

	globalInfo.rendererResourceManager.models[modelName] = lightModel;
	globalInfo.rendererResourceManager.modelsToLoad.push_back(modelName);
}
