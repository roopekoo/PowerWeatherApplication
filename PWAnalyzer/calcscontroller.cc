#include "calcscontroller.hh"

namespace pwa {

// Each calculation type has datatypes in vector.
// Temperatures only need one source. Power needs three.
inline const std::unordered_map<CalcType, std::vector<DataType>>
		CALC_TO_DATATYPES = {
				{CalcType::POWER,
						{DataType::WIND_PWR_PROD, DataType::NUCLEAR_PWR_PROD,
								DataType::HYDRO_PWR_PROD}},
				{CalcType::AVER_TEMP, {DataType::TEMP}},
				{CalcType::MIN_MAX_AVER_TEMP, {DataType::TEMP}}};

// Preferred colors for Power% pie chart
inline const std::unordered_map<DataType, QColor> DATATYPE_TO_COLOR = {
		{DataType::WIND_PWR_PROD, Qt::blue},
		{DataType::NUCLEAR_PWR_PROD, Qt::green},
		{DataType::HYDRO_PWR_PROD, Qt::cyan}};

inline const QDateTime POWER_BEGIN_DATE =
		QDateTime(QDate(2012, 11, 27), QTime(12, 15, 0));
inline const QDateTime TEMP_BEGIN_DATE =
		QDateTime(QDate(1970, 1, 1), QTime(2, 0, 0));

// Shortest time between data updates in seconds.
const int SHORTEST_UPDATE_LENGTH_SEC = 3 * 60;
inline const QString MONTH_AND_YEAR = "MM.yyyy";

CalcsController::CalcsController(QWidget* parent):
		QObject(parent),
		fetchError_{false},
		updateNeeded_{true},
		typeChanged_{false},
		inProgress_{0},
		location_(""),
		calcType_(CalcType::NONE),
		calcsWidget_{new CalcsWidget(parent)} {
}

CalcsController::~CalcsController() = default;

void CalcsController::closeWindow() {
	calcsWidget_->closeWindow();
}

void CalcsController::openWindow() {
	selectTypeToFetch();
	calculateExistingData();
	if (!fetchError_) {
		calcsWidget_->openWindow();
	} else {
		calcsWidget_->closeWindow();
		qDebug() << "Data fetching has errored previously and the graph cannot "
					"be shown. Try changing the timeSpan or location values!";
	}
}

void CalcsController::setTimeSpan(const TimeSpan& timeSpan) {
	updateRefDates(timeSpan);
	// Check if the current month has changed for temperature data
	if (calcType_ != CalcType::POWER) {
		updateNewDatesTemp();
	}
	// Update power data newDates
	else {
		updateNewDatesPower();
	}
	// Check if equal data can be found
	if (!updateNeeded_ && typeChanged_) {
		updateNeeded_ = !equalDataTimeSpan();
	}
}

void CalcsController::validateDate(const QDateTime& beginDate) {
	// Check if the dates are earlier than first data values
	// Change startDate_ to beginDate if timeSpan is earlier
	if (refStartDate_.secsTo(beginDate) > 0) {
		newStartDate_ = beginDate;
	}
	// startDate_ is correct
	else {
		newStartDate_ = refStartDate_;
	}
	// Change endDate_ to begindate+1 day if earlier than beginDate
	if (refEndDate_.secsTo(beginDate) > 0) {
		newEndDate_ = beginDate.addDays(1);
	}
	// endDate_ is correct
	else {
		newEndDate_ = refEndDate_;
	}
}

void CalcsController::setType(const CalcType& type) {
	if (calcType_ != type) {
		calcType_ = type;
		typeChanged_ = true;
	}
}

void CalcsController::setLocation(const QString& location) {
	// Location string has been changed
	if (location_.toLower() != location.toLower()) {
		location_ = location;
	}
	if (!updateNeeded_) {
		// Check if same location exist on data_
		if (calcType_ != CalcType::POWER) {
			updateNeeded_ = !equalDataLocation();
		}
	}
}

void CalcsController::selectTypeToFetch() {
	if (updateNeeded_) {
		fetchError_ = false;
		if (calcType_ == CalcType::POWER) {
			// Loop Power datatypes and fetch
			for (DataType dataType : CALC_TO_DATATYPES.at(calcType_)) {
				fetchData(Provider::FINGRID, dataType);
			}
		} else {
			// Fetch weather data
			std::vector<TimeSpan> timeSpans =
					utils::splitTimeSpan({newStartDate_, newEndDate_}, 1);
			fetchAll(Provider::FMI, DataType::TEMP, timeSpans);
		}
		updateNeeded_ = false;
		typeChanged_ = false;
	}
}

void CalcsController::calculateExistingData() {
	if (typeChanged_) {
		fetchError_ = false;
		// Data is only calculated, fetch was skipped
		dataCalculations();
		typeChanged_ = false;
	}
}

void CalcsController::updateRefDates(const TimeSpan& timeSpan) {
	if (refStartDate_ != timeSpan.start || refEndDate_ != timeSpan.end) {
		if (calcType_ != CalcType::POWER) {
			if (refEndDate_.toString(MONTH_AND_YEAR) !=
					timeSpan.end.toString(MONTH_AND_YEAR)) {
				updateNeeded_ = true;
			}
		} else {
			updateNeeded_ = true;
		}
		refStartDate_ = timeSpan.start;
		refEndDate_ = timeSpan.end;
	}
}

void CalcsController::updateNewDatesTemp() {
	// update newDates to correct timespan despite of needing an update
	// Earliest temperature data begins on 01.01.1970 (02:00)
	validateDate(TEMP_BEGIN_DATE);
	int currMonth = newEndDate_.date().month();
	int currYear = newEndDate_.date().year();
	// First day of next month
	QDateTime endDay(QDate(currYear, currMonth + 1, 1), QTime(0, 0, 0));
	QDateTime firstDay(QDate(currYear, currMonth, 1), QTime(0, 0, 0));

	newEndDate_ = endDay;
	newStartDate_ = firstDay;
}

void CalcsController::updateNewDatesPower() {
	// Earliest wind data begins on 27.11.2012 (12:15)
	// newDates update on this function
	validateDate(POWER_BEGIN_DATE);
	// start and endDate are equal
	if (refStartDate_.secsTo(refEndDate_) == 0) {
		// Deduct 3 minutes from startDate, so fetch can find at least one
		// datapoint for each power type
		int timeDeduct = -SHORTEST_UPDATE_LENGTH_SEC;
		newStartDate_ = newStartDate_.addSecs(timeDeduct);
	}
}

bool CalcsController::equalDataTimeSpan() {
	for (auto& dataType : CALC_TO_DATATYPES.at(calcType_)) {
		if (data_.find(dataType) == data_.end()) {
			return false;
		}
		QDateTime firstDate = data_.at(dataType).front().dataPoints.front().x;
		QDateTime lastDate = data_.at(dataType).back().dataPoints.back().x;

		int timeLimit = SHORTEST_UPDATE_LENGTH_SEC;
		if (abs(newStartDate_.secsTo(firstDate)) > timeLimit) {
			return false;
		}
		if (abs(lastDate.secsTo(newEndDate_)) > timeLimit) {
			return false;
		}
	}
	return true;
}

bool CalcsController::equalDataLocation() {
	for (auto& dataType : CALC_TO_DATATYPES.at(calcType_)) {
		if (data_.find(dataType) == data_.end()) {
			return false;
		}
		if (data_.at(dataType).front().location != location_) {
			return false;
		}
	}
	return true;
}

void CalcsController::dataCalculations() {
	if (calcType_ == CalcType::POWER) {
		calculatePower();
	}
	// Type is Temperature
	else {
		calculateTemp();
	}
	calcsWidget_->openWindow();
}

void CalcsController::calculatePower() {
	// Empty initalizations
	double sum = 0;
	double average = 0;
	double procent = 0;
	std::unordered_map<DataType, PowerData> powerMap = {};
	QString unit = data_.at(DataType::HYDRO_PWR_PROD).front().yUnit;

	// Go through the Datatypes from map
	// dataType is DataType from vector.
	for (DataType dataType : CALC_TO_DATATYPES.at(calcType_)) {
		std::vector<DataPoint> DataPoints =
				data_.at(dataType).front().dataPoints;
		average = utils::averageDataPoints(DataPoints);

		// Partial insertion to the map
		powerMap.insert(
				{dataType, {average, 0, DATATYPE_TO_COLOR.at(dataType)}});
		sum += average;
	}
	// Loop for calculating percentages
	// averages is being calculated on the previous loop, so percentages
	// must be calculated later here
	for (DataType dataType : CALC_TO_DATATYPES.at(calcType_)) {
		average = powerMap.at(dataType).average;
		procent = average / sum * 100;
		powerMap.at(dataType).procent = procent;
	}
	newEndDate_ = data_.at(DataType::HYDRO_PWR_PROD).back().dataPoints.back().x;
	calcsWidget_->drawPower({newStartDate_, newEndDate_}, sum, unit, powerMap);
}

void CalcsController::calculateTemp() {
	newEndDate_ = data_.at(DataType::TEMP).back().dataPoints.back().x;

	QString unit = data_.at(DataType::TEMP).front().yUnit;

	std::vector<TempType> tempTypes = {};
	QStringList days = {};
	// Average temperatures
	tempTypes = {TempType::AVERAGE};
	// Change to min&max
	if (calcType_ == CalcType::MIN_MAX_AVER_TEMP) {
		tempTypes = {TempType::MIN, TempType::MAX};
	}
	std::map<TempType, TempData> data = {};
	calcTempTypes(tempTypes, data);
	fillDaysVector(days);
	calcsWidget_->drawTemp(newEndDate_, location_, days, unit, data);
}

void CalcsController::calcTempTypes(const std::vector<TempType>& tempTypes,
		std::map<TempType, TempData>& data) {
	// Empty values
	std::vector<double> averTemps = {};
	std::vector<DataPoint> dataPoints = {};
	double monthAverage = 0;

	for (auto& value : tempTypes) {
		averTemps = {};
		for (auto& dataLine : data_.at(DataType::TEMP)) {
			dataPoints = dataLine.dataPoints;
			if (value == TempType::AVERAGE) {
				averTemps.push_back(utils::averageDataPoints(dataPoints));
			} else if (value == TempType::MIN) {
				averTemps.push_back(utils::getDataPointsMin(dataPoints));
			} else if (value == TempType::MAX) {
				averTemps.push_back(utils::getDataPointsMax(dataPoints));
			}
		}
		monthAverage = utils::averageDoubles(averTemps);
		data.insert({value, {averTemps, monthAverage}});
	}
}

void CalcsController::fillDaysVector(QStringList& days) {
	int day = 0;
	for (auto& dataLine : data_.at(DataType::TEMP)) {
		day = dataLine.dataPoints.front().x.date().day();
		days.push_back(QString::number(day));
	}
}

void CalcsController::fetchData(
		const Provider& provider, const DataType& dataType) {
	inProgress_ += 1;
	FetchRequest request = {.provider = provider,
			.dataType = dataType,
			.timeSpan = {newStartDate_, newEndDate_},
			.location = location_};
	WebAPI::getInstance().fetch(request, [=](const FetchResult& result) {
		if (!fetchError_) {
			if (result.error != FetchError::NONE) {
				qDebug() << FETCH_ERROR_MSG.at(result.error);
				return;
			}
			saveResults({result.dataLine});
		}
		inProgress_ -= 1;
		if (inProgress_ == 0 && !fetchError_) {
			dataCalculations();
		}
	});
}

void CalcsController::fetchAll(const Provider& provider,
		const DataType& dataType, const std::vector<TimeSpan>& timeSpans) {
	QDateTime endDate = QDateTime();
	std::vector<FetchRequest> requests = {};
	for (auto& timeSpan : timeSpans) {
		// Fetched data would be incorrect because current day last moment would
		// be next day's first moment. 1 Second must be subtracted
		endDate = timeSpan.end.addSecs(-1);
		FetchRequest request = {.provider = provider,
				.dataType = dataType,
				.timeSpan = {timeSpan.start, endDate},
				.location = location_};
		requests.push_back(request);
	}
	WebAPI::getInstance().fetchAll(
			requests, [=](const std::vector<FetchResult>& results) {
				std::vector<DataLine> dataLines = ResultToDataLines(results);
				saveResults(dataLines);
				if (!fetchError_) {
					dataCalculations();
				}
			});
}

std::vector<DataLine> CalcsController::ResultToDataLines(
		const std::vector<FetchResult>& results) {
	std::vector<DataLine> dataLines = {};
	for (auto& result : results) {
		if (result.error != FetchError::NONE) {
			qDebug() << FETCH_ERROR_MSG.at(result.error);
			fetchError_ = true;
			return {};
		}
		if (!result.dataLine.dataPoints.empty()) {
			dataLines.push_back(result.dataLine);
		}
	}
	return dataLines;
}

void CalcsController::saveResults(const std::vector<DataLine>& dataLine) {
	if (!isdataLineValid(dataLine)) {
		return;
	}
	DataType dataType = dataLine.front().dataType;
	if (data_.find(dataType) != data_.end()) {
		// Replace with new data
		data_.at(dataType) = dataLine;
	} else {
		// Insert new data to map
		data_.insert({dataType, dataLine});
	}
}

bool CalcsController::isdataLineValid(const std::vector<DataLine>& dataLine) {
	// dataLine is empty
	if (dataLine.empty()) {
		qDebug() << "Error: 'To Date' value is set to the future or data is "
					"not available for selected month!";
		fetchError_ = true;
		calcsWidget_->closeWindow();
		return false;
	}
	// Dataline is not empty but datapoints is empty
	if (dataLine.front().dataPoints.empty()) {
		qDebug() << "Error: No data! Time span is set to too far to the future";
		fetchError_ = true;
		calcsWidget_->closeWindow();
		return false;
	}
	return true;
}

}  // namespace pwa
