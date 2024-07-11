#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include <QWidget>

class RenderableComponentWidget : public QWidget {
	Q_OBJECT
public:
	RenderableComponentWidget(GlobalInfo& globalInfo);

private:
	void updateWidgets(const Renderable& renderable);

private slots:
	void onSelectEntity();
	void onAddEntityRenderable(EntityID entityID);
	void onRemoveEntityRenderable(EntityID entityID);
	void onChangeEntityRenderable(EntityID entityID, const Renderable& renderable);
	void onStringUpdated(const std::string& string);

private:
	GlobalInfo& m_globalInfo;

public:
	FileSelectorWidget* modelPathWidget;
};