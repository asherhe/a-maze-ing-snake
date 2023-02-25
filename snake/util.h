#ifndef UTIL_H
#define UTIL_H

#include <vector>

class UnionFind
{
private:
  unsigned sets;  // number of unconnected sets
  std::vector<unsigned> id;
  std::vector<unsigned> height;  // height of the tree at each value
public:
  UnionFind(unsigned size);

  inline unsigned numSets() { return sets; }

  unsigned setID(unsigned p);

  // combines two sets
  void setUnion(unsigned s1, unsigned s2);

  // combine the two sets that contain p and q
  inline void valUnion(unsigned p, unsigned q) { setUnion(setID(p), setID(q)); }
};

#endif  // UTIL_H
