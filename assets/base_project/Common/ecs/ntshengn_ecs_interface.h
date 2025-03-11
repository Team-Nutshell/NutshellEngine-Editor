#pragma once
#include "../utils/ntshengn_defines.h"
#include "../utils/ntshengn_utils_bimap.h"
#include "components/ntshengn_ecs_transform.h"
#include "components/ntshengn_ecs_renderable.h"
#include "components/ntshengn_ecs_camera.h"
#include "components/ntshengn_ecs_light.h"
#include "components/ntshengn_ecs_rigidbody.h"
#include "components/ntshengn_ecs_collidable.h"
#include "components/ntshengn_ecs_sound_listener.h"
#include "components/ntshengn_ecs_scriptable.h"
#include <stdexcept>
#include <bitset>
#include <deque>
#include <array>
#include <unordered_map>
#include <memory>
#include <set>
#include <string>
#include <algorithm>

#define NTSHENGN_MAX_ENTITIES 4096
#define NTSHENGN_MAX_COMPONENTS 32

namespace NtshEngn {

	typedef uint32_t Entity;
	#define NTSHENGN_ENTITY_UNKNOWN 0xFFFFFFFF

	typedef uint8_t Component;
	typedef std::bitset<NTSHENGN_MAX_COMPONENTS> ComponentMask;

	class EntityManager {
	public:
		EntityManager() {
			for (Entity entity = 0; entity < NTSHENGN_MAX_ENTITIES; entity++) {
				m_availableEntities.push_back(entity);
			}
		}

		Entity createEntity() {
			NTSHENGN_ASSERT(m_numberOfEntities < NTSHENGN_MAX_ENTITIES, "Too many Entities.");

			Entity id = m_availableEntities.front();
			m_availableEntities.pop_front();
			m_numberOfEntities++;

			m_existingEntities.insert(id);

			return id;
		}

		Entity createEntity(const std::string& name) {
			NTSHENGN_ASSERT(!m_entityNames.exist(name), "An Entity named \"" + name + "\" already exists.");

			Entity id = createEntity();
			m_entityNames.insert_or_assign(id, name);

			return id;
		}

		void destroyEntity(Entity entity) {
			NTSHENGN_ASSERT(m_existingEntities.find(entity) != m_existingEntities.end(), "Entity " + std::to_string(entity) + " does not exist.");

			m_componentMasks[entity].reset();
			m_availableEntities.push_front(entity);
			m_numberOfEntities--;

			m_existingEntities.erase(entity);

			if (m_entityNames.exist(entity)) {
				m_entityNames.erase(entity);
			}

			if (m_persistentEntities.find(entity) != m_persistentEntities.end()) {
				m_persistentEntities.erase(entity);
			}

			if (m_entityGroupsOfEntities.find(entity) != m_entityGroupsOfEntities.end()) {
				const std::set<std::string>& entityGroupNames = m_entityGroupsOfEntities[entity];
				for (const std::string& entityGroupName : entityGroupNames) {
					m_entityGroups[entityGroupName].erase(entity);

					if (m_entityGroups[entityGroupName].empty()) {
						m_entityGroups.erase(entityGroupName);
					}
				}

				m_entityGroupsOfEntities.erase(entity);
			}
		}

		void setComponents(Entity entity, ComponentMask componentMask) {
			NTSHENGN_ASSERT(entity < NTSHENGN_MAX_ENTITIES, "Entity " + std::to_string(entity) + " does not exist.");

			m_componentMasks[entity] = componentMask;
		}

		ComponentMask getComponents(Entity entity) {
			NTSHENGN_ASSERT(entity < NTSHENGN_MAX_ENTITIES, "Entity " + std::to_string(entity) + " does not exist.");

			return m_componentMasks[entity];
		}

		bool entityExists(Entity entity) {
			return m_existingEntities.find(entity) != m_existingEntities.end();
		}

		const std::set<Entity>& getExistingEntities() {
			return m_existingEntities;
		}

		void setEntityName(Entity entity, const std::string& name) {
			NTSHENGN_ASSERT(!m_entityNames.exist(name), "An Entity named \"" + name + "\" already exists.");

			m_entityNames.insert_or_assign(entity, name);
		}

		bool entityHasName(Entity entity) {
			return m_entityNames.exist(entity);
		}

		std::string getEntityName(Entity entity) {
			if (m_entityNames.exist(entity)) {
				return m_entityNames[entity];
			}
			else {
				return "";
			}
		}

