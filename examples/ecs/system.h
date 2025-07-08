#ifndef SYSTEM_H
#define SYSTEM_H

#include "ecs.h"

void parentSystem(ECS * ecs);
void physicsSystem(ECS * ecs);
void transformSystem(ECS * ecs);
void renderSystem(ECS * ecs);

#endif // SYSTEM_H