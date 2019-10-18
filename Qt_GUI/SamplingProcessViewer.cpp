#include "SamplingProcessViewer.h"

#include <QPrinter>
#include <QSvgGenerator>

//#define PALETTE {"#4c78a8", "#9ecae9", "#f58518", "#ffbf79", "#54a24b", "#88d27a", "#b79a20", "#f2cf5b", "#439894", "#83bcb6", "#e45756", "#ff9d98", "#79706e", "#bab0ac", "#d67195", "#fcbfd2", "#b279a2", "#d6a5c9", "#9e765f", "#d8b5a5"}
//#define PALETTE {"#215FAC", "#AD3944", "#F89F32", "#e15759", "#f6af5b", "#edc948", "#ee2f7f", "#4b62ad", "#9c755f", "#bab0ac", "#59a583"}
//#define PALETTE {"#9ecae9","#fb9a99","#00FF00","#000033","#FF00B6","#005300","#FFD300","#009FFF","#9A4D42","#00FFBE","#783FC1","#1F9698","#FFACFD","#B1CC71","#F1085C","#FE8F42","#DD00FF","#201A01","#720055","#766C95","#02AD24","#C8FF00","#886C00","#FFB79F","#858567","#A10300","#14F9FF","#00479E","#DC5E93","#93D4FF","#004CFF","#FFFFFF"}
#define PALETTE {"#98df8a", "#42c66a", "#e15759", "#f28e2b", "#76b7b2", "#edc948", "#bca8c9", "#1f77b4", "#ff9da7", "#9c755f", "#ff7f0e", "#9f1990"} // teaser palette
//#define PALETTE {"#069ef2", "#42c66a", "#e15759", "#f28e2b", "#76b7b2", "#edc948", "#94d88b", "#f4a93d", "#ff9da7", "#9c755f", "#dd0505", "#9f1990"} // teaser new palette
//#define PALETTE {"#f9a435", "#1f7cea", "#59ba04" }
//#define PALETTE {"#2378cc", "#1cf1a2", "#3f862c", "#c05733", "#c5a674", "#f79add", "#f9e536", "#a1fa11", "#a24ddf", "#ea1240"}
//#define PALETTE {"#4c78a8", "#9ecae9", "#f58518", "#59ba04", "#54a24b", "#88d27a", "#b79a20", "#f2cf5b", "#439894", "#83bcb6", "#e45756", "#ff9d98", "#79706e", "#bab0ac", "#d67195", "#fcbfd2", "#b279a2", "#d6a5c9", "#9e765f", "#d8b5a5"}

SamplingProcessViewer::SamplingProcessViewer(const PointSet* points, QWidget* parent) : QGraphicsView(parent)
{
	setInteractive(false);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setRenderHint(QPainter::Antialiasing, true);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setOptimizationFlag(QGraphicsView::DontSavePainterState);
	setFrameStyle(0);
	setAttribute(Qt::WA_TranslucentBackground, false);
	setCacheMode(QGraphicsView::CacheBackground);

	setScene(new QGraphicsScene(this));
	this->scene()->setSceneRect(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
	this->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

	bound_pen = QPen(Qt::black, 2, Qt::DashLine);
	
	new_area = nullptr;

	paletteToColors();
	setDataset(points);

	abs.setStatusCallback([this](const auto& status) {
		emit iterationStatus(status.iteration, status.seed_num, status.split_num);
	});
	abs.setCanvasCallback([this](const auto& nodes) { displayTreeNodes(nodes); });
}

void SamplingProcessViewer::mousePressEvent(QMouseEvent *me)
{
	new_area = new IndividualSampleArea(me->localPos());
	this->scene()->addItem(new_area->getGraphicsItem());
}

void SamplingProcessViewer::mouseMoveEvent(QMouseEvent *me)
{
	new_area->setBoundingRect(me->localPos());
}

void SamplingProcessViewer::mouseReleaseEvent(QMouseEvent *me)
{
	bool ok;
	uint radius = QInputDialog::getInt(this->parentWidget(), "Set the radius inside", "The radius is:", 1, 1, 1000, 1, &ok);
	if (ok) {
		new_area->setRadius(radius);
		PointSet *points = new PointSet();
		for (auto &p : *points_in_visual_space) {
			if (new_area->rect().contains(p->pos))
				points->push_back(p);
		}
		new_area->getGraphicsItem()->setBrush(Qt::white);
		new_area->sampleLocally(points);
		drawArea(new_area);
		areas.push_back(new_area);
		emit areaCreated(new_area);
	}
	else {
		StatisticalInfo *total_info = new StatisticalInfo(), *sample_info = new StatisticalInfo();
		for (auto &p : *points_in_visual_space) {
			if (std::find(selected_class_order.begin(), selected_class_order.end(), p->label) != selected_class_order.end() && new_area->rect().contains(p->pos)) {
				++total_info->total_num;
				++total_info->class_point_num[p->label];
			}
		}
		for (auto &i : seeds) {
			auto &p = points_in_visual_space->at(i);
			if (std::find(selected_class_order.begin(), selected_class_order.end(), p->label) != selected_class_order.end() && new_area->rect().contains(p->pos)) {
				++sample_info->total_num;
				++sample_info->class_point_num[p->label];
				emit pointSelected(i, p->label);
			}
		}
		emit areaCounted(total_info, sample_info);
	}
	new_area = nullptr;
}

void SamplingProcessViewer::setDataset(const PointSet * points)
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	
	deleteAreas();
	seeds.clear();
	if (points_in_visual_space)
		deletePointSet(points_in_visual_space);

	points_in_visual_space = deepCopy(points);
	linearScale(points_in_visual_space, MARGIN.left, CANVAS_WIDTH - MARGIN.right, MARGIN.top, CANVAS_HEIGHT - MARGIN.bottom);

	//drawPointByClass(*points_in_visual_space);
	drawPointRandomly(*points_in_visual_space);
	//drawPointLowDensityUpper(*points_in_visual_space);

	tree_is_built = false;
	emit inputImageChanged();
}

