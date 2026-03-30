#pragma once
#include <QTextEdit>
#include <QWidget>
#include <QPaintEvent>

class TextEditLineNumber : public QWidget {
	Q_OBJECT
public:
	TextEditLineNumber(QTextEdit* textEdit);

	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent* event);

private:
	QTextEdit* m_textEdit;
};