/*
 * luabson.c
 *
 *  Created on: Jun 22, 2015
 *      Author: blc
 */

#include <bson.h>

#include <luaapi/luabson.h>

static const luaL_Reg funs[] = {
//		{"version", ldb_version},
//		{"isSupported", ldb_isSupported},
//
//		//////////////////// pool
//		{"newPool", ldb_pool_new},
//		{"setInitialConn", ldb_pool_setInitialConn},
//		{"setMaxConn", ldb_pool_setMaxConn},
//		{"setSqlTimeout", ldb_pool_setSqlTimeout},
//		{"start", ldb_pool_start},
//		{"stop", ldb_pool_stop},
//		{"freePool", ldb_pool_free},
//		{"getConntion", ldb_pool_getConn},
//		{"active", ldb_pool_active},
//
//		/////////////////// connection
//		{"execute", ldb_conn_execute},
//		{"executeQuery", ldb_conn_executeQuery},
//		{"close", ldb_conn_close},
//		{"beginTransaction", ldb_conn_beginTransaction},
//		{"commit", ldb_conn_commit},
//		{"rollback", ldb_conn_rollback},
//		{"prepareStatement", ldb_conn_prepareStatement},
//
//		/////////////////// prepare statement
//		{"setString", ldb_pst_setString},
//		{"setInt", ldb_pst_setInt},
//		{"setLLong", ldb_pst_setLLong},
//		{"setDouble", ldb_pst_setDouble},
//		{"executePst", ldb_pst_execute},
//		{"executePstQuery", ldb_pst_executeQuery},
//
//		///////////////////// result
//		{"next", ldb_rs_next},
//		{"getColumnCount", ldb_rs_getColumnCount},
//		{"getColumnName", ldb_rs_getColumnName},
//		{"getColumnSize", ldb_rs_getColumnSize},
//		{"isnull", ldb_rs_isnull},
//		// get value,
//		{"getString", ldb_rs_getString},
//		{"getStringByName", ldb_rs_getStringByName},
//		{"getInt", ldb_rs_getInt},
//		{"getIntByName", ldb_rs_getIntByName},
//		{"getLLong", ldb_rs_getLLong},
//		{"getLLongByName", ldb_rs_getLLongByName},
//		{"getDouble", ldb_rs_getDouble},
//		{"getDoubleByName", ldb_rs_getDoubleByName},
//
//		//////////////////// help function ///
//		{"get", ldb_help_nosqlGet},
//		{"set", ldb_help_nosqlSet},
		{NULL, NULL}
};

LUA_API int luaopen_bson(lua_State* l) {
	luaL_register(l, LUA_BSON_LIBNAME, funs);
	return 1;
}