void SamplingProcessViewer::displayTreeNodes(const std::vector<TreeNode>& nodes)
{
	this->scene()->clear();
	bool tmp = params.use_alpha_channel; params.use_alpha_channel = false;
	for (auto &n : nodes) {
		auto p = (*points_in_visual_space)[n.node.lock()->getSeedIndex()];
		drawPoint(p->pos.x(), p->pos.y(), params.point_radius, color_brushes[p->label]);

		if (params.show_border) {
			auto &box = n.node.lock()->getBoundBox();
			if (box.left != 0) // left bound
				drawLine(grid2visual(box.left, MARGIN.left), grid2visual(box.top, MARGIN.top), grid2visual(box.left, MARGIN.left), grid2visual(box.top + box.height, MARGIN.top), n.leftIsNew);
			if (box.top != 0) // top bound
				drawLine(grid2visual(box.left, MARGIN.left), grid2visual(box.top, MARGIN.top), grid2visual(box.left + box.width, MARGIN.left), grid2visual(box.top, MARGIN.top), n.topIsNew);
		}
	}
	params.use_alpha_channel = tmp;
}

void SamplingProcessViewer::sample()
{
	emit sampleStart();
	deleteAreas();
	//seeds = abs.execute(points_in_visual_space, QRect(MARGIN.left, MARGIN.top, CANVAS_WIDTH-MARGIN.left-MARGIN.right, CANVAS_HEIGHT-MARGIN.top-MARGIN.bottom));
	seeds = abs.executeWithoutCallback(points_in_visual_space, QRect(MARGIN.left, MARGIN.top, CANVAS_WIDTH - MARGIN.left - MARGIN.right, CANVAS_HEIGHT - MARGIN.top - MARGIN.bottom));
	//drawPointByClass(indicesToPointSet(points_in_visual_space, seeds));
	drawPointRandomly(indicesToPointSet(points_in_visual_space, seeds));
	//drawPointLowDensityUpper(indicesToPointSet(points_in_visual_space, seeds));
	nodes = abs.getAllLeaves();
	tree_is_built = true;
	emit finished();
}

void SamplingProcessViewer::sampleWithoutTreeConstruction()
{
	emit adjustmentStart();
	seeds = abs.KDTreeGuidedSampling();
	//drawPointByClass(indicesToPointSet(points_in_visual_space, seeds));
	drawPointRandomly(indicesToPointSet(points_in_visual_space, seeds));
	//drawPointLowDensityUpper(indicesToPointSet(points_in_visual_space, seeds));
	if (params.show_border) drawBorders();
	emit finished();
}

void SamplingProcessViewer::saveImagePNG(const QString & path)
{
	QPixmap pixmap(this->scene()->sceneRect().size().toSize());
	pixmap.fill(Qt::white);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	this->scene()->render(&painter);
	pixmap.save(path);
}

void SamplingProcessViewer::saveImageSVG(const QString & path)
{
	QSvgGenerator generator;
	generator.setFileName(path);
	generator.setSize(QSize(this->scene()->width(), this->scene()->height()));
	generator.setViewBox(this->scene()->sceneRect());
	generator.setTitle("Resampling Result");
	generator.setDescription(("SVG File created by "+ALGORITHM_NAME).c_str());
	QPainter painter;
	painter.begin(&generator);
	this->scene()->render(&painter);
	painter.end();
}

void SamplingProcessViewer::saveImagePDF(const QString & path)
{
	adjustSize();

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setCreator(ALGORITHM_NAME.c_str());
	printer.setOutputFileName(path);
	printer.setPaperSize(this->size(), QPrinter::Point);
	printer.setFullPage(true);

	QPainter painter(&printer);
	this->render(&painter);
}

bool SamplingProcessViewer::coveredByArea(LabeledPoint * p)
{
	return std::any_of(areas.begin(), areas.end(), [p](IndividualSampleArea* a) {return a->rect().contains(p->pos); });
}

