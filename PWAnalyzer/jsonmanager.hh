/*!
	\file
	\brief class JsonManager with
	small JSON validation utils.
 */

#ifndef JSON_MANAGER_HH
#define JSON_MANAGER_HH

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

/*!
	\brief The namespace of the project
*/
namespace pwa {
/*!
	\brief JsonManager is a tool for writing JSON
	into a file and for reading a JSON file into a QJsonDocument.
*/
class JsonManager {
  public:
	/*!
		\brief Construct JsonManager associated with
		the saveFilePath.
		\param saveFilePath The (absolute or relative) path
		to the file used for writing and reading
		the JSON document under management.
		\note Be careful not to overwrite others' files unknowingly.
	*/
	JsonManager(const QString& saveFilePath);

	/*!
		\param directoryPath The (absolute or relative) path to
		the file (except the filename) used for loading
		and saving the JSON document under management.
		If a directory doesn't exist, it will be created.
		\param filename The name of the file to read/write.
		\note Be careful not to overwrite others' files unknowingly.
		\note If the creation of the directory path fails,
		all read/write method calls will return false.
	*/
	JsonManager(const QString& directoryPath, const QString& filename);

	JsonManager(const JsonManager&) = delete;
	JsonManager(JsonManager&&) = delete;
	JsonManager& operator=(const JsonManager&) = delete;
	JsonManager& operator=(JsonManager&&) = delete;

	virtual ~JsonManager() = default;

	/*!
		\brief Access the JSON document.
		\param document The JSON document to store the JSON into.
		\return true if succesful, false otherwise.
		\see write
	*/
	bool readTo(QJsonDocument& document) const;

	/*!
		\brief Access the JSON document as QJsonObject.
		\param document The QJsonObject to read the JSON to.
		\return true if successful, false otherwise.
		\see write
	*/
	bool readTo(QJsonObject& document) const;

	/*!
		\brief Access the JSON document as QJsonArray.
		\param document The QJsonArray to read the JSON to.
		\return true if successful, false otherwise.
		\see write
	*/
	bool readTo(QJsonArray& document) const;

	/*!
		\brief Save the JSON document to the file.
		The QJsonType of the document should be
		either QJsonArray or QJsonObject.
		\return true if successful, false otherwise.
		\param document The JSON document used for
		(over)writing the file with new content.
		\see readTo
	*/
	template<typename QJsonType>
	bool write(const QJsonType& document) const {
		QFile jsonFile{filePath_};
		if (!jsonFile.open(QIODevice::WriteOnly)) {
			return false;
		}

		jsonFile.write(QJsonDocument{document}.toJson());
		jsonFile.close();

		return true;
	}

	/*
		JSON validation helper functions that
		need to be in the header because of inline.
		https://www.cplusplus.com/forum/unices/21237/
	*/
	/*!
		\brief Check the existence of the key
		and the type correctness of the value.
		\param jsonKey the key corresponding
		to the value to validate.
		\param json the container possibly
		containing the jsonKey - bool pair
		\return true if valid
	*/
	static inline bool isValidBool(
			const QString& jsonKey, const QJsonObject& json) {
		return (json.contains(jsonKey) && json[jsonKey].isBool());
	}
	/*!
		\brief Check the existence of the key
		and the type correctness of the value.
		\param jsonKey the key corresponding
		to the value to validate.
		\param json the container possibly
		containing the jsonKey - double pair
		\return true if valid
	*/
	static inline bool isValidDouble(
			const QString& jsonKey, const QJsonObject& json) {
		return (json.contains(jsonKey) && json[jsonKey].isDouble());
	}
	/*!
		\brief Check the existence of the key
		and the type correctness of the value.
		\param jsonKey the key corresponding
		to the value to validate.
		\param json the container possibly
		containing the jsonKey - string pair
		\return true if valid
	*/
	static inline bool isValidString(
			const QString& jsonKey, const QJsonObject& json) {
		return (json.contains(jsonKey) && json[jsonKey].isString());
	}

  private:
	QString filePath_;
};
}  // namespace pwa

#endif  //	JSON_MANAGER_HH
