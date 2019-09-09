#pragma once
#include <QtWidgets>

#include "global.h"
#include "SamplingProcessViewer.h"

extern std::vector<int> selected_class_order;

class DisplayPanelWidget : public QWidget
{
public:
	explicit DisplayPanelWidget(SamplingProcessViewer* viewer, std::unordered_map<uint, std::string>* class2label, QWidget* parent = nullptr);

protected:
	void resizeEvent(QResizeEvent* e);

private:
	void addClassToColorMappingTable();
	void addAreaInfo(const QString& name, StatisticalInfo* total_info, StatisticalInfo* sample_info);

	void removeSpecifiedChildren(std::function<bool(QWidget*)> pred);
	void clearAllChildren();
	SamplingProcessViewer* viewer;
	QScrollArea* container;
	std::unordered_map<uint, std::string>* class2label;
};