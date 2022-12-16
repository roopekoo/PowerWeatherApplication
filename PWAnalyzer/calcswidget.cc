#include "calcswidget.hh"

namespace pwa {

const int WINDOW_WIDTH = 990;
const int WINDOW_HEIGHT = 550;
const int TICK_COUNT = 10;

inline const std::unordered_map<TempType, QString> TEMPTYPE_NAMES_LONG = {
		{TempType::NONE, "None"}, {TempType::AVERAGE, "Average temperatures"},
		{TempType::MIN, "Average min temperatures"},
		{TempType::MAX, "Average max temperatures"}};

inline const std::unordered_map<TempType, QString> TEMPTYPE_NAMES_SHORT = {
		{TempType::NONE, "None"}, {TempType::AVERAGE, "Average"},
		{TempType::MIN, "Average min"}, {TempType::MAX, "Average max"}};

CalcsWidget::CalcsWidget(QWidget* parent):
		QWidget(parent), chartView_{new QChartView} {

	chartView_->resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	chartView_->setRenderHint(QPainter::Antialiasing);
}
CalcsWidget::~CalcsWidget() = default;

void CalcsWidget::drawPower(const TimeSpan& timespan, const double& total,
		const QString& unit,
		const std::unordered_map<DataType, PowerData>& data) {
	auto series = new QPieSeries();

	// empty initalizations
	PowerData PData = {};
	DataType dataType = DataType::UNSET;

	// iter is a pair of key DataType and values PowerData
	for (auto& iter : data) {
		dataType = iter.first;
		PData = data.at(dataType);
		QPieSlice* slice = new QPieSlice(DATA_TYPE_NAMES.at(dataType) + " " +
						QString::number(PData.procent, 'f', 2) + "%",
				PData.average);
		slice->setColor(PData.color);
		slice->setLabelVisible();
		series->append(slice);
	}

	QString startDateStr = timespan.start.toString("dd.MM.yyyy");
	QString endDateStr = timespan.end.toString("dd.MM.yyyy");

	clearChart();
	QChart* chart = chartView_->chart();

	chart->addSeries(series);
	chart->setTitle(
			"Percentages of power forms<br><br>Total power production: " +
			QString::number(total, 'g', 2) + unit + "<br>" + startDateStr +
			"-" + endDateStr);
	chart->legend()->setAlignment(Qt::AlignBottom);
	chart->setAnimationOptions(QChart::SeriesAnimations);

	chartView_->setChart(chart);
}

void CalcsWidget::drawTemp(const QDateTime& endDate, const QString& location,
		const QStringList& days, const QString& unit,
		const std::map<TempType, TempData>& data) {
	clearChart();
	QChart* chart = chartView_->chart();

	auto* axisX = new QBarCategoryAxis();
	axisX->append(days);
	axisX->setTitleText("Day of month");
	auto* axisY = new QValueAxis();

	chart->addAxis(axisX, Qt::AlignBottom);
	chart->addAxis(axisY, Qt::AlignLeft);

	QBarSeries* barSeries = new QBarSeries();
	QLineSeries* lineSeries = nullptr;

	for (auto& value : data) {
		lineSeries = new QLineSeries();
		QBarSet* set = new QBarSet(TEMPTYPE_NAMES_LONG.at(value.first));
		for (auto& dataValue : value.second.DailyAverages) {
			set->append(dataValue);
		}
		barSeries->append(set);
		lineSeries->setName(TEMPTYPE_NAMES_SHORT.at(value.first) + ": " +
				QString::number(value.second.monthlyAverage, 'f', 2) + unit);

		QList<QPointF> dailyAverage;
		QPointF values;
		for (auto& day : days) {
			values.setX(day.toInt());
			values.setY(value.second.monthlyAverage);
			dailyAverage.push_back(values);
		}
		lineSeries->append(dailyAverage);
		chart->addSeries(lineSeries);
		lineSeries->attachAxis(axisY);
	}

	QString endDateStr = endDate.toString("MMM yyyy");

	chart->addSeries(barSeries);
	chart->setTitle(
			"Average temperatures in " + location + "<br>" + endDateStr);
	chart->setAnimationOptions(QChart::SeriesAnimations);
	chart->legend()->setAlignment(Qt::AlignBottom);

	barSeries->attachAxis(axisX);
	barSeries->attachAxis(axisY);
	// Apply nice number/axel formatting
	axisY->applyNiceNumbers();
	axisY->setMinorTickCount(TICK_COUNT);
	axisY->setGridLineColor(Qt::black);
	axisY->setTitleText(unit);

	chartView_->setChart(chart);
}

void CalcsWidget::closeWindow() {
	chartView_->close();
}
void CalcsWidget::openWindow() {
	chartView_->show();
	chartView_->activateWindow();
}

void CalcsWidget::clearChart() {
	QChart* chart = chartView_->chart();

	// remove current axis
	QList axels = chart->axes();
	for (auto& axis : axels) {
		chart->removeAxis(axis);
		delete axis;
	}

	// Clear pervious chart from memory to avoid leaks
	chart->removeAllSeries();
}
};  // namespace pwa