		Entity findEntityByName(const std::string& name) {
			if (m_entityNames.exist(name)) {
				return m_entityNames[name];
			}
			else {
				return NTSHENGN_ENTITY_UNKNOWN;
			}
		}

		void setEntityPersistence(Entity entity, bool persistent) {
			if (persistent) {
				m_persistentEntities.insert(entity);
			}
			else {
				if (m_persistentEntities.find(entity) != m_persistentEntities.end()) {
					m_persistentEntities.erase(entity);
				}
			}
		}

		bool isEntityPersistent(Entity entity) {
			return m_persistentEntities.find(entity) != m_persistentEntities.end();
		}

		const std::set<Entity>& getPersistentEntities() {
			return m_persistentEntities;
		}

		void addEntityToEntityGroup(Entity entity, const std::string& entityGroupName) {
			m_entityGroups[entityGroupName].insert(entity);
			m_entityGroupsOfEntities[entity].insert(entityGroupName);
		}

		void removeEntityFromEntityGroup(Entity entity, const std::string& entityGroupName) {
			if (m_entityGroups.find(entityGroupName) != m_entityGroups.end()) {
				m_entityGroups[entityGroupName].erase(entity);

				if (m_entityGroups[entityGroupName].empty()) {
					m_entityGroups.erase(entityGroupName);
				}
			}

			if (m_entityGroupsOfEntities.find(entity) != m_entityGroupsOfEntities.end()) {
				m_entityGroupsOfEntities[entity].erase(entityGroupName);
			}
		}

		bool entityGroupExists(const std::string& entityGroupName) {
			return m_entityGroups.find(entityGroupName) != m_entityGroups.end();
		}

		bool isEntityInEntityGroup(Entity entity, const std::string& entityGroupName) {
			if (m_entityGroupsOfEntities.find(entity) != m_entityGroupsOfEntities.end()) {
				return m_entityGroupsOfEntities[entity].find(entityGroupName) != m_entityGroupsOfEntities[entity].end();
			}

			return false;
		}

		std::set<Entity> getEntitiesInEntityGroup(const std::string& entityGroupName) {
			if (m_entityGroups.find(entityGroupName) != m_entityGroups.end()) {
				return m_entityGroups[entityGroupName];
			}

			return std::set<Entity>();
		}

		std::set<std::string> getEntityGroupsOfEntity(Entity entity) {
			if (m_entityGroupsOfEntities.find(entity) != m_entityGroupsOfEntities.end()) {
				return m_entityGroupsOfEntities[entity];
			}

			return std::set<std::string>();
		}

	private:
		std::deque<Entity> m_availableEntities;
		std::set<Entity> m_existingEntities;
		std::array<ComponentMask, NTSHENGN_MAX_ENTITIES> m_componentMasks;
		Bimap<Entity, std::string> m_entityNames;
		std::set<Entity> m_persistentEntities;
		std::unordered_map<std::string, std::set<Entity>> m_entityGroups;
		std::unordered_map<Entity, std::set<std::string>> m_entityGroupsOfEntities;
		uint32_t m_numberOfEntities = 0;
	};

	class ComponentArrayInterface {
	public:
		virtual ~ComponentArrayInterface() = default;
		virtual void entityDestroyed(Entity entity) = 0;
	};

	template <typename T>
	class ComponentArray : public ComponentArrayInterface {
	public:
		void insertData(Entity entity, T component) {
			NTSHENGN_ASSERT(m_entityToIndex.find(entity) == m_entityToIndex.end(), "Entity " + std::to_string(entity) + " already has this component.");

			m_entityToIndex[entity] = m_validSize;
			m_indexToEntity[m_validSize] = entity;
			m_components[m_validSize] = component;
			m_validSize++;
		}

		void removeData(Entity entity) {
			NTSHENGN_ASSERT(m_entityToIndex.find(entity) != m_entityToIndex.end(), "Entity " + std::to_string(entity) + " does not have this component.");

			size_t tmp = m_entityToIndex[entity];
			m_components[tmp] = m_components[m_validSize - 1];
			Entity entityLast = m_indexToEntity[m_validSize - 1];
			m_entityToIndex[entityLast] = tmp;
			m_indexToEntity[tmp] = entityLast;
			m_entityToIndex.erase(entity);
			m_indexToEntity.erase(m_validSize - 1);
			m_validSize--;
		}

		bool hasComponent(Entity entity) {
			return m_entityToIndex.find(entity) != m_entityToIndex.end();
		}