void SamplingProcessViewer::drawPointByClass(const PointSet& points)
{
	this->scene()->clear();
	for (auto &a : areas)
		drawArea(a);

	std::vector<PointSet> _class(CLASS_NUM);
	for (auto &p : points) {
		if(!coveredByArea(p) && std::find(selected_class_order.begin(), selected_class_order.end(), p->label) != selected_class_order.end())
			_class[p->label].push_back(p);
	}
	for (size_t i = 0; i < selected_class_order.size(); i++) {
		//if (selected_class_order[i] == 7)
		//	params.use_alpha_channel = false;
		//else
		//	params.use_alpha_channel = true;
		for (auto p : _class[selected_class_order[i]]) {
			drawPoint(p->pos.x(), p->pos.y(), params.point_radius, color_brushes[p->label]);
		}
	}
}

void SamplingProcessViewer::drawPointRandomly(const PointSet & points)
{
	this->scene()->clear();
	for (auto &a : areas)
		drawArea(a);

	PointSet shuffled_points;
	for (auto &p : points) {
		if (!coveredByArea(p) && std::find(selected_class_order.begin(), selected_class_order.end(), p->label) != selected_class_order.end())
			shuffled_points.push_back(p);
	}
	std::random_shuffle(shuffled_points.begin(), shuffled_points.end());
	for (auto &p : shuffled_points) {
		drawPoint(p->pos.x(), p->pos.y(), params.point_radius, color_brushes[p->label]);
	}
}

void SamplingProcessViewer::drawPointLowDensityUpper(const PointSet & points)
{
	this->scene()->clear();
	for (auto &a : areas)
		drawArea(a);

	using namespace std;
	int area_size = 100;
	// assign points to area
	unordered_map<uint, unordered_map<uint, PointSet>> small_areas;
	for (size_t i = 0, sz = points.size(); i < sz; ++i) {
		auto &p = points[i];
		if (std::find(selected_class_order.begin(), selected_class_order.end(), p->label) == selected_class_order.end())
			continue;
		int x = (int)p->pos.x() / area_size, y = (int)p->pos.y() / area_size, pos = x*CANVAS_HEIGHT + y;
		small_areas[pos][p->label].push_back(p);
	}

	for (auto &u : small_areas) {
		priority_queue<pair<size_t, uint>> q;
		for (auto &u2 : u.second) {
			q.push(make_pair(u2.second.size(), u2.first));
		}
		while (!q.empty()) {
			uint label = q.top().second;
			q.pop();
			for (auto &p : u.second[label]) {
				drawPoint(p->pos.x(), p->pos.y(), params.point_radius, color_brushes[p->label]);
			}
		}
	}
}

void SamplingProcessViewer::drawBorders()
{
	for (auto &n : nodes) {
		auto &box = n.lock()->getBoundBox();
		if (box.left != 0) // left bound
			drawLine(grid2visual(box.left, MARGIN.left), grid2visual(box.top, MARGIN.top), grid2visual(box.left, MARGIN.left), grid2visual(box.top + box.height, MARGIN.top), false);
		if (box.top != 0) // top bound
			drawLine(grid2visual(box.left, MARGIN.left), grid2visual(box.top, MARGIN.top), grid2visual(box.left + box.width, MARGIN.left), grid2visual(box.top, MARGIN.top), false);
	}
}

void SamplingProcessViewer::redrawPoints()
{
	emit redrawStart();
	if(seeds.empty()) 
		drawPointRandomly(*points_in_visual_space);
	else
		drawPointRandomly(indicesToPointSet(points_in_visual_space, seeds));
	if (params.show_border) drawBorders();
}

void SamplingProcessViewer::drawArea(IndividualSampleArea * area)
{
	this->scene()->addItem(area->createAndGetGraphicsItem());
	for (auto &i : area->getSelectedIndices()) {
		auto p = area->getPointByIndex(i);
		drawPoint(p->pos.x(), p->pos.y(), area->getRadius(), color_brushes[p->label]);
	}
}

void SamplingProcessViewer::deleteAreas()
{
	for (auto &a : areas)
		delete a;
	areas.clear();
}

void SamplingProcessViewer::paletteToColors()
{
	std::vector<const char*> palette(PALETTE);
	for (const char* hex : palette) {
		color_brushes.push_back(QColor(hex));
	}
}

void SamplingProcessViewer::drawPoint(qreal x, qreal y, qreal radius, QBrush b)
{
	if (params.use_alpha_channel) { // draw gaussian shape
		//QColor c2 = b.color();
		//c2.setAlpha(60);
		//this->scene()->addEllipse(x - 1.0, y - 1.0, 2.0, 2.0, QPen(c2, 1), b);
		//c2.setAlpha(30);
		//QPen p(c2, 3);
		//this->scene()->addEllipse(x - 2.0, y - 2.0, 4.0, 4.0, p, Qt::NoBrush);
		QColor c = Qt::gray;//b.color();
		c.setAlpha(30);
		this->scene()->addEllipse(x - radius, y - radius, 2 * radius, 2 * radius, Qt::NoPen, c);
	}
	else {
		//this->scene()->addEllipse(x, y, 1.0, 1.0, Qt::NoPen, b);
		this->scene()->addEllipse(x - radius, y - radius, 2 * radius, 2 * radius, Qt::NoPen, b);
	}
}