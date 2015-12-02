/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <round/node_internal.h>

/****************************************
* round_remote_node_new
****************************************/

RoundRemoteNode* round_remote_node_new(void)
{
  RoundRemoteNode* node;

  node = (RoundRemoteNode*)malloc(sizeof(RoundRemoteNode));

  if (!node)
    return NULL;

  round_node_init((RoundNode*)node);
  round_oo_setdescendantdestoroyfunc(node, round_remote_node_destory);

  round_node_setpostmessagefunc(node, round_remote_node_postmessage);

  return node;
}

/****************************************
 * round_remote_node_copy
 ****************************************/

RoundRemoteNode* round_remote_node_copy(RoundNode* node)
{
  if (!node)
    return NULL;

  RoundRemoteNode* newNode = round_remote_node_new();
  if (!newNode)
    return NULL;

  const char* addr;
  if (round_node_getaddress(node, &addr)) {
    round_remote_node_setaddress(newNode, addr);
  }

  int port;
  if (round_node_getport(node, &port)) {
    round_remote_node_setport(newNode, port);
  }

  const char* clusterName;
  if (round_node_getclustername(node, &clusterName)) {
    round_remote_node_setclustername(newNode, clusterName);
  }

  return newNode;
}

/****************************************
 * round_remote_node_destory
 ****************************************/

bool round_remote_node_destory(RoundRemoteNode* node)
{
  if (!node)
    return false;

  return true;
}

/****************************************
* round_remote_node_delete
****************************************/

bool round_remote_node_delete(RoundRemoteNode* node)
{
  if (!node)
    return false;

  round_remote_node_destory(node);
  round_node_destroy((RoundNode*)node);

  free(node);

  return true;
}

/****************************************
 * round_remote_node_sethttpresponse
 ****************************************/

bool round_remote_node_sethttpresponse(RoundRemoteNode* node, const char *resContent, RoundJSONObject* resMap, RoundError* err)
{
  RoundJSON *json = round_json_new();
  if (!json) {
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_INTERNAL_ERROR, err);
    return false;
  }
  
  bool isSuccess = round_json_parse(json, resContent, err);
  if (isSuccess) {
    RoundJSONObject *resultObj = round_json_poprootobject(json);
    round_json_map_c
  }
  else {
    round_node_rpcerrorcode2error(node, ROUND_RPC_ERROR_CODE_PARSER_ERROR, err);
    return false;
  }
  
  round_json_delete(json);
  
  return isSuccess;
}

/****************************************
 * round_remote_node_postjsonrequest
 ****************************************/

bool round_remote_node_postjsonrequest(RoundRemoteNode* node, RoundJSONObject* reqObj, RoundJSONObject* resMap, RoundError* err)
{
  const char* reqContent = NULL;
  if (!round_node_jsonrpcrequest2string(node, reqObj, &reqContent, err))
    return false;
  
  return round_remote_node_posthttpjsonrequest(node, reqContent, resMap, err);
}

/****************************************
 * round_remote_node_postmessage
 ****************************************/

bool round_remote_node_postmessage(RoundRemoteNode* node, RoundJSONObject* reqMap, RoundJSONObject* resMap, RoundError* err)
{
  RoundJSONObject* resultObj;
  if (round_remote_node_postjsonrequest(node, reqMap, resMap, err)) {
    if (resultObj) {
      round_json_rpc_setresult(resMap, resultObj);
      round_json_object_delete(resultObj);
    }
  }
  else {
    round_json_rpc_seterror(resMap, err);
  }
  
  return true;
}
