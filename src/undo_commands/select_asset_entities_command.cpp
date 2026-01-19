#include "select_asset_entities_command.h"
#include "../widgets/main_window.h"
#include <QSignalBlocker>
#include <algorithm>
#include <filesystem>

SelectAssetEntitiesCommand::SelectAssetEntitiesCommand(GlobalInfo& globalInfo, SelectionType newType, const std::string& newSelectedAssetPath, EntityID newCurrentEntityID, const std::set<EntityID>& newOtherSelectedEntityIDs) : m_globalInfo(globalInfo) {
	m_oldType = m_globalInfo.lastSelectionType;
	m_newType = newType;

	if (m_oldType == SelectionType::Asset) {
		m_oldSelectedAssetPath = m_globalInfo.mainWindow->infoPanel->assetInfoPanel->assetInfoScrollArea->assetInfoList->currentAssetPath;
	}
	else if (m_oldType == SelectionType::Entities) {
		m_oldCurrentEntityID = m_globalInfo.currentEntityID;
		m_oldOtherSelectedEntityIDs = m_globalInfo.otherSelectedEntityIDs;
	}

	if (m_newType == SelectionType::Asset) {
		m_newSelectedAssetPath = newSelectedAssetPath;
	}
	else if (m_newType == SelectionType::Entities) {
		m_newCurrentEntityID = newCurrentEntityID;
		m_newOtherSelectedEntityIDs = newOtherSelectedEntityIDs;
	}
}

void SelectAssetEntitiesCommand::undo() {
	if (m_oldType == SelectionType::Asset) {
		if (std::filesystem::exists(m_oldSelectedAssetPath)) {
			emit m_globalInfo.signalEmitter.selectAssetSignal(m_oldSelectedAssetPath);
		}
		else {
			emit m_globalInfo.signalEmitter.selectAssetSignal("");
		}

		{
			const QSignalBlocker signalBlocker(m_globalInfo.mainWindow->entityPanel->entityList);
			m_globalInfo.mainWindow->entityPanel->entityList->clearSelection();
		}
	}
	else if (m_oldType == SelectionType::Entities) {
		m_globalInfo.currentEntityID = NO_ENTITY;
		m_globalInfo.otherSelectedEntityIDs.clear();

		if (m_globalInfo.entities.find(m_oldCurrentEntityID) != m_globalInfo.entities.end()) {
			m_globalInfo.currentEntityID = m_oldCurrentEntityID;
		}

		for (EntityID otherSelectedEntityID : m_oldOtherSelectedEntityIDs) {
			if (m_globalInfo.entities.find(otherSelectedEntityID) != m_globalInfo.entities.end()) {
				m_globalInfo.otherSelectedEntityIDs.insert(otherSelectedEntityID);
			}
		}

		emit m_globalInfo.signalEmitter.selectEntitySignal();

		{
			const QSignalBlocker signalBlocker(m_globalInfo.mainWindow->resourceSplitter->assetPanel->assetList);
			m_globalInfo.mainWindow->resourceSplitter->assetPanel->assetList->clearSelection();
		}
	}

	m_globalInfo.lastSelectionType = m_oldType;
}

void SelectAssetEntitiesCommand::redo() {
	if (m_newType == SelectionType::Asset) {
		if (std::filesystem::exists(m_newSelectedAssetPath)) {
			emit m_globalInfo.signalEmitter.selectAssetSignal(m_newSelectedAssetPath);
		}
		else {
			emit m_globalInfo.signalEmitter.selectAssetSignal("");
		}

		{
			const QSignalBlocker signalBlocker(m_globalInfo.mainWindow->entityPanel->entityList);
			m_globalInfo.mainWindow->entityPanel->entityList->clearSelection();
		}
	}
	else if (m_newType == SelectionType::Entities) {
		m_globalInfo.currentEntityID = NO_ENTITY;
		m_globalInfo.otherSelectedEntityIDs.clear();

		if (m_globalInfo.entities.find(m_newCurrentEntityID) != m_globalInfo.entities.end()) {
			m_globalInfo.currentEntityID = m_newCurrentEntityID;
		}

		for (EntityID otherSelectedEntityID : m_newOtherSelectedEntityIDs) {
			if (m_globalInfo.entities.find(otherSelectedEntityID) != m_globalInfo.entities.end()) {
				m_globalInfo.otherSelectedEntityIDs.insert(otherSelectedEntityID);
			}
		}

		emit m_globalInfo.signalEmitter.selectEntitySignal();

		{
			const QSignalBlocker signalBlocker(m_globalInfo.mainWindow->resourceSplitter->assetPanel->assetList);
			m_globalInfo.mainWindow->resourceSplitter->assetPanel->assetList->clearSelection();
		}
	}

	m_globalInfo.lastSelectionType = m_newType;
}
