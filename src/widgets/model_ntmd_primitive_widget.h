#pragma once
#include "../common/global_info.h"
#include "file_selector_widget.h"
#include "../../external/nlohmann/json.hpp"
#include <QPushButton>

class ModelNtmdPrimitiveWidget : public QWidget {
	Q_OBJECT
public:
	ModelNtmdPrimitiveWidget(GlobalInfo& globalInfo);

	nlohmann::json toJson() const;

signals:
	void valueChanged();
	void removePrimitive();

private slots:
	void onValueChanged();
	void onRemovePrimitiveButtonClicked();

private:
	GlobalInfo& m_globalInfo;

public:
	QPushButton* removePrimitiveButton;
	FileSelectorWidget* primitiveMeshWidget;
	FileSelectorWidget* primitiveMaterialWidget;
};