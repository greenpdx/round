/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <round/util/strings.h>
#include <round/util/consistent_hashing.h>

/****************************************
 * round_consistenthashing_ring_new
 ****************************************/

RoundConsistentHashingRing *round_consistenthashing_ring_new()
{
  RoundConsistentHashingRing *ring;
  
  ring = (RoundConsistentHashingRing *)malloc(sizeof(RoundConsistentHashingRing));
  if (!ring)
    return NULL;
  
  ring->orderedList = round_ordered_list_new();
  if (!ring->orderedList) {
    round_consistenthashing_ring_delete(ring);
    return false;
  }
  
  round_ordered_list_setcmpfunc(ring->orderedList, round_consistenthashing_ring_comp);
  round_consistenthashing_ring_setnodedestructor(ring, NULL);
  
  return ring;
}

/****************************************
 * round_consistenthashing_ring_delete
 ****************************************/

bool round_consistenthashing_ring_delete(RoundConsistentHashingRing *ring)
{
  if (!ring)
    return false;
  
  if (ring->orderedList) {
    round_ordered_list_delete(ring->orderedList);
    ring->orderedList = NULL;
  }
  
  free(ring);
  
  return true;
}

/****************************************
 * round_consistenthashing_ring_clear
 ****************************************/

bool round_consistenthashing_ring_clear(RoundConsistentHashingRing *ring)
{
  if (!ring)
    return false;
  
  if (ring->orderedList) {
    round_ordered_list_clear(ring->orderedList, ring->nodeDstructorFunc);
  }

  return true;
}

/****************************************
 * round_consistenthashing_ring_addnode
 ****************************************/

bool round_consistenthashing_ring_addnode(RoundConsistentHashingRing *ring, void *node)
{
  if (!ring || !node)
    return false;
  
  if (round_consistenthashing_ring_hasnode(ring, node))
    return false;

  return round_ordered_list_add(ring->orderedList, node);
}

/****************************************
 * round_consistenthashing_ring_hasnode
 ****************************************/

bool round_consistenthashing_ring_hasnode(RoundConsistentHashingRing *ring, void *node)
{
  return (0 <= round_consistenthashing_ring_getnodeindex(ring, node)) ? true : false;
}
/****************************************
 * round_consistenthashing_ring_getnodeindex
 ****************************************/

ssize_t round_consistenthashing_ring_getnodeindex(RoundConsistentHashingRing *ring, void *targetNode)
{
  RoundConsistentHashingNode *node;
  const char *nodeHash, *targetNodeHash;
  ssize_t nodeIdx;
  
  targetNodeHash = round_consistenthashing_node_gethash(targetNode);
  if (!targetNodeHash)
    return -1;
  
  nodeIdx = 0;
  for (node = round_consistenthashing_ring_getnodes(ring); node; node = round_consistenthashing_node_next(node)) {
    nodeHash = round_consistenthashing_node_gethash(node);
    if (nodeHash) {
      if (round_strcmp(targetNodeHash, nodeHash) == 0)
        return nodeIdx;
    }
    nodeIdx++;
  }
  
  return -1;
}

/****************************************
 * round_consistenthashing_ring_islastnode
 ****************************************/

bool round_consistenthashing_ring_islastnode(RoundConsistentHashingRing *ring, void *node)
{
  ssize_t ringSize, nodeIdx;
  nodeIdx = round_consistenthashing_ring_getnodeindex(ring, node);
  ringSize = round_consistenthashing_ring_size(ring);
  return (nodeIdx == (ringSize-1)) ? true : false;
}

/****************************************
 * round_consistenthashing_ring_getlastnode
 ****************************************/

RoundConsistentHashingNode *round_consistenthashing_ring_getlastnode(RoundConsistentHashingRing *ring)
{
  ssize_t ringSize;
  
  ringSize = round_consistenthashing_ring_size(ring);
  if (ringSize <= 0)
    return NULL;
  return round_consistenthashing_ring_getnode(ring, (ringSize-1));
}

/****************************************
 * round_consistenthashing_ring_comp
 ****************************************/

int round_consistenthashing_ring_comp(RoundConsistentHashingNode *thisNode, RoundConsistentHashingNode *otherNode)
{
  const char *thisHash, *otherHash;
  
  if (!thisNode || !otherNode)
    return 0;

  thisHash =round_consistenthashing_node_gethash(thisNode);
  otherHash =round_consistenthashing_node_gethash(otherNode);
  
  if (!thisHash || !otherHash)
    return 0;
  
  return round_strcmp(thisHash, otherHash) * -1;
}
