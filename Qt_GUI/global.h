#pragma once

#include <vector>
#include <unordered_map>
#include <QPoint>
#include <QDebug>

//constants
#define MY_DATASET_FILENAME "./data/synthesis1.csv"

const static int CANVAS_WIDTH = 1600;//316;//1080; //720; //480; 
const static int CANVAS_HEIGHT = 900;//316;//810; //540; //360; 

const static struct {
	const int left = 20;
	const int right = 40;
	const int top = 20;
	const int bottom = 40;
} MARGIN;

//type definition
struct LabeledPoint
{
	QPointF pos;
	uint label;
	LabeledPoint() {}
	LabeledPoint(double x, double y, uint l) : pos(x, y), label(l) {}
	LabeledPoint(const LabeledPoint* p) : pos(p->pos), label(p->label) {}
};
typedef std::vector<LabeledPoint*> PointSet;

typedef std::vector<uint> Indices;

struct Box {
	uint left;
	uint top;
	uint width;
	uint height;
	Box() {}
	Box(uint left, uint top, uint w, uint h) :left(left), top(top), width(w), height(h) {}
};

struct MinGrid {
	Box b;
	Indices contents;
	MinGrid(uint l, uint t) : b(l,t,1,1) {}
};

// total number and number of each class
struct StatisticalInfo {
	size_t total_num;
	std::unordered_map<uint, size_t> class_point_num;
	StatisticalInfo() { total_num = 0; }
	StatisticalInfo(const StatisticalInfo& info) : total_num(info.total_num), class_point_num(info.class_point_num){}
	StatisticalInfo(StatisticalInfo&& info) : total_num(info.total_num), class_point_num(std::move(info.class_point_num)) {}
};

struct Param {
	uint point_radius;
	uint grid_width;
	uint backtracking_depth;
	double occupied_space_ratio;
	double threshold;
	bool show_border;
	bool use_alpha_channel;
};