#include "collider_mesh.h"

void ColliderMesh::update(GlobalInfo& globalInfo, EntityID entityID) {
	const Entity& entity = globalInfo.entities[entityID];

	if (!entity.collidable) {
		return;
	}

	const Collidable& collidable = entity.collidable.value();

	RendererResourceManager::Model colliderModel;

	if (collidable.type == "Box") {
		OBB obb;
		obb.center = collidable.center;
		obb.halfExtent = collidable.halfExtent;
		obb.rotation = collidable.rotation;

		RendererResourceManager::ModelPrimitive modelPrimitive;

		const nml::mat4 boxRotation = nml::rotate(obb.rotation.x, nml::vec3(1.0f, 0.0f, 0.0f)) *
			nml::rotate(obb.rotation.y, nml::vec3(0.0f, 1.0f, 0.0f)) *
			nml::rotate(obb.rotation.z, nml::vec3(0.0f, 0.0f, 1.0f));

		modelPrimitive.mesh.vertices.resize(8);
		modelPrimitive.mesh.vertices[0].position = obb.center + nml::vec3(boxRotation * nml::vec4(-obb.halfExtent.x, -obb.halfExtent.y, -obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[1].position = obb.center + nml::vec3(boxRotation * nml::vec4(obb.halfExtent.x, -obb.halfExtent.y, -obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[2].position = obb.center + nml::vec3(boxRotation * nml::vec4(obb.halfExtent.x, -obb.halfExtent.y, obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[3].position = obb.center + nml::vec3(boxRotation * nml::vec4(-obb.halfExtent.x, -obb.halfExtent.y, obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[4].position = obb.center + nml::vec3(boxRotation * nml::vec4(-obb.halfExtent.x, obb.halfExtent.y, -obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[5].position = obb.center + nml::vec3(boxRotation * nml::vec4(obb.halfExtent.x, obb.halfExtent.y, -obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[6].position = obb.center + nml::vec3(boxRotation * nml::vec4(obb.halfExtent.x, obb.halfExtent.y, obb.halfExtent.z, 1.0f));
		modelPrimitive.mesh.vertices[7].position = obb.center + nml::vec3(boxRotation * nml::vec4(-obb.halfExtent.x, obb.halfExtent.y, obb.halfExtent.z, 1.0f));

		modelPrimitive.mesh.indices = {
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 4,
			1, 5,
			2, 6,
			3, 7
		};

		colliderModel.primitives.push_back(modelPrimitive);
	}
	else if (collidable.type == "Sphere") {
		Sphere sphere;
		sphere.center = collidable.center;
		sphere.radius = collidable.radius;

		RendererResourceManager::ModelPrimitive modelPrimitive;

		const size_t nbLongLat = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);
		const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongLat));

		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			for (float phi = 0.0f; phi < nml::PI; phi += phiStep) {
				if ((phi + phiStep) >= nml::PI) {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = sphere.center + nml::vec3(0.0f, -sphere.radius, 0.0f);
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
				else {
					RendererResourceManager::Mesh::Vertex vertex;
					vertex.position = sphere.center + (nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * sphere.radius);
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
			}
		}

		for (size_t j = 1; j < modelPrimitive.mesh.vertices.size(); j++) {
			if (j % (nbLongLat / 2 + 1) != 0) {
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j) - 1);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j));
			}
		}

		colliderModel.primitives.push_back(modelPrimitive);
	}
	else if (collidable.type == "Capsule") {
		Capsule capsule;
		capsule.base = collidable.base;
		capsule.tip = collidable.tip;
		capsule.radius = collidable.radius;

		const size_t nbLongLat = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);
		const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongLat));

		RendererResourceManager::ModelPrimitive modelPrimitive;

		const nml::vec3 baseTipDifference = capsule.tip - capsule.base;
		const nml::vec3 facing = nml::vec3(0.0f, 1.0f, 0.0f);
		const nml::vec3 toB = nml::normalize(baseTipDifference);
		nml::mat4 rotation = nml::mat4::identity();
		if (facing == -toB) { // Flip
			rotation = nml::rotate(nml::toRad(180.0f), nml::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (facing != toB) { // Cross product of parallel vectors is undefined
			const nml::vec3 rotationAxis = nml::normalize(nml::cross(facing, toB));
			const float rotationAngle = std::acos(nml::dot(facing, toB));

			rotation = nml::rotate(rotationAngle, rotationAxis);
		}

		// Base
		std::vector<uint32_t> baseFinal;
		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			baseFinal.push_back(static_cast<uint32_t>(modelPrimitive.mesh.vertices.size()));
			for (float phi = nml::PI / 2.0f; phi < nml::PI; phi += phiStep) {
				if ((phi + phiStep) >= nml::PI) {
					RendererResourceManager::Mesh::Vertex vertex;
					nml::vec3 position = nml::vec3(0.0f, -capsule.radius, 0.0f);
					nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
					vertex.position = { transformedPosition.x + capsule.base.x,
						transformedPosition.y + capsule.base.y,
						transformedPosition.z + capsule.base.z };
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
				else {
					RendererResourceManager::Mesh::Vertex vertex;
					nml::vec3 position = nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * capsule.radius;
					nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
					vertex.position = { transformedPosition.x + capsule.base.x,
						transformedPosition.y + capsule.base.y,
						transformedPosition.z + capsule.base.z };
					modelPrimitive.mesh.vertices.push_back(vertex);
				}
			}
		}

		for (size_t j = 1; j < modelPrimitive.mesh.vertices.size(); j++) {
			if (j % (nbLongLat / 4 + 1) != 0) {
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j) - 1);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j));
			}
		}

		size_t baseVertexCount = modelPrimitive.mesh.vertices.size();

		// Tip
		std::vector<uint32_t> tipFinal;
		for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
			for (float phi = 0.0f; phi < nml::PI / 2.0f; phi += phiStep) {
				RendererResourceManager::Mesh::Vertex vertex;
				nml::vec3 position = nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * capsule.radius;
				nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
				vertex.position = { transformedPosition.x + capsule.tip.x,
					transformedPosition.y + capsule.tip.y,
					transformedPosition.z + capsule.tip.z };
				modelPrimitive.mesh.vertices.push_back(vertex);
			}
			tipFinal.push_back(static_cast<uint32_t>(modelPrimitive.mesh.vertices.size() - 1));
		}

		for (size_t j = baseVertexCount; j < modelPrimitive.mesh.vertices.size(); j++) {
			if (j % (nbLongLat / 4 + 1) != 0) {
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j) - 1);
				modelPrimitive.mesh.indices.push_back(static_cast<uint32_t>(j));
			}
		}

		// Connect base and tip
		for (size_t j = 0; j < baseFinal.size(); j++) {
			modelPrimitive.mesh.indices.push_back(baseFinal[j]);
			modelPrimitive.mesh.indices.push_back(tipFinal[j]);
		}

		colliderModel.primitives.push_back(modelPrimitive);
	}

	const std::string modelName = "Collider_" + std::to_string(entityID);

	globalInfo.rendererResourceManager.models[modelName] = colliderModel;
	globalInfo.rendererResourceManager.modelsToLoad.push_back(modelName);
}
