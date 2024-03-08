#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <string>
#include <memory>

class ScalarWidget : public QWidget {
	Q_OBJECT
public:
	ScalarWidget(GlobalInfo& globalInfo, const std::string& name);

signals:
	void editingFinished(float);

private slots:
	void onEditingFinished();

private:
	GlobalInfo& m_globalInfo;
	
	float m_value = 0.0f;

public:
	std::unique_ptr<QLabel> nameLabel;
	std::unique_ptr<QLineEdit> valueLineEdit;
};