#include "AdaptiveBinningSampling.h"

const int AdaptiveBinningSampling::max_iteration = 10000;

using namespace std;

std::vector<std::weak_ptr<BinningTreeNode>> AdaptiveBinningSampling::getAllLeaves()
{
	vector<weak_ptr<BinningTreeNode>> leaves;
	queue<weak_ptr<BinningTreeNode>> fifo;
	fifo.push(tree->getRoot());
	while (!fifo.empty()) {
		shared_ptr<BinningTreeNode> ptr = fifo.front().lock();
		fifo.pop();
		auto &child1 = ptr->getChild1().lock();
		if (child1) {
			fifo.push(child1);
			fifo.push(ptr->getChild2());
		}
		else
			leaves.push_back(ptr);
	}
	return leaves;
}

Indices AdaptiveBinningSampling::execute(const PointSet * origin, const QRect& bounding_rect)
{
	if (origin->empty()) return Indices();

	tree = make_unique<BinningTree>(origin, bounding_rect);
	//initialize seed List and status
	Indices seeds(selectNewSeed(vector<TreeNode>{ {tree->getRoot(), true, true}}));
	current_iteration_status = { 0 };
	//start iteration
	do {
		current_iteration_status.split_num = 0;

		vector<TreeNode> leaves;
		divide(tree->getRoot().lock(), &leaves, true);

		seeds = selectNewSeed(leaves);

		current_iteration_status.seed_num = seeds.size();
		++current_iteration_status.iteration;

		canvas_callback(leaves);
		status_callback(current_iteration_status);
	} while (notFinish());
	return seeds;
}

void AdaptiveBinningSampling::divide(shared_ptr<BinningTreeNode> root, vector<TreeNode>* leaves, bool should_split)
{
	auto child1 = root->getChild1().lock(), child2 = root->getChild2().lock();
	
	if (child1) { // if root has child nodes, then process tree recursion
		bool c1_ss = should_split, c2_ss = should_split;
		if (should_split) {
			c1_ss = root->childShouldSplit(child1, params.threshold);
			c2_ss = root->childShouldSplit(child2, params.threshold);
		}
		divide(child1, leaves, c1_ss);
		divide(child2, leaves, c2_ss);
		tree->updateLeafNum(root);
	}
	else { // no child nodes
		if ((should_split || root->tooManyFreeSpace()) // splitting condition
			&& tree->split(root)) {
			tree->updateLeafNum(root);
			if (leaves) {
				bool leftIsNew = root->getChild1().lock()->getBoundBox().top == root->getChild2().lock()->getBoundBox().top;
				leaves->push_back({ root->getChild1(), false, false });
				leaves->push_back({ root->getChild2(), leftIsNew, !leftIsNew });
			}
			++current_iteration_status.split_num;
		}
		else {
			// keep
			if (leaves) {
				leaves->push_back({ root, false, false });
			}
		}
	}
}

Indices AdaptiveBinningSampling::selectNewSeed(const vector<TreeNode>& leaves)
{
	Indices seeds;
	for (auto &leaf : leaves) {
		uint index = tree->selectSeedIndex(leaf.node.lock());
		seeds.push_back(index);
	}
	return seeds;
}

Indices AdaptiveBinningSampling::adjustSeeds()
{
	auto &leaves = determineLabelOfLeaves();

	Indices seeds;
	for (auto &leaf : leaves) {
		uint index = tree->adjustSeedIndex(leaf.first.lock(), leaf.second.begin()->first);
		seeds.push_back(index);
	}
	return seeds;
}

Indices AdaptiveBinningSampling::executeWithoutCallback(const PointSet * origin, const QRect& bounding_rect)
{
	if (origin->empty()) return Indices();

	tree = make_unique<BinningTree>(origin, bounding_rect);
	current_iteration_status = { 0,0 };
	//start iteration
	std::clock_t start = std::clock();
	do {
		current_iteration_status.split_num = 0;

		divide(tree->getRoot().lock(), nullptr, true);
		++current_iteration_status.iteration;
	} while (notFinish());
	auto &seeds = adjustSeeds();//leavesToSeeds();
	qDebug() << (std::clock() - start) / (double)CLOCKS_PER_SEC;

	current_iteration_status.seed_num = seeds.size();
	if(status_callback) status_callback(current_iteration_status);
	return seeds;
}

inline bool AdaptiveBinningSampling::notFinish()
{
	return !(current_iteration_status.split_num == 0 || current_iteration_status.iteration == max_iteration);
}

vector<NodeWithQuota> AdaptiveBinningSampling::determineLabelOfLeaves()
{
	vector<weak_ptr<BinningTreeNode>> leaves = getAllLeaves();

	unordered_map<shared_ptr<BinningTreeNode>, unordered_map<uint, size_t>> forest;
	{ // backtrack and form a forest
		set<shared_ptr<BinningTreeNode>> visited_leaves;
		for (auto &l : leaves) {
			auto &leaf = l.lock();
			if (visited_leaves.find(leaf) == visited_leaves.end()) { // not visited
				NodeWithQuota root = tree->backtrack(leaf, params.backtracking_depth);
				// update visited leaves
				queue<weak_ptr<BinningTreeNode>> fifo;
				fifo.push(root.first);
				while (!fifo.empty()) {
					shared_ptr<BinningTreeNode> ptr = fifo.front().lock();
					fifo.pop();
					forest.erase(ptr); // remove if ptr is the offspring of root 
					auto &child1 = ptr->getChild1().lock();
					if (child1) {
						fifo.push(child1);
						fifo.push(ptr->getChild2());
					}
					else
						visited_leaves.insert(ptr);
				}
				forest.insert(root);
			}
		}
	}

	vector<NodeWithQuota> leavesWithLabel;
	for (auto &root : forest) {
		tree->splitQuota(root, &leavesWithLabel);
	}
	return leavesWithLabel;
}

Indices AdaptiveBinningSampling::leavesToSeeds()
{
	auto &leaves = getAllLeaves();
	Indices seeds;
	for (auto &leaf : leaves) {
		uint index = tree->selectSeedIndex(leaf.lock());
		seeds.push_back(index);
	}
	return seeds;
}
