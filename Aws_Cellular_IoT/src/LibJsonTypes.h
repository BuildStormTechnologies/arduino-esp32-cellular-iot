/**
 * \copyright Copyright (c) 2021, Buildstorm Pvt Ltd
 *
 * \file lib_json.h
 * \brief JSON library header file.
 */

#ifndef __LIB_JSON_TYPES_H__
#define __LIB_JSON_TYPES_H__

#include <stdbool.h>
#include <stdint.h>

#define LENGTH_KEY_SIZE 32 // Max 3-char keyStr ex:"typ", "dat", "cmd", "sts" etc

/**
 * @brief A structure to represent key-value pairs in a JSON string.
 */
typedef struct
{
    char *keyStr;    /*!< A key */
    char *pValueStr; /*!< Value associated with the key */
} tagStructure_st;

#define MAX_JSON_ARRAY_OBJS 10
#define MAX_JSON_ARRAY_INTEGERS 10

/**
 * @brief A structure to represent array of JSON objects.
 */
typedef struct
{
    uint8_t numOfJosnObjs_u8;                /*!< Number of JSON objects in array */
    char jsonObjs[MAX_JSON_ARRAY_OBJS][150]; /*!< Array of objects */
} jsonArray_st;

/**
 * @brief A structure to represent array of integers
 */
typedef struct
{
    uint32_t arrayElements[MAX_JSON_ARRAY_INTEGERS]; /*!< Array of integaers */
} jsonArrayList_st;

#endif //__LIB_JSON_TYPES_H__