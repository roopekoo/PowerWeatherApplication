#include "webapi.hh"

#include "fingrid.hh"
#include "fmi.hh"
#include "utils.hh"

#include <QNetworkReply>

namespace pwa {

// Have to use lambda to initialize static const map with unique ptrs
const ProviderMap WebAPI::PROVIDERS = []() {
	ProviderMap providers{};
	providers[Provider::FINGRID] = std::make_unique<Fingrid>();
	providers[Provider::FMI] = std::make_unique<FMI>();
	return providers;
}();

WebAPI::WebAPI(QObject* parent):
		QObject(parent),
		freeParallelRequestIds_{},
		parallelRequestCounters_{},
		parallelRequestResults_{},
		manager_{new QNetworkAccessManager(this)} {

	qDebug() << "https works:" << QSslSocket::supportsSsl()
			 << "\nqt built with:" << QSslSocket::sslLibraryBuildVersionString()
			 << "\nloaded ssl library:" << QSslSocket::sslLibraryVersionString()
			 << '\n';

	if (!QSslSocket::supportsSsl()) {
		qCritical()
				<< "CRITICAL: Data fetching will not work "
				   "because libraries mentioned above are not found. "
				   "Consider installing openssl and ensure that its libraries "
				   "are in PATH or next to this executable. "
				   "(Needed libraries are most likely named "
				   "libcrypto-1_1-x64.dll and libssl-1_1-x64.dll)\n";
	}
}

WebAPI& WebAPI::getInstance() {
	static WebAPI instance;
	return instance;
}

void WebAPI::fetch(const FetchRequest& request,
		const std::function<void(FetchResult)>& callback) {
	Q_ASSERT(PROVIDERS.find(request.provider) != PROVIDERS.end());
	Q_ASSERT(request.dataType != DataType::UNSET);
	Q_ASSERT(
			request.timeSpan.start.isValid() && request.timeSpan.end.isValid());
	Q_ASSERT(request.timeSpan.start <= request.timeSpan.end);

	IProvider* provider = PROVIDERS.at(request.provider).get();

	if (!provider->implementsDataType(request.dataType)) {
		qDebug() << "Data type" << static_cast<int>(request.dataType)
				 << "not implemented by provider";
		callback({.error = FetchError::TYPE_NOT_IMPL_BY_PROVIDER});
		return;
	}

	int safeDaysPerRequest = provider->getSupportedDaysPerRequest(request);

	if (request.timeSpan.start.daysTo(request.timeSpan.end) >
			safeDaysPerRequest) {

		std::vector<FetchRequest> requests =
				splitFetchRequest(request, safeDaysPerRequest);
		fetchAll(requests, [=](std::vector<FetchResult> results) {
			callback(combineFetchResults(results));
		});

	} else {
		QNetworkReply* qReply =
				manager_->get(provider->createQtRequest(request));

		connect(qReply, &QNetworkReply::finished, [=]() {
			if (qReply->error() == QNetworkReply::NoError) {
				callback(provider->parseResponse(qReply, request));
			} else {
				FetchError fetchError = provider->parseError(qReply);
				if (fetchError != FetchError::UNSET) {
					callback({.error = fetchError});
					return;
				}
				// If provider can't parse the error, return a generic
				// connection error
				qDebug() << "Request error:" << qReply->error();
				qDebug() << "Status code:"
						 << qReply->attribute(
									QNetworkRequest::HttpStatusCodeAttribute);
				callback({.error = FetchError::CONNECTION_FAILED});
			}
			qReply->deleteLater();
		});
	}
}

void WebAPI::fetchAll(const std::vector<FetchRequest>& requests,
		const std::function<void(std::vector<FetchResult>)>& callback) {
	int id = getParallelRequestId();
	parallelRequestCounters_[id] = requests.size();
	parallelRequestResults_[id] =
			std::vector<FetchResult>(requests.size(), FetchResult{});

	for (size_t i = 0; i < requests.size(); ++i) {
		fetch(requests[i], [=](FetchResult result) {
			parallelRequestResults_[id][i] = std::move(result);
			--parallelRequestCounters_[id];

			if (parallelRequestCounters_[id] == 0) {
				callback(std::move(parallelRequestResults_[id]));
				freeParallelRequestIds_.push(id);
			}
		});
	}
}

DataTypeMap WebAPI::getProviderDataTypes() const {
	DataTypeMap dataTypes;
	for (const auto& [key, provider] : PROVIDERS) {
		dataTypes[key] = provider->getImplementedDataTypes();
	}
	return dataTypes;
}

int WebAPI::getParallelRequestId() {
	if (!freeParallelRequestIds_.empty()) {
		int id = freeParallelRequestIds_.top();
		freeParallelRequestIds_.pop();
		return id;
	}

	return parallelRequestCounters_.size();
}

std::vector<FetchRequest> WebAPI::splitFetchRequest(
		const FetchRequest& request, int safeDaysPerRequest) {
	std::vector<TimeSpan> timeSpans =
			utils::splitTimeSpan(request.timeSpan, safeDaysPerRequest);

	std::vector<FetchRequest> requests{};
	requests.reserve(timeSpans.size());

	for (TimeSpan& timeSpan : timeSpans) {
		requests.push_back(request);
		requests.back().timeSpan = std::move(timeSpan);
	}

	return requests;
}

FetchResult WebAPI::combineFetchResults(std::vector<FetchResult>& results) {
	Q_ASSERT(!results.empty());

	if (results.size() == 1) {
		return results.back();
	}

	// Return only error if they happened
	for (const FetchResult& res : results) {
		if (res.error != FetchError::NONE) {
			return FetchResult{.error = res.error};
		}
	}

	// Remove empty results
	results.erase(std::remove_if(results.begin(), results.end(),
						  [](FetchResult& r) {
							  return r.dataLine.dataPoints.size() == 0;
						  }),
			results.end());

	// Sort results in chronological order
	std::sort(
			results.begin(), results.end(), [](FetchResult& a, FetchResult& b) {
				return a.dataLine.dataPoints.front().x <
						b.dataLine.dataPoints.front().x;
			});

	FetchResult result{.error = FetchError::NONE,
			.dataLine = std::move(results.front().dataLine)};

	for (std::size_t i = 1; i < results.size(); ++i) {
		std::vector<DataPoint>& curPoints = results[i].dataLine.dataPoints;

		// Find first non overlapping datapoint in next FetchResult
		auto itStart = curPoints.begin();
		while (itStart != curPoints.end() &&
				itStart->x < result.dataLine.dataPoints.back().x) {
			++itStart;
		}

		// Move points into result ignoring shared values
		std::move(itStart, curPoints.end(),
				std::back_inserter(result.dataLine.dataPoints));
	}

	return result;
}

}  // namespace pwa
