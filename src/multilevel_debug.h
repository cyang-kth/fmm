/**
 * Content
 * Debug information used in development
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_DEBUG_HPP
#define MM_DEBUG_HPP
#include <stdio.h>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h" // must be included for custom operator 

/**
 *  Debug level from 1 to 3
 *  that will deal with various details of debug setting
 *  default debug level is 0
 */

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#ifndef CS_DEBUG_LEVEL
#define CS_DEBUG_LEVEL 0
#endif

#ifndef OPI_DEBUG_LEVEL
#define OPI_DEBUG_LEVEL 0
#endif

#ifndef CPC_DEBUG_LEVEL
#define CPC_DEBUG_LEVEL 0
#endif

#ifndef GC_DEBUG_LEVEL
#define GC_DEBUG_LEVEL 0
#endif

#ifndef UTIL_DEBUG_LEVEL
#define UTIL_DEBUG_LEVEL 0
#endif

#define DEBUG(level) if(level <= DEBUG_LEVEL)

/* Candidate search debug */
#define CS_DEBUG(level) if(level <= CS_DEBUG_LEVEL)

/* Optimal path inference debug */
#define OPI_DEBUG(level) if(level <= OPI_DEBUG_LEVEL)

/* Complete path construction debug */
#define CPC_DEBUG(level) if(level <= CPC_DEBUG_LEVEL)

/* Geometry construction debug*/
#define GC_DEBUG(level) if(level <=GC_DEBUG_LEVEL)

/* Geometry construction debug*/
#define UTIL_DEBUG(level) if(level <=UTIL_DEBUG_LEVEL)

// #define ROUTING_RECORD_DEBUG

#endif // MM_DEBUG_HPP
