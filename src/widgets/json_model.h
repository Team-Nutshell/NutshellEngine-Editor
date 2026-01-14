#pragma once
#include "../common/global_info.h"
#include "../../external/nlohmann/json.hpp"
#include <QAbstractItemModel>
#include <QJsonValue>
#include <memory>
#include <vector>
#include <string>

class JSONModelItem {
public:
	JSONModelItem();
	JSONModelItem(const QString& key, const QJsonValue& value, JSONModelItem* parent = nullptr);

	void appendChild(std::unique_ptr<JSONModelItem>&& child);
	JSONModelItem* child(int row);
	int childCount() const;
	int row() const;
	JSONModelItem* parentItem();

	void setKey(const QString& key);
	QString key() const;
	void setValue(const QVariant& value);
	QVariant value() const;
	void setType(const QJsonValue::Type& type);
	QJsonValue::Type type() const;

private:
	QJsonValue::Type m_type;
	QString m_key;
	QVariant m_value;
	std::vector<std::unique_ptr<JSONModelItem>> m_children;
	JSONModelItem* m_parent = nullptr;
};

class JSONModel : public QAbstractItemModel {
	Q_OBJECT
public:
	JSONModel(GlobalInfo& globalInfo, const std::string& path);

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::ItemDataRole::EditRole);
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::ItemDataRole::DisplayRole) const;
	QVariant data(const QModelIndex& index, int role = Qt::ItemDataRole::DisplayRole) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::ItemDataRole::EditRole);
	Qt::ItemFlags flags(const QModelIndex& index) const;

private:
	GlobalInfo& m_globalInfo;

	std::string m_jsonFilePath;

	std::unique_ptr<JSONModelItem> m_root = nullptr;

	std::array<QString, 2> m_headers = { "Key", "Value" };
};