#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <string>
#include <vector>
#include <memory>

class ComboBoxWidget : public QWidget {
	Q_OBJECT
public:
	ComboBoxWidget(GlobalInfo& globalInfo, const std::string& name, std::vector<std::string>& elements);

signals:
	void elementSelected(const std::string&);

private slots:
	void onElementSelected(const QString& element);

private:
	GlobalInfo& m_globalInfo;

public:
	std::unique_ptr<QLabel> nameLabel;
	std::unique_ptr<QComboBox> comboBox;
};