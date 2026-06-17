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

		const size_t nbLatitudes = 5;
		const size_t nbLongitudes = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLatitudes);
		const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongitudes));

		for (float theta = 0.0f; theta < (2.0f * nml::PI); theta += thetaStep) {
			for (float phi = 0.0f; phi < nml::PI; phi += phiStep) {
				if ((phi + phiStep) >= nml::PI) {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = nml::vec3(0.0f, -light.distance, 0.0f);
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
				else {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * light.distance;
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
			}
		}

		for (size_t i = 1; i < modelPrimitive.mesh.vertices.size(); i++) {
			if (i % (nbLongitudes / 2 + 1) != 0) {
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
		nml::vec3 orthogonalVector = nml::vec3(0.0f, 0.0f, 0.0f);
		uint8_t nonZeroIndex = 3;
		for (uint8_t i = 0; i < 3; i++) {
			if (normalizedDirection[i] != 0.0f) {
				nonZeroIndex = i;
				break;
			}
		}
		if (nonZeroIndex == 3) { // Direction is (0, 0, 0)
			return;
		}
		uint8_t secondIndex = (nonZeroIndex + 1) % 3;
		orthogonalVector[secondIndex] = normalizedDirection[nonZeroIndex];
		orthogonalVector[nonZeroIndex] = -normalizedDirection[secondIndex];
		orthogonalVector = nml::normalize(orthogonalVector);
		nml::vec3 baseLastVector = nml::cross(normalizedDirection, orthogonalVector);

		nml::vec3 tip = nml::vec3(normalizedDirection * light.distance);

		const size_t nbSections = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbSections);

		for (float theta = 0.0f; theta < (2.0f * nml::PI); theta += thetaStep) {
			RendererResourceManager::Mesh::Vertex vertex;
			vertex.position = (((orthogonalVector * std::sin(theta)) + (baseLastVector * std::cos(theta))) * (nml::toRad(light.cutoff.x) * light.distance)) + tip;
			modelPrimitive.mesh.vertices.push_back(vertex);
		}

		for (size_t i = 2; i < modelPrimitive.mesh.vertices.size(); i++) {
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i) - 1);
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));

			if ((i % 8) == 0) {
				modelPrimitive.mesh.indices.push_back(0);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));
			}
		}
		uint32_t innerCircleLastIndex = static_cast<uint32_t>(modelPrimitive.mesh.vertices.size()) - 1;
		modelPrimitive.mesh.indices.push_back(innerCircleLastIndex);
		modelPrimitive.mesh.indices.push_back(2);

		for (float theta = 0.0f; theta < (2.0f * nml::PI); theta += thetaStep) {
			RendererResourceManager::Mesh::Vertex vertex;
			vertex.position = (((orthogonalVector * std::sin(theta)) + (baseLastVector * std::cos(theta))) * (nml::toRad(light.cutoff.y) * light.distance)) + tip;
			modelPrimitive.mesh.vertices.push_back(vertex);
		}

		for (size_t i = innerCircleLastIndex + 2; i < modelPrimitive.mesh.vertices.size(); i++) {
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i) - 1);
			modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));

			if ((i % 8) == 0) {
				modelPrimitive.mesh.indices.push_back(0);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(i));
			}
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
