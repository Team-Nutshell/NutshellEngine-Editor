#pragma once
#include <QLineEdit>
#include <QShowEvent>

class AssetLineEdit : public QLineEdit {
	Q_OBJECT
public:
	AssetLineEdit(QWidget* parent = nullptr);
	AssetLineEdit(const QString& contents, QWidget* parent = nullptr);

private slots:
	void showEvent(QShowEvent* event);
};