/*!
	DataLineJSON model:
	{
		"provider": double,
		"datatype": double,
		"location": string,
		"yunit": string,
		"timespan": {
			"start": qdatetime.tostring,
			"end": qdatetime.tostring
		},
		"datapoints": [
			{
				"x": qdatetime.tostring,
				"y" double
			}
		]
	}

	DataLines JSON model:
	[
		DataLineJSON,
		DataLineJSON,
		...
	]
*/

#include "datalinesaver.hh"

#include "jsonmanager.hh"
//	toIsoStringWithTZ, fromIsoString
#include "utils.hh"

namespace pwa {

/*!
	\brief Contains JSON key constants for DataLine JSON.
*/
//	Why does inline fix this?
//	https://www.fluentcpp.com/2019/07/23/how-to-define-a-global-constant-in-cpp/
namespace dataline {
	inline const QString PROVIDER = "provider";
	inline const QString DATATYPE = "datatype";
	inline const QString TIME_SPAN = "timespan";
	inline const QString TIME_SPAN_START = "start";
	inline const QString TIME_SPAN_END = "end";
	inline const QString DATAPOINTS = "datapoints";
	inline const QString DATAPOINT_X = "x";
	inline const QString DATAPOINT_Y = "y";
	inline const QString LOCATION = "location";
	inline const QString YUNIT = "yunit";
}  // namespace dataline

DataLineSaver::DataLineSaver(const QString& saveFilePath):
		dataLineManager_{saveFilePath} {
}

bool DataLineSaver::readTo(DataLine& data) const {
	QJsonArray jsonDataLines{};
	return (dataLineManager_.readTo(jsonDataLines) &&
			!jsonDataLines.isEmpty() &&
			restore(data, jsonDataLines.first().toObject()));
}

bool DataLineSaver::readTo(std::vector<DataLine>& data) const {
	QJsonArray jsonDataLines{};
	if (!dataLineManager_.readTo(jsonDataLines)) {
		return false;
	}

	//	Not clearing data would make readTo
	//	append instead of restore.
	data.clear();
	for (QJsonValue&& dataLineValue : jsonDataLines) {
		DataLine dataLine{};
		if (!(/*dataLineValue.isObject() &&*/
					restore(dataLine, dataLineValue.toObject()))) {
			return false;
		}
		data.push_back(dataLine);
	}

	return true;
}

bool DataLineSaver::write(const DataLine& data) const {
	/*
		Wrap in QJsonArray for JSON file format
		compatibility between write/read method overloads.
	*/
	return dataLineManager_.write(QJsonArray{toQJsonObject(data)});
}

bool DataLineSaver::write(
		const std::vector<std::shared_ptr<DataLine>>& data) const {
	return dataLineManager_.write(toQJsonArray(data));
}

bool DataLineSaver::restore(
		DataLine& dataLine, const QJsonObject& jsonDataLine) const {
	/*	Commenting this out would give a
		speedup but the program might crash.
	*/
	if (!isValidDataLineJson(jsonDataLine)) {
		return false;
	}

	restoreSingleValues(dataLine, jsonDataLine);
	restoreTimeSpan(dataLine, jsonDataLine);
	restoreDataPoints(dataLine, jsonDataLine);

	return true;
}

void DataLineSaver::restoreSingleValues(
		DataLine& dataLine, const QJsonObject& jsonDataLine) const {
	dataLine.provider =
			static_cast<Provider>(jsonDataLine[dataline::PROVIDER].toInt());
	dataLine.dataType =
			static_cast<DataType>(jsonDataLine[dataline::DATATYPE].toInt());
	dataLine.location = jsonDataLine[dataline::LOCATION].toString();
	dataLine.yUnit = jsonDataLine[dataline::YUNIT].toString();
}

void DataLineSaver::restoreTimeSpan(
		DataLine& dataLine, const QJsonObject& jsonDataLine) const {
	QJsonObject timeSpan = jsonDataLine[dataline::TIME_SPAN].toObject();

	dataLine.timeSpan.start = utils::fromIsoString(
			timeSpan[dataline::TIME_SPAN_START].toString());
	dataLine.timeSpan.end =
			utils::fromIsoString(timeSpan[dataline::TIME_SPAN_END].toString());
}

void DataLineSaver::restoreDataPoints(
		DataLine& dataLine, const QJsonObject& jsonDataLine) const {
	QJsonArray jsonDataPoints = jsonDataLine[dataline::DATAPOINTS].toArray();

	dataLine.dataPoints = std::vector<DataPoint>{};
	dataLine.dataPoints.reserve(jsonDataPoints.size());

	for (const QJsonValue& dataPoint : jsonDataPoints) {
		dataLine.dataPoints.push_back(
				DataPoint{dataPointX(dataPoint), dataPointY(dataPoint)});
	}
}

QDateTime DataLineSaver::dataPointX(const QJsonValue& dataPoint) const {
	return utils::fromIsoString(dataPoint[dataline::DATAPOINT_X].toString());
}

double DataLineSaver::dataPointY(const QJsonValue& dataPoint) const {
	return dataPoint[dataline::DATAPOINT_Y].toDouble();
}

QJsonObject DataLineSaver::toQJsonObject(const DataLine& data) const {
	return {{dataline::PROVIDER, static_cast<int>(data.provider)},
			{dataline::DATATYPE, static_cast<int>(data.dataType)},
			{dataline::LOCATION, data.location}, {dataline::YUNIT, data.yUnit},
			{dataline::TIME_SPAN, toQJsonObject(data.timeSpan)},
			{dataline::DATAPOINTS, toQJsonArray(data.dataPoints)}};
}

QJsonArray DataLineSaver::toQJsonArray(
		const std::vector<std::shared_ptr<DataLine>>& data) const {
	QJsonArray resultJson{};
	for (auto&& dataLine : data) {
		resultJson.push_back(QJsonValue{toQJsonObject(*dataLine)});
	}

	return resultJson;
}

QJsonObject DataLineSaver::toQJsonObject(const TimeSpan& timeSpan) const {
	return QJsonObject{{dataline::TIME_SPAN_START,
							   utils::toIsoStringWithTZ(timeSpan.start)},
			{dataline::TIME_SPAN_END, utils::toIsoStringWithTZ(timeSpan.end)}};
}

QJsonArray DataLineSaver::toQJsonArray(
		const std::vector<DataPoint>& dataPoints) const {
	QJsonArray jsonDataPoints{};

	for (const DataPoint& dataPoint : dataPoints) {
		jsonDataPoints.push_back(toQJsonObject(dataPoint));
	}

	return jsonDataPoints;
}

QJsonObject DataLineSaver::toQJsonObject(const DataPoint& dataPoint) const {
	return {{dataline::DATAPOINT_X, utils::toIsoStringWithTZ(dataPoint.x)},
			{dataline::DATAPOINT_Y, dataPoint.y}};
}

bool DataLineSaver::isValidDataLineJson(const QJsonObject& json) const {
	return (JsonManager::isValidDouble(dataline::PROVIDER, json) &&
			JsonManager::isValidDouble(dataline::DATATYPE, json) &&
			JsonManager::isValidString(dataline::LOCATION, json) &&
			JsonManager::isValidString(dataline::YUNIT, json) &&
			isTimeSpanValid(json) && isDataPointsValid(json));
}

bool DataLineSaver::isTimeSpanValid(const QJsonObject& json) const {
	return (json.contains(dataline::TIME_SPAN) &&
			// json[dataline::TIME_SPAN].isObject() &&
			isTimeSpanContentsValid(json[dataline::TIME_SPAN].toObject()));
}

bool DataLineSaver::isTimeSpanContentsValid(QJsonObject&& timeSpan) const {
	return (JsonManager::isValidString(dataline::TIME_SPAN_START, timeSpan) &&
			JsonManager::isValidString(dataline::TIME_SPAN_END, timeSpan));
}

bool DataLineSaver::isDataPointsValid(const QJsonObject& json) const {
	return (json.contains(dataline::DATAPOINTS) &&
			// json[dataline::DATAPOINTS].isArray() &&
			isDataPointsContentsValid(json[dataline::DATAPOINTS].toArray()));
}

bool DataLineSaver::isDataPointsContentsValid(QJsonArray&& dataPoints) const {
	for (QJsonValue&& dataPointValue : dataPoints) {
		// if(!dataPointValue.isObject()) {
		//	return false;
		//}
		if (!isDataPointValid(dataPointValue.toObject())) {
			return false;
		}
	}
	return true;
}

bool DataLineSaver::isDataPointValid(const QJsonObject& dataPoint) const {
	return (JsonManager::isValidString(dataline::DATAPOINT_X, dataPoint) &&
			JsonManager::isValidDouble(dataline::DATAPOINT_Y, dataPoint));
}

}  // namespace pwa
