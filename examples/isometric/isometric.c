#include "lil_uefi/lil_uefi.h"
#include "shared/rand.h"
#include "shared/math.h"
#include "shared/bitmap.h"
#include "shared/matrix.h"
#include "shared/lineset.h"
#include "shared/draw.h"

#include "sprites/tile_000.h"
#include "sprites/tile_001.h"
#include "sprites/tile_002.h"
#include "sprites/tile_003.h"
#include "sprites/tile_004.h"
#include "sprites/tile_005.h"
#include "sprites/tile_006.h"
#include "sprites/tile_007.h"
#include "sprites/tile_008.h"
#include "sprites/tile_009.h"
#include "sprites/tile_010.h"
#include "sprites/tile_022.h"
#include "sprites/tile_023.h"
#include "sprites/tile_040.h"



#define NULL 0

const EFI_UINT32 TILE_WIDTH = 32;
const EFI_UINT32 TILE_HEIGHT = 32;
const EFI_UINT32 TILE_THICKNESS = 8;

typedef struct Map_s
{
    EFI_UINT32 width;
    EFI_UINT32 height;
    EFI_UINT8 tiles[0];
} Map;

Color_BGRA depthgreen(float point[3])
{
    //z values from approx -50 to 250 seems to be the range for the scaled and rotated teapot
    int zscaled = 15 + MIN(MAX(0,point[2]/5), 240);
    return color(0,zscaled,0);
}

Color_BGRA red(float point[3])
{
    return color(255,0,0);
}

void drawMap(Memory * memory, Map * map, Bitmap * backBuffer, Bitmap ** sprites, int selected[3])
{
    fillBitmap(backBuffer, color(0,0,0));

    EFI_UINT32 dx = TILE_WIDTH / 2;
    EFI_UINT32 dy = dx / 2;
    EFI_UINT32 map_xpos = backBuffer->width / 2 - dx - 0.3; //centered on screen    
    EFI_UINT32 map_ypos = backBuffer->height / 2 - map->height * dy - TILE_HEIGHT + dy + 64; 

    EFI_UINT32 numLayers = 10;
    for (EFI_UINT32 z = 0; z < numLayers; z++){
        for (EFI_UINT32 y = 0; y < map->height; y++)
        {
            for (EFI_UINT32 x = 0; x < map->width; x++)
            {
                EFI_UINT8 tileIndex = map->tiles[y * map->width + x];
                Bitmap * sprite = sprites[tileIndex];
                EFI_UINT32 xs = map_xpos + (x - y) * dx;
                EFI_UINT32 ys = map_ypos + (x + y) * dy - z * TILE_THICKNESS;
                if (sprite != NULL) drawBitmapTransparent(xs, ys, sprite, backBuffer);
            }
        }
    }

    Matrix4 mat;
    make_identity(mat);
    
    
    //Then we scale up so that 1 unit in xy space matches the extent of one tile sprite.
    //The scale factor (around 22) is the length of the "diagonal" in the tile image
    float scaleFactor = 32 * cos(degToRad(45));
    scale(mat, scaleFactor, scaleFactor, scaleFactor);

    // These two rotations take a 2D square in xy space to a squashed 2:1 tile in projected space:
    rotateZ(mat, -degToRad(45));//then rotate 45 degrees along the object z axis to orient it along the diagonal tile grid
    rotateX(mat, degToRad(-60));//rotate the bottom of the square forward by 60 degrees along the scene x axis to achieve the squash

    //We move the coordinate system so the origin matches the top left corner of the top left tile
    translate(mat, map_xpos + 16, map_ypos + 8, 0);

    float layerDepth = 0.41;
    float bottomZ = layerDepth * -1;
    float topZ = layerDepth * (numLayers-1);

    float min[3] = {0,0,bottomZ};
    float max[3] = {map->width,map->height,topZ};    
    Lineset * extents = createCuboid(memory, min, max);
    renderLineset(extents, mat, backBuffer, depthgreen);

    float minCell[3] = {selected[0], selected[1], (selected[2] - 1) * layerDepth};
    float maxCell[3] = {selected[0] + 1, selected[1] + 1, selected[2] * layerDepth};
    Lineset * cell = createCuboid(memory, minCell, maxCell);
    renderLineset(cell, mat, backBuffer, red);

    memory->free(memory, extents);
    memory->free(memory, cell);
}


