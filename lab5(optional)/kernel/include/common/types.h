#ifndef __TYPES_H__
#define __TYPES_H__

/* 定义数据类型 */
typedef unsigned int   uint32_t;
typedef          int   int32_t;
typedef unsigned short uint16_t;
typedef          short int16_t;
typedef unsigned char  uint8_t;
typedef          char  int8_t;
typedef unsigned int   size_t;

struct stat {
  int16_t type;  // Type of file
  int16_t linkCount; // link number of this file
  int32_t blockCount;// block number of this file
  int32_t size;  // size of this file, byte as unit
};
#endif
