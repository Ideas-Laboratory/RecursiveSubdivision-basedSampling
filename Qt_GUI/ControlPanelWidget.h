#pragma once
#include <QtWidgets>

#include "global.h"
#include "SamplingProcessViewer.h"

class ControlPanelWidget : public QWidget
{
public:
	explicit ControlPanelWidget(SamplingProcessViewer* viewer, PointSet* points, std::unordered_map<uint, std::string>* class2label, QWidget* parent = nullptr);

private:
	void showSaveDialog(const std::string& caption, const std::string& filter_desc, const std::string& ext, std::function<void(const QString&)> doSomeStuff); //TODO
	void addClassSelectionBox();
	void showCurrentFileName(const QString& s);
	void disableButtons();
	void enableButtons();
	
	SamplingProcessViewer* viewer;

	PointSet* points;
	std::unordered_map<uint, std::string>* class2label;
	std::unordered_map<std::string, uint> label2class;

	std::vector<QPushButton*> buttons;
	QGroupBox* class_selection_box = nullptr;

	QLabel* fn = new QLabel;
};