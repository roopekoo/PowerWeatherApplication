#include "datalinesmodel.hh"

#include "datalinesaver.hh"
#include "utils.hh"
#include "webapi.hh"

#include <QFileDialog>

namespace pwa {

const int REAL_TIME_UPDATE_INTERVAL_MS = 2 * 60 * 1000;

DataLinesModel::DataLinesModel(QObject* parent):
		QObject{parent}, initialDataTypes_{getInitialDataTypes()} {

	for (DataLineId id = 0; id < initialDataTypes_.size(); ++id) {
		DataLineWrapper wrapper = DataLineWrapper{.id = id,
				.name = DATA_TYPE_NAMES.at(initialDataTypes_.at(id).second),
				.enabled = false,
				.dataLine = nullptr};

		dataLineWrappers_.push_back(wrapper);
		wantedDataLineEnabled_.push_back(false);
	}

	stateUpdateTimer_.setSingleShot(true);
	stateUpdateTimer_.callOnTimeout([&]() { updateState(); });

	realTimeUpdateTimer_.setSingleShot(false);
	realTimeUpdateTimer_.setInterval(REAL_TIME_UPDATE_INTERVAL_MS);
	realTimeUpdateTimer_.start();
	realTimeUpdateTimer_.callOnTimeout([&]() { realTimeUpdate(); });
}

std::vector<DataLineId> DataLinesModel::getAllIds() const {
	std::vector<DataLineId> ids(dataLineWrappers_.size(), 0);
	std::iota(ids.begin(), ids.end(), 0);
	return ids;
}

std::vector<std::pair<Provider, DataType>>
DataLinesModel::getInitialDataTypes() {
	std::vector<std::pair<Provider, DataType>> result;
	for (const auto& [provider, typeVector] :
			WebAPI::getInstance().getProviderDataTypes()) {
		for (DataType type : typeVector) {
			result.push_back({provider, type});
		}
	}
	return result;
}

std::vector<DataLineId> DataLinesModel::getInitialDataIds() const {
	std::vector<DataLineId> result(initialDataTypes_.size());
	std::iota(result.begin(), result.end(), 0);
	return result;
}

QString DataLinesModel::getDataName(DataLineId id) const {
	if (id >= dataLineWrappers_.size()) {
		return "";
	}
	return dataLineWrappers_.at(id).name;
}

std::shared_ptr<DataLine> DataLinesModel::getDataLine(DataLineId id) const {
	if (id >= dataLineWrappers_.size()) {
		return nullptr;
	}
	return dataLineWrappers_.at(id).dataLine;
}

void DataLinesModel::setTimeSpan(const TimeSpan& timeSpan) {
	timeSpan_ = timeSpan;
	stateUpdateTimer_.start(0);
	timeSpanChanged = true;
}

TimeSpan DataLinesModel::getTimeSpan() const {
	return timeSpan_;
}

void DataLinesModel::setLocation(const QString& location) {
	location_ = location;
	stateUpdateTimer_.start(0);
	locationChanged = true;
}

QString DataLinesModel::getLocation() const {
	return location_;
}

void DataLinesModel::setDataEnabled(DataLineId id, bool enabled) {
	if (id >= dataLineWrappers_.size()) {
		return;
	}
	wantedDataLineEnabled_.at(id) = enabled;
	stateUpdateTimer_.start(0);
}

bool DataLinesModel::isDataEnabled(DataLineId id) {
	if (id >= dataLineWrappers_.size()) {
		return false;
	}

	return dataLineWrappers_.at(id).enabled;
}

DataLineId DataLinesModel::addDataLine(const DataLine& dataLine) {
	DataLineId id = dataLineWrappers_.size();
	DataLineWrapper wrapper = DataLineWrapper{.id = id,
			.name = "Imported " +
					QString::number(id - initialDataTypes_.size() + 1) + ": " +
					DATA_TYPE_NAMES.at(dataLine.dataType),
			.enabled = false,
			.dataLine = std::make_shared<DataLine>(std::move(dataLine))};

	dataLineWrappers_.push_back(wrapper);
	wantedDataLineEnabled_.push_back(false);

	return id;
}

void DataLinesModel::updateState() {
	for (DataLineId id = 0; id < dataLineWrappers_.size(); ++id) {
		DataLineWrapper& currWrapper = dataLineWrappers_.at(id);

		bool wantedEnabled = wantedDataLineEnabled_.at(id);
		bool enabledChanged = wantedEnabled != currWrapper.enabled;
		bool settingsChanged = locationChanged || timeSpanChanged;

		if (enabledChanged || (wantedEnabled && settingsChanged)) {

			if (id >= initialDataTypes_.size()) {
				updateImportedWrapper(currWrapper, wantedEnabled);
			} else {
				updateInitialWrapper(currWrapper, wantedEnabled);
			}
		}
	}

	locationChanged = false;
	timeSpanChanged = false;
}

void DataLinesModel::updateImportedWrapper(
		DataLineWrapper& wrapper, bool wantedEnabled) {
	if (wantedEnabled) {
		wrapper.enabled = true;
		emit dataLineChanged(wrapper.id);
	} else {
		wrapper.enabled = false;
		emit dataLineDisabled(wrapper.id);
	}
}

void DataLinesModel::updateInitialWrapper(
		DataLineWrapper& wrapper, bool wantedEnabled) {

	if (wantedEnabled) {
		FetchRequest request = {
				.provider = initialDataTypes_.at(wrapper.id).first,
				.dataType = initialDataTypes_.at(wrapper.id).second,
				.timeSpan = timeSpan_,
				.location = location_};

		auto it = requestCache_.find(wrapper.id);
		// If cache does not contain this request or it is different
		// than cache, try fetching
		if (it == requestCache_.end() || it->second != request) {
			WebAPI::getInstance().fetch(request, [=](FetchResult res) {
				onReceiveFetch(request, std::move(res), wrapper.id);
			});
			requestCache_[wrapper.id] = request;
		}
		// Else just accept old values
		else {
			wrapper.enabled = true;
			emit dataLineChanged(wrapper.id);
		}

	} else {
		wrapper.enabled = false;
		emit dataLineDisabled(wrapper.id);
	}
}

void DataLinesModel::realTimeUpdate() {
	// Only update in real time if user selected timespan reaches the current
	// moment in time
	const QDateTime currentDateTime = QDateTime::currentDateTime();
	if (timeSpan_.end < currentDateTime) {
		return;
	}

	for (DataLineId id = 0; id < initialDataTypes_.size(); ++id) {
		DataLineWrapper& currWrapper = dataLineWrappers_.at(id);

		if (currWrapper.enabled && currWrapper.dataLine != nullptr &&
				currWrapper.dataLine->dataPoints.size() > 0 &&
				requestCache_.find(id) != requestCache_.end()) {

			// qDebug() << "realtime update dataline" << currWrapper.name;

			QDateTime start =
					currWrapper.dataLine->dataPoints.back().x.addSecs(1);

			// If this is a forecast dataline, then we need to update the whole
			// thing (forecasts get more precise over time), instead of just
			// appending datapoints to the end
			if (start > currentDateTime) {
				// qDebug() << "is forecast -> update whole thing";
				updateInitialWrapper(currWrapper, true);
				continue;
			}

			TimeSpan requestTimeSpan = {.start = start, .end = currentDateTime};

			FetchRequest request = {.provider = initialDataTypes_.at(id).first,
					.dataType = initialDataTypes_.at(id).second,
					.timeSpan = requestTimeSpan,
					.location = location_};

			FetchRequest originalRequestCache = requestCache_.at(id);

			WebAPI::getInstance().fetch(request, [=](FetchResult res) {
				// Some settings have changed if request cache is different.
				// In that case don't apply these real time updates.
				if (requestCache_.at(id) == originalRequestCache) {
					onReceiveRealTimeFetch(request, res, id);
				}
			});
		}
	}
}

void DataLinesModel::onReceiveFetch(
		const FetchRequest& origReq, const FetchResult& result, DataLineId id) {

	if (requestCache_.count(id) == 0 || id >= dataLineWrappers_.size()) {
		return;
	}

	// If this is the latest request for this id
	if (requestCache_.at(id) == origReq) {
		if (result.error == FetchError::NONE) {
			dataLineWrappers_.at(id).dataLine =
					std::make_shared<DataLine>(std::move(result.dataLine));

			// If user still wants to see this
			if (wantedDataLineEnabled_.at(id)) {
				emit dataLineChanged(id);
			}

			dataLineWrappers_.at(id).enabled = true;
		} else {
			// If user has requested non errored values before, now hide
			// datapoints to show user that error has occurred
			if (dataLineWrappers_.at(id).dataLine != nullptr) {
				dataLineWrappers_.at(id).dataLine->dataPoints = {};
				if (wantedDataLineEnabled_.at(id)) {
					emit dataLineChanged(id);
				}
				dataLineWrappers_.at(id).enabled = true;
			}

			if (result.error == FetchError::LOC_NOT_SUPPORTED_BY_PROVIDER) {
				emit invalidLocation(origReq.location);
			}
		}
	}
}

void DataLinesModel::onReceiveRealTimeFetch(
		const FetchRequest& origReq, const FetchResult& result, DataLineId id) {
	Q_UNUSED(origReq);
	if (requestCache_.count(id) == 0 || id >= dataLineWrappers_.size()) {
		return;
	}

	DataLineWrapper currWrapper = dataLineWrappers_.at(id);

	if (result.error == FetchError::NONE && currWrapper.dataLine != nullptr) {
		std::vector<DataPoint> newDataPoints = utils::findNewDataPoints(
				currWrapper.dataLine->dataPoints, result.dataLine.dataPoints);

		if (newDataPoints.size() > 0) {
			currWrapper.dataLine->dataPoints.insert(
					currWrapper.dataLine->dataPoints.end(),
					newDataPoints.begin(), newDataPoints.end());

			currWrapper.dataLine->timeSpan.end =
					currWrapper.dataLine->dataPoints.back().x;

			if (currWrapper.enabled && wantedDataLineEnabled_.at(id)) {
				emit newRealTimeData(newDataPoints, id);
			}
		}
	}
}

QString DataLinesModel::idToPersistentName(DataLineId id) {
	Q_ASSERT(id < initialDataTypes_.size());

	return PROVIDER_NAMES.at(initialDataTypes_.at(id).first) + " " +
			DATA_TYPE_NAMES.at(initialDataTypes_.at(id).second);
}

bool DataLinesModel::persistentNameToId(
		const QString& name, DataLineId& outId) {
	for (DataLineId id = 0; id < initialDataTypes_.size(); ++id) {
		if (name == idToPersistentName(id)) {
			outId = id;
			return true;
		}
	}
	return false;
}

}  // namespace pwa
