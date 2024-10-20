#pragma once
#include "../common/global_info.h"
#include "combo_box_no_scroll_widget.h"
#include <QWidget>
#include <QLabel>
#include <string>
#include <vector>

class ComboBoxWidget : public QWidget {
	Q_OBJECT
public:
	ComboBoxWidget(GlobalInfo& globalInfo, const std::string& name, std::vector<std::string>& elements);

	void setElementByText(const std::string& text);
	std::string getElementText();

signals:
	void elementSelected(const std::string&);

private slots:
	void onElementSelected(const QString& element);

private:
	GlobalInfo& m_globalInfo;

public:
	QLabel* nameLabel;
	ComboBoxNoScrollWidget* comboBox;
};