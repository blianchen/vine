/*
 * ffid.c
 *
 *  Created on: Mar 6, 2015
 *      Author: blc
 */

#include <exception/sys_exception.h>
#include <mem.h>

#include <ffid.h>

struct _ffid_s {
	uint16_t m_uFirst;
    uint16_t m_uEnd;		// first的值是id:m_ts[m_uIndex], end的值是index:m_uIndex;
    uint16_t m_uRestCnt; 	//剩余的可分配id的个数
    uint16_t size;
    uint16_t *m_ts;
} ffid_s;

ffid_t ffid_create(uint16_t size) {
	ffid_t ffid = MALLOC(sizeof(ffid_s) + size * sizeof(uint16_t));
	ffid->m_ts = (uint16_t*) ( (char*)ffid + sizeof(ffid_s) );

	ffid->size = size;
	ffid->m_uEnd = size - 1;

	uint16_t idx;
	for (idx = 0; idx < size - 1; ++idx) {
		ffid->m_ts[idx] = idx + 1;
	}
	ffid->m_uFirst = 0;
	ffid->m_ts[ffid->m_uEnd] = UINT16_MAX;
	ffid->m_uRestCnt = size;

	return ffid;
}

void ffid_destroy(ffid_t ffid) {
	FREE(ffid);
}

uint16_t ffid_getid(ffid_t ffid) {
    if (ffid->m_uRestCnt == 0) {
    	THROW(sys_exception, "No id is usable, current max id num = %d", ffid->size);
    	// 可以使用的m_uRestCnt个数已经为0
    	return UINT16_MAX;
	}
//	uint16_t uIndex = ffid->m_uFirst % ffid->size;
	uint16_t id = ffid->m_uFirst;
	ffid->m_uFirst = ffid->m_ts[id];
	ffid->m_ts[id] = id;
	--ffid->m_uRestCnt;
	return id;
}

void ffid_releaseid(ffid_t ffid, uint16_t id) {
	// 是否是当前分配的id
//	uint16_t uIndex = id;
//	uIndex = id % ffid->size;
	if (ffid->m_ts[id] != id) {
		return;
	}
	// 是否此id下的index还能继续使用,修改此处代码，就可以循环使用此index重新分发id
//	if (MAX_LIMIT <= id || (MAX_LIMIT - id) <= static_cast<uint32>(SIZE))
//	{
//		return;
//	}
//	uint32_t newID = id + ffid->size;
	if (ffid->m_uRestCnt == 0) {
		ffid->m_uFirst = id;
	} else {
		ffid->m_ts[ffid->m_uEnd] = id;
	}
	ffid->m_uEnd = id;
	ffid->m_ts[ffid->m_uEnd] = UINT16_MAX;
	++ffid->m_uRestCnt;
}

int ffid_has(ffid_t ffid, uint16_t id) {
	return ffid->m_ts[id] == id;
}

uint16_t ffid_rest(ffid_t ffid) {
	return ffid->m_uRestCnt;
}

uint16_t ffid_size(ffid_t ffid) {
	return ffid->size - ffid->m_uRestCnt;
}
