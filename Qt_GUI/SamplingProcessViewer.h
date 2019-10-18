#pragma once

#include <QGraphicsView>
#include <QMouseEvent>
#include <QInputDialog>

#include "global.h"
#include "utils.h"
#include "AdaptiveBinningSampling.h"
#include "IndividualSampleArea.h"

#define CLASS_NUM 30

class SamplingProcessViewer : public QGraphicsView
{
	Q_OBJECT

public:
	SamplingProcessViewer(const PointSet* points, QWidget* parent);
	~SamplingProcessViewer() { deleteAreas(); deletePointSet(points_in_visual_space); }
	void setDataset(const PointSet* points);
	void displayTreeNodes(const std::vector<TreeNode>& nodes);

	bool treeHasBeenBuilt() { return tree_is_built; }
	void drawBorders();
	void redrawPoints();

	void sample();
	void sampleWithoutTreeConstruction();
	
	void saveImagePNG(const QString& path);
	void saveImageSVG(const QString& path);
	void saveImagePDF(const QString& path);

	const Indices& getSeleted() { return seeds; }
	const std::vector<QBrush>& getColorBrushes() { return color_brushes; }

	const std::string ALGORITHM_NAME = "Tree-based Resampling for Multiclass Scatterplot";

signals:
	void finished();
	void redrawStart();
	void sampleStart();
	void adjustmentStart();
	void inputImageChanged();
	void iterationStatus(int iteration, int numberPoints, int splits);
	void areaCreated(IndividualSampleArea* area);
	void areaCounted(StatisticalInfo* total_info, StatisticalInfo* sample_info);
	void pointSelected(uint index, uint class_);

protected:
	void mousePressEvent(QMouseEvent *me);
	void mouseMoveEvent(QMouseEvent *me);
	void mouseReleaseEvent(QMouseEvent *me);

private:
	bool coveredByArea(LabeledPoint* p);
	void drawArea(IndividualSampleArea* area);
	void deleteAreas();
	void drawPointByClass(const PointSet& points);
	void drawPointRandomly(const PointSet& points);
	void drawPointLowDensityUpper(const PointSet& points);

	void paletteToColors();
	void drawPoint(qreal x, qreal y, qreal radius, QBrush c);
	void drawLine(qreal x1, qreal y1, qreal x2, qreal y2, bool isRed)
	{
		QColor c = isRed ? Qt::red : Qt::black;
		c.setAlphaF(0.5);
		bound_pen.setColor(c);
		this->scene()->addLine(x1, y1, x2, y2, bound_pen);
	}

	bool tree_is_built;

	AdaptiveBinningSampling abs;
	PointSet* points_in_visual_space = nullptr;
	Indices seeds;
	std::vector<std::weak_ptr<BinningTreeNode>> nodes;

	std::vector<QBrush> color_brushes;

	QPen bound_pen;

	std::vector<IndividualSampleArea*> areas;
	IndividualSampleArea* new_area;
};