		T& getData(Entity entity) {
			NTSHENGN_ASSERT(m_entityToIndex.find(entity) != m_entityToIndex.end(), "Entity " + std::to_string(entity) + " does not have this component.");

			return m_components[m_entityToIndex[entity]];
		}

		void entityDestroyed(Entity entity) override {
			if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
				removeData(entity);
			}
		}

	private:
		std::array<T, NTSHENGN_MAX_ENTITIES> m_components;
		std::unordered_map<Entity, size_t> m_entityToIndex;
		std::unordered_map<size_t, Entity> m_indexToEntity;
		size_t m_validSize;
	};

	class ComponentManager {
	public:
		template <typename T>
		void registerComponent() {
			std::string typeName = std::string(typeid(T).name());

			NTSHENGN_ASSERT(m_componentTypes.find(typeName) == m_componentTypes.end(), "Component is already registered.");

			m_componentTypes.insert({ typeName, m_nextComponent });
			m_componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
			m_nextComponent++;
		}

		template <typename T>
		Component getComponentID() {
			std::string typeName = std::string(typeid(T).name());

			NTSHENGN_ASSERT(m_componentTypes.find(typeName) != m_componentTypes.end(), "Component is not registered.");

			return m_componentTypes[typeName];
		}

		template <typename T>
		void addComponent(Entity entity, T component) {
			getComponentArray<T>()->insertData(entity, component);
		}

		template <typename T>
		void removeComponent(Entity entity) {
			getComponentArray<T>()->removeData(entity);
		}

		template <typename T>
		bool hasComponent(Entity entity) {
			return getComponentArray<T>()->hasComponent(entity);
		}

		template <typename T>
		T& getComponent(Entity entity) {
			return getComponentArray<T>()->getData(entity);
		}

		void entityDestroyed(Entity entity) {
			for (const auto& pair : m_componentArrays) {
				const std::shared_ptr<ComponentArrayInterface>& componentArray = pair.second;
				componentArray->entityDestroyed(entity);
			}
		}

	private:
		std::unordered_map<std::string, Component> m_componentTypes;
		std::unordered_map<std::string, std::shared_ptr<ComponentArrayInterface>> m_componentArrays;
		Component m_nextComponent = 0;

		template <typename T>
		std::shared_ptr<ComponentArray<T>> getComponentArray() {
			std::string typeName = std::string(typeid(T).name());

			NTSHENGN_ASSERT(m_componentTypes.find(typeName) != m_componentTypes.end(), "Component does not exist.");

			return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[typeName]);
		}
	};

	class System {
	public:
		virtual void onEntityComponentAdded(Entity entity, Component componentID) { NTSHENGN_UNUSED(entity); NTSHENGN_UNUSED(componentID); }
		virtual void onEntityComponentRemoved(Entity entity, Component componentID) { NTSHENGN_UNUSED(entity); NTSHENGN_UNUSED(componentID); }
		
	public:
		std::set<Entity> entities;
	};

	class SystemManager {
	public:
		template <typename T>
		void registerSystem(System* system) {
			std::string typeName = std::string(typeid(T).name());

			NTSHENGN_ASSERT(m_systems.find(typeName) == m_systems.end(), "System is already registered.");

			m_systems.insert({ typeName, system });
		}

		template <typename T>
		void setComponents(ComponentMask componentMask) {
			std::string typeName = std::string(typeid(T).name());

			NTSHENGN_ASSERT(m_systems.find(typeName) != m_systems.end(), "System does not exist.");

			m_componentMasks.insert({ typeName, componentMask });
		}

		void entityDestroyed(Entity entity, ComponentMask entityComponents) {
			for (const auto& pair : m_systems) {
				const std::string& type = pair.first;
				System* system = pair.second;
				const ComponentMask systemComponentMask = m_componentMasks[type];
				const ComponentMask entityAndSystemComponentMask = entityComponents & systemComponentMask;

				bool entityInSystem = false;
				for (uint8_t i = 0; i < NTSHENGN_MAX_COMPONENTS; i++) {
					if (entityAndSystemComponentMask[i]) {
						entityInSystem = true;
						system->onEntityComponentRemoved(entity, i);
					}
				}

				if (entityInSystem) {
					system->entities.erase(entity);
				}
			}
		}

		void entityComponentMaskChanged(Entity entity, ComponentMask oldEntityComponentMask, ComponentMask newEntityComponentMask, Component componentID) {
			for (const auto& pair : m_systems) {
				const std::string& type = pair.first;
				System* system = pair.second;
				const ComponentMask systemComponentMask = m_componentMasks[type];
				const ComponentMask oldAndSystemComponentMasks = oldEntityComponentMask & systemComponentMask;
				const ComponentMask newAndSystemComponentMasks = newEntityComponentMask & systemComponentMask;
				if (oldAndSystemComponentMasks != newAndSystemComponentMasks) { // A Component used in the system has been added or removed
					if (newAndSystemComponentMasks.to_ulong() > oldAndSystemComponentMasks.to_ulong()) { // A Component has been added
						system->onEntityComponentAdded(entity, componentID);
						if (oldAndSystemComponentMasks.none()) { // The entity is new in the system
							system->entities.insert(entity);
						}
					}
					else if (newAndSystemComponentMasks.to_ulong() < oldAndSystemComponentMasks.to_ulong()) { // A Component has been removed
						system->onEntityComponentRemoved(entity, componentID);
						if (newAndSystemComponentMasks.none()) { // The entity has no more component for the system
							system->entities.erase(entity);
						}
					}
				}
			}
		}

	private:
		std::unordered_map<std::string, ComponentMask> m_componentMasks;
		std::unordered_map<std::string, System*> m_systems;
	};

	class ECSInterface {
	public:
		virtual void init() = 0;

		// Entity
		virtual Entity createEntity() = 0;
		virtual Entity createEntity(const std::string& name) = 0;

		virtual void destroyEntity(Entity entity) = 0;
		virtual void destroyAllEntities() = 0;
		virtual void destroyNonPersistentEntities() = 0;

		virtual bool entityExists(Entity entity) = 0;

		virtual void setEntityName(Entity entity, const std::string& name) = 0;
		virtual bool entityHasName(Entity entity) = 0;
		virtual std::string getEntityName(Entity entity) = 0;
		virtual Entity findEntityByName(const std::string& name) = 0;

		virtual void setEntityPersistence(Entity entity, bool persistent) = 0;
		virtual bool isEntityPersistent(Entity entity) = 0;

		virtual void addEntityToEntityGroup(Entity entity, const std::string& entityGroupName) = 0;
		virtual void removeEntityFromEntityGroup(Entity entity, const std::string& entityGroupName) = 0;

		virtual bool entityGroupExists(const std::string& entityGroupName) = 0;

		virtual bool isEntityInEntityGroup(Entity entity, const std::string& entityGroupName) = 0;
		virtual std::set<Entity> getEntitiesInEntityGroup(const std::string& entityGroupName) = 0;
		virtual std::set<std::string> getEntityGroupsOfEntity(Entity entity) = 0;

		// Component
		template <typename T>
		void registerComponent() {
			return m_componentManager->registerComponent<T>();
		}

		template <typename T>
		void addComponent(Entity entity, T component) {
			m_componentManager->addComponent<T>(entity, component);
			ComponentMask oldComponents = m_entityManager->getComponents(entity);
			ComponentMask newComponents = oldComponents;
			Component componentID = m_componentManager->getComponentID<T>();
			newComponents.set(componentID, true);
			m_entityManager->setComponents(entity, newComponents);
			m_systemManager->entityComponentMaskChanged(entity, oldComponents, newComponents, componentID);
		}

		template <typename T>
		void removeComponent(Entity entity) {
			ComponentMask oldComponents = m_entityManager->getComponents(entity);
			ComponentMask newComponents = oldComponents;
			Component componentID = m_componentManager->getComponentID<T>();
			newComponents.set(componentID, false);
			m_entityManager->setComponents(entity, newComponents);
			m_systemManager->entityComponentMaskChanged(entity, oldComponents, newComponents, componentID);
			m_componentManager->removeComponent<T>(entity);
		}

		template <typename T>
		bool hasComponent(Entity entity) {
			return m_componentManager->hasComponent<T>(entity);
		}

		template <typename T>
		T& getComponent(Entity entity) {
			return m_componentManager->getComponent<T>(entity);
		}

		template <typename T>
		Component getComponentID() {
			return m_componentManager->getComponentID<T>();
		}

		// System
		template <typename T>
		void registerSystem(System* system) {
			m_systemManager->registerSystem<T>(system);
		}

		template <typename T>
		void setSystemComponents(ComponentMask componentMask) {
			m_systemManager->setComponents<T>(componentMask);
		}

	protected:
		std::unique_ptr<EntityManager> m_entityManager;
		std::unique_ptr<ComponentManager> m_componentManager;
		std::unique_ptr<SystemManager> m_systemManager;
	};

}