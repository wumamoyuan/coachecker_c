#ifndef _AABACIO_H
#define _AABACIO_H

#include "AABACInstance.h"

/**
 * Read an AABAC instance from a file
 * 
 * @param filename[in]: The path of the file to read
 * @return The AABAC instance
 */
AABACInstance *readAABACInstance(char *filename);

/**
 * Read an ARBAC instance from a file and convert it to an AABAC instance
 * 
 * @param filename[in]: The path of the file to read
 * @return The AABAC instance
 */
AABACInstance *readARBACInstance(char *filename);

/**
 * Write an AABAC instance to a file
 * 
 * @param pInst[in]: The AABAC instance to write
 * @param filename[in]: The path of the file to write
 * @return 0 if write successfully, error code if failed
 */
int writeAABACInstance(AABACInstance *pInst, char *filename);

#endif