#ifndef ECS_H
#define ECS_H

#include "inttypes.h"

typedef uint64_t EntityId; // Type for entity IDs
typedef uint64_t ComponentFlags; // Type for component IDs

static const int PAGE_SIZE = 4096; // Assuming a page size of 4096 bytes
static const int POOL_SIZE = 4096; // Number of instances per component type to allocate for

void initializeEcs();

#endif // ECS_H
