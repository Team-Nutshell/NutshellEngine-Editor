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
	void updateComponents(const std::vector<EntityID>& entityIDs, std::vector<Renderable>& renderables);

private slots:
	void onEntitySelected();
	void onEntityRenderableAdded(EntityID entityID);
	void onEntityRenderableRemoved(EntityID entityID);
	void onEntityRenderableChanged(EntityID entityID, const Renderable& renderable);
	void onPathChanged(const std::string& path);
	void onElementChanged(const std::string& element);

private:
	GlobalInfo& m_globalInfo;

public:
	FileSelectorWidget* modelPathWidget;
	ComboBoxWidget* primitiveIndexWidget;
	FileSelectorWidget* materialPathWidget;
};