/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#include <boost/test/unit_test.hpp>

#include <round/node_internal.h>
#include <round/server.h>
#include <round/client.h>

#include "RoundTest.h"
#include "NodeTestController.h"

BOOST_AUTO_TEST_SUITE(node)

BOOST_AUTO_TEST_CASE(RemoteNodeNew)
{
  RoundRemoteNode* node = round_remote_node_new();
  BOOST_CHECK(node);

  BOOST_CHECK(round_remote_node_delete(node));
}

BOOST_AUTO_TEST_CASE(RemoteNodeCopy)
{
  const char* TEST_CLUSTER = "round";

  RoundRemoteNode* node0 = round_remote_node_new();
  BOOST_CHECK(node0);
  round_remote_node_setaddress(node0, ROUND_TEST_ADDR);
  round_remote_node_setport(node0, ROUND_TEST_PORT);
  round_remote_node_setclustername(node0, TEST_CLUSTER);

  RoundRemoteNode* node1 = round_remote_node_copy((RoundNode*)node0);
  BOOST_CHECK(node1);

  BOOST_CHECK(round_node_equals((RoundNode*)node0, (RoundNode*)node1));

  BOOST_CHECK(round_remote_node_delete(node0));
  BOOST_CHECK(round_remote_node_delete(node1));
}

#if defined(ROUND_ENABLE_FINDER)

BOOST_AUTO_TEST_CASE(RemoteNodeScriping)
{
  RoundClient* client = round_client_new();
  BOOST_CHECK(client);
  BOOST_CHECK(round_client_start(client));
  Round::Test::Sleep();

  RoundServer* server = round_server_new();
  BOOST_CHECK(server);
  BOOST_CHECK(round_server_start(server));
  Round::Test::Sleep();

  // Get remote node

  ROUND_TEST_RETRY_COUNT_INIT();
  while (round_client_getclustersize(client) < 1) {
    BOOST_TEST_MESSAGE("Cluster is not found ...");
    ROUND_TEST_RETRY_COUNT_CHECK();
    Round::Test::Sleep();
  }
  BOOST_CHECK_EQUAL(round_client_getclustersize(client), 1);

  RoundCluster* cluster = round_client_getclusters(client);
  BOOST_CHECK(cluster);
  RoundNode* node = round_cluster_getnodes(cluster);
  BOOST_CHECK(node);

  // Run Scripts

  Round::NodeTestController nodeTestController;
  nodeTestController.runScriptManagerTest(node);
  nodeTestController.runSystemMethodTest(node);

  BOOST_CHECK(round_client_delete(client));
  BOOST_CHECK(round_server_delete(server));
}

#endif

BOOST_AUTO_TEST_SUITE_END()
