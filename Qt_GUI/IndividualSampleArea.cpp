#include "IndividualSampleArea.h"

IndividualSampleArea::IndividualSampleArea(const QPointF & initial_pos) : inner_rect(initial_pos, initial_pos), radius_inside(-1)
{
	item = new QGraphicsRectItem();
	item->setPen(QPen(Qt::black, 5, Qt::DashDotLine));
}

QGraphicsRectItem * IndividualSampleArea::createAndGetGraphicsItem()
{
	item = new QGraphicsRectItem();
	item->setPen(QPen(Qt::black, 5, Qt::DashDotLine));
	item->setRect(inner_rect);
	item->setBrush(Qt::white);
	return item;
}

void IndividualSampleArea::setBoundingRect(const QPointF & current_pos)
{
	inner_rect.setWidth(current_pos.x() - inner_rect.x());
	inner_rect.setHeight(current_pos.y() - inner_rect.y());
	item->setRect(inner_rect);
}

void IndividualSampleArea::sampleLocally(PointSet* dataset)
{
	points = dataset;

	uint tmp = params.grid_width;
	params.grid_width *= (double)radius_inside/params.point_radius;
	AdaptiveBinningSampling abs;
	selected = abs.executeWithoutCallback(points, QRect(qMin(inner_rect.left(), inner_rect.right()), qMin(inner_rect.top(), inner_rect.bottom()), qAbs(inner_rect.width()), qAbs(inner_rect.height())));
	params.grid_width = tmp;
}

StatisticalInfo * IndividualSampleArea::getTotalInfo()
{
	StatisticalInfo *info = new StatisticalInfo();
	for (auto &p : *points) {
		++info->total_num;
		++info->class_point_num[p->label];
	}
	return info;
}

StatisticalInfo * IndividualSampleArea::getSampleInfo()
{
	StatisticalInfo *info = new StatisticalInfo();
	for (auto &i : selected) {
		++info->total_num;
		++info->class_point_num[points->at(i)->label];
	}
	return info;
}
