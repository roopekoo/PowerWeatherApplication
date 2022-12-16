#include "chartwidget.hh"

namespace pwa {

const QString DEFAULT_WEATHER_PLACE = "Helsinki";
const QDateTime DEFAULT_FROM_DATE = QDateTime::currentDateTime().addDays(-1);
const QDateTime DEFAULT_TO_DATE = QDateTime::currentDateTime().addDays(1);

ChartWidget::ChartWidget(QWidget* parent):
		QWidget(parent), calcsController_{new CalcsController(this)} {

	setMainLayout();
	// ChartControls c;
	// Need to initialize here because presenter need ui to be done
	chartPresenter_ = new ChartPresenter{this, chartView_, controls_};

	connect(getCalcsButton_, &QPushButton::clicked, this,
			&ChartWidget::openCalcsWindow);
}

ChartWidget::~ChartWidget() {
}

void ChartWidget::openCalcsWindow() {

	int index = controls_.calcComboBox->currentIndex();
	// Combobox does not include NONE type, so index must be added by one
	pwa::CalcType type = static_cast<pwa::CalcType>(index + 1);
	calcsController_->setType(type);

	TimeSpan timeSpan = {controls_.fromDateLine->dateTime(),
			controls_.toDateLine->dateTime()};
	calcsController_->setTimeSpan(timeSpan);

	QString location = controls_.weatherPlaceLine->text();
	calcsController_->setLocation(location);
	calcsController_->openWindow();
}

void ChartWidget::mainWindowClosed() {
	calcsController_->closeWindow();
}

void ChartWidget::setTopLayout(QHBoxLayout* buttonLayout) {
	{
		const int TOP_LAYOUT_GENERAL_BUTTON_WIDTH = 150;

		controls_.plotComboBox = new QComboBox();
		controls_.plotComboBox->addItem("Line Graph");
		controls_.plotComboBox->addItem("Scatter Graph");
		controls_.plotComboBox->setFixedWidth(TOP_LAYOUT_GENERAL_BUTTON_WIDTH);

		controls_.importPreferenceButton = new QPushButton("Import Preference");
		controls_.importPreferenceButton->setFixedWidth(
				TOP_LAYOUT_GENERAL_BUTTON_WIDTH);

		controls_.exportPreferenceButton = new QPushButton("Export Preference");
		controls_.exportPreferenceButton->setFixedWidth(
				TOP_LAYOUT_GENERAL_BUTTON_WIDTH);

		controls_.importDataButton = new QPushButton("Import Data");
		controls_.importDataButton->setFixedWidth(
				TOP_LAYOUT_GENERAL_BUTTON_WIDTH);

		controls_.exportDataButton = new QPushButton("Export Data");
		controls_.exportDataButton->setFixedWidth(
				TOP_LAYOUT_GENERAL_BUTTON_WIDTH);

		controls_.exportImageButton = new QPushButton("Export Image");
		controls_.exportImageButton->setFixedWidth(
				TOP_LAYOUT_GENERAL_BUTTON_WIDTH);
	}

	{
		buttonLayout->addStretch();
		buttonLayout->addWidget(controls_.plotComboBox);
		buttonLayout->addWidget(controls_.importPreferenceButton);
		buttonLayout->addWidget(controls_.exportPreferenceButton);
		buttonLayout->addWidget(controls_.importDataButton);
		buttonLayout->addWidget(controls_.exportDataButton);
		buttonLayout->addWidget(controls_.exportImageButton);
	}
}

void ChartWidget::setMiddleLayout(QHBoxLayout* middlelayout) {

	const int SCROLLBAR_WIDTH = 220;

	QFrame* frame = new QFrame;
	controls_.scrollBarLayout = new QVBoxLayout(frame);

	QScrollArea* scrollarea = new QScrollArea;
	scrollarea->setWidget(frame);
	scrollarea->setWidgetResizable(true);
	scrollarea->setAlignment(Qt::AlignTop);
	scrollarea->setFixedWidth(SCROLLBAR_WIDTH);

	chartView_ = new QChartView;
	middlelayout->addWidget(scrollarea);
	middlelayout->addWidget(chartView_);
}

void ChartWidget::setBottomLayout(QHBoxLayout* bottomLayout) {

	QLabel* calcLabel = new QLabel("Select calculation type");
	controls_.calcComboBox = new QComboBox;
	controls_.calcComboBox->addItem("Power%");
	controls_.calcComboBox->addItem("Average temperature");
	controls_.calcComboBox->addItem("Average min&max temperatures");
	getCalcsButton_ = new QPushButton("Get calculations");

	QLabel* weatherPlace = new QLabel("Weather Place");
	controls_.weatherPlaceLine = new QLineEdit;
	controls_.weatherPlaceLine->setFixedWidth(200);
	controls_.weatherPlaceLine->setText(DEFAULT_WEATHER_PLACE);
	controls_.errorText = new QLabel();
	controls_.errorText->setVisible(false);
	controls_.errorText->setAlignment(Qt::AlignHCenter);
	controls_.errorText->setStyleSheet("color : red;");

	QGridLayout* leftGridLayout = new QGridLayout;
	leftGridLayout->addWidget(calcLabel, 0, 0, 1, 1);
	leftGridLayout->addWidget(controls_.calcComboBox, 1, 0, 1, 1);
	leftGridLayout->addWidget(getCalcsButton_, 2, 0, 1, 1);
	leftGridLayout->addWidget(weatherPlace, 0, 1, 1, 1);
	leftGridLayout->addWidget(controls_.weatherPlaceLine, 1, 1, 1, 1);
	leftGridLayout->addWidget(controls_.errorText, 2, 1, 1, 1);

	QLabel* fromDate = new QLabel("From Date");
	fromDate->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	QLabel* toDate = new QLabel("To Date");
	toDate->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	controls_.fromDateLine = new QDateTimeEdit;
	controls_.fromDateLine->setDateTime(DEFAULT_FROM_DATE);
	controls_.toDateLine = new QDateTimeEdit;
	controls_.toDateLine->setDateTime(DEFAULT_TO_DATE);
	controls_.applyButton = new QPushButton("Apply");
	controls_.applyButton->setFixedWidth(150);

	QGridLayout* rightGRidLayout = new QGridLayout;
	rightGRidLayout->addWidget(fromDate, 0, 0, 1, 1);
	rightGRidLayout->addWidget(toDate, 0, 1, 1, 1);
	rightGRidLayout->addWidget(controls_.fromDateLine, 1, 0, 1, 1);
	rightGRidLayout->addWidget(controls_.toDateLine, 1, 1, 1, 1);
	rightGRidLayout->addWidget(controls_.applyButton, 1, 2, 1, 1);

	bottomLayout->addLayout(leftGridLayout);
	bottomLayout->addStretch();
	bottomLayout->addLayout(rightGRidLayout);
}

void ChartWidget::setMainLayout() {

	QHBoxLayout* topLayout = new QHBoxLayout;
	QHBoxLayout* middleLayout = new QHBoxLayout;
	QHBoxLayout* bottomLayout = new QHBoxLayout;

	setTopLayout(topLayout);
	setMiddleLayout(middleLayout);
	setBottomLayout(bottomLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(middleLayout);
	mainLayout->addLayout(bottomLayout);

	this->setLayout(mainLayout);
}

}  // namespace pwa
