#pragma once
#include <QGraphicsItem>
#include <QPen>
#include <QPainter>

#include "global.h"
#include "AdaptiveBinningSampling.h"

class IndividualSampleArea
{
public:
	IndividualSampleArea(const QPointF& initial_pos);
	~IndividualSampleArea() { delete points; }

	QGraphicsRectItem* getGraphicsItem() { return item; }
	QGraphicsRectItem* createAndGetGraphicsItem();

	const QRectF& rect() { return inner_rect; }
	void setBoundingRect(const QPointF& current_pos);
	void setRadius(const uint& r) { radius_inside = r; }
	uint getRadius() { return radius_inside; }

	void sampleLocally(PointSet* dataset);
	const Indices& getSelectedIndices() { return selected; }
	LabeledPoint* getPointByIndex(uint i) { return points->at(i); }

	StatisticalInfo* getTotalInfo();
	StatisticalInfo* getSampleInfo();
private:
	QGraphicsRectItem* item;
	PointSet* points;
	Indices selected;
	QRectF inner_rect;
	uint radius_inside;
};