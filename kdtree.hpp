/**
 * @file kdtree.cpp
 * Implementation of KDTree class.
 */

#include <utility>
#include <algorithm>

using namespace std;

template <int Dim>
bool KDTree<Dim>::smallerDimVal(const Point<Dim>& first,
                                const Point<Dim>& second, int curDim) const
{
    /**
     * @todo Implement this function!
     */
    if(curDim < 0 || curDim >= Dim)
    return false;
    if(first[curDim] > second[curDim])
    return false;
    else if (first[curDim] < second[curDim])
    return true;
    else 
    return (first < second);
    }

template <int Dim>
bool KDTree<Dim>::shouldReplace(const Point<Dim>& target,
                                const Point<Dim>& currentBest,
                                const Point<Dim>& potential) const
{
    /**
     * @todo Implement this function!
     */
    int c_distance = 0;
    int p_distance = 0;
    for(int x = 0; x  <Dim; x++){
      c_distance += (target[x] - currentBest[x]) * (target[x] - currentBest[x]);
      p_distance += (target[x] - potential[x]) * (target[x] - potential[x]);
    }
    if (c_distance < p_distance)
    return false;
    else if(p_distance < c_distance)
    return true;
    else
    return potential < currentBest;
}

template <int Dim>
KDTree<Dim>::KDTree(const vector<Point<Dim>>& newPoints)
{
    /**
     * @todo Implement this function!
     */
    size = 0;
    vector<Point<Dim>> points = newPoints;
    root = chelper(points, 0, points.size()-1, 0);
}

template <int Dim>
typename KDTree<Dim>::KDTreeNode* KDTree<Dim>::chelper(vector<Point<Dim>>& points, unsigned start, unsigned end, int dim){
if(points.empty() == true || start >= points.size() || end >= points.size() || start > end || start < 0 || end < 0)
return NULL;
KDTreeNode * subroot = new KDTreeNode();
int dim_cycle1 = dim%Dim;
int dim_cycle2 = (dim + 1)%Dim;
int med = (start + end) / 2;
quick_select(points, start, end, med, dim_cycle1);
subroot->point = points[med];
subroot->left = chelper(points, start, med - 1, dim_cycle2);
subroot->right = chelper(points, med + 1, end, dim_cycle2);
return subroot;
}
template <int Dim>
void KDTree<Dim>::quick_select(vector<Point<Dim>>& list, unsigned start, unsigned end, unsigned med, int dim){
if(start== end) 
return;
unsigned pivotIndex = partition(list, start, end, med, dim);
if(med == pivotIndex) 
return;
else if(med < pivotIndex) 
return quick_select(list, start, pivotIndex-1, med, dim);
else
return quick_select(list, pivotIndex+1, end, med, dim);
}

template <int Dim>
unsigned KDTree<Dim>::partition(vector<Point<Dim>>& list, unsigned start, unsigned end, unsigned pivotIndex, int dim){
  Point<Dim> pivotValue = list[pivotIndex];
  Point<Dim> temp = list[end];
  list[end] = list[pivotIndex];
  list[pivotIndex] = temp;
  unsigned storeIndex = start;
  for(unsigned i = start; i < end; i++)
    if(smallerDimVal(list[i], pivotValue, dim)){
      Point<Dim> temp = list[storeIndex];
      list[storeIndex] = list[i];
      list[i] = temp;
      storeIndex++;
    } 
  Point<Dim> temp2 = list[end];
  list[end] = list[storeIndex];
  list[storeIndex] = temp2;
  return storeIndex;
}



template <int Dim>
KDTree<Dim>::KDTree(const KDTree<Dim>& other) {
  /**
   * @todo Implement this function!
   */
  copy(other.root);
  size = other.size;
}

template <int Dim>
const KDTree<Dim>& KDTree<Dim>::operator=(const KDTree<Dim>& rhs) {
  /**
   * @todo Implement this function!
   */
  
  free(root);
  copy(rhs.root);
  size = rhs.size;
  return *this;
}

template <int Dim>
KDTree<Dim>::~KDTree() {
  /**
   * @todo Implement this function!
   */
  free(root);
}

template <int Dim>
void KDTree<Dim>::free(KDTreeNode* subroot) {
	if (subroot == NULL) 
  return;
	if (subroot->left != NULL) 
  free(subroot->left);
	if (subroot->right != NULL) 
  free(subroot->right);
	delete subroot;
	subroot = NULL;
}

template <int Dim>
typename KDTree<Dim>::KDTreeNode* KDTree<Dim>::copy(const KDTreeNode* subroot) {
  KDTreeNode* new_root = new KDTreeNode();
  new_root->point = subroot->point;
  new_root->left = copy(subroot->left);
  new_root->right = copy(subroot->right);
  return new_root;
}

template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighbor(const Point<Dim>& query) const
{
    /**
     * @todo Implement this function!
     */
    return findNearestNeighborHelper(query, root, 0);
}

template <int Dim>
Point<Dim> KDTree<Dim>::findNearestNeighborHelper(const Point<Dim>& query, KDTreeNode* subroot, int dim) const {
  if(subroot->left == NULL && subroot->right == NULL) 
  return subroot->point;
  int dim_cycle = (dim + 1) % Dim;
  Point<Dim> current_best = subroot->point;
  Point<Dim> p_best;

  if (smallerDimVal(query, subroot->point, dim) == true) {
    if(subroot->left != NULL)
	  p_best = findNearestNeighborHelper(query, subroot->left, dim_cycle);
    else
    {
	  p_best = findNearestNeighborHelper(query, subroot->right, dim_cycle);
    }   
  } 
  else if(smallerDimVal(query, subroot->point, dim) == false) {
    if(subroot->right != NULL)
	  p_best = findNearestNeighborHelper(query, subroot->right, dim_cycle);
    else
    {
	  p_best = findNearestNeighborHelper(query, subroot->left, dim_cycle);
    }
  }

  if (shouldReplace(query, current_best, p_best)) 
  current_best = p_best;
  
  double cur_sep = (subroot->point[dim] - query[dim]) * (subroot->point[dim] - query[dim]);
  double radius= 0.0;
	
  for (int x = 0; x < Dim; x++) 
    radius += (query[x] - current_best[x]) * (query[x] - current_best[x]);

  if (cur_sep <= radius && smallerDimVal(query, subroot->point, dim) == false && subroot->left != NULL) {
	  Point<Dim> potential = findNearestNeighborHelper(query, subroot->left, dim_cycle);
		if (shouldReplace(query, current_best, potential)) 
    current_best = potential;
	}
  else if(cur_sep <= radius && smallerDimVal(query, subroot->point, dim) == true && subroot->right != NULL){
    Point<Dim> potential = findNearestNeighborHelper(query, subroot->right, dim_cycle);
		if (shouldReplace(query, current_best, potential)) 
    current_best = potential;
  }
  return current_best;
}