#pragma once
#include "../common/common.h"
#include "file_selector_widget.h"
#include <QWidget>
#include <memory>

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
	std::unique_ptr<FileSelectorWidget> modelPathWidget;
};