#include "fingrid.hh"

#include "utils.hh"
#include "webapi.hh"

namespace pwa {

const QString Fingrid::API_KEY = "UN10oA3i1e1lFPewAEaq35Vg0eBaTixQ8VDOjSzu";

const QString Fingrid::URL_TEMPLATE =
		"https://api.fingrid.fi/v1/variable/%1/events/"
		"csv?start_time=%2&end_time=%3";

const std::unordered_map<DataType, int> Fingrid::DATA_TYPE_IDS = {
		{DataType::EL_CONS, 193}, {DataType::EL_CONS_FORECAST_24H, 165},
		{DataType::EL_PROD, 192}, {DataType::EL_PROD_FORECAST_24H, 242},
		{DataType::HYDRO_PWR_PROD, 191}, {DataType::NUCLEAR_PWR_PROD, 188},
		{DataType::WIND_PWR_PROD, 181}};

const std::unordered_set<DataType> Fingrid::REAL_TIME_DATA_TYPES = {
		DataType::EL_CONS, DataType::EL_PROD, DataType::HYDRO_PWR_PROD,
		DataType::NUCLEAR_PWR_PROD, DataType::WIND_PWR_PROD};

const int Fingrid::SAFE_DAYS_PER_REALTIME_REQUEST = 4 * 30;
const int Fingrid::SAFE_DAYS_PER_OTHER_REQUEST = 4 * 365;

const QString Fingrid::UNIT = "MW";

Fingrid::Fingrid() {
}

int Fingrid::getSupportedDaysPerRequest(
		const FetchRequest& fetchRequest) const {
	if (REAL_TIME_DATA_TYPES.count(fetchRequest.dataType) == 1) {
		return SAFE_DAYS_PER_REALTIME_REQUEST;
	}
	return SAFE_DAYS_PER_OTHER_REQUEST;
}

QNetworkRequest Fingrid::createQtRequest(
		const FetchRequest& fetchRequest) const {
	Q_ASSERT(implementsDataType(fetchRequest.dataType));

	const int dataTypeId = DATA_TYPE_IDS.at(fetchRequest.dataType);

	// offsetFromUtc has to be assigned to itself for qt toString function to
	// show the timezone
	const QString startTimeStr =
			utils::toIsoStringWithTZ(fetchRequest.timeSpan.start);
	const QString endTimeStr =
			utils::toIsoStringWithTZ(fetchRequest.timeSpan.end);

	// QUrl does not encode plus sign by itself because
	// https://stackoverflow.com/questions/36551109/qt-does-not-encode-sign.
	// We have to do it ourselves.
	const QString urlString = URL_TEMPLATE.arg(dataTypeId)
									  .arg(startTimeStr, endTimeStr)
									  .replace("+", "%2B");

	QNetworkRequest qRequest(QUrl{urlString});
	qRequest.setRawHeader("x-api-key", API_KEY.toUtf8());
	qRequest.setHeader(QNetworkRequest::ContentTypeHeader, "text/csv");

	return qRequest;
}

FetchResult Fingrid::parseResponse(
		QNetworkReply* response, const FetchRequest& origReq) const {
	std::vector<DataPoint> dataPoints{};

	// Ignore first line, bc it's headers
	QByteArray line = response->readLine();

	while (!response->atEnd()) {
		line = response->readLine();
		auto items = line.split(',');
		QDateTime x = utils::fromIsoString(items[0]);

		double y = stod(items[2].toStdString());
		dataPoints.push_back({.x = std::move(x), .y = y});
	}

	return FetchResult{.error = FetchError::NONE,
			.dataLine{.provider = origReq.provider,
					.dataType = origReq.dataType,
					.timeSpan = origReq.timeSpan,
					.dataPoints = std::move(dataPoints),
					.yUnit = UNIT}};
}

std::vector<DataType> Fingrid::getImplementedDataTypes() const {
	static std::vector<DataType> result{};
	for (auto const& element : DATA_TYPE_IDS) {
		result.push_back(element.first);
	}
	std::sort(result.begin(), result.end());
	return result;
};

FetchError Fingrid::parseError(QNetworkReply* response) const {
	QVariant statusCode =
			response->attribute(QNetworkRequest::HttpStatusCodeAttribute);

	if (!statusCode.isValid())
		return FetchError::UNSET;

	switch (statusCode.toInt()) {
	case 404:
		return FetchError::TYPE_NOT_IMPL_BY_PROVIDER;
	case 416:
		return FetchError::TOO_LARGE_TIME_SPAN;
	case 503:
		return FetchError::SERVER_MAINTENANCE;
	default:
		return FetchError::UNSET;
	}
};

bool Fingrid::implementsDataType(DataType dataType) const {
	return DATA_TYPE_IDS.find(dataType) != DATA_TYPE_IDS.end();
}

}  // namespace pwa
