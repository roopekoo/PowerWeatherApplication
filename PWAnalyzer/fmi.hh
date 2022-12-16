#ifndef FMI_HH
#define FMI_HH

#include "apitypes.hh"
#include "iprovider.hh"

#include <unordered_map>
#include <unordered_set>

namespace pwa {

/*!
 * \brief Provider for the Finnish Meteorological Institute weather data API.
 */
class FMI: public IProvider {
  public:
	FMI();

	virtual int getSupportedDaysPerRequest(
			const FetchRequest& fetchRequest) const override;

	virtual QNetworkRequest createQtRequest(
			const FetchRequest& fetchRequest) const override;

	virtual FetchResult parseResponse(QNetworkReply* response,
			const FetchRequest& originalRequest) const override;

	virtual std::vector<DataType> getImplementedDataTypes() const override;

	virtual bool implementsDataType(DataType dataType) const override;

	virtual FetchError parseError(QNetworkReply* response) const override;

  private:
	QStringRef getStringRefBetween(const QString& input, const QString& start,
			const QString& end) const;

	static const QString URL_TEMPLATE;

	static const QString FORECAST_QUERY_ID;
	static const QString OBSERVATION_QUERY_ID;

	static const std::unordered_map<DataType, QString> DATA_TYPE_PARAM_NAMES;

	static const std::unordered_map<DataType, QString> UNITS;

	static const std::unordered_set<DataType> FORECAST_DATA_TYPES;

	static const int SAFE_DAYS_PER_REQUEST;
	static const int SAFE_DAYS_PER_FORECAST_REQUEST;

	static const QString INVALID_LOCATION_TEXT;
};

}  // namespace pwa

#endif  // FMI_HH
