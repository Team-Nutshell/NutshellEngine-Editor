#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLineEdit>

class NewProjectNameWidget : public QWidget {
	Q_OBJECT
public:
	NewProjectNameWidget(GlobalInfo& globalInfo);

signals:
	void textChanged(const std::string&);

private slots:
	void onTextChanged();

private:
	GlobalInfo& m_globalInfo;

	QLineEdit* m_projectNameLineEdit;
};