#include "qt_gui.h"

Qt_GUI::Qt_GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	class2label = new std::unordered_map<uint, std::string>();
	points = readDataset(MY_DATASET_FILENAME, class2label); // read from file

	layout()->setSizeConstraint(QLayout::SetFixedSize);

	viewer = new SamplingProcessViewer(points, this);
	viewer->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	setCentralWidget(viewer);

	QDockWidget* dock_widget = new QDockWidget("Control Panel", this);
	dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::LeftDockWidgetArea, dock_widget);

	ControlPanelWidget* cp = new ControlPanelWidget(viewer, points, class2label, dock_widget);
	dock_widget->setWidget(cp);

	QDockWidget* display_dock_widget = new QDockWidget("Display Panel", this);
	display_dock_widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, display_dock_widget);

	QScrollArea *sa = new QScrollArea(display_dock_widget);
	DisplayPanelWidget* dp = new DisplayPanelWidget(viewer, class2label, sa);
	sa->setWidget(dp);
	sa->setWidgetResizable(true);
	sa->setFixedWidth(470);
	sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	display_dock_widget->setWidget(sa);

	status_bar = new QStatusBar(this);
	status_bar->setSizeGripEnabled(false);
	setStatusBar(status_bar);

	connect(viewer, &SamplingProcessViewer::iterationStatus,
		[this](int iteration, int numberPoints, int splits) {
		status_bar->showMessage(
			"Iteration: " + QString::number(iteration) +
			" | Number points: " + QString::number(numberPoints) +
			" | Splits: " + QString::number(splits));
	});
	connect(viewer, &SamplingProcessViewer::inputImageChanged,
		[this]() { status_bar->clearMessage(); });

	setWindowTitle(viewer->ALGORITHM_NAME.c_str());
	//set position on screen
	move(200, 200);
}