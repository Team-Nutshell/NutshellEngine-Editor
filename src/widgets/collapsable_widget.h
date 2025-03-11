#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>

class CollapseBar : public QWidget {
	Q_OBJECT
public:
	CollapseBar(const std::string& text);

	void setText(const std::string& text);

signals:
	void collapseButtonPressed();

private slots:
	void onCollapseButtonPressed();

private:
	QPushButton* m_collapseButton;
	QLabel* m_collapseLabel;

	bool m_collapsed = true;
};

class CollapsableWidget : public QWidget {
	Q_OBJECT
public:
	CollapsableWidget(const std::string& text, QWidget* widget);

	void setText(const std::string& text);

private slots:
	void onCollapseButtonPressed();

private:
	CollapseBar* m_collapseBar;
	QWidget* m_widget;
};