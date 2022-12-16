/*!
	PreferenceJSON:

	{
		"plot graph type": string,
		"calculation type": string,
		"weather place": string,
		"fromDateOffset": stringInteger,
		"toDateOffset": stringInteger,
		"checkBoxStates": {
			"0": true,
			"1": false,
			"2": true
		}
	}
*/

#include "preference.hh"

namespace pwa {

/*!
	\brief Contains JSON key constants for Preference JSON.
*/
namespace preferenceJson {
	inline const QString PLOT_GRAPH_TYPE = "plot graph type";
	inline const QString CALCULATION_TYPE = "calculation type";
	inline const QString WEATHER_PLACE = "weather place";
	inline const QString FROMDATE_OFFSET = "fromDate offset";
	inline const QString TODATE_OFFSET = "toDate offset";
	inline const QString CHECKBOX_STATES = "checkbox states";
}  // namespace preferenceJson

const QString Preference::NO_PLOT_GRAPH_TYPE = "";
const QString Preference::NO_CALCULATION_TYPE = "";
const QString Preference::NO_WEATHER_PLACE = "";
const Preference::DateTimeInterval Preference::NO_DATETIME_INTERVAL = {0, 0};

Preference::Preference(const QString& saveFilePath):
		preferenceManager_{saveFilePath},
		plotGraphType_{NO_PLOT_GRAPH_TYPE},
		calculationType_{NO_CALCULATION_TYPE},
		weatherPlace_{NO_WEATHER_PLACE},
		offsetInterval_{NO_DATETIME_INTERVAL},
		checkBoxStates_{} {
}

bool Preference::read() {
	QJsonObject jsonPreference{};
	return (preferenceManager_.readTo(jsonPreference) &&
			!jsonPreference.isEmpty() && restoreWith(jsonPreference));
}

bool Preference::write() const {
	return preferenceManager_.write(asQJsonObject());
}

void Preference::setPlotGraphType(const QString& plotGraphType) {
	plotGraphType_ = plotGraphType;
}

void Preference::setCalculationType(const QString& calculationType) {
	calculationType_ = calculationType;
}

void Preference::setWeatherPlace(const QString& weatherPlace) {
	weatherPlace_ = weatherPlace;
}

void Preference::setDateTimeInterval(const QDateTime& fromDate,
		const QDateTime& currentDate, const QDateTime& toDate) {
	offsetInterval_.fromDate = currentDate.msecsTo(fromDate);
	offsetInterval_.toDate = currentDate.msecsTo(toDate);
}

void Preference::setCheckBoxStates(
		const pwa::IdnameBoolUnorderedMap& checkBoxStates) {
	checkBoxStates_ = checkBoxStates;
}

bool Preference::hasDateTimeInterval() const {
	return (offsetInterval_.fromDate != offsetInterval_.toDate);
}

QString Preference::plotGraphType() const {
	return plotGraphType_;
}

QString Preference::calculationType() const {
	return calculationType_;
}

QString Preference::weatherPlace() const {
	return weatherPlace_;
}

QDateTime Preference::fromDate(const QDateTime& currentDate) const {
	return currentDate.addMSecs(offsetInterval_.fromDate);
}

QDateTime Preference::toDate(const QDateTime& currentDate) const {
	return currentDate.addMSecs(offsetInterval_.toDate);
}

IdnameBoolUnorderedMap Preference::checkBoxStates() const {
	return checkBoxStates_;
}

bool Preference::restoreWith(const QJsonObject& preferenceJson) {
	if (!isPreferenceJsonValid(preferenceJson)) {
		return false;
	}

	restoreSingleValuesWith(preferenceJson);
	restoreCheckBoxStatesWith(preferenceJson);

	return true;
}

void Preference::restoreSingleValuesWith(const QJsonObject& preferenceJson) {
	plotGraphType_ = preferenceJson[preferenceJson::PLOT_GRAPH_TYPE].toString();
	calculationType_ =
			preferenceJson[preferenceJson::CALCULATION_TYPE].toString();
	weatherPlace_ = preferenceJson[preferenceJson::WEATHER_PLACE].toString();
	/*	toInt() would likely cause an overflow.
		toLongLong() returns 0 if string to
		longlong conversion fails. As the actual
		returned value could be 0 as well, there's
		not point at validating this.
	*/
	offsetInterval_.fromDate = preferenceJson[preferenceJson::FROMDATE_OFFSET]
									   .toString()
									   .toLongLong();
	offsetInterval_.toDate = preferenceJson[preferenceJson::TODATE_OFFSET]
									 .toString()
									 .toLongLong();
}

void Preference::restoreCheckBoxStatesWith(const QJsonObject& preferenceJson) {
	QJsonObject checkBoxStatesJson =
			preferenceJson[preferenceJson::CHECKBOX_STATES].toObject();

	checkBoxStates_.clear();
	for (const QString& idnameJsonKey : checkBoxStatesJson.keys()) {
		checkBoxStates_.try_emplace(idnameJsonKey,
				checkBoxStatesJson.value(idnameJsonKey).toBool());
	}
}

bool Preference::isPreferenceJsonValid(const QJsonObject& json) const {
	const std::vector<QString> STRING_VALUE_JSON_KEYS{
			preferenceJson::PLOT_GRAPH_TYPE, preferenceJson::CALCULATION_TYPE,
			preferenceJson::WEATHER_PLACE, preferenceJson::FROMDATE_OFFSET,
			preferenceJson::TODATE_OFFSET};
	for (const QString& stringJsonKey : STRING_VALUE_JSON_KEYS) {
		if (!JsonManager::isValidString(stringJsonKey, json)) {
			return false;
		}
	}

	return isCheckBoxStatesJsonValid(json);
}

bool Preference::isCheckBoxStatesJsonValid(const QJsonObject& json) const {
	if (!(json.contains(preferenceJson::CHECKBOX_STATES) &&
				json[preferenceJson::CHECKBOX_STATES].isObject())) {
		return false;
	}

	QJsonObject checkBoxStatesJson =
			json[preferenceJson::CHECKBOX_STATES].toObject();
	for (const QString& idnameJsonKey : checkBoxStatesJson.keys()) {
		if (!checkBoxStatesJson[idnameJsonKey].isBool()) {
			return false;
		}
	}

	return true;
}

QJsonObject Preference::asQJsonObject() const {
	QJsonObject checkBoxStates{};
	for (const auto& [idName, checkBoxIsChecked] : checkBoxStates_) {
		checkBoxStates.insert(idName, checkBoxIsChecked);
	}

	/*
		offsetInterval_ values need to be stored as
		strings in order to enable reading them back from
		JSON without overflow (toInt() vs toString().toLongLong()).
	*/
	return {{preferenceJson::PLOT_GRAPH_TYPE, plotGraphType_},
			{preferenceJson::CALCULATION_TYPE, calculationType_},
			{preferenceJson::WEATHER_PLACE, weatherPlace_},
			{preferenceJson::FROMDATE_OFFSET,
					QString::number(offsetInterval_.fromDate)},
			{preferenceJson::TODATE_OFFSET,
					QString::number(offsetInterval_.toDate)},
			{preferenceJson::CHECKBOX_STATES, checkBoxStates}};
}

}  // namespace pwa
