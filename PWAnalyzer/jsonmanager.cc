
#include "jsonmanager.hh"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace pwa {

JsonManager::JsonManager(const QString& saveFilePath): filePath_{saveFilePath} {
}

JsonManager::JsonManager(const QString& directoryPath, const QString& filename):
		filePath_{directoryPath + "/" + filename} {
	Q_ASSERT(directoryPath.back() != '/');
	Q_ASSERT(!filename.contains("/"));

	QDir dir{};
	//	- If this fails, everything else will fail.
	dir.mkpath(directoryPath);
}

bool JsonManager::readTo(QJsonDocument& document) const {
	QByteArray fileData{};
	{
		QFile jsonFile{filePath_};
		if (!jsonFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		fileData = jsonFile.readAll();
		jsonFile.close();
	}

	document = QJsonDocument::fromJson(fileData);

	return true;
}

bool JsonManager::readTo(QJsonObject& document) const {
	QJsonDocument jsonDocument{};
	readTo(jsonDocument);
	if (!jsonDocument.isObject()) {
		return false;
	}
	document = jsonDocument.object();
	return true;
}

bool JsonManager::readTo(QJsonArray& document) const {
	QJsonDocument jsonDocument{};
	readTo(jsonDocument);
	if (!jsonDocument.isArray()) {
		return false;
	}
	document = jsonDocument.array();
	return true;
}

}  // namespace pwa
