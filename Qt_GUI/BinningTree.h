#pragma once

#include <memory>
#include <functional>
#include <ctime>
#include <random>
#include <numeric>
#include <queue>

#include <QRect>

#include "global.h"

extern Param params;
extern std::vector<int> selected_class_order;

class BinningTreeNode
{
public:
	friend class BinningTree;
	BinningTreeNode(Box&& b, std::vector<std::weak_ptr<MinGrid>>&& vec, StatisticalInfo&& info, std::weak_ptr<BinningTreeNode> parent) : b(std::move(b)), min_grids_inside(std::move(vec)), info(std::move(info)), parent(parent)
	{
		child1 = nullptr;
		child2 = nullptr;
		seed_index = -1;
		leaf_num_inside = 1;
	}

	bool childShouldSplit(const std::shared_ptr<BinningTreeNode> &child, double threshold)
	{
		auto &sibling = child1 == child ? child2 : child1;
		double sample_rate = (double)child->leaf_num_inside / child->info.total_num, sibling_sample_rate = (double)sibling->leaf_num_inside / sibling->info.total_num;
		return sample_rate - sibling_sample_rate <= threshold;
	}
	bool tooManyClasses() { return info.class_point_num.size() > leaf_num_inside; }
	bool tooManyFreeSpace() { return min_grids_inside.size() < params.occupied_space_ratio*b.width*b.height; }
	std::weak_ptr<BinningTreeNode> getParent() { return parent; }
	std::weak_ptr<BinningTreeNode> getChild1() { return child1; }
	std::weak_ptr<BinningTreeNode> getChild2() { return child2; }
	size_t getPointNum() { return info.total_num; }
	const Box& getBoundBox()  { return b; }
	uint getSeedIndex() { return seed_index; }

private:
	const Box b;
	std::vector<std::weak_ptr<MinGrid>> min_grids_inside;
	std::weak_ptr<BinningTreeNode> parent;
	std::shared_ptr<BinningTreeNode> child1; // left or top
	std::shared_ptr<BinningTreeNode> child2; // right or bottom
	uint seed_index; // the seed in this bin
	StatisticalInfo info;
	uint leaf_num_inside;
};

typedef std::pair<std::weak_ptr<BinningTreeNode>, std::unordered_map<uint, size_t>> NodeWithQuota;

class BinningTree
{
public:
	BinningTree(const PointSet* origin, const QRect& bounding_rect);

	std::weak_ptr<BinningTreeNode> getRoot() { return root; }

	bool split(std::shared_ptr<BinningTreeNode> node);
	bool split_new(std::shared_ptr<BinningTreeNode> node);
	
	uint selectSeedIndex(std::shared_ptr<BinningTreeNode> node);
	uint selectSeedIndex(std::shared_ptr<BinningTreeNode> node, uint label);

	void updateLeafNum(std::shared_ptr<BinningTreeNode> node);

	NodeWithQuota backtrack(std::shared_ptr<BinningTreeNode> leaf, uint max_depth);

	std::unordered_map<uint, size_t> determineQuota(std::shared_ptr<BinningTreeNode> node);
	void splitQuota(const NodeWithQuota& node, std::vector<NodeWithQuota>* leaves);
private:
	void fillQuota(uint remaining_leaf_num, const std::unordered_map<uint, size_t> &label_point_map, std::unordered_map<uint, size_t>& quato, const std::function<size_t(uint)>& getAmount);
	bool splitHelper(std::shared_ptr<BinningTreeNode> node, std::function<bool(std::shared_ptr<MinGrid>)> comp, std::vector<std::weak_ptr<MinGrid>> *child1_vec, std::vector<std::weak_ptr<MinGrid>> *child2_vec);
	StatisticalInfo countStatisticalInfo(const std::vector<std::weak_ptr<MinGrid>>& vec);
	
	uint rouletteSelection(const std::vector<uint>& items, std::function<double(uint)> scoreFunc);
	
	std::shared_ptr<BinningTreeNode> root;

	const PointSet* dataset; // original dataset

	struct pairhash {
	public:
		template <typename T, typename U>
		std::size_t operator()(const std::pair<T, U> &x) const
		{
			return std::hash<T>()(x.first)*CANVAS_HEIGHT + std::hash<U>()(x.second);
		}
	};
	std::unordered_map<std::pair<int,int>, std::shared_ptr<MinGrid>, pairhash> min_grids;

	const uint horizontal_bin_num;
	const uint vertical_bin_num;
	const uint margin_left;
	const uint margin_top;

	std::mt19937 gen{ time(NULL) };
};

inline int visual2grid(qreal pos, qreal margin)
{
	return static_cast<int>(pos - margin) / params.grid_width;
}

inline qreal grid2visual(qreal pos, qreal margin)
{
	return pos*params.grid_width + margin;
}

template<class T>
struct ClassPointComparator {
	bool operator()(const std::pair<uint, T>& a, const std::pair<uint, T>& b)	{
		return a.second < b.second;
	}
};