#include "room.h"
#include "save.h"
#include "mercdb.h"
#include "def.h"

void debug_save_world( ) __attribute__ ((constructor));

void debug_save_world( )
{
    for (auto r: roomPrototypes) {
        save_room_objects( r );
        save_room_mobiles( r );
    }
}


