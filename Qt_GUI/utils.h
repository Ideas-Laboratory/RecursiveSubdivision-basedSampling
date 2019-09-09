#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>

#include "global.h"

extern std::vector<int> selected_class_order;

void setClassToLabelMapping(const std::unordered_map<std::string, uint>& label2class, std::unordered_map<uint, std::string>* class2label);
PointSet* readDataset(std::string filename, std::unordered_map<uint, std::string>* class2label);
void writeDataset(const std::string & filename, const PointSet& points, const std::unordered_map<uint, std::string>& class2label);
PointSet* deepCopy(const PointSet* points);

PointSet indicesToPointSet(const PointSet* points, const Indices& selected);
void deletePointSet(PointSet* points);

inline double linearScale(double val, double oldLower, double oldUpper, double lower, double upper)
{
	return (val - oldLower)*(upper - lower) / (oldUpper - oldLower) + lower;
}
// will modify points
void linearScale(PointSet* points, double lower, double upper);
void linearScale(PointSet* points, int left, int right, int top, int bottom);
void linearScale(PointSet* points, std::function<double(double, double, double)> horizontalScale, std::function<double(double, double, double)> verticalScale);

inline double squaredEuclideanDistance(const LabeledPoint * a, const LabeledPoint * b)
{
	double dx = a->pos.x() - b->pos.x(), dy = a->pos.y() - b->pos.y();
	return dx*dx + dy*dy;
}