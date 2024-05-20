/* $Id$
 *
 * ruffina, 2004
 */
#include "move_utils.h"
#include "exitsmovement.h"
#include "portalmovement.h"
#include "movetypes.h"
#include "transfermovement.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "character.h"
#include "affect.h"
#include "skill.h"
#include "room.h"
#include "roomutils.h"
#include "core/object.h"
#include "clanreference.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"

#include "def.h"

CLAN(none);
WEARLOC(none);
WEARLOC(stuck_in);

int move_char( Character *ch, int door, const char *argument )
{
    return ExitsMovement( ch, door, movetype_resolve( ch, argument ) ).move( );
}

int move_char( Character *ch, EXTRA_EXIT_DATA *peexit, const char *argument )
{
    return ExitsMovement( ch, peexit, movetype_resolve( ch, argument ) ).move( );
}

int move_char( Character *ch, Object *portal )
{
    return PortalMovement( ch, portal ).move( );
}


void transfer_char( Character *ch, Character *actor, Room *to_room, 
                    const char *msgRoomLeave, const char *msgSelfLeave, 
                    const char *msgRoomEnter, const char *msgSelfEnter )
{
    TransferMovement( ch, actor, to_room, 
                      msgRoomLeave, msgSelfLeave, 
                      msgRoomEnter, msgSelfEnter ).move( );
}



/* random room generation procedure */
Room * get_random_room( Character *ch )
{
    Room *room;

    for ( ; ; )
    {
        room = get_room_instance( number_range( 200, 39999 ) ); /* old stuff, but appears all new interesting areas lie beyond this limit */
        if ( room != 0 )
        if ( ch->can_see(room)
        &&   !room->isPrivate()
        &&   room->isCommon()
        &&   !IS_SET(room->areaIndex()->area_flag, AREA_HIDDEN|AREA_SYSTEM)
        &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
        &&   !IS_SET(room->room_flags, ROOM_SOLITARY)
        &&   !IS_SET(room->room_flags, ROOM_SAFE)
        &&   !IS_SET(room->room_flags, ROOM_MANSION)
        &&   (!ch->is_npc() 
               || !IS_SET(ch->act,ACT_AGGRESSIVE)
               || !IS_SET(room->room_flags,ROOM_LAW)))
            break;
    }

    return room;
}

Room * get_random_room_vanish( Character *ch )
{
    Room *target = NULL;
    int count = 0;

    for (auto &room: roomInstances) {
        if (room->area != ch->in_room->area || room == ch->in_room)
            continue;
        
        if (IS_SET(room->room_flags, ROOM_NO_VANISH|ROOM_SAFE)) 
            continue;
        
        if (room->pIndexData->clan != clan_none && room->pIndexData->clan != ch->getClan( )) 
            continue;
        
        if (room->isPrivate( )) 
            continue;
        
        if (!ch->can_see( room )) 
            continue;
    
        if (number_range( 0, count++ ) == 0) 
            target = room;
    }

    return target;
}

bool is_flying( Character *ch )
{
    if (ch->posFlags.isSet(POS_FLY_DOWN))
        return false;

    return can_fly( ch );
}

bool can_fly( Character *ch )
{
    if (IS_AFFECTED(ch, AFF_FLYING))
        return true;
        
    if (ch->getRace( )->getAff( ).isSet( AFF_FLYING ))
        return true;
            
    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content) {
        if (obj->wear_loc == wear_none || obj->wear_loc == wear_stuck_in)
            continue;
        
        if (!obj->affected.findAllWithBits(TO_AFFECTS, AFF_FLYING ).empty())
            return true;
        
        if (!obj->pIndexData->affected.findAllWithBits(TO_AFFECTS, AFF_FLYING ).empty())
            return true;
    }
    
    return false;
}

