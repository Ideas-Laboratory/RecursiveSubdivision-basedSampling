#pragma once
#include <utility>
#include <set>

#include "global.h"
#include "utils.h"
#include "BinningTree.h"

struct TreeNode
{
	std::weak_ptr<BinningTreeNode> node;
	bool leftIsNew;
	bool topIsNew;
};

class AdaptiveBinningSampling
{
public:
	struct Status {
		uint iteration;
		uint seed_num;
		uint split_num;
	};

	template<class T>
	using Report = std::function<void(const T&)>;

	std::vector<std::weak_ptr<BinningTreeNode>> getAllLeaves();

	Indices execute(const PointSet* origin, const QRect& bounding_rect);
	Indices adjustSeeds();
	Indices executeWithoutCallback(const PointSet* origin, const QRect& bounding_rect);

	void setStatusCallback(Report<Status> cb) {	status_callback = cb; }
	void setCanvasCallback(Report<std::vector<TreeNode>> cb) { canvas_callback = cb; }

	const static int max_iteration;
private:
	void divide(std::shared_ptr<BinningTreeNode> root, std::vector<TreeNode>* leaves, bool shouldSplit); // assignment
	Indices selectNewSeed(const std::vector<TreeNode>& leaves); // update
	bool notFinish(); // termination

	std::vector<NodeWithQuota> determineLabelOfLeaves();
	Indices leavesToSeeds();

	Status current_iteration_status;

	std::unique_ptr<BinningTree> tree;

	Report<Status> status_callback;
	Report<std::vector<TreeNode>> canvas_callback;
};

