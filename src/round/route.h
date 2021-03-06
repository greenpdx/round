/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUND_ROUTE_H_
#define _ROUND_ROUTE_H_

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <round/util/strings.h>
#include <round/util/map.h>

#ifdef  __cplusplus
extern "C" {
#endif
  
/****************************************
 * Data Type
 ****************************************/

typedef struct {
  size_t tokenCnt;
  char **tokens;
  void **tokenObjs;
} RoundRouteObject;

typedef struct {
  RoundString *key;
  RoundString *value;
  clock_t ts;
  clock_t lts;
} RoundRoute;

typedef struct {
  RoundMap *map;
} RoundRouteManager;
  
/****************************************
 * Function (Route Object)
 ****************************************/
  
RoundRouteObject* round_route_object_new();
bool round_route_object_init(RoundRouteObject* obj);
bool round_route_object_delete(RoundRouteObject* obj);
bool round_route_object_clear(RoundRouteObject* obj);

/****************************************
 * Function (Route)
 ****************************************/
  
RoundRoute *round_route_new();
bool round_route_delete(RoundRoute *route);
  
#define round_route_setkey(reg, val) round_string_setvalue(reg->key, val)
#define round_route_getkey(reg) round_string_getvalue(reg->key)

#define round_route_setvalue(reg, val) round_string_setvalue(reg->value, val)
#define round_route_getvalue(reg) round_string_getvalue(reg->value)

#define round_route_setts(reg, val) (reg->ts = val)
#define round_route_getts(reg) (reg->ts)

#define round_route_setlts(reg, val) (reg->lts = val)
#define round_route_getlts(reg) (reg->lts)

/****************************************
 * Function (Map)
 ****************************************/
  
RoundMap *round_route_map_new();
  
#define round_route_map_delete(map) round_map_delete(map)
#define round_route_map_size(map) round_map_size(map)
#define round_route_map_set(map, reg) round_map_setobject(map, round_route_getkey(reg), reg)
#define round_route_map_get(map, name) ((RoundRoute*)round_map_getobjectbykey(map, name))
#define round_route_map_remove(map, name) round_map_removeobjectbykey(map, name)

/****************************************
 * Function (Manager)
 ****************************************/
  
RoundRouteManager *round_route_manager_new();
bool round_route_manager_delete(RoundRouteManager *mgr);
  
#define round_route_manager_set(mgr, reg) round_route_map_set(mgr->map, reg)
#define round_route_manager_get(mgr, name) round_route_map_get(mgr->map, name)
#define round_route_manager_remove(mgr, name) round_route_map_remove(mgr->map, name)
#define round_route_manager_size(mgr) round_route_map_size(mgr->map)
  
#ifdef  __cplusplus
}
#endif

#endif
