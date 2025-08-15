

/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <round/cluster_internal.h>
#include <round/node_internal.h>

#include <round/script/native.h>
#include <round/script/js.h>
#include <round/script/lua.h>
#include <round/script/ruby.h>
#include <round/script/python.h>
#include <round/method/system_method.h>

/****************************************
* round_local_node_new
****************************************/

RoundLocalNode* round_local_node_new(void)
{
  RoundLocalNode* node = (RoundLocalNode*)malloc(sizeof(RoundLocalNode));

  if (!node)
    return NULL;

  if (!round_local_node_init(node)) {
    round_local_node_delete(node);
    return NULL;
  }

  return node;
}

/****************************************
 * round_local_node_init
 ****************************************/

bool round_local_node_init(RoundLocalNode* node)
{
  if (!node)
    return false;
  
  round_node_init((RoundNode*)node);
  round_oo_setdescendantdestoroyfunc(node, round_local_node_destory);

  node->methodMgr = round_method_manager_new();
  node->regMgr = round_registry_manager_new();
  node->threadMgr = round_thread_manager_new();
  node->msgMgr = round_message_manager_new();
  node->triggerMgr = round_trigger_manager_new();
  
  if (!node->methodMgr || !node->regMgr || !node->threadMgr || !node->msgMgr)
    return false;

  round_node_setpostmessagefunc(node, round_local_node_postmessage);
  
  if (!round_local_node_initthreads(node))
    return false;

  if (!round_local_node_initscriptengines(node))
    return false;

  if (!round_local_node_initsystemmethods(node))
    return false;

  return true;
}

/****************************************
 * round_local_node_initthreads
 ****************************************/

bool round_local_node_initthreads(RoundLocalNode* node)
{
  RoundThreadFunc threadFuncs[] = { round_local_node_message_thread };
  RoundThread* thread;
  size_t n, threadFuncCnt;

  threadFuncCnt = sizeof(threadFuncs) / sizeof(RoundThreadFunc);
  for (n = 0; n < threadFuncCnt; n++) {
    thread = round_thread_new();
    if (!thread)
      return false;
    round_thread_setuserdata(thread, node);
    round_thread_setaction(thread, threadFuncs[n]);
    if (!round_thread_manager_add(node->threadMgr, thread))
      return false;
  }

  return true;
}

/****************************************
 * round_local_node_initscriptengines
 ****************************************/

bool round_local_node_initscriptengines(RoundLocalNode* node)
{
  bool areAllEnginesAdded;

  areAllEnginesAdded = true;

  areAllEnginesAdded &= round_local_node_addengine(node, (RoundScriptEngine*)round_native_engine_new());

#if defined(ROUND_SUPPORT_JS_SM)
  areAllEnginesAdded &= round_local_node_addengine(node, (RoundScriptEngine*)round_js_engine_new());
#endif

#if defined(ROUND_SUPPORT_LUA)
  areAllEnginesAdded &= round_local_node_addengine(node, (RoundScriptEngine*)round_lua_engine_new());
#endif

#if defined(ROUND_SUPPORT_RUBY)
  areAllEnginesAdded &= round_local_node_addengine(node, (RoundScriptEngine*)round_ruby_engine_new());
#endif

#if defined(ROUND_SUPPORT_PYTHON)
  areAllEnginesAdded &= round_local_node_addengine(node, (RoundScriptEngine*)round_python_engine_new());
#endif

  return areAllEnginesAdded;
}

/****************************************
 * round_local_node_initsystemmethods
 ****************************************/

bool round_local_node_initsystemmethods(RoundLocalNode* node)
{
  bool areAllMethodsAdded;

  areAllMethodsAdded = true;

  areAllMethodsAdded &= round_local_node_setmethod(node, round_system_method_setmethod_new());
  areAllMethodsAdded &= round_local_node_setmethod(node, round_system_method_removemethod_new());

  areAllMethodsAdded &= round_local_node_setmethod(node, round_system_method_setregistry_new());
  areAllMethodsAdded &= round_local_node_setmethod(node, round_system_method_getregistry_new());
  areAllMethodsAdded &= round_local_node_setmethod(node, round_system_method_removeregistry_new());

  return areAllMethodsAdded;
}

/****************************************
 * round_local_node_destory
 ****************************************/

bool round_local_node_destory(RoundLocalNode* node)
{
  if (!node)
    return false;

  if (!round_local_node_stop(node))
    return false;

  round_method_manager_delete(node->methodMgr);
  round_registry_manager_delete(node->regMgr);
  round_thread_manager_delete(node->threadMgr);
  round_message_manager_delete(node->msgMgr);
  round_trigger_manager_delete(node->triggerMgr);

  return true;
}

/****************************************
* round_local_node_delete
****************************************/

