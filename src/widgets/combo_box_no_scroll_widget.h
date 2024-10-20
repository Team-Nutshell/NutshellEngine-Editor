#pragma once
#include <QComboBox>
#include <QWheelEvent>

class ComboBoxNoScrollWidget : public QComboBox {
	Q_OBJECT
public:
	ComboBoxNoScrollWidget();

private slots:
	void wheelEvent(QWheelEvent* event);
};