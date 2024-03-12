#pragma once
#include "../common/common.h"
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <string>
#include <vector>

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
	QLabel* nameLabel;
	QComboBox* comboBox;
};