bool round_local_node_delete(RoundLocalNode* node)
{
  if (!node)
    return false;

  if (!round_local_node_destory(node))
    return false;

  round_node_destroy((RoundNode*)node);

  free(node);

  return true;
}

/****************************************
 * round_local_node_clear
 ****************************************/

bool round_local_node_clear(RoundLocalNode* node)
{
  if (!node)
    return false;

  bool isSuccess = true;
  isSuccess &= round_cluster_manager_clear(node->clusterMgr);

  return isSuccess;
}

/****************************************
 * round_local_node_start
 ****************************************/

bool round_local_node_start(RoundLocalNode* node)
{
  if (!node)
    return false;

  if (!round_local_node_stop(node))
    return false;

  bool isSuccess = true;

  isSuccess &= round_local_node_clear(node);

  isSuccess &= round_cluster_manager_addnode(node->clusterMgr, (RoundNode*)node);
  isSuccess &= round_thread_manager_start(node->threadMgr);
  isSuccess &= round_trigger_manager_start(node->triggerMgr);

  if (!isSuccess) {
    round_local_node_stop(node);
    return false;
  }

  return true;
}

/****************************************
 * round_local_node_stop
 ****************************************/

bool round_local_node_stop(RoundLocalNode* node)
{
  bool isSuccess = true;

  if (!node)
    return false;

  isSuccess &= round_thread_manager_stop(node->threadMgr);
  isSuccess &= round_trigger_manager_stop(node->triggerMgr);

  return isSuccess;
}

/****************************************
 * round_local_node_isrunning
 ****************************************/

bool round_local_node_isrunning(RoundLocalNode* node)
{
  if (!node)
    return false;

  if (!round_thread_manager_isrunning(node->threadMgr))
    return false;

  if (!round_trigger_manager_isrunning(node->threadMgr))
    return false;
  
  return true;
}

/****************************************
 * round_local_node_setmethod
 ****************************************/

bool round_local_node_setmethod(RoundLocalNode* node, RoundMethod* method)
{
  const char* methodName;
  RoundMethod* exMethod;

  if (!node)
    return false;

  methodName = round_method_getname(method);

  exMethod = round_method_manager_getmethod(node->methodMgr, methodName);
  if (exMethod) {
    if (round_method_isfinal(exMethod))
      return false;
    round_method_manager_removemethod(node->methodMgr, methodName);
  }

  if (!round_method_manager_addmethod(node->methodMgr, method))
    return false;

  return true;
}

/****************************************
 * round_local_node_removemethod
 ****************************************/

bool round_local_node_removemethod(RoundLocalNode* node, const char* name)
{
  RoundMethod* method;

  if (!node)
    return false;

  method = round_method_manager_getmethod(node->methodMgr, name);
  if (!method)
    return false;

  if (round_method_isfinal(method))
    return false;

  return round_method_manager_removemethod(node->methodMgr, name);
}

/****************************************
 * round_local_node_isfinalmethod
 ****************************************/

bool round_local_node_isfinalmethod(RoundLocalNode* node, const char* name)
{
  RoundMethod* method;

  if (!node)
    return false;

  method = round_method_manager_getmethod(node->methodMgr, name);
  if (!method)
    return false;

  return round_method_isfinal(method);
}

/****************************************
 * round_local_node_addengine
 ****************************************/

bool round_local_node_addengine(RoundLocalNode* node, RoundScriptEngine* engine)
{
  if (!node)
    return false;

  if (!round_method_manager_addengine(node->methodMgr, engine))
    return false;

  round_script_engine_setlocalnode(engine, node);

  return true;
}

/****************************************
 * round_local_node_getenginebylanguage
 ****************************************/

RoundScriptEngine* round_local_node_getenginebylanguage(RoundLocalNode* node, const char* lang)
{
  if (!node)
    return NULL;

  return round_method_manager_getengine(node->methodMgr, lang);
}

/****************************************
 * round_local_node_setregistry
 ****************************************/

bool round_local_node_setregistry(RoundLocalNode* node, const char* key, const char* val)
{
  RoundRegistry* reg;

  if (!node)
    return false;

  reg = round_registry_manager_get(node->regMgr, key);
  if (!reg) {
    reg = round_registry_new();
    if (!reg)
      return false;
    round_registry_setkey(reg, key);
    if (!round_registry_manager_set(node->regMgr, reg)) {
      round_registry_delete(reg);
      return false;
    }
  }

  round_registry_setstring(reg, val);
  round_registry_setts(reg, time(NULL));
  round_registry_setlts(reg, round_local_node_getclock(node));

  return true;
}

/****************************************
 * round_local_node_getregistry
 ****************************************/