Map * createMap(Memory * memory, EFI_UINT32 width, EFI_UINT32 height)
{
    Map * map = (Map *) memory->malloc(memory, sizeof(Map) + sizeof(EFI_UINT8) * width * height);
    map->width = width;
    map->height = height;
    return map;
}

//Idea for more advanced generation: Start with an ocean, create a land mass in the middle (like an island). 
//Correct the ocean tiles closest to the island to ones that match the placement of land tiles next to them.
//Possibly make the "island" more like a thin land mass with ocean on both sides, but possible to scroll infinitely in one direction.
void fillMap(Map * map, EFI_UINT32 numTiles)
{
    for (int y = 0; y < map->height; y++)
    {
        for (int x = 0; x < map->width; x++)
        {
            map->tiles[y * map->width + x] = randRange(0, numTiles - 1);
        }
    }
}

int dimensions[3] = {20,20,10};
void move(int pos[3], int axis, int direction)
{
    int newpos = pos[axis] + direction;
    pos[axis] = MAX(0, MIN(dimensions[axis]-1, newpos));
}

// entry point
EFI_UINTN EfiMain(EFI_HANDLE handle, EFI_SYSTEM_TABLE *system_table)
{
    *((int volatile *)&_fltused)=0; //prevent LTO from removing the marker symbol _fltused

    EFI_BOOT_SERVICES *boot_services = system_table->BootServices;
    EFI_STATUS status;
    int ns;

    Memory memory = initializeMemory(boot_services);
    
    EFI_GUID gfx_out_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL * gfx_out_prot;
    status = boot_services->LocateProtocol(&gfx_out_guid, 0, (void **)&gfx_out_prot);
    if (status != 0)
    {
        return status;
    }
    gfx_out_prot->SetMode(gfx_out_prot, 0);

    EFI_UINTN event;
    EFI_INPUT_KEY key;

    EFI_UINT32 *frame_buffer_addr = (EFI_UINT32 *)gfx_out_prot->Mode->frame_buffer_base;
    EFI_UINT64 frame_buffer_size = gfx_out_prot->Mode->frame_buffer_size;

    EFI_TIME time;
    system_table->RuntimeServices->GetTime(&time, 0);

    srand(time.Second);

    Bitmap * tiles[] = {
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_000),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_001),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_002),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_004),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_005),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_006),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_007),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_008),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_009),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_010),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_022),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_023),
        allocateBitmapForBuffer(&memory, 32, 32, 32, tile_040),
        NULL,
        NULL,
        NULL,
        NULL,
    };

    Bitmap screen;
    initializeBitmapFromScreenBuffer(&screen, gfx_out_prot);
    Bitmap * backBuffer = allocateBitmap(screen.width, screen.height, &memory);

    int selected[3] = {1,19,9};
    
    Map * map = createMap(&memory, 20, 20);
    fillMap(map, 16); 
    
    drawMap(&memory, map, backBuffer, tiles, selected);
    drawBitmapToScreen(gfx_out_prot, 0, 0, backBuffer);
    for (;;)
    {
        system_table->BootServices->WaitForEvent(1, &system_table->ConIn->WaitForKey, &event);
        system_table->ConIn->ReadKeyStroke(system_table->ConIn, &key);

        int cont = 0;

        switch(key.ScanCode)
        {
            case EFI_SCAN_Esc: 
                system_table->RuntimeServices->ResetSystem(EFI_RESET_TYPE_Shutdown, 0, 0, NULL);
            case EFI_SCAN_Up:
                move(selected, 1, -1);
                break;
            case EFI_SCAN_Down:
                move(selected, 1, 1);
                break;
            case EFI_SCAN_Left:
                move(selected, 0, -1);
                break;
            case EFI_SCAN_Right:
                move(selected, 0, 1);
                break;
            case EFI_SCAN_PageDown:
                move(selected, 2, -1);
                break;
            case EFI_SCAN_PageUp:
                move(selected, 2, 1);
                break;
            default:
                cont = 1;
                break;
        }
        if (!cont){
            drawMap(&memory, map, backBuffer, tiles, selected);
            drawBitmapToScreen(gfx_out_prot, 0, 0, backBuffer);
        } 
        else switch (key.UnicodeChar)
        {
            case 'm':
                fillScreen(gfx_out_prot, color(0,0,0));
                fillMap(map, 16);
                drawMap(&memory, map, backBuffer, tiles, selected);
                drawBitmapToScreen(gfx_out_prot, 0, 0, backBuffer);      
                break;
            default:              
                break;
        }
    }

    return (0);
}
