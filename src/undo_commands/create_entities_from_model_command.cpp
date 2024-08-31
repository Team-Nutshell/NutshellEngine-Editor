#include "create_entities_from_model_command.h"

CreateEntitiesFromModelCommand::CreateEntitiesFromModelCommand(GlobalInfo& globalInfo, const std::string& name, const std::string& modelPath) : m_globalInfo(globalInfo), m_modelPath(modelPath) {
	setText("Create Entities from model \"" + QString::fromStdString(modelPath) + "\"");
	if (m_globalInfo.findEntityByName(name) == NO_ENTITY) {
		m_baseEntityName = name;
	}
	else {
		uint32_t entityNameIndex = 0;
		while (m_globalInfo.findEntityByName(name + "_" + std::to_string(entityNameIndex)) != NO_ENTITY) {
			entityNameIndex++;
		}
		m_baseEntityName = name + "_" + std::to_string(entityNameIndex);
	}
}

void CreateEntitiesFromModelCommand::undo() {
	for (EntityID& entityID : m_entityIDs) {
		m_globalInfo.entities.erase(entityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(entityID);
	}
	m_globalInfo.clearSelectedEntities();
}

void CreateEntitiesFromModelCommand::redo() {
	RendererResourceManager::Model& model = m_globalInfo.rendererResourceManager.models[m_modelPath];
	for (size_t i = 0; i < model.primitives.size(); i++) {
		const RendererResourceManager::ModelPrimitive& primitive = model.primitives[i];
		std::string entityName;
		if (primitive.name.empty()) {
			if (m_globalInfo.findEntityByName(m_baseEntityName + "_" + std::to_string(i)) == NO_ENTITY) {
				entityName = m_baseEntityName + "_" + std::to_string(i);
			}
			else {
				uint32_t entityNameIndex = 0;
				while (m_globalInfo.findEntityByName(m_baseEntityName + "_" + std::to_string(entityNameIndex) + "_" + std::to_string(i)) != NO_ENTITY) {
					entityNameIndex++;
				}
				entityName = m_baseEntityName + "_" + std::to_string(entityNameIndex) + "_" + std::to_string(i);
			}
		}
		else {
			if (m_globalInfo.findEntityByName(m_baseEntityName + "_" + primitive.name) == NO_ENTITY) {
				entityName = m_baseEntityName + "_" + primitive.name;
			}
			else {
				uint32_t entityNameIndex = 0;
				while (m_globalInfo.findEntityByName(m_baseEntityName + "_" + std::to_string(entityNameIndex) + "_" + primitive.name) != NO_ENTITY) {
					entityNameIndex++;
				}
				entityName = m_baseEntityName + "_" + std::to_string(entityNameIndex) + "_" + primitive.name;
			}
		}

		Entity newEntity;
		if (i >= m_entityIDs.size()) {
			newEntity.entityID = m_globalInfo.globalEntityID++;
			m_entityIDs.push_back(newEntity.entityID);
		}
		else {
			newEntity.entityID = m_entityIDs[i];
		}
		newEntity.name = entityName;
		m_globalInfo.entities[m_entityIDs[i]] = newEntity;
		emit m_globalInfo.signalEmitter.createEntitySignal(m_entityIDs[i]);

		nml::vec3 position;
		nml::quat rotation;
		nml::vec3 scale;
		nml::decomposeTransform(primitive.modelMatrix, position, rotation, scale);

		Transform& newEntityTransform = m_globalInfo.entities[m_entityIDs[i]].transform;
		newEntityTransform.position = position;
		newEntityTransform.rotation = nml::quatToEulerAngles(rotation);
		newEntityTransform.rotation.x = nml::toDeg(newEntityTransform.rotation.x);
		newEntityTransform.rotation.y = nml::toDeg(newEntityTransform.rotation.y);
		newEntityTransform.rotation.z = nml::toDeg(newEntityTransform.rotation.z);
		newEntityTransform.scale = scale;
		emit m_globalInfo.signalEmitter.changeEntityTransformSignal(m_entityIDs[i], newEntityTransform);

		Renderable newEntityRenderable;
		newEntityRenderable.modelPath = m_modelPath;
		newEntityRenderable.primitiveIndex = static_cast<uint32_t>(i);
		m_globalInfo.entities[m_entityIDs[i]].renderable = newEntityRenderable;
		emit m_globalInfo.signalEmitter.addEntityRenderableSignal(m_entityIDs[i]);
	}
}