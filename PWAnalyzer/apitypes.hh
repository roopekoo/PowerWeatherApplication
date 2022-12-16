#ifndef APITYPES_H
#define APITYPES_H

/*!
 * \file
 * \brief Holds multiple structs and global constant datastructures that are
 * used when interfacing with the class WebAPI and in the private implementation
 * of WebAPI.
 */

#include <QDateTime>
#include <QDebug>
#include <unordered_map>

namespace pwa {

/*!
 * \brief
 * Holds all data types supported by any providers
 *
 *  EL: Electricity
 *  CONS: Consumption
 *  PROD: Production
 *  PWR: Power
 */
enum class DataType {
	UNSET,
	EL_CONS,
	EL_CONS_FORECAST_24H,
	EL_PROD,
	EL_PROD_FORECAST_24H,
	HYDRO_PWR_PROD,
	NUCLEAR_PWR_PROD,
	WIND_PWR_PROD,
	TEMP,
	TEMP_FORECAST,
	WIND,
	WIND_FORECAST,
	CLOUDINESS
};

/*!
 * \brief
 * These names are used as display names and also persistent save keys
 * (editing of old entries not advised).
 */
const std::unordered_map<DataType, QString> DATA_TYPE_NAMES{
		{DataType::UNSET, "Unset data type"},
		{DataType::EL_CONS, "Electricity consumption"},
		{DataType::EL_CONS_FORECAST_24H,
				"Electricity consumption forecast (24h)"},
		{DataType::EL_PROD, "Electricity production"},
		{DataType::EL_PROD_FORECAST_24H,
				"Electricity production prediction (24h)"},
		{DataType::HYDRO_PWR_PROD, "Hydro power production"},
		{DataType::NUCLEAR_PWR_PROD, "Nuclear power production"},
		{DataType::WIND_PWR_PROD, "Wind power production"},
		{DataType::TEMP, "Temperature"},
		{DataType::TEMP_FORECAST, "Temperature forecast"},
		{DataType::WIND, "Observed wind"},
		{DataType::WIND_FORECAST, "Wind forecast"},
		{DataType::CLOUDINESS, "Observed cloudiness"}};

enum class Provider { UNSET, FINGRID, FMI };

/*!
 * \brief
 * These names are used as persistent save keys
 * (editing of old entries not advised).
 */
const std::unordered_map<Provider, QString> PROVIDER_NAMES{
		{Provider::UNSET, "Unset provider"}, {Provider::FINGRID, "Fingrid"},
		{Provider::FMI, "FMI"}};

/*!
 * \brief Error types that can be returned from WebAPI fetch functions.
 */
enum class FetchError {
	UNSET,
	NONE,
	TYPE_NOT_IMPL_BY_PROVIDER,
	LOC_NOT_SUPPORTED_BY_PROVIDER,
	TOO_LARGE_TIME_SPAN,
	SERVER_MAINTENANCE,
	CONNECTION_FAILED
};

/*!
 * \brief Messages for FetchErrors
 */
const std::unordered_map<FetchError, QString> FETCH_ERROR_MSG{
		{FetchError::UNSET, "Unset fetch error"},
		{FetchError::NONE, "No errors"},
		{FetchError::TYPE_NOT_IMPL_BY_PROVIDER,
				"The type is not implemented by the provider"},
		{FetchError::LOC_NOT_SUPPORTED_BY_PROVIDER,
				"The location is not supported by the provider"},
		{FetchError::TOO_LARGE_TIME_SPAN, "Time span is too large"},
		{FetchError::SERVER_MAINTENANCE, "The server is at maintenance"},
		{FetchError::CONNECTION_FAILED, "Connection failed"}};

/*!
 * \brief DataPoint struct used in charts. X axis is always time and Y axis is a
 * double of unit that is based on data type.
 */
struct DataPoint {
	QDateTime x;
	double y;
};

/*!
 * \brief Helper struct for holding a span of QDateTimes.
 */
struct TimeSpan {
	QDateTime start = QDateTime{};
	QDateTime end = QDateTime{};
};

/*!
 * \brief The DataLine struct holds all relevant information about a data line.
 */
struct DataLine {
	Provider provider = Provider::UNSET;
	DataType dataType = DataType::UNSET;
	TimeSpan timeSpan = TimeSpan{};
	std::vector<DataPoint> dataPoints{};
	QString location = "";
	QString yUnit = "";
};

/*!
 * \brief The FetchRequest struct is used in WebAPI fetch function as a simple
 * holder of request information. No need to mess with urls.
 */
struct FetchRequest {
	Provider provider = Provider::UNSET;
	DataType dataType = DataType::UNSET;
	TimeSpan timeSpan = TimeSpan{};
	QString location = "";

	bool operator==(const FetchRequest& other) {
		return provider == other.provider && dataType == other.dataType &&
				timeSpan.start == other.timeSpan.start &&
				timeSpan.end == other.timeSpan.end &&
				location == other.location;
	}

	bool operator!=(const FetchRequest& other) {
		return !operator==(other);
	}
};

/*!
 * \brief The FetchResult struct contains a possible error
 * (FetchError:NONE if no error) and the returned dataline.
 */
struct FetchResult {
	FetchError error = FetchError::UNSET;
	DataLine dataLine = DataLine{};
};

QDebug operator<<(QDebug debug, const pwa::DataPoint& point);

QDebug operator<<(QDebug debug, const std::vector<pwa::DataPoint>& points);

}  // namespace pwa

#endif  // APITYPES_H
