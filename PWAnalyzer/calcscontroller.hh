/*!

*/

#ifndef CALCSWINDOW_HH
#define CALCSWINDOW_HH

#include "apitypes.hh"
#include "calcswidget.hh"
#include "utils.hh"
#include "webapi.hh"

namespace pwa {

class WebAPI;

/*!
 * \brief The calculated types that determine the selected type to be
 * calculated. These values represent the values that are in the comboBox in the
 * main window.
 */
enum class CalcType { NONE, POWER, AVER_TEMP, MIN_MAX_AVER_TEMP };

/*!
 * \brief The CalcsController class requests data from a provider (FMI or
 * Fingrid) and calculates required information (such as averages, percentages,
 * percentages) that will be sent to calcsWidget to be visualized.
 */
class CalcsController: public QObject {
	Q_OBJECT
  public:
	CalcsController(QWidget* parent);
	~CalcsController();

	/*!
	 * \brief closeWindow closes the graphics window from CalcsWidget.
	 */
	void closeWindow();

	/*!
	 * \brief openWindow opens the graphics window from CalcsWidget.
	 */
	void openWindow();

	/*!
	 * \brief setTimeSpan sets a new timespan from CalcsWidget date values. If
	 * the data changes, new data much be fetched from the provider. Update is
	 * not needed if the timespan stays the same and the same data can be found
	 * in stored fetched data. \param timeSpan has a beginning date and end date
	 * as DateTime type. Updates the time span values for the data that will be
	 * calculated.
	 */
	void setTimeSpan(const TimeSpan& timeSpan);

	/*!
	 * \brief setType updates the type from selected comboBox from CalcsWidget.
	 * If the type changes, a boolean value will be saved for the set functions
	 * to use if a data update is required. \param type is a calculation type
	 * that is used to determine which type of data should be fetched or
	 * calculated.
	 */
	void setType(const CalcType& type);

	/*!
	 * \brief setLocation updates location value to this class. Data need to be
	 * updated only if the current calcsType is temperature related. If the same
	 * location data exists in fetched data, update is not needed. \param
	 * location is a string that is used for fetching temperature data for a
	 * given location.
	 */
	void setLocation(const QString& location);

  private:
	// Fetching correct data type based on current calcType.
	void selectTypeToFetch();

	// Calculates new data if it already exists on the data and the type has
	// changed. Fetch have been skipped on selectFetchType.
	void calculateExistingData();

	// Updates reference dates if the given timespan differs from the refDates.
	void updateRefDates(const TimeSpan& timeSpan);

	// First, validateDate is run and then newDates get updated to new values.
	// newBeginDate is set to first day of the current month at (00:00) and the
	// end date is set to the first date of the next month at 00:00.
	void updateNewDatesTemp();

	// First, validateDate is run and then newDates get updated to new values.
	void updateNewDatesPower();

	// Check and correct newDate values ​​if necessary if they are less than
	// beginDate.
	void validateDate(const QDateTime& beginDate);

	// Return true if newDate timeSpan is equal to the stored timespan for
	// current calcType, otherwise false.
	bool equalDataTimeSpan();

	// Return true if location_ matches the location stored in data_. Used only
	// for temperature calcType.
	bool equalDataLocation();

	// Fetches data from provider with certain dataType.
	void fetchData(const Provider& provider, const DataType& dataType);

	// Fetches data requested in vector with FetchRequests and outputs a single
	// vector with FetchResults. Vector of timeSpans determine one result
	// length. Return vector has results in the same order that they were
	// requested.
	void fetchAll(const Provider& provider, const DataType& dataType,
			const std::vector<TimeSpan>& timeSpans);

	// Cut FetchError out from FetchResult giving vector of DataLines.
	std::vector<DataLine> ResultToDataLines(
			const std::vector<FetchResult>& results);

	// Save dataline data to data_ map.
	void saveResults(const std::vector<DataLine>& dataLine);

	// Does calculations for the certain calcsType and provides calcsWidget data
	// to draw a chart.
	void dataCalculations();

	// Handles the calculations for the power type.
	void calculatePower();

	// Handles the calculations for the temperature type.
	void calculateTemp();

	// Goes through either one item for average month temperatures or two items
	// from the vector for minimum and maximum temperature calculations.
	void calcTempTypes(const std::vector<TempType>& tempTypes,
			std::map<TempType, TempData>& data);

	// Creates a vector that have days from 1 to end of the month.
	void fillDaysVector(QStringList& days);

	// Does error checking for the dataLine.
	bool isdataLineValid(const std::vector<DataLine>& dataLine);

	bool fetchError_;
	bool updateNeeded_;
	bool typeChanged_;
	int inProgress_;

	QString location_;
	CalcType calcType_;

	QDateTime refStartDate_;
	QDateTime refEndDate_;
	QDateTime newStartDate_;
	QDateTime newEndDate_;

	CalcsWidget* calcsWidget_;
	std::unordered_map<DataType, std::vector<DataLine>> data_;
};
}  // namespace pwa
#endif  // CALCSWINDOW_HH
