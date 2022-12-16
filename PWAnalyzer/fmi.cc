#include "fmi.hh"

#include "utils.hh"

namespace pwa {

const int FMI::SAFE_DAYS_PER_REQUEST = 7;
const int FMI::SAFE_DAYS_PER_FORECAST_REQUEST = std::numeric_limits<int>::max();

const QString FMI::URL_TEMPLATE =
		"https://opendata.fmi.fi/"
		"wfs?request=getFeature&version=2.0.0"
		"&storedquery_id=%1"
		"&place=%2"
		"&starttime=%3"
		"&endtime=%4"
		"&parameters=%5";

const QString FMI::FORECAST_QUERY_ID =
		"fmi::forecast::hirlam::surface::point::multipointcoverage";
const QString FMI::OBSERVATION_QUERY_ID =
		"fmi::observations::weather::multipointcoverage";

const std::unordered_map<DataType, QString> FMI::DATA_TYPE_PARAM_NAMES = {
		{DataType::TEMP, "t2m"}, {DataType::TEMP_FORECAST, "Temperature"},
		{DataType::WIND, "ws_10min"}, {DataType::WIND_FORECAST, "WindSpeedMS"},
		{DataType::CLOUDINESS, "n_man"}};

const std::unordered_set<DataType> FMI::FORECAST_DATA_TYPES = {
		DataType::TEMP_FORECAST, DataType::WIND_FORECAST};

const std::unordered_map<DataType, QString> FMI::UNITS = {
		{DataType::TEMP, "°C"}, {DataType::TEMP_FORECAST, "°C"},
		{DataType::WIND, "m/s"}, {DataType::WIND_FORECAST, "m/s"},
		{DataType::CLOUDINESS, "Oktas"}};

const QString FMI::INVALID_LOCATION_TEXT =
		"No locations found for the place with the requested language!";

FMI::FMI() {
}

int FMI::getSupportedDaysPerRequest(
		const pwa::FetchRequest& fetchRequest) const {
	Q_UNUSED(fetchRequest);
	if (FORECAST_DATA_TYPES.count(fetchRequest.dataType) == 1) {
		return SAFE_DAYS_PER_FORECAST_REQUEST;
	}
	return SAFE_DAYS_PER_REQUEST;
}

QNetworkRequest FMI::createQtRequest(const FetchRequest& fetchRequest) const {
	Q_ASSERT(implementsDataType(fetchRequest.dataType));

	QString queryId = OBSERVATION_QUERY_ID;

	QDateTime startTime = fetchRequest.timeSpan.start;
	QDateTime endTime = fetchRequest.timeSpan.end;

	if (FORECAST_DATA_TYPES.count(fetchRequest.dataType) == 1) {
		queryId = FORECAST_QUERY_ID;

		// No need to fetch history (with a little padding)
		//, because FMI will just return NaN for it for forecasts
		QDateTime currTime = QDateTime::currentDateTime().addSecs(-60);
		if (startTime < currTime) {
			startTime = currTime;
		}
		if (endTime < currTime) {
			endTime = currTime;
		}
	}

	// offsetFromUtc has to be assigned to itself for qt toString function
	// to show the timezone
	const QString startTimeStr = utils::toIsoStringWithTZ(startTime);
	const QString endTimeStr = utils::toIsoStringWithTZ(endTime);

	const QString paramString = DATA_TYPE_PARAM_NAMES.at(fetchRequest.dataType);

	// QUrl does not encode plus sign by itself because
	// https://stackoverflow.com/questions/36551109/qt-does-not-encode-sign.
	// We have to do it ourselves.
	const QString urlString =
			URL_TEMPLATE
					.arg(queryId, fetchRequest.location, startTimeStr,
							endTimeStr, paramString)
					.replace("+", "%2B");

	QNetworkRequest qRequest(QUrl{urlString});
	qRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

	return qRequest;
}

FetchResult FMI::parseResponse(
		QNetworkReply* response, const FetchRequest& origReq) const {
	std::vector<DataPoint> dataPoints;

	const QString data = response->readAll();

	QStringRef timestampsString = getStringRefBetween(
			data, "<gmlcov:positions>", "</gmlcov:positions>");

	QStringRef valuesString =
			getStringRefBetween(data, "<gml:doubleOrNilReasonTupleList>",
					"</gml:doubleOrNilReasonTupleList>");

	QVector<QStringRef> splitTimestampStrings = timestampsString.split("\n");
	QVector<QStringRef> splitValueStrings = valuesString.split("\n");

	for (int i = 0; i < splitTimestampStrings.size(); ++i) {
		try {
			if (splitTimestampStrings.at(i).isEmpty() ||
					splitValueStrings.at(i).isEmpty()) {
				continue;
			}

			// First and second part of line are coordinates, third one is
			// timestamp
			QVector<QStringRef> parts =
					splitTimestampStrings.at(i).split(" ", Qt::SkipEmptyParts);
			QDateTime x = QDateTime::fromTime_t(parts.last().toUInt());

			double y = splitValueStrings.at(i).toDouble();
			if (qIsNaN(y)) {
				continue;
			}

			dataPoints.push_back({.x = std::move(x), .y = y});
		} catch (const std::out_of_range& e) {
			break;
		}
	}

	return FetchResult{.error = FetchError::NONE,
			.dataLine = DataLine{.provider = origReq.provider,
					.dataType = origReq.dataType,
					.timeSpan = origReq.timeSpan,
					.dataPoints = std::move(dataPoints),
					.location = origReq.location,
					.yUnit = UNITS.at(origReq.dataType)}};
}

std::vector<DataType> FMI::getImplementedDataTypes() const {
	static std::vector<DataType> result{};
	for (auto const& element : DATA_TYPE_PARAM_NAMES) {
		result.push_back(element.first);
	}
	std::sort(result.begin(), result.end());
	return result;
};

bool FMI::implementsDataType(DataType dataType) const {
	return DATA_TYPE_PARAM_NAMES.find(dataType) != DATA_TYPE_PARAM_NAMES.end();
}

FetchError FMI::parseError(QNetworkReply* response) const {
	QVariant statusCode =
			response->attribute(QNetworkRequest::HttpStatusCodeAttribute);

	if (!statusCode.isValid())
		return FetchError::UNSET;

	if (statusCode.toInt() == 400) {
		const QString data = response->readAll();
		if (data.contains(INVALID_LOCATION_TEXT)) {
			return FetchError::LOC_NOT_SUPPORTED_BY_PROVIDER;
		}
	}

	return FetchError::UNSET;
}

QStringRef FMI::getStringRefBetween(
		const QString& input, const QString& start, const QString& end) const {
	int startIndex = input.indexOf(start);
	if (startIndex == -1) {
		return QStringRef{};
	}

	int endIndex = input.indexOf(end, startIndex);
	if (endIndex == -1) {
		return QStringRef{};
	}

	QStringRef result = input.midRef(
			startIndex + start.size(), endIndex - startIndex - start.size());
	return result.trimmed();
}

}  // namespace pwa
