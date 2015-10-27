/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUNDC_CLUSTER_INTERNAL_H_
#define _ROUNDC_CLUSTER_INTERNAL_H_

#include <round/typedef.h>
#include <round/util/list.h>
#include <round/node_internal.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
 * Data Type
 ****************************************/

typedef struct _RoundCluster {
  ROUND_LIST_STRUCT_MEMBERS
  RoundString *name;
  RoundNodeRing *nodeRing;
} RoundCluster;

/****************************************
 * Header
 ****************************************/
  
#include <round/cluster.h>
  
/****************************************
 * Function
 ****************************************/

#define round_cluster_setname(cluster,value) round_string_setvalue(cluster->name, value)
#define round_cluster_isname(cluster,value) round_streq(round_cluster_getname(cluster), value)
#define round_cluster_remove(cluster) round_list_remove((RoundList *)cluster)
#define round_cluster_equals(cluster1, cluster2) round_string_equals(cluster1->name, cluster2->name)

bool round_cluster_addnode(RoundCluster *cluster, RoundNode *node);

/****************************************
 * Function (Class Manager)
 ****************************************/
  
RoundClusterManager *round_cluster_manager_new(void);
bool round_cluster_manager_delete(RoundClusterManager *mgr);
  
bool round_cluster_manager_addnode(RoundClusterManager *mgr, RoundNode *node);
RoundCluster *round_cluster_manager_getcluster(RoundClusterManager *mgr, const char *name);
  
#define round_cluster_manager_hascluster(mgr, name) (round_cluster_manager_getcluster(mgr,name) ? true : false)
#define round_cluster_manager_clear(mgr) round_list_clear((RoundList *)mgr, (ROUND_LIST_DESTRUCTORFUNC)round_cluster_delete)
#define round_cluster_manager_size(mgr) round_list_size((RoundList *)mgr)
#define round_cluster_manager_getclusters(mgr) (RoundCluster *)round_list_next((RoundList *)mgr)
#define round_cluster_manager_addcluster(mgr,cluster) round_list_add((RoundList *)mgr, (RoundList *)cluster)

#ifdef  __cplusplus
} /* extern C */
#endif

#endif /* _ROUNDC_CLUSTER_H_ */
