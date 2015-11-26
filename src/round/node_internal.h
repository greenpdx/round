/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUNDC_NODE_INTERNAL_H_
#define _ROUNDC_NODE_INTERNAL_H_

#include <round/typedef.h>
#include <round/util/oo.h>
#include <round/util/list.h>
#include <round/util/json_internal.h>
#include <round/util/strings.h>
#include <round/util/thread.h>
#include <round/util/consistent_hashing.h>
#include <round/util/digest.h>
#include <round/script.h>
#include <round/clock.h>
#include <round/registry.h>
#include <round/message.h>

#include <round/method.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
 * Data Type
 ****************************************/

typedef bool (*ROUND_NODE_POSTMESSAGE_FUNC)(void *node, RoundJSONObject *reqObj, RoundJSONObject *resObj, RoundError *err);

#if !defined(RoundClusterManager)
typedef struct {
  ROUND_LIST_STRUCT_MEMBERS
} RoundClusterManager;
#endif

#define ROUND_NODE_STRUCT_MEMBERS \
  ROUND_CONSISTENTHASHING_NODE_STRUCT_MEMBERS \
  ROUND_OO_STRUCT_MEMBERS \
  RoundString *addr; \
  int port; \
  RoundString *digest; \
  RoundString *cluster; \
  RoundClock *clock; \
  time_t requestTimeout; \
  RoundClusterManager *clusterMgr; \
  ROUND_NODE_POSTMESSAGE_FUNC postMsg;

typedef struct {
  ROUND_NODE_STRUCT_MEMBERS
} RoundNode;

typedef struct {
  RoundConsistentHashingRing *consHashRing;
} RoundNodeRing;

typedef struct {
  ROUND_NODE_STRUCT_MEMBERS
} RoundRemoteNode;

typedef struct {
  ROUND_NODE_STRUCT_MEMBERS
  
  RoundMethodManager *methodMgr;
  RoundRegistryManager *regMgr;
  RoundMessageManager *msgMgr;
  RoundThreadManager *threadMgr;
} RoundLocalNode;

typedef struct {
  RoundJSONObject **resObj;
  RoundError *err;
} RoundLocalMessageData;

/****************************************
 * Public Header
 ****************************************/
  
#include <round/node.h>
  
/****************************************
 * Function (Node)
 ****************************************/

RoundNode *round_node_new(void);

bool round_node_init(RoundNode *node);
bool round_node_destroy(RoundNode *node);

#define round_node_remove(node) round_list_remove((RoundList *)node)

bool round_node_setaddress(RoundNode *node, const char *addr);
bool round_node_setport(RoundNode *node, int port);
bool round_node_setcluster(RoundNode *node, const char *cluster);

#define round_node_setclockvalue(node, value) round_clock_setvalue(node->clock, value)
#define round_node_setremoteclockvalue(node, value) round_clock_setremotevalue(node->clock, value)
#define round_node_incrementclock(node) round_clock_increment(node->clock)
#define round_node_getclockvalue(node) round_clock_getvalue(node->clock)

#define round_node_setpostmessagefunc(node, func) (node->postMsg = (ROUND_NODE_POSTMESSAGE_FUNC)func)
#define round_node_getpostmessagefunc(node) (node->postMsg)

#define round_node_digest(str,buf) round_sha256_digest(str,buf)
bool round_node_updatedigest(RoundNode *node);
const char *round_node_getdigest(RoundNode *node);
#define round_node_setdigest(node, value) round_string_setvalue(node->digest, value)
#define round_node_hasdigest(node) round_string_hasvalue(node->digest)

bool round_node_addclusternode(RoundNode *node, RoundNode *clusterNode);
bool round_node_removeclusternode(RoundNode *node, RoundNode *clusterNode);
bool round_node_clearclusternode(RoundNode *node, RoundNode *clusterNode);
bool round_node_haclusternode(RoundNode *node, RoundNode *clusterNode);
  
/****************************************
 * Function (LocalNode)
 ****************************************/
  
RoundLocalNode *round_local_node_new(void);

bool round_local_node_init(RoundLocalNode *node);
bool round_local_node_initsystemmethods(RoundLocalNode *node);
bool round_local_node_initscriptengines(RoundLocalNode *node);
bool round_local_node_initthreads(RoundLocalNode *node);

bool round_local_node_destory(RoundLocalNode *node);
bool round_local_node_delete(RoundLocalNode *node);

bool round_local_node_start(RoundLocalNode *node);
bool round_local_node_stop(RoundLocalNode *node);
bool round_local_node_isrunning(RoundLocalNode *node);

bool round_local_node_setmethod(RoundLocalNode *node, RoundMethod *method);
bool round_local_node_removemethod(RoundLocalNode *node, const char *name);
bool round_local_node_isfinalmethod(RoundLocalNode *node, const char *name);

bool round_local_node_addengine(RoundLocalNode *node, RoundScriptEngine *engine);

bool round_local_node_postmessage(RoundLocalNode *node, RoundJSONObject *reqObj, RoundJSONObject *resObj, RoundError *err);
bool round_local_node_execmessage(RoundLocalNode *node, RoundMessage *msg, RoundJSONObject **resObj, RoundError *err);

bool round_local_node_setregistry(RoundLocalNode *node, const char *key, const char *val);
RoundRegistry *round_local_node_getregistry(RoundLocalNode *node, const char *key);
bool round_local_node_removeregistry(RoundLocalNode *node, const char *key);

void round_local_node_message_thread(RoundThread *thread);
  
#define round_local_node_getclockvalue(node) round_clock_getvalue(node->clock)
#define round_local_node_getrequesttimeout(node) round_node_getrequesttimeout((RoundNode *)node)

/****************************************
 * Function (Local Node Message)
 ****************************************/

RoundMessage *round_local_node_message_new();
bool round_local_node_message_delete(RoundMessage *msg);
bool round_local_node_message_seterror(RoundMessage *msg, RoundError *err);
RoundError *round_local_node_message_geterror(RoundMessage *msg);
bool round_local_node_message_setresponsejsonobject(RoundMessage *msg, RoundJSONObject **resObj);
RoundJSONObject **round_local_node_message_getresponsejsonobject(RoundMessage *msg);
  
/****************************************
 * Function (Remot eNode)
 ****************************************/
  
RoundRemoteNode *round_remote_node_new(void);
bool round_remote_node_destory(RoundRemoteNode *node);
bool round_remote_node_delete(RoundRemoteNode *node);
bool round_remote_node_postmessage(RoundLocalNode *node, RoundJSONObject *reqMap, RoundJSONObject *resMap, RoundError *err);

/****************************************
 * Function (Node Ring)
 ****************************************/
  
RoundNodeRing *round_node_ring_new(void);
bool round_node_ring_delete(RoundNodeRing *ring);
  
#define round_node_ring_size(ring) round_consistenthashing_ring_size(ring->consHashRing)
#define round_node_ring_add(ring,node) round_consistenthashing_ring_addnode(ring->consHashRing, node)
#define round_node_ring_remove(ring,node) round_consistenthashing_ring_removenode(ring->consHashRing, node)
#define round_node_ring_getequalnode(ring,node) round_consistenthashing_ring_getequalnode(ring->consHashRing, node)

#ifdef  __cplusplus
} /* extern C */
#endif

#endif /* _ROUNDC_NODE_INTERNAL_H_ */
