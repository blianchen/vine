/*
 * ffid.h
 *
 *  Created on: Mar 6, 2015
 *      Author: blc
 *
 *  安全id分配器,内部使用一个数组,可以分配唯一id。id最大分配数量为数组大小
 *  使用先入先出队列来记录未分配的id，对于每个数组下标Index，id的分配规则为Index + N*Size
 *  适用于频繁的网络连接,而又需要唯一id（或者一定时间内的唯一id）
 *  可以考虑用动态数组实现，但是有时候简单就是美
 *  步骤：
 *  1  初始化。将数组初始化为一个先入先出队列（数组单元保存下一个单元的下标，组成一个单链）
 *  2  分配id。从先入先出队列中取出id（最先压入的id）,将此数组单元Index的值修改为id值，id % Size == Index 为真
 *  3  检测id。检测id % Size == Index ， 数组单元index的值 == id 为真。表明这个id是当前分配出去的id
 *  4  释放id。首先检测id，为真，则计算出新的id。 newID = id + N.如果newID合法，则让先入先出队列中的末尾值保存为newID，将此数组单元入列
 *  5  2和4步骤可无顺序重复执行,对于数组单元Index， 只要单元值 id % Size == Index,则表示这是一个分配出去的id，否则这是一个指向下一个单元或者不可再使用的id
 */

#ifndef _FFID_H_
#define _FFID_H_

#include <stdint.h>

typedef struct _ffid_s* ffid_t;

ffid_t ffid_create(uint16_t size);
void ffid_destroy(ffid_t ffid);
uint16_t ffid_getid(ffid_t ffid);
void ffid_releaseid(ffid_t ffid, uint16_t id);
int ffid_has(ffid_t ffid, uint16_t id);
uint16_t ffid_rest(ffid_t ffid);
uint16_t ffid_size(ffid_t ffid);

#endif /* _FFID_H_ */
