#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <string>

class EntityGroupNameWidget : public QWidget {
	Q_OBJECT
public:
	EntityGroupNameWidget(GlobalInfo& globalInfo);

	void setText(const std::string& text);
	const std::string& getText();

signals:
	void entityGroupNameChanged(const std::string&);
	void removeEntityGroupClicked();

private slots:
	void onEditingFinished();
	void onRemoveEntityGroupClicked();

private:
	GlobalInfo& m_globalInfo;

	std::string m_text = "";

public:
	QLineEdit* entityGroupNameLineEdit;
	QPushButton* removeEntityGroupButton;
};