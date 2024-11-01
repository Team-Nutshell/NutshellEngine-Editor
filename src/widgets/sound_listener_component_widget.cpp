#include "sound_listener_component_widget.h"
#include "component_title_widget.h"
#include "separator_line.h"
#include "main_window.h"
#include "../common/save_title_changer.h"
#include "../undo_commands/change_entities_component_command.h"
#include <QVBoxLayout>

SoundListenerComponentWidget::SoundListenerComponentWidget(GlobalInfo& globalInfo) : m_globalInfo(globalInfo) {
	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignTop);
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(new ComponentTitleWidget(m_globalInfo, m_globalInfo.localization.getString("component_sound_listener")));
	forwardWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_sound_listener_forward"));
	layout()->addWidget(forwardWidget);
	upWidget = new Vector3Widget(m_globalInfo, m_globalInfo.localization.getString("component_sound_listener_up"));
	layout()->addWidget(upWidget);
	layout()->addWidget(new SeparatorLine(m_globalInfo));

	connect(forwardWidget, &Vector3Widget::valueChanged, this, &SoundListenerComponentWidget::onVec3Changed);
	connect(upWidget, &Vector3Widget::valueChanged, this, &SoundListenerComponentWidget::onVec3Changed);
	connect(&globalInfo.signalEmitter, &SignalEmitter::selectEntitySignal, this, &SoundListenerComponentWidget::onEntitySelected);
	connect(&globalInfo.signalEmitter, &SignalEmitter::addEntitySoundListenerSignal, this, &SoundListenerComponentWidget::onEntitySoundListenerAdded);
	connect(&globalInfo.signalEmitter, &SignalEmitter::removeEntitySoundListenerSignal, this, &SoundListenerComponentWidget::onEntitySoundListenerRemoved);
	connect(&globalInfo.signalEmitter, &SignalEmitter::changeEntitySoundListenerSignal, this, &SoundListenerComponentWidget::onEntitySoundListenerChanged);
}

void SoundListenerComponentWidget::updateWidgets(const SoundListener& soundListener) {
	forwardWidget->setValue(soundListener.forward);
	upWidget->setValue(soundListener.up);
}

void SoundListenerComponentWidget::updateComponents(const std::vector<EntityID>& entityIDs, std::vector<SoundListener>& soundListeners) {
	std::vector<Component*> componentPointers;
	for (size_t i = 0; i < soundListeners.size(); i++) {
		componentPointers.push_back(&soundListeners[i]);
	}

	m_globalInfo.undoStack->push(new ChangeEntitiesComponentCommand(m_globalInfo, entityIDs, "SoundListener", componentPointers));
}

void SoundListenerComponentWidget::onEntitySelected() {
	if ((m_globalInfo.currentEntityID != NO_ENTITY) && m_globalInfo.entities[m_globalInfo.currentEntityID].soundListener.has_value()) {
		show();
		updateWidgets(m_globalInfo.entities[m_globalInfo.currentEntityID].soundListener.value());
	}
	else {
		hide();
	}
}

void SoundListenerComponentWidget::onEntitySoundListenerAdded(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		const SoundListener& soundListener = m_globalInfo.entities[m_globalInfo.currentEntityID].soundListener.value();
		updateWidgets(soundListener);
		show();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void SoundListenerComponentWidget::onEntitySoundListenerRemoved(EntityID entityID) {
	if (entityID == m_globalInfo.currentEntityID) {
		hide();
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void SoundListenerComponentWidget::onEntitySoundListenerChanged(EntityID entityID, const SoundListener& soundListener) {
	QObject* senderWidget = sender();
	if (senderWidget != this) {
		if (entityID == m_globalInfo.currentEntityID) {
			updateWidgets(soundListener);
		}
	}

	SaveTitleChanger::change(m_globalInfo.mainWindow);
}

void SoundListenerComponentWidget::onVec3Changed(const nml::vec3& value) {
	QObject* senderWidget = sender();

	std::vector<EntityID> entityIDs;
	std::vector<SoundListener> newSoundListeners;

	SoundListener newSoundListener = m_globalInfo.entities[m_globalInfo.currentEntityID].soundListener.value();

	uint8_t changedIndex = 255;
	if (senderWidget == forwardWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newSoundListener.forward[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newSoundListener.forward = value;
	}
	else if (senderWidget == upWidget) {
		for (uint8_t i = 0; i < 3; i++) {
			if (newSoundListener.up[i] != value[i]) {
				changedIndex = i;
				break;
			}
		}
		newSoundListener.up = value;
	}
	entityIDs.push_back(m_globalInfo.currentEntityID);
	newSoundListeners.push_back(newSoundListener);

	for (EntityID otherSelectedEntityID : m_globalInfo.otherSelectedEntityIDs) {
		if (m_globalInfo.entities[otherSelectedEntityID].soundListener) {
			newSoundListener = m_globalInfo.entities[otherSelectedEntityID].soundListener.value();

			if (senderWidget == forwardWidget) {
				newSoundListener.forward[changedIndex] = value[changedIndex];
			}
			else if (senderWidget == upWidget) {
				newSoundListener.up[changedIndex] = value[changedIndex];
			}

			entityIDs.push_back(otherSelectedEntityID);
			newSoundListeners.push_back(newSoundListener);
		}
	}

	updateComponents(entityIDs, newSoundListeners);
}
