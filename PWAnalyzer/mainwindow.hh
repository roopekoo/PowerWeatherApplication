#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "webapi.hh"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}  // namespace Ui
QT_END_NAMESPACE

/*!
	\brief The namespace of the project
*/
namespace pwa {

class ChartWidget;
class WebAPI;

class MainWindow: public QMainWindow {
	Q_OBJECT

  public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

  private:
	Ui::MainWindow* ui;

	ChartWidget* chartWidget_;
	void closeEvent(QCloseEvent* event);
};

}  // namespace pwa
#endif  // MAINWINDOW_H
