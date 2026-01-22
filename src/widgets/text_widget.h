#pragma once
#include "../common/global_info.h"
#include <QTextEdit>
#include <string>

class TextWidget : public QTextEdit {
	Q_OBJECT
protected:
	void focusOutEvent(QFocusEvent* event);

signals:
	void valueChanged(const std::string&);
};