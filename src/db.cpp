/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "mallocexception.h"
#include "fileformatexception.h"
#include "logstream.h"
#include "class.h"
#include "char.h"
#include "grammar_entities_impl.h"

#include "dlscheduler.h"

#include "skillreference.h"
#include "skillgroup.h"

#include "mobilebehavior.h"
#include "areabehavior.h"

#include "fenia/register-impl.h"
#include "feniamanager.h"
#include "wrapperbase.h"

#include "affect.h"
#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

using namespace std;

/*
 * Globals.
 */
Character *                char_list;
Character * newbie_list = 0;
KILL_DATA                kill_table        [MAX_LEVEL];
Object *                object_list;
TIME_INFO_DATA                time_info;
WEATHER_DATA                weather_info;

AUCTION_DATA        *        auction = new auction_data( );

RoomVector roomPrototypes;
RoomVnumMap roomPrototypeMap;
RoomVector roomInstances;
RoomSet roomAffected;

/*
 * Locals.
 */
MOB_INDEX_DATA *        mob_index_hash                [MAX_KEY_HASH];
OBJ_INDEX_DATA *        obj_index_hash                [MAX_KEY_HASH];
char *                        string_hash                [MAX_KEY_HASH];

AREA_DATA *                area_first;
AREA_DATA *                area_last;

char                        str_empty        [1];

int                        top_affect;
int                        top_area;
int                        top_ed;
int                        top_exit;
int                        top_mob_index;
int                        top_obj_index;
int                        top_reset;

int                        top_vnum_mob;
int                        top_vnum_obj;

int mobile_count = 0;
int newobjs = 0;
int newmobs = 0;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.        
 * Tune the others only if you understand what you're doing.
 */

void *                        rgFreeList        [MAX_MEM_LIST];
const int                rgSizeList        [MAX_MEM_LIST]        =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};

int                        nAllocPerm;
int                        sAllocPerm;


/*
 * Local booting procedures.
 */
struct area_file * area_file_list;

struct area_file *
new_area_file(const char *name)
{
    struct area_file *rc = (struct area_file *)alloc_perm(sizeof (struct area_file));

    rc->file_name = str_dup(name);
        
    rc->next = area_file_list;
    area_file_list = rc;
    return rc;
}

/** AreaInstance constructor, setting age to high number to trigger area update. */
AreaInstance::AreaInstance()
            : area(0), age(100), nplayer(0), empty(true)
{

}

void AreaInstance::addRoom(Room *room)
{
    room->areaInstance = this;
    rooms[room->vnum] = room;    
    roomInstances.push_back(room);

    if (isPrimary()) {
        // For prototype rooms (i.e. those on a primary area instance), remember their details for quick access.
        roomPrototypeMap[room->vnum] = room;
        roomPrototypes.push_back(room);
    } else {
        // For instance rooms, remember a link to their prototype.
        room->pIndexData = get_room_index(room->vnum);
        if (!room->pIndexData)
            bug("AreaInstance::addRoom no proto found for %d %s in %s", room->vnum, key.c_str(), area->area_file->file_name);
    }   
}

Room * AreaInstance::getRoom(int vnum)
{
    auto it = rooms.find(vnum);
    if (it == rooms.end())
        return NULL;
    else
        return it->second;
}

bool AreaInstance::isPrimary() const
{
    return area->instances.front() == this;
}

area_data::area_data( ) : behavior( AreaBehavior::NODE_NAME )
{
}

void area_data::addRoomInstance(Room *room, const DLString &key)
{
    getOrCreateInstance(key)->addRoom(room);
}

void area_data::addRoomProto(Room *pRoom)
{
    getDefaultInstance()->addRoom(pRoom);
}

void AreaInstance::eventCharPlaced(Character *ch)
{
    if (ch->is_npc()) {
        mobs[ch->getNPC()->pIndexData->vnum]++;
    } else {
        nplayer++;
        if (empty) {
            empty = false;
            age = 0;
        }
    }
}

void AreaInstance::eventCharRemoved(Character *ch)
{
    if (ch->is_npc()) {
        int vnum = ch->getNPC()->pIndexData->vnum;
        if (mobs[vnum] > 0)
            mobs[vnum]--;
    } else
        nplayer--;
}

const DLString DEFAULT_INSTANCE = "";

AreaInstance * area_data::getDefaultInstance()
{
    if (instances.empty())
        return createInstance(DEFAULT_INSTANCE);

    return instances.front();
}

AreaInstance * area_data::getInstance(const DLString &key)
{
    auto it = instanceMap.find(key);
    if (it != instanceMap.end())
        return it->second;
    else
        return NULL;
}

