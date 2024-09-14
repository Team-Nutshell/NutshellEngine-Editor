#pragma once
#include "../common/global_info.h"
#include <QPushButton>
#include <string>

class ModelNtmdFileWidget : public QWidget {
	Q_OBJECT
public:
	ModelNtmdFileWidget(GlobalInfo& globalInfo);

	void setPath(const std::string& path);

	void save();

private slots:
	void onAddPrimitiveButtonClicked();
	void onRemovePrimitiveButtonClicked();
	void onValueChanged();

private:
	GlobalInfo& m_globalInfo;

	std::string m_modelFilePath;

public:
	QWidget* primitivesWidget;
	QPushButton* addPrimitiveButton;
};