#ifndef LINESET_H
#define LINESET_H

#include "memory.h"
#include "matrix.h"
#include "bitmap.h"

typedef struct Lineset_s
{
    int numVertices;
    int numLines;
    int lineLength;
    int loop;
    float * vertices;
    int * lines;
} Lineset;

Lineset * allocateLineset(Memory * memory, int numVertices, int numLines, int lineLength, int loop)
{
    int verticesSize = numVertices * 3 * sizeof(float);
    int linesSize = numLines * lineLength * sizeof(int);
    Lineset * lineset = (Lineset*)memory->malloc(memory, sizeof(Lineset) + verticesSize + linesSize);
    lineset->numVertices = numVertices;
    lineset->numLines = numLines;
    lineset->lineLength = lineLength;
    lineset->loop = loop;
    lineset->vertices = (float *)((void*)(lineset) + sizeof(Lineset));
    lineset->lines = (int *)((void*)(lineset) + sizeof(Lineset) + verticesSize);
    return lineset;
}

Lineset * createCuboid(Memory * memory, float min[3], float max[3])
{
    Lineset * lineset = allocateLineset(memory, 8, 6, 4, 1);
    float vertices[8][3] = {
        {min[0],min[1],min[2]},
        {max[0],min[1],min[2]},
        {max[0],max[1],min[2]},
        {min[0],max[1],min[2]},
        {min[0],min[1],max[2]},
        {max[0],min[1],max[2]},
        {max[0],max[1],max[2]},
        {min[0],max[1],max[2]},
    };
    memory->memcpy(memory, lineset->vertices, vertices, 8 * 3 * sizeof(float));

    int lines[6][4] = {
        {0,1,2,3},
        {4,5,6,7},
        {3,7,6,2},
        {0,4,5,1},
        {0,4,7,3},
        {1,2,6,5},
    };
    memory->memcpy(memory, lineset->lines, lines, 6 * 4 * sizeof(int));
    return lineset;
}

void renderLineset(Lineset * lineset, Matrix4 matrix, Bitmap * target, EFI_GRAPHICS_OUTPUT_BLT_PIXEL (*colorfunc)(float[3]))
{
    float transformedVertices[lineset->numVertices * 3];
    for (int i = 0; i < lineset->numVertices; i++)
    {
        transformVec3(matrix, &(lineset->vertices[i * 3]), &(transformedVertices[i * 3]));
    }

    for (int i = 0; i < lineset->numLines; i++)
    {
        int * line = (int*)&(lineset->lines[i * lineset->lineLength]);
        for (int j = 1; j < lineset->lineLength; j++)
        {
            int i0 = line[j-1];
            int i1 = line[j];
            float * p0 = &(transformedVertices[i0 * 3]);
            float * p1 = &(transformedVertices[i1 * 3]);
            
            drawShadedLine(p0[0], p0[1], p0[2], p1[0], p1[1], p1[2], target, colorfunc);
        }
        if (lineset->loop)
        {
            int i0 = line[lineset->lineLength - 1];
            int i1 = line[0];
            float * p0 = &(transformedVertices[i0 * 3]);
            float * p1 = &(transformedVertices[i1 * 3]);

            drawShadedLine(p0[0], p0[1], p0[2], p1[0], p1[1], p1[2], target, colorfunc);
        }
    }
}

#endif
