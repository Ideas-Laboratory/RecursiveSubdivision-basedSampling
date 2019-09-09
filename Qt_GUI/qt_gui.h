#ifndef QT_GUI_H
#define QT_GUI_H

#include <QtWidgets>
#include "ui_qt_gui.h"

#include "utils.h"
#include "SamplingProcessViewer.h"
#include "ControlPanelWidget.h"
#include "DisplayPanelWidget.h"
#include "AdaptiveBinningSampling.h"

class Qt_GUI : public QMainWindow
{
	Q_OBJECT

public:
	Qt_GUI(QWidget *parent = 0);
	~Qt_GUI() { delete class2label; }

private:
	Ui::Qt_GUIClass ui;
	PointSet* points;
	std::unordered_map<uint, std::string>* class2label;
	SamplingProcessViewer* viewer;
	QStatusBar* status_bar;
};

#endif // QT_GUI_H
