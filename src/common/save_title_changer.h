#pragma once
#include <QWidget>

class SaveTitleChanger {
public:
	static void change(QWidget* widget) {
		if (!widget->windowTitle().isEmpty() && (widget->windowTitle()[0] != '*')) {
			widget->setWindowTitle("*" + widget->windowTitle());
		}
	}
	
	static void save(QWidget* widget) {
		if (!widget->windowTitle().isEmpty() && (widget->windowTitle()[0] == '*')) {
			widget->setWindowTitle(widget->windowTitle().remove(0, 1));
		}
	}
};