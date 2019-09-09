#include "ControlPanelWidget.h"

using namespace std;

ControlPanelWidget::ControlPanelWidget(SamplingProcessViewer * viewer, PointSet * points, unordered_map<uint, string>* class2label, QWidget * parent) : QWidget(parent), points(points), class2label(class2label), viewer(viewer)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	{// point related stuff
		QGroupBox* point_group = new QGroupBox("Point Settings:", this);
		QLabel* point_size_label = new QLabel("Point radius:", this);
		QSpinBox* spin_point_size = new QSpinBox(this);
		spin_point_size->setRange(1, 100000);
		spin_point_size->setValue(params.point_radius);
		spin_point_size->setToolTip(
			"The radius of point, used in the rendering procedure.");
		connect(spin_point_size, QOverload<int>::of(&QSpinBox::valueChanged),
			[this](int value) { params.point_radius = value; if (this->viewer->treeHasBeenBuilt()) { this->viewer->redrawPoints(); } });

		QGridLayout* point_group_layout = new QGridLayout(point_group);
		point_group->setLayout(point_group_layout);
		point_group_layout->addWidget(point_size_label, 0, 0);
		point_group_layout->addWidget(spin_point_size, 0, 1);

		layout->addWidget(point_group);
	}

	{// algo related stuff
		QGroupBox* algorithm_group = new QGroupBox("Algorithm Settings:", this);
		QCheckBox* show_bound_option = new QCheckBox("Show the bounds of each bin.", this);
		show_bound_option->setChecked(params.show_border);
		show_bound_option->setToolTip("If enabled the algorithm will show bin bounds "
			"during and after the tree construction.");
		connect(show_bound_option, &QCheckBox::clicked,
			[this](bool value) { params.show_border = value; if (this->viewer->treeHasBeenBuilt()) { if (value) this->viewer->drawBorders(); else this->viewer->redrawPoints(); } });

		QLabel* grid_width_label = new QLabel("Grid width:", this);
		QSpinBox* spin_grid_width = new QSpinBox(this);
		spin_grid_width->setRange(1, 100);
		spin_grid_width->setValue(params.grid_width);
		spin_grid_width->setToolTip(
			"The width of uniform grid, used in the sampling procedure.");
		connect(spin_grid_width, QOverload<int>::of(&QSpinBox::valueChanged),
			[this](int value) { params.grid_width = value; });

		QLabel* threshold_label = new QLabel("Unbalance threshold:", this);
		QDoubleSpinBox* spin_threshold = new QDoubleSpinBox(this);
		spin_threshold->setDecimals(4);
		spin_threshold->setRange(0.0, 1.0);
		spin_threshold->setSingleStep(0.01);
		spin_threshold->setValue(params.threshold);
		spin_threshold->setToolTip(
			"How much relative density unbalance between siblings can be tolerated.");
		connect(spin_threshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double value) { params.threshold = value; });

		QLabel* occupied_space_ratio_label = new QLabel("Occupied space ratio:", this);
		QDoubleSpinBox* spin_occupied_space_ratio = new QDoubleSpinBox(this);
		spin_occupied_space_ratio->setDecimals(4);
		spin_occupied_space_ratio->setRange(0.0, 1.0);
		spin_occupied_space_ratio->setSingleStep(0.1);
		spin_occupied_space_ratio->setValue(params.occupied_space_ratio);
		connect(spin_occupied_space_ratio, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double value) { params.occupied_space_ratio = value; });

		QLabel* depth_label = new QLabel("Maximum backtracking depth:", this);
		QSpinBox* spin_depth = new QSpinBox(this);
		spin_depth->setRange(1, 100000);
		spin_depth->setValue(params.backtracking_depth);
		connect(spin_depth, QOverload<int>::of(&QSpinBox::valueChanged),
			[this](int value) { params.backtracking_depth = value; });

		QGridLayout* algoGroupLayout = new QGridLayout(algorithm_group);
		algorithm_group->setLayout(algoGroupLayout);
		algoGroupLayout->addWidget(grid_width_label, 0, 0);
		algoGroupLayout->addWidget(spin_grid_width, 0, 1);
		algoGroupLayout->addWidget(threshold_label, 1, 0);
		algoGroupLayout->addWidget(spin_threshold, 1, 1);
		algoGroupLayout->addWidget(occupied_space_ratio_label, 2, 0);
		algoGroupLayout->addWidget(spin_occupied_space_ratio, 2, 1);
		algoGroupLayout->addWidget(depth_label, 3, 0);
		algoGroupLayout->addWidget(spin_depth, 3, 1);
		algoGroupLayout->addWidget(show_bound_option, 4, 0, 1, -1);

		layout->addWidget(algorithm_group);
	}

	// open button
	QGroupBox* openBox = new QGroupBox("Open file as input:", this);
	QVBoxLayout* openBoxLayout = new QVBoxLayout(openBox);
	openBox->setLayout(openBoxLayout);
	QPushButton* fileButton = new QPushButton("Select", this);
	buttons.push_back(fileButton);
	openBoxLayout->addWidget(fileButton);
    layout->addWidget(openBox);

	// save buttons
	QGroupBox* saveGroup = new QGroupBox("Save as:", this);
	QGridLayout* saveLayout = new QGridLayout(saveGroup);
	QPushButton* save_PNG = new QPushButton("PNG", this);
	QPushButton* save_SVG = new QPushButton("SVG", this);
	QPushButton* save_PDF = new QPushButton("PDF", this);
	QPushButton* save_CSV = new QPushButton("CSV", this);
	save_CSV->setEnabled(false);

	saveLayout->addWidget(save_PNG, 0, 0); saveLayout->addWidget(save_SVG, 0, 1); saveLayout->addWidget(save_PDF, 0, 2); saveLayout->addWidget(save_CSV, 1, 0, 1, -1);
	buttons.push_back(save_PNG); buttons.push_back(save_SVG); buttons.push_back(save_PDF); buttons.push_back(save_CSV);
	saveGroup->setLayout(saveLayout);
	layout->addWidget(saveGroup);

	// start button
	QGroupBox* startGroup = new QGroupBox("Run Algorithm:", this);
	QVBoxLayout* startLayout = new QVBoxLayout(startGroup);

	QPushButton* startButton = new QPushButton("Start", this);
	buttons.push_back(startButton);
	startLayout->addWidget(startButton);

	// adjust button
	QPushButton* adjustButton = new QPushButton("Adjust", this);
	adjustButton->setEnabled(false);
	startLayout->addWidget(adjustButton);
	buttons.push_back(adjustButton);
	startGroup->setLayout(startLayout);
	layout->addWidget(startGroup);

	{
		QGroupBox* fn_group = new QGroupBox("Filename Box", this);
		QLabel* filename_label = new QLabel("Filename:", fn_group);
		showCurrentFileName(MY_DATASET_FILENAME);
		QGridLayout* fnGroupLayout = new QGridLayout(fn_group);
		fn_group->setLayout(fnGroupLayout);
		fnGroupLayout->addWidget(filename_label, 0, 0);
		fnGroupLayout->addWidget(fn, 0, 1);
		layout->addWidget(fn_group);
	}

	addClassSelectionBox();

	connect(fileButton, &QPushButton::pressed, [this, adjustButton, save_CSV]() {
		QString path = QFileDialog::getOpenFileName(this, tr("Open Dataset"), QString(),
			tr("Comma-Separated Values Files (*.csv)"));

		if (path.isEmpty())
			return;

		adjustButton->setEnabled(false);
		save_CSV->setEnabled(false);
		deletePointSet(this->points);
		showCurrentFileName(QFileInfo(path).fileName());
		this->class2label->clear();
		this->points = readDataset(path.toStdString(), this->class2label);
		this->viewer->setDataset(this->points);
		this->addClassSelectionBox();
	});
	connect(save_PNG, &QPushButton::pressed, [this]() { showSaveDialog("Save Image as PNG", "PNG Image", "png", [this](const QString& path) { this->viewer->saveImagePNG(path); }); });
	connect(save_SVG, &QPushButton::pressed, [this]() { showSaveDialog("Save Image as SVG", "SVG Image", "svg", [this](const QString& path) { this->viewer->saveImageSVG(path); }); });
	connect(save_PDF, &QPushButton::pressed, [this]() { showSaveDialog("Save Image as PDF", "PDF", "pdf", [this](const QString& path) { this->viewer->saveImagePDF(path); }); });
	connect(save_CSV, &QPushButton::pressed, [this]() { showSaveDialog("Save Image as CSV", "Comma-Separated Values Files", "csv", [this](const QString& path) {
		writeDataset(path.toStdString(), indicesToPointSet(this->points, this->viewer->getSeleted()), *(this->class2label));
	}); });
	connect(viewer, &SamplingProcessViewer::finished, [this]() {
		this->enableButtons();
		this->viewer->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	});
	connect(startButton, &QPushButton::released, [this]() {
		this->disableButtons();
		this->viewer->sample();
	});
	connect(adjustButton, &QPushButton::released, [this]() {
		this->disableButtons();
		this->viewer->adjustSeeds();
	});
}

