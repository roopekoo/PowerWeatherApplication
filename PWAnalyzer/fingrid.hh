#ifndef FINGRID_H
#define FINGRID_H

#include "apitypes.hh"
#include "iprovider.hh"

#include <unordered_map>
#include <unordered_set>

namespace pwa {

/*!
 * \brief Provider for the Fingrid electricity statistics API.
 */
class Fingrid: public IProvider {
  public:
	Fingrid();

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
	static const QString API_KEY;

	static const QString URL_TEMPLATE;

	static const std::unordered_map<DataType, int> DATA_TYPE_IDS;

	static const std::unordered_set<DataType> REAL_TIME_DATA_TYPES;

	static const int SAFE_DAYS_PER_REALTIME_REQUEST;
	static const int SAFE_DAYS_PER_OTHER_REQUEST;

	static const QString UNIT;
};

}  // namespace pwa
#endif  // FINGRID_H
