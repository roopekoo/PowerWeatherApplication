/*!

*/

#ifndef CALCSWIDGET_HH
#define CALCSWIDGET_HH
#include "webapi.hh"

#include <QWidget>
#include <QtCharts>

namespace pwa {

class WebAPI;

/*!
 * \brief The PowerData struct for saving the data that is used for drawing
 * power percentages to pie chart.
 */
struct PowerData {
	double average;
	double procent;
	QColor color;
};

/*!
 * \brief The TempData struct is used for saving the data that is used for
 * drawing Temperature data bar chart.
 */
struct TempData {
	std::vector<double> DailyAverages = {};
	double monthlyAverage;
};

/*!
 * \brief The TempType enum is used to separate Temperature types for
 * calculating averages, minimum or maximum values from fecthed data.
 */
enum class TempType { NONE, AVERAGE, MIN, MAX };

/*!
 * \brief The CalcsWidget class draws charts from the data given by the
 * CalcsController class.
 */
class CalcsWidget: public QWidget {
	Q_OBJECT
  public:
	CalcsWidget(QWidget* parent);
	~CalcsWidget();

	/*!
	 * \brief drawPower draws a pie chart of percentages of the produced power
	 * forms (nuclear-, wind-, and hydro-energy). \param timespan has beginDate
	 * and endDate that are used to show the date span where the data if from.
	 * \param total is  the sum of all power forms total output.
	 * \param unit for total output value (Presumably always in the unit MV).
	 * \param data is a map of PowerData with a key of DataType.
	 */
	void drawPower(const TimeSpan& timespan, const double& total,
			const QString& unit,
			const std::unordered_map<DataType, PowerData>& data);

	/*!
	 * \brief drawTemp draws a bar chart with horizontal line(s) as the monthly
	 * average temperature(s) from given parameters. \param endDate used to get
	 * shown month and year \param location is the place where the temperature
	 * data was taken. \param days vector of the day numbers of a month. Same
	 * order as dailyAverages in struct TempData. \param unit for temperatures
	 * (Presumably always in the unit °C). \param data as a map where TempType
	 * is the key and has TempData.
	 */
	void drawTemp(const QDateTime& endDate, const QString& location,
			const QStringList& days, const QString& unit,
			const std::map<TempType, TempData>& data);

	/*!
	 * \brief closeWindow closes the graphics chartWindow.
	 */
	void closeWindow();

	/*!
	 * \brief openWindow opens graphics chartWindow.
	 */
	void openWindow();

  private:
	// Empties chart axis and series.
	void clearChart();

	QChartView* chartView_;
};
}  // namespace pwa
#endif  // CALCSWIDGET_HH
