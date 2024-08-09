#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "combo_box_widget.h"
#include <QWidget>

class RenderableComponentWidget : public QWidget {
	Q_OBJECT
public:
	RenderableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Renderable& renderable);
	void updateComponent(EntityID entityID, Component* component);

private slots:
	void onEntitySelected();
	void onAddEntityRenderable(EntityID entityID);
	void onRemoveEntityRenderable(EntityID entityID);
	void onChangeEntityRenderable(EntityID entityID, const Renderable& renderable);
	void onStringChanged(const std::string& string);
	void onElementChanged(const std::string& element);

private:
	GlobalInfo& m_globalInfo;

public:
	FileSelectorWidget* modelPathWidget;
	ComboBoxWidget* primitiveIndexWidget;
};