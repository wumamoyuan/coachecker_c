#ifndef _AABACIO_H
#define _AABACIO_H

#include "AABACInstance.h"

#define USERS "Users"
#define ATTRIBUTES "Attributes"
#define DEFAULT_VALUE "Default Value"
#define UAV "UAV"
#define RULES "Rules"
#define SPEC "Spec"

/****************************************************************************************************
 * 功能：从指定文件中读取AABAC实例
 * 参数：
 *      @filename[in]: 待读取的文件名
 * 返回值：
 *      AABAC实例
 ***************************************************************************************************/
AABACInstance *readAABACInstance(char *filename);

/****************************************************************************************************
 * 功能：将AABAC实例写入指定文件中
 * 参数：
 *      @pInst[in]： AABAC实例
 *      @filename[in]: 待写入的文件名
 * 返回值：
 *      0：成功，-1：失败
 ***************************************************************************************************/
int writeAABACInstance(AABACInstance *pInst, char *filename);

#endif