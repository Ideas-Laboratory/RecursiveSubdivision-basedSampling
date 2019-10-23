#include "DisplayPanelWidget.h"

using namespace std;

DisplayPanelWidget::DisplayPanelWidget(SamplingProcessViewer *spv, unordered_map<uint, string> *c2l_mapping, QWidget * parent) : QWidget(parent), viewer(spv), class2label(c2l_mapping)
{
	setFixedWidth(440);
	container = dynamic_cast<QScrollArea*>(parent);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignTop);
	setLayout(layout);

	addClassToColorMappingTable();

	connect(spv, &SamplingProcessViewer::inputImageChanged, [this]() {
		clearAllChildren();
		addClassToColorMappingTable();
	});
	connect(spv, &SamplingProcessViewer::areaCreated, [this](IndividualSampleArea* area) {
		addAreaInfo("r", area->getTotalInfo(), area->getSampleInfo());
	});
	connect(spv, &SamplingProcessViewer::areaCounted, [this](StatisticalInfo* total_info, StatisticalInfo* sample_info) {
		addAreaInfo("o", total_info, sample_info);
	});
	connect(spv, &SamplingProcessViewer::adjustmentStart, [this]() {
		removeSpecifiedChildren([](QWidget* w) { return w->objectName() == "o"; });
	});
	connect(spv, &SamplingProcessViewer::sampleStart, [this]() {
		removeSpecifiedChildren([](QWidget* w) { return !w->objectName().isEmpty(); });
	});
	connect(spv, &SamplingProcessViewer::redrawStart, [this]() {
		removeSpecifiedChildren([](QWidget* w) { return !w->objectName().isEmpty(); });
	});
	//connect(spv, &SamplingProcessViewer::pointSelected, [this](uint index, uint class_) {
	//	qDebug() << index << ',' << QString(this->class2label->at(class_).c_str());
	//});
}

void DisplayPanelWidget::resizeEvent(QResizeEvent * e)
{
	QWidget::resizeEvent(e);
	container->verticalScrollBar()->setValue(height());
}

void DisplayPanelWidget::addClassToColorMappingTable()
{
	QGroupBox *mapping_box = new QGroupBox("Class to Color mapping:", this);
	QVBoxLayout *box_layout = new QVBoxLayout(mapping_box);
	mapping_box->setLayout(box_layout);
	QTableWidget *tw = new QTableWidget(this);
	tw->setShowGrid(false);
	tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tw->verticalHeader()->setVisible(false);

	tw->setRowCount(class2label->size());
	tw->setColumnCount(2);
	tw->setHorizontalHeaderLabels(QStringList{ "Class", "Color" });
	tw->setColumnWidth(0, 300);
	tw->setColumnWidth(1, 70);
	int rows = 0;
	for (auto &pr : *class2label) {
		QTableWidgetItem *newItem = new QTableWidgetItem(tr(pr.second.c_str()));
		tw->setItem(rows, 0, newItem);
		newItem = new QTableWidgetItem();
		newItem->setBackground(this->viewer->getColorBrushes()[pr.first]);
		tw->setItem(rows, 1, newItem);
		++rows;
	}
	tw->setRowCount(rows);

	box_layout->addWidget(tw);
	mapping_box->setFixedHeight(tw->horizontalHeader()->height() + 60 + tw->rowHeight(0)*rows);
	this->layout()->addWidget(mapping_box);
}

void DisplayPanelWidget::addAreaInfo(const QString& name, StatisticalInfo * total_info, StatisticalInfo * sample_info)
{
	if (total_info->total_num > 0) {
		QTableWidget *tw = new QTableWidget(this);
		tw->setObjectName(name);
		tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tw->verticalHeader()->setVisible(false);
		tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

		tw->setRowCount(total_info->class_point_num.size() + 1);
		tw->setColumnCount(4);
		tw->setHorizontalHeaderLabels(QStringList{ "", "Actual", "Sample", "Ratio" });
		tw->setColumnWidth(0, 70);
		auto &map_t = total_info->class_point_num, &map_s = sample_info->class_point_num;
		int rows = 0;
		for (int i = 0, sz = selected_class_order.size(); i < sz; ++i) {
			int class_index = selected_class_order[i];
			if (map_t.find(class_index) != map_t.end()) {
				QTableWidgetItem *newItem = new QTableWidgetItem();
				newItem->setBackground(this->viewer->getColorBrushes()[class_index]);
				tw->setItem(rows, 0, newItem);
				newItem = new QTableWidgetItem(QString::number(map_t[class_index]));
				tw->setItem(rows, 1, newItem);
				uint sample_num = map_s.find(class_index) != map_s.end() ? map_s[class_index] : 0;
				newItem = new QTableWidgetItem(QString::number(sample_num));
				tw->setItem(rows, 2, newItem);
				newItem = new QTableWidgetItem(QString::number(100.0*sample_num/map_t[class_index], 'f', 2) + "%");
				tw->setItem(rows, 3, newItem);
				++rows;
			}
		}
		tw->setItem(rows, 0, new QTableWidgetItem(tr("Sum:")));
		tw->setItem(rows, 1, new QTableWidgetItem(QString::number(total_info->total_num)));
		tw->setItem(rows, 2, new QTableWidgetItem(QString::number(sample_info->total_num)));
		tw->setItem(rows, 3, new QTableWidgetItem(QString::number(100.0*sample_info->total_num/total_info->total_num, 'f', 2) + "%"));
		tw->setRowCount(++rows);

		tw->setFixedHeight(tw->horizontalHeader()->height() + 4 + tw->rowHeight(0)*rows);
		this->layout()->addWidget(tw);
	}
	delete total_info;
	delete sample_info;
}

void DisplayPanelWidget::removeSpecifiedChildren(std::function<bool(QWidget*)> pred)
{
	for (int i = this->layout()->count() - 1; i > -1; --i) {
		QWidget *w = this->layout()->itemAt(i)->widget();
		if (pred(w)) {
			this->layout()->removeWidget(w);
			w->close();
		}
	}
}

void DisplayPanelWidget::clearAllChildren()
{
	removeSpecifiedChildren([](QWidget* w) { return true; });
}