RoundRegistry* round_local_node_getregistry(RoundLocalNode* node, const char* key)
{
  if (!node)
    return false;

  return round_registry_manager_get(node->regMgr, key);
}

/****************************************
 * round_local_node_removeregistry
 ****************************************/

bool round_local_node_removeregistry(RoundLocalNode* node, const char* key)
{
  if (!node)
    return false;

  return round_registry_manager_remove(node->regMgr, key);
}

/****************************************
 * round_local_node_execjsonrequest
 ****************************************/

bool round_local_node_execjsonrequest(RoundLocalNode* node, const char *jsonReq, const char **jsonRes, RoundError* err)
{
  if (!node || !jsonReq || !jsonRes || !err)
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
  
  RoundJSON* json = round_json_new();
  if (!json)
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
  
  if (!round_json_parse(json, jsonReq, err)) {
    round_json_delete(json);
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INVALID_REQUEST, err);
  }
  
  // Post request
  
  RoundJSONObject* reqObj = round_json_getrootobject(json);
  if (!reqObj) {
    round_json_delete(json);
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
  }
  
  RoundJSONObject *resObj = NULL;
  if (round_local_node_postmessage(node, reqObj, &resObj, err)) {
    round_json_delete(json);
    return false;
  }


  if (resObj) {
    round_json_object_tocompactstring(resObj, jsonRes);
    round_json_object_delete(resObj);
  }

  round_json_delete(json);
  
  return true;
}

/****************************************
 * round_local_node_execmessage
 ****************************************/

bool round_local_node_execmessage(RoundLocalNode* node, RoundMessage* msg, RoundJSONObject** resObj, RoundError* err)
{
  if (!node || !msg || !resObj || !err)
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);

  // Parse JSON-RPC request

  const char* msgContent = round_message_getstring(msg);
  if (!msgContent)
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INVALID_REQUEST, err);

  RoundJSON* json = round_json_new();
  if (!json)
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);

  if (!round_json_parse(json, msgContent, err)) {
    round_json_delete(json);
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INVALID_REQUEST, err);
  }

  // Post request

  RoundJSONObject* reqObj = round_json_getrootobject(json);
  if (!reqObj) {
    round_json_delete(json);
    return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
  }

  return round_local_node_postmessage(node, reqObj, resObj, err);
}

/****************************************
 * round_local_node_postrequest
 ****************************************/

bool round_local_node_postrequest(RoundLocalNode* node, RoundMessage* msg, RoundJSONObject** resultObj, RoundError* err)
{
  if (!msg) {
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INVALID_PARAMS, err);
    return false;
  }

  if (!node) {
    round_message_delete(msg);
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }

  if (!resultObj || !err) {
    round_message_delete(msg);
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INVALID_PARAMS, err);
    return false;
  }

  if (!round_message_setnotifyenabled(msg, true)) {
    round_message_delete(msg);
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }

  round_local_node_message_seterror(msg, err);
  round_local_node_message_setresponsejsonobject(msg, resultObj);

  if (!round_message_manager_pushmessage(node->msgMgr, msg)) {
    round_message_delete(msg);
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }

  if (!round_message_timedwaitnotify(msg, round_local_node_getrequesttimeout(node))) {
    round_message_setnotifyenabled(msg, false);
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }

  return true;
}

/****************************************
 * round_local_node_postjsonrequest
 ****************************************/

bool round_local_node_postjsonrequest(RoundLocalNode* node, RoundJSONObject* reqObj, RoundJSONObject** resultObj, RoundError* err)
{
  const char* reqStr = NULL;
  if (!round_node_jsonrpcrequest2string(node, reqObj, &reqStr, err))
    return false;

  RoundMessage* msg = round_local_node_message_new();
  if (!msg) {
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }
  round_message_setstring(msg, reqStr);

  return round_local_node_postrequest(node, msg, resultObj, err);
}

/****************************************
 * round_local_node_execrequest
 ****************************************/

