#include "util.h"

UnionFind::UnionFind(unsigned size) : sets(size), height(size, 1)
{
  for (int i = 0; i < size; ++i)
  {
    id.push_back(i);
  }
}

unsigned UnionFind::setID(unsigned p)
{
  // traverse to the top of the tree
  while (id[p] != p) p = id[p];
  return p;
}

void UnionFind::setUnion(unsigned s1, unsigned s2)
{
  if (s1 == s2) return;
  --sets;
  if (height[s1] > height[s2])
  {
    id[s2] = s1;
    ++height[s1];
  }
  else
  {
    id[s1] = s2;
    ++height[s2];
  }
}