AreaInstance * area_data::createInstance(const DLString &key)
{
    AreaInstance *ai = new AreaInstance;
    ai->area = this;
    ai->key = key;

    instanceMap[key] = ai;
    instances.push_back(ai);
    notice("Created instance (%s) for area %s.", key.c_str(), name);
    return ai;
}

AreaInstance * area_data::getOrCreateInstance(const DLString &key)
{
    AreaInstance *existing = getInstance(key);
    if (existing)
        return existing;

    return createInstance(key);
}

mob_index_data::mob_index_data( ) 
                     : practicer( skillGroupManager ), 
                       religion( religionManager ),
                       wrapper ( 0 )
{
}

auction_data::auction_data( )
                     : item( NULL ), seller( NULL ), buyer( NULL )
{
}

EXTRA_DESCR_DATA *new_extra_descr(void)
{
    static EXTRA_DESCR_DATA ed_zero;
    EXTRA_DESCR_DATA *ed;
    
    ed = new EXTRA_DESCR_DATA;
    *ed = ed_zero;
    return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
    free_string(ed->keyword);
    free_string(ed->description);
    delete ed;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed != 0; ed = ed->next )
    {
        if ( is_name( name, ed->keyword ) )
            return ed->description;
    }
    return 0;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
          pMobIndex != 0;
          pMobIndex  = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }

    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
        throw FileFormatException( "get_mob_index: vnum %d not found on world startup", vnum );

    return 0;
}



/*
 * Translates room virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
          pObjIndex != 0;
          pObjIndex  = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }

    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
        throw FileFormatException( "get_obj_index: vnum %d not found on world startup", vnum );

    return 0;
}



/*
 * Translates room virtual number to its room index struct.
 */
Room *get_room_index( int vnum )
{
    RoomVnumMap::const_iterator r = roomPrototypeMap.find(vnum);
    if (r != roomPrototypeMap.end())
        return r->second;

    if (DLScheduler::getThis( )->getCurrentTick( ) == 0 && !dreamland->hasOption( DL_BUILDPLOT )) 
        throw FileFormatException( "get_room_index: vnum %d not found on world startup", vnum );

    return 0;
}



/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
    if (pstr == 0 || pstr == str_empty)
        return;

    free_mem( pstr, strlen(pstr) + 1 );
    return;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
char *str_dup( const char *str )
{
    char *str_new;

    if ( !str || !*str )
        return &str_empty[0];

    str_new = ( char* )alloc_mem( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



int memAllocCount = 0, memAllocSize = 0;
/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;
    int *magic;
    int iList;

    memAllocSize += sMem;
    memAllocCount++;

    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
        if ( sMem <= rgSizeList[iList] )
            break;
    }

    if ( iList == MAX_MEM_LIST )
    {
        bug( "Alloc_mem: size %d too large.", sMem );
        exit( 1 );
    }

    if ( rgFreeList[iList] == 0 )
    {
        pMem              = alloc_perm( rgSizeList[iList] );
    }
    else
    {
        pMem              = rgFreeList[iList];
        rgFreeList[iList] = * ((void **) rgFreeList[iList]);
    }

    magic = (int *) pMem;
    *magic = MAGIC_NUM;
/*  ( char* )pMem += sizeof( *magic );

    return pMem;*/ return (void *)( (char *)pMem+sizeof(*magic) );
}

/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    int iList;
    int *magic;

    memAllocSize -= sMem;
    memAllocCount--;

    pMem = (void*)( (char *)pMem-sizeof(*magic));
    magic = (int *) pMem;

    if (*magic != MAGIC_NUM)
    {
            bug("Attempt to recyle invalid memory of size %d.",sMem);
            bug((char*) pMem + sizeof(*magic),0);
            abort( );
            return;
    }

    *magic = 0;
    sMem += sizeof(*magic);

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
            if ( sMem <= rgSizeList[iList] )
                    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
            bug( "Free_mem: size %d too large.", sMem );
            exit( 1 );
    }

    * ((void **) pMem) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;
}


/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
        static char *pMemPerm;
        static int iMemPerm;
        void *pMem;

        while ( sMem % sizeof(long) != 0 )
                sMem++;

        if ( sMem > MAX_PERM_BLOCK )
        {
                bug( "Alloc_perm: %d too large.", sMem );
                exit( 1 );
        }

        if ( pMemPerm == 0 || iMemPerm + sMem > MAX_PERM_BLOCK )
        {
                iMemPerm = 0;

                if ( ( pMemPerm = ( char* )calloc( 1, MAX_PERM_BLOCK ) ) == 0 )
                {
                        perror( "Alloc_perm" );
                        exit( 1 );
                }
        }

        pMem        = pMemPerm + iMemPerm;
        iMemPerm   += sMem;
        nAllocPerm += 1;
        sAllocPerm += sMem;
        return pMem;
}