bool round_local_node_execrequest(RoundLocalNode* node, RoundJSONObject* reqObj, RoundJSONObject** resObj, RoundError* err)
{
  // Set Response

  *resObj = round_json_rpc_response_new();
  if (!resObj) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR);
    return false;
  }

  // Set id and timestamp

  round_json_rpc_setrequestid(*resObj, reqObj);
  round_json_rpc_settimestamp(*resObj, round_local_node_getclock(node));

  // Check node

  if (!node) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR);
    round_json_rpc_seterror(*resObj, err);
    return false;
  }

  // Check request

  if (!round_json_object_ismap(reqObj)) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INVALID_REQUEST);
    round_json_rpc_seterror(*resObj, err);
    return false;
  }

  /*
   // Check dest
   
   if (!nodeReq->isDestValid()) {
   setError(RPC::JSON::ErrorCodeInvalidParams, error);
   return false;
   }
   
   bool isDestHash = nodeReq->isDestHash();
   if (isDestHash) {
   std::string nodeHash;
   if (nodeReq->getDest(&nodeHash)) {
   NodeGraph *nodeGraph = getNodeGraph();
   if (!nodeGraph->isHandleNode(this, nodeHash)) {
   setError(RPC::JSON::ErrorCodeMovedPermanently, error);
   return false;
   }
   }
   }
   
   // Exec Method (One node)
   
   bool isDestOne = nodeReq->isDestOne();
   if (isDestOne) {
   return execMethod(nodeReq, nodeRes, error);
   }
   
   // Exec Method (Multi node)
   
   JSONArray *batchArray = new JSONArray();
   nodeRes->setResult(batchArray);
   
   Error thisNodeError;
   NodeResponse *thisNodeRes = new NodeResponse();
   execMethod(nodeReq, thisNodeRes, &thisNodeError);
   thisNodeRes->setDest(this);
   batchArray->add(thisNodeRes);
   
   NodeList otherNodes;
   if (nodeReq->isDestAll()) {
   getAllOtherNodes(&otherNodes);
   }
   else if (nodeReq->hasQuorum()) {
   size_t quorum;
   if (nodeReq->getQuorum(&quorum)) {
   getQuorumNodes(&otherNodes, quorum);
   }
   }
   for (NodeList::iterator node = otherNodes.begin(); node != otherNodes.end();
   node++) {
   Error otherNodeError;
   NodeResponse *otherNodeRes = new NodeResponse();
   (*node)->postMessage(nodeReq, otherNodeRes, &otherNodeError);
   otherNodeRes->setDest((*node));
   batchArray->add(otherNodeRes);
   }
   */

  // Exec Message

  const char* method = NULL;
  if (!round_json_rpc_getmethod(reqObj, &method) && (0 < round_strlen(method))) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INVALID_REQUEST);
    round_json_rpc_seterror(*resObj, err);
    return false;
  }

  const char* params = NULL;
  round_json_rpc_getparamsstring(reqObj, &params);

  RoundJSONObject* resultObj = NULL;
  bool isMethodExecuted = round_method_manager_execmethod(node->methodMgr, method, params, &resultObj, err);
  if (isMethodExecuted) {
    if (resultObj) {
      round_json_rpc_setresult(*resObj, resultObj);
      round_json_object_delete(resultObj);
    }
  }
  else {
    round_json_rpc_seterror(*resObj, err);
  }

  /*
   bool isMethodExecuted = false;
   bool isMethodSuccess = false;
   
   if (isAliasMethod(name)) {
   isMethodExecuted = true;
   isMethodSuccess = execAliasMethod(nodeReq, nodeRes, error);
   }
   
   if (!isMethodExecuted) {
   setError(ROUND_RPC_ERROR_CODE_METHOD_NOT_FOUND, error);
   return false;
   }
   
   if (!isMethodSuccess)
   return false;
   
   if (!hasRoute(name)) {
   return true;
   }
   
   NodeResponse routeNodeRes;
   if (!execRoute(name, nodeRes, &routeNodeRes, error)) {
   return false;
   }
   
   nodeRes->set(&routeNodeRes);
   */

  return isMethodExecuted;
}

/****************************************
 * round_local_node_postmessage
 ****************************************/

bool round_local_node_postmessage(RoundLocalNode* node, RoundJSONObject* reqObj, RoundJSONObject** resObj, RoundError* err)
{
  if (!node || !reqObj || !resObj || !err) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR);
    return false;
  }

  *resObj = NULL;
  
  // Updated local clock

  round_local_node_updateclockbyjsonobject(node, reqObj);

  // Check request

  if (!round_json_object_ismap(reqObj) && !round_json_object_isarray(reqObj)) {
    round_error_setjsonrpcerrorcode(err, ROUND_RPC_ERROR_CODE_INVALID_REQUEST);
    return false;
  }

  // Single request

  if (round_json_object_ismap(reqObj))
    return round_local_node_execrequest(node, reqObj, resObj, err);

  // Batch request

  if (round_json_object_isarray(reqObj)) {
    *resObj = round_json_array_new();
    size_t msgArrayCnt = round_json_array_size(reqObj);
    size_t n;
    for (n = 0; n < msgArrayCnt; n++) {
      RoundJSONObject* reqArrayObj = round_json_array_get(reqObj, n);
      RoundJSONObject* resArrayObj = NULL;
      round_local_node_execrequest(node, reqArrayObj, &resArrayObj, err);
      if (reqArrayObj) {
        round_json_array_append(*resObj, resArrayObj);
        round_json_object_delete(resArrayObj);
      }
    }
    return true;
  }

  return round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
}
