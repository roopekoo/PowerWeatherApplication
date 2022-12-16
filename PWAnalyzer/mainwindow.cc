#include "mainwindow.hh"

#include "chartwidget.hh"
#include "ui_mainwindow.h"
#include "webapi.hh"

namespace pwa {

MainWindow::MainWindow(QWidget* parent):
		QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	chartWidget_ = new ChartWidget(this);
	ui->centralwidget->layout()->addWidget(chartWidget_);
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event) {
	chartWidget_->mainWindowClosed();
	event->accept();
}

}  // namespace pwa
