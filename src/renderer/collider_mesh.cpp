#include "collider_mesh.h"

void ColliderMesh::update(GlobalInfo& globalInfo, EntityID entityID) {
	const Entity& entity = globalInfo.entities[entityID];

	if (!entity.collidable) {
		return;
	}

	const Collidable& collidable = entity.collidable.value();

	RendererResourceManager::ModelToGPU modelToGPU;

	if (collidable.type == "Box") {
		std::vector<OBB> obbs;
		if (collidable.fromRenderable) {
			if (!entity.renderable) {
				return;
			}

			const Renderable& renderable = entity.renderable.value();
			if (globalInfo.rendererResourceManager.modelsToGPU.find(renderable.modelPath) != globalInfo.rendererResourceManager.modelsToGPU.end()) {
				// The renderable model has not been sent to GPU yet
				const RendererResourceManager::ModelToGPU& model = globalInfo.rendererResourceManager.modelsToGPU[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					obbs.push_back(mesh.obb);
				}
			}
			else {
				const RendererModel& model = globalInfo.rendererResourceManager.models[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					obbs.push_back(mesh.obb);
				}
			}
		}
		else {
			OBB obb;
			obb.center = collidable.center;
			obb.halfExtent = collidable.halfExtent;
			obb.rotation = collidable.rotation;

			obbs.push_back(obb);
		}

		for (size_t i = 0; i < obbs.size(); i++) {
			RendererResourceManager::MeshToGPU meshToGPU;

			const nml::mat4 boxRotation = nml::rotate(obbs[i].rotation.x, nml::vec3(1.0f, 0.0f, 0.0f)) *
				nml::rotate(obbs[i].rotation.y, nml::vec3(0.0f, 1.0f, 0.0f)) *
				nml::rotate(obbs[i].rotation.z, nml::vec3(0.0f, 0.0f, 1.0f));

			meshToGPU.vertices.resize(8);
			meshToGPU.vertices[0].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(-obbs[i].halfExtent.x, -obbs[i].halfExtent.y, -obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[1].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(obbs[i].halfExtent.x, -obbs[i].halfExtent.y, -obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[2].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(obbs[i].halfExtent.x, -obbs[i].halfExtent.y, obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[3].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(-obbs[i].halfExtent.x, -obbs[i].halfExtent.y, obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[4].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(-obbs[i].halfExtent.x, obbs[i].halfExtent.y, -obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[5].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(obbs[i].halfExtent.x, obbs[i].halfExtent.y, -obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[6].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(obbs[i].halfExtent.x, obbs[i].halfExtent.y, obbs[i].halfExtent.z, 1.0f));
			meshToGPU.vertices[7].position = obbs[i].center + nml::vec3(boxRotation * nml::vec4(-obbs[i].halfExtent.x, obbs[i].halfExtent.y, obbs[i].halfExtent.z, 1.0f));

			meshToGPU.indices = {
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

			modelToGPU.meshes.push_back(meshToGPU);
		}
	}
	else if (collidable.type == "Sphere") {
		std::vector<Sphere> spheres;
		if (collidable.fromRenderable) {
			if (!entity.renderable) {
				return;
			}

			const Renderable& renderable = entity.renderable.value();
			if (globalInfo.rendererResourceManager.modelsToGPU.find(renderable.modelPath) != globalInfo.rendererResourceManager.modelsToGPU.end()) {
				// The renderable model has not been sent to GPU yet
				const RendererResourceManager::ModelToGPU& model = globalInfo.rendererResourceManager.modelsToGPU[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					spheres.push_back(mesh.sphere);
				}
			}
			else {
				const RendererModel& model = globalInfo.rendererResourceManager.models[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					spheres.push_back(mesh.sphere);
				}
			}
		}
		else {
			Sphere sphere;
			sphere.center = collidable.center;
			sphere.radius = collidable.radius;

			spheres.push_back(sphere);
		}

		for (size_t i = 0; i < spheres.size(); i++) {
			RendererResourceManager::MeshToGPU meshToGPU;

			const size_t nbLongLat = 25;
			const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);
			const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongLat));

			for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
				for (float phi = 0.0f; phi < nml::PI; phi += phiStep) {
					if ((phi + phiStep) >= nml::PI) {
						RendererResourceManager::MeshToGPU::Vertex vertex;
						vertex.position = spheres[i].center + nml::vec3(0.0f, -spheres[i].radius, 0.0f);
						meshToGPU.vertices.push_back(vertex);
					}
					else {
						RendererResourceManager::MeshToGPU::Vertex vertex;
						vertex.position = spheres[i].center + (nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * spheres[i].radius);
						meshToGPU.vertices.push_back(vertex);
					}
				}
			}

			for (size_t j = 1; j < meshToGPU.vertices.size(); j++) {
				if (j % (nbLongLat / 2 + 1) != 0) {
					meshToGPU.indices.push_back(static_cast<uint32_t>(j) - 1);
					meshToGPU.indices.push_back(static_cast<uint32_t>(j));
				}
			}

			modelToGPU.meshes.push_back(meshToGPU);
		}
	}
	else if (collidable.type == "Capsule") {
		std::vector<Capsule> capsules;
		if (collidable.fromRenderable) {
			if (!entity.renderable) {
				return;
			}

			const Renderable& renderable = entity.renderable.value();
			if (globalInfo.rendererResourceManager.modelsToGPU.find(renderable.modelPath) != globalInfo.rendererResourceManager.modelsToGPU.end()) {
				// The renderable model has not been sent to GPU yet
				const RendererResourceManager::ModelToGPU& model = globalInfo.rendererResourceManager.modelsToGPU[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					capsules.push_back(mesh.capsule);
				}
			}
			else {
				const RendererModel& model = globalInfo.rendererResourceManager.models[renderable.modelPath];

				for (const auto& mesh : model.meshes) {
					capsules.push_back(mesh.capsule);
				}
			}
		}
		else {
			Capsule capsule;
			capsule.base = collidable.base;
			capsule.tip = collidable.tip;
			capsule.radius = collidable.radius;

			capsules.push_back(capsule);
		}

		const size_t nbLongLat = 25;
		const float thetaStep = nml::PI / static_cast<size_t>(nbLongLat);
		const float phiStep = 2.0f * (nml::PI / static_cast<size_t>(nbLongLat));

		for (size_t i = 0; i < capsules.size(); i++) {
			RendererResourceManager::MeshToGPU meshToGPU;

			const nml::vec3 baseTipDifference = capsules[i].tip - capsules[i].base;
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
				baseFinal.push_back(static_cast<uint32_t>(meshToGPU.vertices.size()));
				for (float phi = nml::PI / 2.0f; phi < nml::PI; phi += phiStep) {
					if ((phi + phiStep) >= nml::PI) {
						RendererResourceManager::MeshToGPU::Vertex vertex;
						nml::vec3 position = nml::vec3(0.0f, -capsules[i].radius, 0.0f);
						nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
						vertex.position = { transformedPosition.x + capsules[i].base.x,
							transformedPosition.y + capsules[i].base.y,
							transformedPosition.z + capsules[i].base.z };
						meshToGPU.vertices.push_back(vertex);
					}
					else {
						RendererResourceManager::MeshToGPU::Vertex vertex;
						nml::vec3 position = nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * capsules[i].radius;
						nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
						vertex.position = { transformedPosition.x + capsules[i].base.x,
							transformedPosition.y + capsules[i].base.y,
							transformedPosition.z + capsules[i].base.z };
						meshToGPU.vertices.push_back(vertex);
					}
				}
			}

			for (size_t j = 1; j < meshToGPU.vertices.size(); j++) {
				if (j % (nbLongLat / 4 + 1) != 0) {
					meshToGPU.indices.push_back(static_cast<uint32_t>(j) - 1);
					meshToGPU.indices.push_back(static_cast<uint32_t>(j));
				}
			}

			size_t baseVertexCount = meshToGPU.vertices.size();

			// Tip
			std::vector<uint32_t> tipFinal;
			for (float theta = 0.0f; theta < 2.0f * nml::PI; theta += thetaStep) {
				for (float phi = 0.0f; phi < nml::PI / 2.0f; phi += phiStep) {
					RendererResourceManager::MeshToGPU::Vertex vertex;
					nml::vec3 position = nml::vec3(std::cos(theta) * std::sin(phi), std::cos(phi), std::sin(theta) * std::sin(phi)) * capsules[i].radius;
					nml::vec3 transformedPosition = rotation * nml::vec4(position, 1.0f);
					vertex.position = { transformedPosition.x + capsules[i].tip.x,
						transformedPosition.y + capsules[i].tip.y,
						transformedPosition.z + capsules[i].tip.z };
					meshToGPU.vertices.push_back(vertex);
				}
				tipFinal.push_back(static_cast<uint32_t>(meshToGPU.vertices.size() - 1));
			}

			for (size_t j = baseVertexCount; j < meshToGPU.vertices.size(); j++) {
				if (j % (nbLongLat / 4 + 1) != 0) {
					meshToGPU.indices.push_back(static_cast<uint32_t>(j) - 1);
					meshToGPU.indices.push_back(static_cast<uint32_t>(j));
				}
			}

			// Connect base and tip
			for (size_t j = 0; j < baseFinal.size(); j++) {
				meshToGPU.indices.push_back(baseFinal[j]);
				meshToGPU.indices.push_back(tipFinal[j]);
			}

			modelToGPU.meshes.push_back(meshToGPU);
		}
	}

	globalInfo.rendererResourceManager.modelsToGPU["Collider_" + std::to_string(entityID)] = modelToGPU;
}
