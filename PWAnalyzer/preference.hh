/*!
	This is currently a specialized version
	of storing preferences as JSON. It needs
	to be refactored/generalized, as currently
	adding new preference data is pretty complicated.
*/

#ifndef PREFERENCE_HH
#define PREFERENCE_HH

#include "jsonmanager.hh"

#include <QDateTime>
#include <QString>

namespace pwa {

/*!
	\brief Data type for storing truth values
	in JSON with specific unique idname keys.
*/
using IdnameBoolUnorderedMap = std::unordered_map<QString, bool>;

/*!
	\brief Preference contains states the end user can change in the
	GUI, enabling reading and writing them from/into a file.

	You need to set the wanted preferences yourself,
	Preference doesn't fetch anything from the GUI.
	Not all preferences need to be set. The unset values
	will be written/read as default initialized.
*/
class Preference {
  public:
	/*!
		\brief Null constant for plotGraphType preference
		\see plotGraphType
	*/
	static const QString NO_PLOT_GRAPH_TYPE;
	/*!
		\brief Null constant for plotGraphType preference
		\see calculationType
	*/
	static const QString NO_CALCULATION_TYPE;
	/*!
		\brief Null constant for plotGraphType preference
		\see weatherPlace
	*/
	static const QString NO_WEATHER_PLACE;

	/*!
		\brief Constuct Preference associated with the
		saveFilePath.
		\param saveFilePath the path to the file to use
		for writing/reading the saved Preference data.
		\note Be careful not to overwrite others'
		files unknowingly.
	*/
	explicit Preference(const QString& saveFilePath);

	Preference(const Preference&) = delete;
	Preference(Preference&&) = delete;
	Preference& operator=(const Preference&) = delete;
	Preference& operator=(Preference&&) = delete;

	virtual ~Preference() = default;

	/*!
		\brief Restore Preference from the saved JSON file.
		\return true if successful, false otherwise
		\see write
		\see Preference(const QString& saveFilePath)
	*/
	bool read();

	/*!
		\brief Store Preference into the JSON file.
		\return true if successful, false otherwise
		\see read
		\see Preference(const QString& saveFilePath)
	*/
	bool write() const;

	/*
		TODO: Maybe refactor the DataLineSaver / Preference
		complex so that the API user can add an arbitrary
		number of string key - string value pairs to JSON instead
		of having the need to create a setter, getter,
		member variable and constant etc. here. I.e. this
		Preference is a special case of	a Preference, which
		shouldn't be the case. Remove these specific details
		that break the SRP.
	*/

	/*!
		\brief Set an arbitrary string as the
		plotGraphType preference.
		\param plotGraphType
		\see plotGraphType
	*/
	void setPlotGraphType(const QString& plotGraphType);

	/*!
		\brief Set an arbitrary string as the
		calculation type preference.
		\param calculationType
		\see calculationType
	*/
	void setCalculationType(const QString& calculationType);

	/*!
		\brief Set an arbitrary string as the
		weather place preference.
		\param weatherPlace
		\see weatherPlace
	*/
	void setWeatherPlace(const QString& weatherPlace);

	/*!
		\brief Set an arbitrary time interval preference
		relative to the current datetime.
		\param fromDate The start point of the time interval.
		\param currentDate The point of time to use for
		calculating how much of the relative time interval
		is before and after the current date.
		\param toDate The end point of the time interval.
		\see fromDate
		\see toDate
	*/
	void setDateTimeInterval(const QDateTime& fromDate,
			const QDateTime& currentDate, const QDateTime& toDate);

	/*!
		\brief Store an arbitrary number of truth values
		with unique idname key strings.

		The point is to store for each checkbox its
		isChecked state.
		\param checkBoxStates a collection of
		key - value pairs stating for each checkBox
		whether it should be checked or not.
		\see checkBoxStates
	*/
	void setCheckBoxStates(const IdnameBoolUnorderedMap& checkBoxStates);

	/*!
		\return true if the distance between fromDate
		and toDate is not zero, false otherwise.
		\see setDateTimeInterval
		\see fromDate
		\see toDate
	*/
	bool hasDateTimeInterval() const;

	/*!
		\return the plotGraph type preference
		\see setPlotGraphType
	*/
	QString plotGraphType() const;

	/*!
		\return the calculation type preference
		\see setCalculationType
	*/
	QString calculationType() const;

	/*!
		\return the weather place preference
		\see setWeatherPlace
	*/
	QString weatherPlace() const;

	/*!
		\brief Retrieve the start datetime of the
		dateTimeInterval in relation to currentDate.
		\param currentDate current datetime
		\return the start datetime in
		relation to current datetime.
		\see setDateTimeInterval
	*/
	QDateTime fromDate(const QDateTime& currentDate) const;

	/*!
		\brief Retrieve the end datetime of the
		dateTimeInterval in relation to currentDate.
		\param currentDate current datetime
		\return the end datetime in
		relation to current datetime.
		\see setDateTimeInterval
	*/
	QDateTime toDate(const QDateTime& currentDate) const;

	/*!
		\brief Retrieve the preference for which
		checkboxes should be checked and for which not.
		\return a collection of idname string key -
		truth value pairs stating
		for each checkBox whether it should be
		checked or not according to this Preference.
		\see setCheckBoxStates
	*/
	IdnameBoolUnorderedMap checkBoxStates() const;

  private:
	/*!
		A datetime interval relative to current datetime.
		Both values stored as milliseconds.
	*/
	struct DateTimeInterval {
		qint64 fromDate{};
		qint64 toDate{};
	};

	static const DateTimeInterval NO_DATETIME_INTERVAL;

	JsonManager preferenceManager_;

	//	String preferences
	QString plotGraphType_;
	QString calculationType_;
	QString weatherPlace_;
	//	Other preferences
	DateTimeInterval offsetInterval_;
	IdnameBoolUnorderedMap checkBoxStates_;

	/*!
		\brief Restore this Preference with preferenceJson
		\param preferenceJson
		\return	true if successfull, false otherwise
	*/
	inline bool restoreWith(const QJsonObject& preferenceJson);
	//	Helper functions
	inline void restoreSingleValuesWith(const QJsonObject& preferenceJson);
	inline void restoreCheckBoxStatesWith(const QJsonObject& preferenceJson);

	/*!
		\param preferenceJson
		\return true if the JSON can be used to restore
		Preference, false otherwise
	*/
	inline bool isPreferenceJsonValid(const QJsonObject& json) const;
	inline bool isCheckBoxStatesJsonValid(const QJsonObject& json) const;

	/*!
		\return The QJsonObject equivalent to this Preference.
	*/
	inline QJsonObject asQJsonObject() const;
};

}  // namespace pwa

#endif  //	PREFERENCE_HH
