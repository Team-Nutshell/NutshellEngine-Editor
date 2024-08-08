#include "destroy_entities_command.h"

DestroyEntitiesCommand::DestroyEntitiesCommand(GlobalInfo& globalInfo, const std::vector<EntityID>& entityIDs) : m_globalInfo(globalInfo) {
	if (entityIDs.size() == 1) {
		setText("Destroy Entity " + QString::fromStdString(m_globalInfo.entities[entityIDs[0]].name));
	}
	else {
		setText("Destroy multiple Entities");
	}
	for (EntityID entityID : entityIDs) {
		m_destroyedEntities.push_back(m_globalInfo.entities[entityID]);
	}
}

void DestroyEntitiesCommand::undo() {
	for (const Entity& destroyEntity : m_destroyedEntities) {
		m_globalInfo.entities[destroyEntity.entityID] = destroyEntity;
		emit m_globalInfo.signalEmitter.createEntitySignal(destroyEntity.entityID);
	}
}

void DestroyEntitiesCommand::redo() {
	for (const Entity& destroyEntity : m_destroyedEntities) {
		m_globalInfo.entities.erase(destroyEntity.entityID);
		emit m_globalInfo.signalEmitter.destroyEntitySignal(destroyEntity.entityID);
	}
}