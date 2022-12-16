#include "chartpresenter.hh"

#include "datalinesaver.hh"
#include "preference.hh"
#include "webapi.hh"

#include <QMessageBox>
#include <algorithm>

namespace pwa {

inline const QString QFILEDIALOG_CANCEL_PATH = "";

ChartPresenter::ChartPresenter(
		QObject* parent, QChartView* chartView, ChartControls& controls):
		QObject{parent},
		xAxis_{new QDateTimeAxis},
		chartControls_{controls},
		chartView_{chartView},
		currentChartType_{ChartType::LINE},
		model_{new DataLinesModel{this}} {

	xAxis_->setTitleText("Time");

	// Init model values
	model_->setTimeSpan({chartControls_.fromDateLine->dateTime(),
			chartControls_.toDateLine->dateTime()});
	model_->setLocation(chartControls_.weatherPlaceLine->text());

	initCheckBoxes();
	connectModelSignals();
	connectChartControls();
	initializeChartView();
}

void ChartPresenter::initCheckBoxes() {
	for (DataLineId id : model_->getInitialDataIds()) {
		addCheckBox(id, model_->getDataName(id));
	}
}

void ChartPresenter::addCheckBox(DataLineId id, const QString& name) {
	QCheckBox* checkBox = new QCheckBox(name);
	checkBoxes_[id] = checkBox;
	chartControls_.scrollBarLayout->addWidget(checkBox);
	connect(checkBox, &QCheckBox::stateChanged, [=](int state) {
		if (state == Qt::Checked) {
			model_->setDataEnabled(id, true);
		} else {
			model_->setDataEnabled(id, false);
		}
	});
}

void ChartPresenter::connectModelSignals() {
	connect(chartControls_.applyButton, &QPushButton::clicked, [&]() {
		model_->setTimeSpan({chartControls_.fromDateLine->dateTime(),
				chartControls_.toDateLine->dateTime()});
	});

	connect(chartControls_.weatherPlaceLine, &QLineEdit::editingFinished,
			[&]() {
				model_->setLocation(chartControls_.weatherPlaceLine->text());
			});

	connect(model_, &DataLinesModel::dataLineChanged, this,
			&ChartPresenter::onDataLineChanged);
	connect(model_, &DataLinesModel::dataLineDisabled, this,
			&ChartPresenter::onDataLineDisabled);
	connect(model_, &DataLinesModel::newRealTimeData, this,
			&ChartPresenter::onNewRealTimeData);
	connect(model_, &DataLinesModel::invalidLocation, this,
			&ChartPresenter::onInvalidLocation);
}

void ChartPresenter::connectChartControls() {
	connectButtons();

	{
		connect(chartControls_.weatherPlaceLine, &QLineEdit::textChanged,
				[&]() { chartControls_.errorText->setVisible(false); });

		connect(chartControls_.fromDateLine, &QDateTimeEdit::dateTimeChanged,
				[&]() { fixDatesOrder(chartControls_.fromDateLine); });
		connect(chartControls_.toDateLine, &QDateTimeEdit::dateTimeChanged,
				[&]() { fixDatesOrder(chartControls_.toDateLine); });
	}

	connect(chartControls_.plotComboBox,
			QOverload<int>::of(&QComboBox::currentIndexChanged),
			[=](int index) {
				if (index >= static_cast<int>(ChartType::SIZE)) {
					qDebug() << index << " ChartType not supported";
				} else {
					changeChartType(static_cast<ChartType>(index));
				}
			});
}

void ChartPresenter::connectButtons() {
	connect(chartControls_.importPreferenceButton, &QPushButton::clicked, this,
			&ChartPresenter::onImportPreference);
	connect(chartControls_.exportPreferenceButton, &QPushButton::clicked, this,
			&ChartPresenter::onExportPreference);

	connect(chartControls_.importDataButton, &QPushButton::clicked, this,
			&ChartPresenter::onImportData);
	connect(chartControls_.exportDataButton, &QPushButton::clicked, this,
			&ChartPresenter::onExportData);

	connect(chartControls_.exportImageButton, &QPushButton::clicked, this,
			&ChartPresenter::onExportImage);
}

void ChartPresenter::onDataLineChanged(DataLineId id) {
	std::shared_ptr<DataLine> changedDataLine = model_->getDataLine(id);
	if (changedDataLine == nullptr) {
		return;
	}

	addSeriesToChart(id, *changedDataLine);

	calibrateAxisX();
	calibrateAxesY();
}

void ChartPresenter::onDataLineDisabled(DataLineId id) {
	removeSeriesFromChart(id);
	calibrateAxisX();
	calibrateAxesY();
}

void ChartPresenter::onNewRealTimeData(
		std::vector<DataPoint> newDataPoints, DataLineId id) {

	if (newDataPoints.empty() ||
			currentSeries_.find(id) == currentSeries_.end()) {
		return;
	}

	QXYSeries* series = currentSeries_.at(id);

	for (DataPoint& point : newDataPoints) {
		series->append(point.x.toMSecsSinceEpoch(), point.y);
	}

	// Make sure that chart updates itself
	chart_->removeSeries(series);
	chart_->addSeries(series);

	calibrateAxisX();
	calibrateAxesY();
}

void ChartPresenter::onInvalidLocation(const QString& location) {
	chartControls_.errorText->setText("Not supported: " + location);
	chartControls_.errorText->setVisible(true);
}

void ChartPresenter::addSeriesToChart(DataLineId id, const DataLine& dataLine) {
	if (currentSeries_.find(id) != currentSeries_.end()) {
		removeSeriesFromChart(id);
	}

	QXYSeries* series = createSeries();

	series->setName(DATA_TYPE_NAMES.at(dataLine.dataType));
	std::for_each(dataLine.dataPoints.begin(), dataLine.dataPoints.end(),
			[&series](const DataPoint& point) {
				series->append(point.x.toMSecsSinceEpoch(), point.y);
			});

	chart_->addSeries(series);
	series->attachAxis(xAxis_);
	currentSeries_[id] = series;
}

void ChartPresenter::removeSeriesFromChart(DataLineId id) {
	auto it = currentSeries_.find(id);
	if (it == currentSeries_.end()) {
		return;
	}
	chart_->removeSeries(it->second);
	delete it->second;
	currentSeries_.erase(it);
}

void ChartPresenter::fixDatesOrder(QDateTimeEdit* changedDateEdit) {
	QDateTime fromDate = chartControls_.fromDateLine->dateTime();
	QDateTime toDate = chartControls_.toDateLine->dateTime();

	// If fromDate is larger than toDate, just set them both to the edited value
	if (fromDate > toDate) {
		QDateTime changedDateTime = changedDateEdit->dateTime();
		chartControls_.fromDateLine->setDateTime(changedDateTime);
		chartControls_.toDateLine->setDateTime(changedDateTime);
	}
}

void ChartPresenter::changeChartType(ChartType toType) {
	currentChartType_ = toType;
	std::vector<DataLineId> currentIds{};
	for (auto& [id, series] : currentSeries_) {
		currentIds.push_back(id);
	}

	for (DataLineId id : currentIds) {
		onDataLineChanged(id);
	}
}

QXYSeries* ChartPresenter::createSeries() {
	switch (currentChartType_) {
	case ChartType::LINE:
		return new QLineSeries;
	case ChartType::SCATTER: {
		QScatterSeries* scatterSeries = new QScatterSeries;
		scatterSeries->setBorderColor(QColor(0, 0, 0, 0));
		scatterSeries->setMarkerSize(5);
		return scatterSeries;
	}
	default:
		throw std::runtime_error("currentChartType_ invalid value");
	}
}

void ChartPresenter::initializeChartView() {

	// initialize Chart
	chart_ = new QChart;
	chartView_->setChart(chart_);
	chartView_->setRenderHint(QPainter::Antialiasing);

	// initialize xAxis
	chart_->addAxis(xAxis_, Qt::AlignBottom);
	xAxis_->setMax(chartControls_.toDateLine->dateTime());
	xAxis_->setMin(chartControls_.fromDateLine->dateTime());
}

void ChartPresenter::calibrateAxisX() {

	// Find largest timeSpan that fits all series'
	long long earliest = std::numeric_limits<long long>::max();
	long long latest = 0;

	for (auto& [id, series] : currentSeries_) {
		std::vector<DataPoint>& points = model_->getDataLine(id)->dataPoints;
		if (!points.empty()) {
			earliest = qMin(earliest, points.begin()->x.toMSecsSinceEpoch());
			latest = qMax(latest, (--points.end())->x.toMSecsSinceEpoch());
		}
	}

	// Never show beyond user selected timespan
	TimeSpan selectedTimeSpan = model_->getTimeSpan();
	earliest = qMax(earliest, selectedTimeSpan.start.toMSecsSinceEpoch());
	latest = qMin(latest, selectedTimeSpan.end.toMSecsSinceEpoch());

	if (chart_->series().isEmpty()) {
		xAxis_->setMin(selectedTimeSpan.start);
		xAxis_->setMax(selectedTimeSpan.end);
	} else {
		xAxis_->setMin(QDateTime::fromMSecsSinceEpoch(earliest));
		xAxis_->setMax(QDateTime::fromMSecsSinceEpoch(latest));
	}
}

void ChartPresenter::calibrateAxesY() {
	//	Delete all except x-axis which is at list[0]
	while (chart_->axes().size() > 1) {
		QAbstractAxis* axis = chart_->axes().last();
		chart_->removeAxis(axis);
		delete axis;
	}

	// Add axes only if there are 1 or 2 datalines currently on display
	if (currentSeries_.size() <= 2) {
		for (auto& [id, series] : currentSeries_) {
			addAxisY(id);
		}
	}
}

void ChartPresenter::addAxisY(DataLineId toId) {
	Q_ASSERT(chart_->axes().size() < 3);

	// Always set alignment of first axis to left and second to right
	Qt::AlignmentFlag alignment = Qt::AlignLeft;
	if (currentSeries_.at(toId) != chart_->series().first()) {
		alignment = Qt::AlignRight;
	}

	QValueAxis* axisY = new QValueAxis();
	axisY->setTitleText(model_->getDataName(toId) + " / " +
			model_->getDataLine(toId)->yUnit);
	chart_->addAxis(axisY, alignment);
	currentSeries_.at(toId)->attachAxis(axisY);
}

void ChartPresenter::onExportData() const {
	const QString IMPORT_CAPTION = tr("Export data");
	QString filePath = QFileDialog::getSaveFileName(nullptr, IMPORT_CAPTION,
			USER_EXPORT_IMPORT_HOME_PATH, DATALINE_JSON_QFILEDIALOG_FILTER);

	if ((filePath != QFILEDIALOG_CANCEL_PATH) && !exportData(filePath)) {
		const QString DIALOG_TITLE = tr("Failed to export data");
		const QString DIALOG_MESSAGE = WRITE_ACCESS_PROMPT;
		QMessageBox::information(nullptr, DIALOG_TITLE, DIALOG_MESSAGE);
	}
}

void ChartPresenter::onImportData() {
	const QString IMPORT_CAPTION = tr("Import data");
	QString filePath = QFileDialog::getOpenFileName(nullptr, IMPORT_CAPTION,
			USER_EXPORT_IMPORT_HOME_PATH, DATALINE_JSON_QFILEDIALOG_FILTER,
			nullptr, QFileDialog::ReadOnly);

	if ((filePath != QFILEDIALOG_CANCEL_PATH) && !importData(filePath)) {
		const QString DIALOG_TITLE = tr("Failed to import data");
		const QString DIALOG_MESSAGE =
				tr("Make sure the program has access to proper DataLine JSON.\n"
				   "For example, not Preference JSON.");
		QMessageBox::information(nullptr, DIALOG_TITLE, DIALOG_MESSAGE);
	}
}

bool ChartPresenter::exportData(const QString& filePath) const {
	std::vector<std::shared_ptr<DataLine>> exportDataLines{};
	for (DataLineId id : model_->getAllIds()) {
		if (model_->isDataEnabled(id)) {
			auto dataLine = model_->getDataLine(id);
			if (dataLine == nullptr) {
				continue;
			}
			exportDataLines.push_back(dataLine);
		}
	}

	DataLineSaver dataLineSaver{filePath};
	return dataLineSaver.write(exportDataLines);
}

bool ChartPresenter::importData(const QString& filePath) {
	std::vector<DataLine> importDataLines{};
	{
		DataLineSaver dataLineSaver{filePath};
		if (!dataLineSaver.readTo(importDataLines)) {
			return false;
		}
	}

	for (const DataLine& dataLine : importDataLines) {
		DataLineId id = model_->addDataLine(dataLine);
		addCheckBox(id, model_->getDataName(id));
		checkBoxes_.at(id)->setChecked(true);

		// Make sure that whole imported data is shown
		chartControls_.fromDateLine->setDateTime(
				qMin(chartControls_.fromDateLine->dateTime(),
						dataLine.timeSpan.start));
		chartControls_.toDateLine->setDateTime(qMax(
				chartControls_.toDateLine->dateTime(), dataLine.timeSpan.end));
	}
	model_->setTimeSpan({chartControls_.fromDateLine->dateTime(),
			chartControls_.toDateLine->dateTime()});

	return true;
}

void ChartPresenter::onExportPreference() const {
	const QString IMPORT_CAPTION = tr("Export preference");
	QString filePath = QFileDialog::getSaveFileName(nullptr, IMPORT_CAPTION,
			USER_EXPORT_IMPORT_HOME_PATH, PREFERENCE_JSON_QFILEDIALOG_FILTER);

	if ((filePath != QFILEDIALOG_CANCEL_PATH) && !exportPreference(filePath)) {
		const QString DIALOG_TITLE = tr("Failed to export preference");
		const QString DIALOG_MESSAGE = WRITE_ACCESS_PROMPT;
		QMessageBox::information(nullptr, DIALOG_TITLE, DIALOG_MESSAGE);
	}
}

void ChartPresenter::onImportPreference() {
	const QString IMPORT_CAPTION = tr("Import preference");
	QString filePath = QFileDialog::getOpenFileName(nullptr, IMPORT_CAPTION,
			USER_EXPORT_IMPORT_HOME_PATH, PREFERENCE_JSON_QFILEDIALOG_FILTER,
			nullptr, QFileDialog::ReadOnly);

	if ((filePath != QFILEDIALOG_CANCEL_PATH) && !importPreference(filePath)) {
		const QString DIALOG_TITLE = tr("Failed to import preference");
		const QString DIALOG_MESSAGE = tr(
				"Make sure the program has access to proper Preference JSON.\n"
				"For example, not DataLine JSON.");
		QMessageBox::information(nullptr, DIALOG_TITLE, DIALOG_MESSAGE);
	}
}

bool ChartPresenter::exportPreference(const QString& filePath) const {
	Preference preference{filePath};

	preference.setPlotGraphType(chartControls_.plotComboBox->currentText());
	preference.setCalculationType(chartControls_.calcComboBox->currentText());
	preference.setWeatherPlace(model_->getLocation());
	dateTimeIntervalTo(preference);
	checkBoxStatesTo(preference);

	return preference.write();
}

void ChartPresenter::dateTimeIntervalTo(Preference& preference) const {
	TimeSpan timeSpan = model_->getTimeSpan();
	preference.setDateTimeInterval(
			timeSpan.start, QDateTime::currentDateTime(), timeSpan.end);
}

void ChartPresenter::checkBoxStatesTo(Preference& preference) const {
	IdnameBoolUnorderedMap checkBoxStates{};
	for (DataLineId id : model_->getInitialDataIds()) {
		checkBoxStates.try_emplace(
				model_->idToPersistentName(id), model_->isDataEnabled(id));
	}
	preference.setCheckBoxStates(checkBoxStates);
}

bool ChartPresenter::importPreference(const QString& filePath) {
	Preference preference{filePath};
	if (!preference.read()) {
		return false;
	}

	importSingleItemPreferences(preference);
	importTimeSpan(preference);
	importCheckBoxStates(preference);

	return true;
}

void ChartPresenter::importSingleItemPreferences(const Preference& preference) {
	chartControls_.plotComboBox->setCurrentText(preference.plotGraphType());
	chartControls_.calcComboBox->setCurrentText(preference.calculationType());
	chartControls_.weatherPlaceLine->setText(preference.weatherPlace());
	model_->setLocation(preference.weatherPlace());
}

void ChartPresenter::importTimeSpan(const Preference& preference) {
	if (preference.hasDateTimeInterval()) {
		QDateTime currentDateTime = QDateTime::currentDateTime();
		chartControls_.fromDateLine->setDateTime(
				preference.fromDate(currentDateTime));
		chartControls_.toDateLine->setDateTime(
				preference.toDate(currentDateTime));
		model_->setTimeSpan({preference.fromDate(currentDateTime),
				preference.toDate(currentDateTime)});
	}
}

void ChartPresenter::importCheckBoxStates(const Preference& preference) {
	for (const auto& idnameStatePair : preference.checkBoxStates()) {
		DataLineId id;
		if (model_->persistentNameToId(idnameStatePair.first, id)) {
			checkBoxes_.at(id)->setChecked(idnameStatePair.second);
		}
	}
}

void ChartPresenter::onExportImage() const {
	const QString IMPORT_CAPTION = tr("Export image of chart");
	QString filePath = QFileDialog::getSaveFileName(nullptr, IMPORT_CAPTION,
			USER_EXPORT_IMPORT_HOME_PATH, CHART_IMG_QFILEDIALOG_FILTER);

	if (filePath != QFILEDIALOG_CANCEL_PATH &&
			!chartView_->grab().save(filePath)) {
		const QString DIALOG_TITLE = tr("Failed to export as image");
		const QString DIALOG_MESSAGE = WRITE_ACCESS_PROMPT;
		QMessageBox::information(nullptr, DIALOG_TITLE, DIALOG_MESSAGE);
	}
}

}  // namespace pwa