void ControlPanelWidget::addClassSelectionBox()
{
	if (class_selection_box) {
		class_selection_box->close();
		int i = layout()->count() - 1;
		layout()->removeWidget(layout()->itemAt(i)->widget()); // remove stretch
		layout()->removeWidget(layout()->itemAt(i - 1)->widget()); // remove box
		label2class.clear();
	}

	class_selection_box = new QGroupBox("Select Classes:", this);
	QVBoxLayout *vbox = new QVBoxLayout();
	for (auto &pr : *class2label) {
		QCheckBox *checkbox = new QCheckBox(pr.second.c_str(), class_selection_box);
		checkbox->setChecked(std::find(selected_class_order.begin(), selected_class_order.end(), pr.first) != selected_class_order.end());
		vbox->addWidget(checkbox);
		label2class.emplace(pr.second, pr.first);

		connect(checkbox, &QCheckBox::stateChanged, [this, pr](int state) {
			if (state == Qt::Unchecked) {
				selected_class_order.erase(std::find(selected_class_order.begin(), selected_class_order.end(), pr.first));
			}
			else {
				selected_class_order.push_back(pr.first);
			}
			this->viewer->redrawPoints();
		});
	}
	class_selection_box->setLayout(vbox);
	layout()->addWidget(class_selection_box);
	((QBoxLayout*)layout())->addStretch(1);
}

void ControlPanelWidget::showCurrentFileName(const QString& s)
{
	fn->setText(s);
}

void ControlPanelWidget::showSaveDialog(const string & caption, const string & filter_desc, const string & ext, function<void(const QString&)> doSomeStuff)
{
	QFileDialog dialog(this, tr(caption.c_str()), QString(), tr((filter_desc + " (*." + ext + ")").c_str()));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix(ext.c_str());

	if (dialog.exec() == 0)
		return;

	QString path = dialog.selectedFiles().first();

	if (path.isEmpty())
		return;

	doSomeStuff(path);
}

void ControlPanelWidget::disableButtons()
{
	for (auto b : buttons)
		b->setEnabled(false);
}

void ControlPanelWidget::enableButtons()
{
	for (auto b : buttons)
		b->setEnabled(true);
}
