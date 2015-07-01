#ifndef SPACENODE_HPP
#define SPACENODE_HPP

inline void
SpaceNode::setFlag(int flag, bool value) {
  if (value)
    nstatus |= 1<<(flag-1);
  else
    nstatus &= ~(1<<(flag-1));
}

inline bool
SpaceNode::getFlag(int flag) const {
  return (nstatus & (1<<(flag-1))) != 0;
}

inline unsigned int
SpaceNode::getNumericFlag(int flag, int size) const {
  unsigned int mask = (1 << size) - 1;
  return (nstatus >> (flag-1)) & mask;
}

inline void
SpaceNode::setNumericFlag(int flag, int size, unsigned int value) {
  unsigned int mask = (1 << size) - 1;
  unsigned int clearmask = ~(mask << (flag-1));
  nstatus &= clearmask;
  nstatus |= (value & mask) << (flag-1);
}

inline void
SpaceNode::setHasOpenChildren(bool b) {
  setFlag(HASOPENCHILDREN, b);
}

inline void
SpaceNode::setHasFailedChildren(bool b) {
  setFlag(HASFAILEDCHILDREN, b);
}

inline void
SpaceNode::setHasSolvedChildren(bool b) {
  setFlag(HASSOLVEDCHILDREN, b);
}

inline void
SpaceNode::setStatus(NodeStatus s) {
  nstatus &= ~( STATUSMASK );
  nstatus |= s << STATUSSTART;
}

inline NodeStatus
SpaceNode::getStatus(void) const {
  return static_cast<NodeStatus>((nstatus & STATUSMASK) >> STATUSSTART);
}

inline
SpaceNode::SpaceNode(int p)
: Node(p), nstatus(0) {
  setStatus(UNDETERMINED);
  setHasSolvedChildren(false);
  setHasFailedChildren(false);
}

inline bool
SpaceNode::isOpen(void) {
  return ((getStatus() == UNDETERMINED) ||
          getFlag(HASOPENCHILDREN));
}

inline bool
SpaceNode::hasFailedChildren(void) {
  return getFlag(HASFAILEDCHILDREN);
}

inline bool
SpaceNode::hasSolvedChildren(void) {
  return getFlag(HASSOLVEDCHILDREN);
}

inline bool
SpaceNode::hasOpenChildren(void) {
  return getFlag(HASOPENCHILDREN);
}

inline int
SpaceNode::getAlternative(const NodeAllocator& na) const {
  SpaceNode* p = getParent(na);
  if (p == NULL)
    return -1;
  for (int i=p->getNumberOfChildren(); i--;)
    if (p->getChild(na,i) == this)
      return i;
  GECODE_NEVER;
  return -1;
}

inline void
SpaceNode::dispose(void) {}

#endif // SPACENODE_HPP