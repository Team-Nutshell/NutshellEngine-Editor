#include "json_model.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

JSONModelItem::JSONModelItem() {}

JSONModelItem::JSONModelItem(const QString& key, const QJsonValue& value, JSONModelItem* parent) : m_parent(parent) {
	m_key = key;
	m_type = value.type();
	if (value.isArray()) {
		QJsonArray valueArray = value.toArray();
		for (qsizetype i = 0; i < valueArray.size(); i++) {
			appendChild(std::make_unique<JSONModelItem>(QString::number(i), valueArray[i], this));
		}
	}
	else if (value.isObject()) {
		QJsonObject valueObject = value.toObject();
		QStringList keys = valueObject.keys();
		for (QString objectKey : keys) {
			appendChild(std::make_unique<JSONModelItem>(objectKey, valueObject[objectKey], this));
		}
	}
	else {
		m_value = value.toVariant();
	}
}

void JSONModelItem::appendChild(std::unique_ptr<JSONModelItem>&& child) {
	m_children.push_back(std::move(child));
}

JSONModelItem* JSONModelItem::child(int row) {
	if ((row < 0) || (row > static_cast<int>(m_children.size() - 1))) {
		return nullptr;
	}

	return m_children[row].get();
}

int JSONModelItem::childCount() const {
	return static_cast<int>(m_children.size());
}

int JSONModelItem::row() const {
	if (!m_parent) {
		return 0;
	}

	std::vector<std::unique_ptr<JSONModelItem>>::const_iterator it = std::find_if(m_parent->m_children.cbegin(), m_parent->m_children.cend(), [this](const std::unique_ptr<JSONModelItem>& item) {
		return item.get() == this;
	});

	if (it == m_parent->m_children.cend()) {
		return -1;
	}

	return std::distance(m_parent->m_children.cbegin(), it);
}

JSONModelItem* JSONModelItem::parentItem() {
	return m_parent;
}

void JSONModelItem::setKey(const QString& key) {
	m_key = key;
}

QString JSONModelItem::key() const {
	return m_key;
}

void JSONModelItem::setValue(const QVariant& value) {
	m_value = value;
}

QVariant JSONModelItem::value() const {
	return m_value;
}

void JSONModelItem::setType(const QJsonValue::Type& type) {
	m_type = type;
}

QJsonValue::Type JSONModelItem::type() const {
	return m_type;
}

JSONModel::JSONModel(GlobalInfo& globalInfo, const std::string& path) : m_globalInfo(globalInfo) {
	m_jsonFilePath = path;

	QJsonDocument j;

	QFile jsonFile(QString::fromStdString(m_jsonFilePath));
	if (jsonFile.open(QIODevice::OpenModeFlag::ReadOnly)) {
		j = QJsonDocument::fromJson(jsonFile.readAll());
		if (j.isArray()) {
			m_root = std::make_unique<JSONModelItem>("", j.array());
		}
		else if (j.isObject()) {
			m_root = std::make_unique<JSONModelItem>("", j.object());
		}
	}
	else {
		m_globalInfo.logger.addLog(LogLevel::Warning, m_globalInfo.localization.getString("log_file_is_not_valid_json", { m_jsonFilePath }));
	}
}

QModelIndex JSONModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}

	JSONModelItem* parentItem = parent.isValid() ? static_cast<JSONModelItem*>(parent.internalPointer()) : m_root.get();

	if (JSONModelItem* child = parentItem->child(row)) {
		return createIndex(row, column, child);
	}

	return QModelIndex();
}

QModelIndex JSONModel::parent(const QModelIndex& index) const {
	if (!index.isValid()) {
		return QModelIndex();
	}

	JSONModelItem* childItem = static_cast<JSONModelItem*>(index.internalPointer());
	JSONModelItem* parentItem = childItem->parentItem();

	return (parentItem != m_root.get()) ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex();
}

int JSONModel::rowCount(const QModelIndex& parent) const {
	JSONModelItem* parentItem = parent.isValid() ? static_cast<JSONModelItem*>(parent.internalPointer()) : m_root.get();

	return parentItem->childCount();
}

int JSONModel::columnCount(const QModelIndex& parent) const {
	(void)parent;

	return 2;
}

bool JSONModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role) {
	if (role != Qt::ItemDataRole::EditRole) {
		return false;
	}

	if (orientation == Qt::Orientation::Horizontal) {
		m_headers[section] = value.toString();

		emit headerDataChanged(orientation, section, section);

		return true;
	}

	return false;
}

QVariant JSONModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::ItemDataRole::DisplayRole) {
		return QVariant();
	}

	if (orientation == Qt::Orientation::Horizontal) {
		return m_headers[section];
	}

	return QVariant();
}

QVariant JSONModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || (role != Qt::ItemDataRole::DisplayRole)) {
		return QVariant();
	}

	JSONModelItem* item = static_cast<JSONModelItem*>(index.internalPointer());
	if (index.column() == 0) {
		return item->key();
	}
	else {
		return item->value();
	}
}

bool JSONModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (!index.isValid() || (role != Qt::ItemDataRole::EditRole)) {
		return false;
	}

	JSONModelItem* item = static_cast<JSONModelItem*>(index.internalPointer());
	if (index.column() == 0) {
		item->setKey(value.toString());
	}
	else {
		item->setValue(value);
	}

	emit dataChanged(index, index, { role });

	return true;
}

Qt::ItemFlags JSONModel::flags(const QModelIndex& index) const {
	return QAbstractItemModel::flags(index);
}
