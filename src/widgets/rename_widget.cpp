#include "rename_widget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <filesystem>

RenameWidget::RenameWidget(GlobalInfo& globalInfo, const std::string& directory, const std::string& filename) : m_globalInfo(globalInfo), m_directory(directory) {
	setWindowTitle("NutshellEngine - Rename - " + QString::fromStdString(directory) + "/" + QString::fromStdString(filename));
	setWindowIcon(QIcon("assets/icon.png"));
	setAttribute(Qt::WA_DeleteOnClose);

	m_filename = filename;
	if (m_filename.back() == '/') {
		m_filename.pop_back();
	}
	m_newFilename = filename;

	setLayout(new QVBoxLayout());
	layout()->setAlignment(Qt::AlignmentFlag::AlignHCenter);
	layout()->addWidget(new QLabel("Renaming " + QString::fromStdString(directory) + "/" + QString::fromStdString(filename)));
	filenameEdit = new QLineEdit(QString::fromStdString(m_filename));
	layout()->addWidget(filenameEdit);
	applyButton = new QPushButton("Apply");
	layout()->addWidget(applyButton);

	connect(filenameEdit, &QLineEdit::textChanged, this, &RenameWidget::onTextChanged);
	connect(applyButton, &QPushButton::clicked, this, &RenameWidget::onButtonClicked);
}

void RenameWidget::onTextChanged() {
	m_newFilename = filenameEdit->text().toStdString();
	std::string newFullPath = m_directory + "/" + m_newFilename;
	if (m_newFilename == m_filename) {
		applyButton->setEnabled(true);
		return;
	}

	const std::regex validFilenameRegex(R"(^[a-zA-Z0-9._ -]+$)");
	if (!std::regex_search(m_newFilename, validFilenameRegex)) {
		applyButton->setEnabled(false);
		return;
	}

	if (m_newFilename.empty()) {
		applyButton->setEnabled(false);
		return;
	}

	if (std::filesystem::exists(newFullPath)) {
		applyButton->setEnabled(false);
		return;
	}

	applyButton->setEnabled(true);
	return;
}

void RenameWidget::onButtonClicked() {
	if (m_newFilename != m_filename) {
		std::filesystem::rename(m_directory + "/" + m_filename, m_directory + "/" + m_newFilename);
	}
	close();
}

void RenameWidget::keyPressEvent(QKeyEvent* event) {
	if (event->isAutoRepeat()) {
		event->accept();
		return;
	}

	if (event->key() == Qt::Key_Return) {
		if (applyButton->isEnabled()) {
			onButtonClicked();
		}
	}
	else if (event->key() == Qt::Key_Escape) {
		close();
	}
}
