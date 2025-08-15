/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUND_NODE_INTERNAL_H_
#define _ROUND_NODE_INTERNAL_H_

#include <round/typedef.h>
#include <round/util/object.h>
#include <round/util/list.h>
#include <round/util/json_internal.h>
#include <round/util/strings.h>
#include <round/util/thread.h>
#include <round/util/consistent_hashing.h>
#include <round/util/digest.h>
#include <round/util/status.h>
#include <round/script_internal.h>
#include <round/clock.h>
#include <round/registry.h>
#include <round/message.h>
#include <round/trigger.h>

#include <round/method.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
 * Data Type
 ****************************************/

typedef bool (*ROUND_NODE_POSTMESSAGE_FUNC)(void *node, RoundJSONObject *reqObj, RoundJSONObject **resultObj, RoundError *err);

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
  RoundString *clusterName; \
  void *cluster; \
  RoundClock *clock; \
  time_t requestTimeout; \
  RoundClusterManager *clusterMgr; \
  ROUND_NODE_POSTMESSAGE_FUNC postMsgFunc; \
  RoundStatus status; \
  void *userData;

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
  RoundTriggerManager *triggerMgr;
} RoundLocalNode;

typedef struct {
  RoundJSONObject **resultObj;
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
RoundNode *round_node_local_new(void);
RoundNode *round_node_remote_new(void);
  
bool round_node_init(RoundNode *node);
bool round_node_clear(RoundNode *node);
bool round_node_destroy(RoundNode *node);
bool round_node_delete(RoundNode *node);

#define round_node_remove(node) round_list_remove((RoundList *)node)

bool round_node_setaddress(RoundNode *node, const char *addr);
bool round_node_setport(RoundNode *node, int port);

const char *round_node_gethashcode(RoundNode *node);

#define round_node_setcluster(node,value) (node->cluster = value)

#define round_node_setclock(node, value) round_clock_setvalue(node->clock, value)
#define round_node_setremoteclock(node, value) round_clock_setremotevalue(node->clock, value)
#define round_node_incrementclock(node) round_clock_increment(node->clock)
bool round_node_updateclockbyjsonobject(RoundNode* node, RoundJSONObject* jsonObj);

#define round_node_setpostmessagefunc(node, func) (node->postMsgFunc = (ROUND_NODE_POSTMESSAGE_FUNC)func)
#define round_node_getpostmessagefunc(node) (node->postMsgFunc)

#define round_node_digest(str,buf) round_sha256_digest(str,buf)
bool round_node_updateid(RoundNode *node);
#define round_node_setid(node, value) round_string_setvalue(node->digest, value)
#define round_node_hasid(node) round_string_hasvalue(node->digest)

bool round_node_addclusternode(RoundNode *node, RoundNode *clusterNode);
bool round_node_removeclusternode(RoundNode *node, RoundNode *clusterNode);
bool round_node_clearclusternodes(RoundNode *node, RoundNode *clusterNode);
bool round_node_hasclusternode(RoundNode *node, RoundNode *clusterNode);

bool round_node_rpcerrorcode2error(void* node, int rpcErrCode, RoundError* err);
bool round_node_jsonrpcrequest2string(void* node, RoundJSONObject* reqObj, const char **reqStr, RoundError* err);
  
/****************************************
 * Function (LocalNode)
 ****************************************/
  
bool round_local_node_init(RoundLocalNode *node);
bool round_local_node_initsystemmethods(RoundLocalNode *node);
bool round_local_node_initscriptengines(RoundLocalNode *node);
bool round_local_node_initthreads(RoundLocalNode *node);

bool round_local_node_destory(RoundLocalNode *node);

#define round_local_node_setaddress(node,addr) round_node_setaddress((RoundNode*)node,addr)
#define round_local_node_getaddress(node,addr) round_node_getaddress((RoundNode*)node,addr)

#define round_local_node_setport(node,port) round_node_setport((RoundNode*)node,port)
#define round_local_node_getport(node,port) round_node_getport((RoundNode*)node,port)
  
#define round_local_node_setclustername(node,cluster) round_node_setclustername((RoundNode*)node,cluster)
  
#define round_local_node_setrequesttimeout(node,value) round_node_setrequesttimeout((RoundNode*)node,value)
#define round_local_node_getrequesttimeout(node) round_node_getrequesttimeout((RoundNode*)node)

#define round_local_node_setclock(node,value) round_clock_setvalue(node->clock,value)
#define round_local_node_getclock(node) round_clock_getvalue(node->clock)
#define round_local_node_updateclockbyjsonobject(node,jsonobj) round_node_updateclockbyjsonobject((RoundNode*)node,jsonobj)

bool round_local_node_clear(RoundLocalNode *node);
bool round_local_node_isrunning(RoundLocalNode *node);

bool round_local_node_setmethod(RoundLocalNode *node, RoundMethod *method);
bool round_local_node_removemethod(RoundLocalNode *node, const char *name);
bool round_local_node_isfinalmethod(RoundLocalNode *node, const char *name);

bool round_local_node_addengine(RoundLocalNode *node, RoundScriptEngine *engine);
  
bool round_local_node_execmessage(RoundLocalNode *node, RoundMessage *msg, RoundJSONObject **resultObj, RoundError *err);
bool round_local_node_execrequest(RoundLocalNode* node, RoundJSONObject* reqObj, RoundJSONObject** resObj, RoundError* err);

bool round_local_node_setregistry(RoundLocalNode *node, const char *key, const char *val);
RoundRegistry *round_local_node_getregistry(RoundLocalNode *node, const char *key);
bool round_local_node_removeregistry(RoundLocalNode *node, const char *key);

#define round_local_node_addclusternode(node, clusterNode) round_cluster_manager_addnode(node->clusterMgr, clusterNode);
#define round_local_node_removeclusternode(node, clusterNode) round_cluster_manager_removenode(node->clusterMgr, clusterNode);

void round_local_node_message_thread(RoundThread *thread);
  
/****************************************
 * Function (Local Node Message)
 ****************************************/

RoundMessage *round_local_node_message_new();
bool round_local_node_message_delete(RoundMessage *msg);
bool round_local_node_message_seterror(RoundMessage *msg, RoundError *err);
RoundError *round_local_node_message_geterror(RoundMessage *msg);
bool round_local_node_message_setresponsejsonobject(RoundMessage *msg, RoundJSONObject **resultObj);
RoundJSONObject **round_local_node_message_getresponsejsonobject(RoundMessage *msg);
  
/****************************************
 * Function (Remote Node)
 ****************************************/
  
RoundRemoteNode *round_remote_node_new(void);
RoundRemoteNode *round_remote_node_copy(RoundNode *node);
bool round_remote_node_destory(RoundRemoteNode *node);
bool round_remote_node_delete(RoundRemoteNode *node);
  
bool round_remote_node_postmessage(RoundRemoteNode *node, RoundJSONObject *reqMap, RoundJSONObject **resObj, RoundError *err);
#define round_remote_node_poststringmessage(node, reqStr, resObj, err) round_node_postmessagestring((RoundNode*)node, reqStr, resObj, err)

bool round_remote_node_posthttpjsonrequest(RoundRemoteNode* node, const char *reqContent, RoundJSONObject** resObj, RoundError* err);

#define round_remote_node_setaddress(node,addr) round_node_setaddress((RoundNode*)node,addr)
#define round_remote_node_getaddress(node,addr) round_node_getaddress((RoundNode*)node,addr)

#define round_remote_node_setport(node,port) round_node_setport((RoundNode*)node,port)
#define round_remote_node_getport(node,port) round_node_getport((RoundNode*)node,port)

#define round_remote_node_setclustername(node,cluster) round_node_setclustername((RoundNode*)node,cluster)
#define round_remote_node_getclustername(node,cluster) round_node_getclustername((RoundNode*)node,cluster)
#define round_remote_node_getcluster(node) round_node_getcluster((RoundNode*)node)
  
#define round_remote_node_setrequesttimeout(node,value) round_node_setrequesttimeout((RoundNode*)node,value)
#define round_remote_node_getrequesttimeout(node) round_node_getrequesttimeout((RoundNode*)node)

#define round_remote_node_setclock(node,value) round_clock_setvalue(node->clock,value)
#define round_remote_node_getclock(node) round_clock_getvalue(node->clock)
#define round_remote_node_updateclockbyjsonobject(node,jsonobj) round_node_updateclockbyjsonobject((RoundNode*)node,jsonobj)
  
/****************************************
 * Function (Node Ring)
 ****************************************/
  
RoundNodeRing *round_node_ring_new(void);
bool round_node_ring_delete(RoundNodeRing *ring);
  
#define round_node_ring_size(ring) round_consistenthashing_ring_size(ring->consHashRing)
#define round_node_ring_add(ring,node) round_consistenthashing_ring_addnode(ring->consHashRing, node)
#define round_node_ring_remove(ring,node) round_consistenthashing_ring_removenode(ring->consHashRing, node)
#define round_node_ring_getequalnode(ring,node) round_consistenthashing_ring_getequalnode(ring->consHashRing, node)
#define round_node_ring_hasequalnode(ring,node) round_consistenthashing_ring_hasequalnode(ring->consHashRing, node)
#define round_node_ring_getnodes(ring) round_consistenthashing_ring_getnodes(ring->consHashRing)
#define round_node_ring_getnode(ring,idx) round_consistenthashing_ring_getnode(ring->consHashRing, idx)
#define round_node_ring_getnodebyid(ring,id) round_consistenthashing_ring_getnodebyhashcode(ring->consHashRing, id)
  
#ifdef  __cplusplus
} /* extern C */
#endif

#endif /* _ROUND_NODE_INTERNAL_H_ */
