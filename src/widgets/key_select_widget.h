#pragma once
#include "../common/global_info.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QFocusEvent>
#include <string>

class KeySelectWidget : public QWidget {
	Q_OBJECT
public:
	KeySelectWidget(GlobalInfo& globalInfo, const std::string& name, const std::string& key);

signals:
	void keyChanged(const std::string&);

private slots:
	void onButtonClicked();

	void keyPressEvent(QKeyEvent* event);
	void focusOutEvent(QFocusEvent* event);

private:
	GlobalInfo& m_globalInfo;

	bool m_changeKey = false;

public:
	QLabel* nameLabel;
	QPushButton* button;
};