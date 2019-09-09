#include "utils.h"

using namespace std;

void setClassToLabelMapping(const unordered_map<string, uint>& label2class, unordered_map<uint, string>* class2label)
{
	for (auto &u : label2class)
		class2label->emplace(u.second, u.first);
}

PointSet * readDataset(string filename, unordered_map<uint, string>* class2label)
{
	ifstream input(filename);
	if(!input) //TODO opening error, should throw exception and show error message
		return nullptr;

	//skip invalid char
	while ((char)input.get() < 0);
	input.unget();

	vector<tuple<double, double, string>> raw_data;
	string value;
	double x = 0, y = 0;
	unordered_map<string, uint> label2class;
	while (getline(input, value, ',')) {
		x = atof(value.c_str());
		getline(input, value, ',');
		y = atof(value.c_str());

		getline(input, value);
		if (label2class.find(value) == label2class.end()) { // mapping label (string) to class (unsigned int)
			label2class[value] = label2class.size();
		}
		raw_data.push_back(make_tuple(x, y, move(value)));
	}
	setClassToLabelMapping(label2class, class2label);

	PointSet* points = new PointSet();
	for (auto &d : raw_data) {
		points->push_back(new LabeledPoint(get<0>(d), get<1>(d), label2class[get<2>(d)]));
	}
	return points;
}

void writeDataset(const string & filename, const PointSet& points, const unordered_map<uint, string>& class2label)
{
	ofstream output(filename, ios_base::trunc);
	for (auto &p : points) {
		output << p->pos.x() << ',' << p->pos.y() << ',' << class2label.at(p->label) << endl;
	}
}

PointSet * deepCopy(const PointSet * points)
{
	PointSet* copy = new PointSet();
	for (auto &p : *points) {
		copy->push_back(new LabeledPoint(p));
	}
	return copy;
}

void linearScale(PointSet* points, double lower, double upper)
{
	auto scale = [=](double val, double oldLower, double oldUpper) { return linearScale(val, oldLower, oldUpper, lower, upper); };

	linearScale(points, scale, scale);
}

void linearScale(PointSet* points, int left, int right, int top, int bottom)
{
	auto horizontalScale = [=](double val, double oldLower, double oldUpper) { return linearScale(val, oldLower, oldUpper, left, right); };
	auto verticalScale = [=](double val, double oldLower, double oldUpper) { return linearScale(val, oldLower, oldUpper, bottom, top); };

	linearScale(points, horizontalScale, verticalScale);
}

void linearScale(PointSet * points, std::function<double(double, double, double)> horizontalScale, std::function<double(double, double, double)> verticalScale)
{
	auto xRange = minmax_element(points->begin(), points->end(),
		[](LabeledPoint const *a, LabeledPoint const *b) { return a->pos.x() < b->pos.x(); });
	auto yRange = minmax_element(points->begin(), points->end(),
		[](LabeledPoint const *a, LabeledPoint const *b) { return a->pos.y() < b->pos.y(); });
	double xMin = (*xRange.first)->pos.x(), yMin = (*yRange.first)->pos.y(), xMax = (*xRange.second)->pos.x(), yMax = (*yRange.second)->pos.y();
	auto xScale = [=](double val) { return horizontalScale(val, xMin, xMax); };
	auto yScale = [=](double val) { return verticalScale(val, yMin, yMax); };

	for (auto &p : *points) {
		p->pos.setX(xScale(p->pos.x()));
		p->pos.setY(yScale(p->pos.y()));
	}
}

PointSet indicesToPointSet(const PointSet* points, const Indices& selected)
{
	PointSet new_points;
	for (auto i : selected) {
		new_points.push_back((*points)[i]);
	}
	return new_points;
}

void deletePointSet(PointSet * points)
{
	for (auto &p : *points) {
		delete p;
	}
	delete points;
}