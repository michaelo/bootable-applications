#include "system.h"
#include "ecs.h"

void addChild(ECS * ecs, EntityId parent, EntityId child)
{
    if (!(ecs->entityData.entities[parent].components & COMPONENT_CHILDREN))
    {
        return;
    }
    Children * children = &ecs->componentData.children[parent];
    list_add(&children->childrenList, child);
}

void removeChild(ECS * ecs, EntityId parent, EntityId child)
{
    if (!(ecs->entityData.entities[parent].components & COMPONENT_CHILDREN))
    {
        return;
    }
    Children * children = &ecs->componentData.children[parent];
    list_remove(&children->childrenList, child);
}

void parentSystem(ECS * ecs)
{
    //update children for entities that have a parent component pointing to them
    for (EntityId i = 0; i < ecs->entityData.nextEntityId; i++)
    {        
        Entity *entity = &ecs->entityData.entities[i];
        if (entity->components & COMPONENT_PARENT)
        {            
            // Get parent component
            Parent * parent = &ecs->componentData.parent[i];
            if (parent->parent != parent->previousParent)
            {
                if (parent->previousParent != UndefinedEntity)
                {
                    removeChild(ecs, parent->previousParent, i);
                }
                if (parent->parent != UndefinedEntity)
                {
                    addChild(ecs, parent->parent, i);
                }
                parent->previousParent = parent->parent;
            }
        }
    }
}