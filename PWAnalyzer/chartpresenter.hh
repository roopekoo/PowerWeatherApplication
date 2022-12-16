/*!
	This class needs large scale refactoring.
*/

#ifndef CHARTPRESENTER_H
#define CHARTPRESENTER_H

#include "apitypes.hh"
#include "datalinesmodel.hh"

#include <QtCharts>

namespace pwa {

class Preference;
class WebAPI;

/*!
 * \brief The ChartControls struct contains grapgic UI element pointers, that
 * are used by chartWidget and chartPresenter. Elements are created in
 * ChartWidget
 */
struct ChartControls {
	QComboBox* plotComboBox;
	QPushButton* importPreferenceButton;
	QPushButton* exportPreferenceButton;
	QPushButton* importDataButton;
	QPushButton* exportDataButton;
	QPushButton* exportImageButton;
	QPushButton* applyButton;
	QDateTimeEdit* fromDateLine;
	QDateTimeEdit* toDateLine;
	QLineEdit* weatherPlaceLine;
	QVBoxLayout* scrollBarLayout;
	QComboBox* calcComboBox;
	QLabel* errorText;
};

/*!
 * \brief The ChartType determine the plotting type of graph. The user selects
 * type from the ChartControls calcComboBox
 */
enum class ChartType { LINE, SCATTER, SIZE };

/*!
 * \brief The ChartPresenter class manages the presentation of data points to
 * user in chartView. It also handles signals from ChartControls struct.
 */
class ChartPresenter: public QObject {
	Q_OBJECT
  public:
	/*!
	 * \brief ChartPresenter costructor creates initialize chartView, x-axis and
	 * connects and handles ChartsControls signals.
	 * \param parent
	 * \param chartView
	 * \param controls
	 */
	ChartPresenter(
			QObject* parent, QChartView* chartView, ChartControls& controls);

  private:
	// Initialize ChartView and Chart. Adds chart to view and set min and max
	// values to x-axes.
	void initializeChartView();

	// Connects ChartControls buttons signals
	inline void connectButtons();

	// Connects signals of ChartControls elements that are used by
	// datalinesmodel.
	void connectModelSignals();

	// Connects signals of ChartControls elements that are used in this class.
	void connectChartControls();

	// Initializes Checkboxes from model_'s DataId vector
	void initCheckBoxes();

	// Add Checkbox to UI and connect checkbox signals
	void addCheckBox(DataLineId id, const QString& name);

	// Called if DataLine is changed
	void onDataLineChanged(DataLineId id);

	// Called if DataLine is disabled. Removes series from chart and calibrate
	// axes.
	void onDataLineDisabled(DataLineId id);

	// called if there is realtimedata to add. Adds the data to the series
	// identifed by given id.
	void onNewRealTimeData(std::vector<DataPoint> newDataPoints, DataLineId id);

	// Called if user's weatherplace is unkown. Shows error label to user.
	void onInvalidLocation(const QString& location);

	// Adds series to Chart and creates new element to currentSeries_ map where
	// id is the key. Serie type is selected by ChartType.
	void addSeriesToChart(DataLineId id, const DataLine& dataLine);

	// Remove series from chart and from currentSeries_ map.
	void removeSeriesFromChart(DataLineId id);

	// creates new series. Called by addSeriesToChart.
	inline QXYSeries* createSeries();

	// fixes user input if fromDate is bigger than toDate.
	void fixDatesOrder(QDateTimeEdit* changedDateEdit);

	// Called if user changes the chart type. Changes all series in chart to
	// given type.
	void changeChartType(ChartType toType);

	// Calibrates x-axis for current data in chart.
	void calibrateAxisX();

	// Calibrates y-axes for current data in chart.
	void calibrateAxesY();

	// creates new y axis and attach axis to serie.
	void addAxisY(DataLineId toId);



	/*!
		\brief Ask the user an export file and
		export data or display an error dialog.
		\see onImportData
		\see exportData
	*/
	void onExportData() const;

	/*!
		\brief Ask the user an import file and
		import data or display an error dialog.
		\see onExportData
		\see importData
	*/
	void onImportData();

	/*!
		\brief Export enabled DataLines as JSON
		to the file specified by filePath.
		\param filePath the path to the JSON file
		\return true if successful, false otherwise
		\see onExportData
	*/
	inline bool exportData(const QString& filePath) const;

	/*!
		\brief Import DataLines from the JSON file
		specified by the filePath, create checkboxes
		for them and enable the checkboxes. Update the GUI.
		\param filePath the path to the JSON file
		\return true if successful, false otherwise
		\see onExportData
	*/
	inline bool importData(const QString& filePath);



	/*!
		\brief Ask the user an export file and
		export preference or display an error dialog.
		\see onImportPreference
		\see exportPreference
	*/
	void onExportPreference() const;

	/*!
		\brief Ask the user an import file and
		import preference or display an error dialog.
		\see onExportPreference
		\see importPreference
	*/
	void onImportPreference();

	/*!
		\brief Export the state of the GUI
		to the file specified by filePath.
		\param filePath the path to the JSON file
		\return true if successful, false otherwise
		\see onExportPreference
	*/
	inline bool exportPreference(const QString& filePath) const;
	//	Helper functions
	inline void dateTimeIntervalTo(Preference& preference) const;
	inline void checkBoxStatesTo(Preference& preference) const;

	/*!
		\brief Import the state of the GUI
		from the file specified by filePath.
		\param filePath the path to the JSON file
		\return true if successful, false otherwise
		\see onImportPreference
	*/
	inline bool importPreference(const QString& filePath);
	//	Helper functions
	inline void importSingleItemPreferences(const Preference& preference);
	inline void importTimeSpan(const Preference& preference);
	inline void importCheckBoxStates(const Preference& preference);


	/*!
		\brief Export the Chart displaying enabled DataLines
		to the file specified by filePath, as image.
		\param filePath the path to the JSON file
		\return true if successful, false otherwise
		\see onExportPreference
	*/
	void onExportImage() const;


	const QString USER_EXPORT_IMPORT_HOME_PATH = ".";
	const QString DATALINE_JSON_QFILEDIALOG_FILTER =
			tr("DataLine JSON (*.json)");
	const QString PREFERENCE_JSON_QFILEDIALOG_FILTER =
			tr("Preference JSON (*.json)");
	const QString CHART_IMG_QFILEDIALOG_FILTER = tr("Chart image (*.png)");
	const QString WRITE_ACCESS_PROMPT =
			tr("Make sure the program has write access there.");

	// Horisontal axis that shows range from min to max of all serie values.
	QDateTimeAxis* xAxis_;

	// Struct of grapical control elements.
	ChartControls& chartControls_;

	// Stores checkBoxes with DataLineID key.
	std::unordered_map<DataLineId, QCheckBox*> checkBoxes_;

	// Stores dataseries with DataLineID key.
	std::map<DataLineId, QXYSeries*> currentSeries_;

	// Graphic element which shows dataseries
	QChart* chart_;

	// ChartView graphical view which contains chart.
	QChartView* chartView_;

	// ChartType enum currentChartType_ contains information of current chart
	// type.
	ChartType currentChartType_;

	// DataLinesModel class model_
	DataLinesModel* model_;
};

}  // namespace pwa
#endif  // CHARTPRESENTER_H
