#ifndef WEBAPI_H
#define WEBAPI_H

#include "apitypes.hh"
#include "iprovider.hh"

#include <QDate>
#include <QNetworkAccessManager>
#include <QObject>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace pwa {

using ProviderMap = std::unordered_map<Provider, std::unique_ptr<IProvider>>;
using DataTypeMap = std::unordered_map<Provider, std::vector<DataType>>;

/*!
 * \brief WebAPI class aggregates all different DataLine web endpoints behind a
 * single fetch-function. Different providers handle parts of the fetch that are
 * specific to their endpoint. Operations that are common between all providers
 * are handled here with a QNetworkAccessManager. Implemented as a singleton for
 * easy access.
 */
class WebAPI: public QObject {
	Q_OBJECT
  public:
	/*!
	 * \brief Returns a singleton instance.
	 * \return Singleton instance.
	 */
	static WebAPI& getInstance();

	/*!
	 * \brief Fetches data from an API endpoint based on request.
	 * Asynchroniously calls callback in the future with results.
	 *
	 * Example usage
	 * \code
		FetchRequest request = {.provider = Provider::FMI,
				.dataType = DataType::TEMP,
				.timeSpan = {QDateTime{QDate(2020, 1, 1), QTime(0, 0, 0)},
						QDateTime{QDate(2020, 1, 5), QTime(0, 0, 0)}},
				.location = "Lohja"};

		webAPI_->fetch(request, [=](FetchResult result) {
			if (result.error != FetchError::NONE) {
				// fetch errored
				qDebug() << "fetch error: " << (int)result.error;
			} else {
				// fetch succeeded
				qDebug() << "DataPoints received:\n" <<
						result.dataLine.dataPoints;
			}
		});
	* \endcode

	 * \param request
	 * \param callback

	 */
	void fetch(const FetchRequest& request,
			const std::function<void(FetchResult)>& callback);

	/*!
	 * \brief Works like fetch, but multiple requests can be supplied. Waits for
	 * all requests to fail or succeed, then calls callback with all results.
	 * \param requests
	 * \param callback
	 */
	void fetchAll(const std::vector<FetchRequest>& requests,
			const std::function<void(std::vector<FetchResult>)>& callback);

	/*!
	 * \brief Returns unordered_map with providers as keys and vector of
	 * DataTypes as values. Represents all datatypes that are supported by each
	 * provider.
	 * \return unordered_map of Providers and their supported DataTypes.
	 */
	DataTypeMap getProviderDataTypes() const;

  private:
	// Private constructor for singleton
	WebAPI(QObject* parent = nullptr);

	// Returns a free id for parallel requests
	int getParallelRequestId();

	// Splits request up based on safeDaysPerRequest. Each timeSpan is less than
	// safeDaysPerRequest.
	std::vector<FetchRequest> splitFetchRequest(
			const FetchRequest& request, int safeDaysPerRequest);

	// Combines FetchResults. Takes ownership of results and constructs a new
	// one with all combined. Of one of the input results errored, then the
	// whole result is considered errored.
	FetchResult combineFetchResults(std::vector<FetchResult>& results);

	// Request holding structures
	std::stack<int> freeParallelRequestIds_;
	std::unordered_map<int, int> parallelRequestCounters_;
	std::unordered_map<int, std::vector<FetchResult>> parallelRequestResults_;

	// Qt provided manager for doing web requests.
	QNetworkAccessManager* manager_;

	// Holds all currently used providers
	static const ProviderMap PROVIDERS;
};

}  // namespace pwa
#endif  // WEBAPI_H
