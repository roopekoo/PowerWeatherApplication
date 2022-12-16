#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "calcscontroller.hh"
#include "chartpresenter.hh"

#include <QPushButton>
#include <QWidget>
#include <QtCharts>
#include <unordered_map>

namespace Ui {
class ChartWidget;
}  // namespace Ui

namespace pwa {
class WebAPI;

class ChartWidget: public QWidget {
	Q_OBJECT
  public:
	ChartWidget(QWidget* parent);
	~ChartWidget();

	/*!
	 * \brief mainWindowClosed passes the information forward to the
	 * calcsController that the mainWindow is closing. This is used so
	 * calcsWidget could be closed and the program quits.
	 */
	void mainWindowClosed();

  private:
	// creates top layout and create and adds ChartControls objects to this
	// layout.
	void setTopLayout(QHBoxLayout* buttonLayout);

	// creates grid layout for middle layout of window and adds chartView_ and
	// scroll bar area into it.
	void setMiddleLayout(QHBoxLayout* boxLayout);

	// creates grid layout for bottom layout of window and creates and adds
	// ChartControls objects to this layout.
	void setBottomLayout(QHBoxLayout* boxLayout);

	// Adds top, middle and bottom layout to mainlayout.
	void setMainLayout();

	void openCalcsWindow();

	// The ChartControls struct that contains pointers of graphical elements of
	// ui
	ChartControls controls_ = ChartControls{};

	// Button that opens calculation window;
	QPushButton* getCalcsButton_ = nullptr;

	// ChartView is the graphical area for graph visualitation.
	QChartView* chartView_ = nullptr;

	// ChartPresenter class manages chartview and most of buttons in windows.
	ChartPresenter* chartPresenter_ = nullptr;
	CalcsController* calcsController_;
};

}  // namespace pwa
#endif  // CHARTWIDGET_H
