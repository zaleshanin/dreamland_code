/* $Id$
 *
 * ruffina, 2004
 */
#include "fight_position.h"

#include "commonattributes.h"
#include "playerattributes.h"
#include "skill.h"
#include "skillreference.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "wiznet.h"
#include "interp.h"
#include "loadsave.h"
#include "act.h"
#include "occupations.h"

#include "merc.h"
#include "def.h"

GSN(cavalry);
CLAN(none);

/*
 * Stop fights.
 */
void stop_fighting( Character *ch, bool fBoth )
{
    Character *fch;

    for( fch = char_list; fch; fch = fch->next )
    {
        if( fch == ch || ( fBoth && fch->fighting == ch ) )
        {
            fch->fighting    = 0;
            fch->position    = fch->is_npc() ? fch->getNPC()->default_pos : POS_STANDING;

            if (IS_AFFECTED(fch, AFF_SLEEP)) {
                REMOVE_BIT(fch->affected_by, AFF_SLEEP);
                affect_bit_strip(fch, &affect_flags, AFF_SLEEP);
            }

            update_pos( fch );
            
            if (!fch->is_npc()) {
                fch->getPC()->getAttributes().handleEvent(StopFightArguments(fch->getPC()));
            }
        }
  }
}

static bool dismount_attacked( Character *ch )
{
    if (RIDDEN(ch)) {
        if (!ch->is_npc( ))
            return true;

        if (!gsn_cavalry->available( RIDDEN(ch) )) 
            return true;

        return false;
    }

    if (MOUNTED(ch)) {
        if (!ch->mount->is_npc( ))
            return true;

        if (!gsn_cavalry->available( ch ))
            return true;

        return false;
    }

    return false;
}

/*
 * Start fights, configuring ch to attack victim and adjusting positions.
 */
void set_fighting( Character *ch, Character *victim )
{
    if (ch->fighting != 0)
        return;

    if (ch->position <= POS_INCAP)
        return;
    
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
        REMOVE_BIT(ch->affected_by, AFF_SLEEP);
        affect_bit_strip(ch, &affect_flags, AFF_SLEEP);
    }
    
    if (dismount_attacked( ch ))
        interpret_raw( ch, "dismount" );
    
    if (dismount_attacked( victim ))
        interpret_raw( victim, "dismount" );

    if (ch->in_room == victim->in_room)
    {
        ch->fighting = victim;
        ch->position = POS_FIGHTING;
    }
}


/*
 * Set position of a victim.
 */
void update_pos( Character *victim )
{
    if (victim->position > POS_SITTING)
        victim->on = 0;

    if ( victim->hit > 0 )
    {
        if ( victim->position <= POS_STUNNED ) {
            if(IS_AFFECTED(victim, AFF_SLEEP)) 
                victim->position = POS_SLEEPING;
            else 
                victim->position = POS_STANDING;
        }

        return;
    }

    if ( victim->is_npc() && victim->hit < 1 )
    {
        victim->position = POS_DEAD;
        return;
    }

    if ( victim->hit <= -11 )
    {
        victim->position = POS_DEAD;
        return;
    }

    if ( victim->hit <= -6 )
        victim->position = POS_MORTAL;
    else
    if ( victim->hit <= -3 )
        victim->position = POS_INCAP;
    else
        victim->position = POS_STUNNED;

    return;
}


void set_violent( Character *ch, Character *victim, bool fAlways )
{
    ostringstream buf;

    if (ch == victim || ch->is_npc( ))
        return;
        
    if (victim->is_npc() ) {
        if (victim->master && !victim->master->is_npc() && ch != victim->master) {
            victim = victim->master;
            buf << ch->getNameC() << " атакует чармиса " << victim->getNameC();
        } else if (victim->in_room->pIndexData->clan != clan_none && 
                   ch->getClan( ) != victim->in_room->pIndexData->clan && ( 
            IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)) ||
            IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_HEALER)) || 
            IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)) ) ) {
            buf << ch->getNameC() << " атакует моба на территории " << victim->in_room->pIndexData->clan->getRussianName().ruscase('2') ;
        } else return;
    } else buf << ch->getNameC() << " атакует " << victim->getNameC();

    if (victim->is_mirror( ) )
        buf <<  "зеркало " << victim->getNameC() ;

    buf << " в " << ch->in_room->getName() << "[" << ch->in_room->vnum << "]" << endl;

    if (!IS_VIOLENT(ch))
        wiznet( WIZ_FLAGS, 0, 0, buf.str( ).c_str( ) );
    
    set_violent( ch );

    if (fAlways || !IS_VIOLENT(victim)) {
        set_violent( victim );
    }
}

void set_violent( Character *ch )
{
    if (ch->is_npc( ) || ch->is_immortal( ))
        return;
    
    SET_BIT( ch->getPC( )->PK_flag, PK_VIOLENT ); 
    REMOVE_SLAIN( ch ); 
    ch->getPC( )->PK_time_v = PK_TIME_VIOLENT; 
}

void set_killer( Character *ch )
{
    if (ch->is_npc( ) || ch->is_immortal( ))
        return;

    SET_BIT( ch->getPC( )->PK_flag, PK_KILLER ); 
    set_violent( ch ); 
    ch->getPC( )->PK_time_sk = PK_TIME_KILLER;
}

void set_slain( Character *ch )
{
    if (ch->is_npc( ) || ch->is_immortal( ))
        return;

    REMOVE_KILLER( ch ); 
    REMOVE_VIOLENT( ch ); 
    REMOVE_THIEF( ch ); 
    
    SET_BIT( ch->getPC( )->PK_flag, PK_SLAIN );

    ch->getPC( )->PK_time_sk = PK_TIME_SLAIN;
}

void set_ghost( Character *ch )
{
    if (ch->is_npc( ))
        return;

    SET_BIT( ch->getPC( )->PK_flag, PK_GHOST ); 

    REMOVE_VIOLENT( ch ); 
    REMOVE_KILLER( ch ); 
    REMOVE_THIEF( ch ); 
    
    ch->getPC( )->ghost_time = GHOST_TIME;
    SET_DEATH_TIME(ch);
}

void set_thief( Character *ch )
{
    if (ch->is_npc( ))
        return;

    SET_BIT( ch->getPC( )->PK_flag, PK_THIEF ); 
    REMOVE_SLAIN( ch ); 
    ch->getPC( )->PK_time_t = PK_TIME_THIEF;
}

void do_visible(Character *ch)
{
    strip_hide_and_fade(ch);

    strip_invisibility(ch);

    strip_camouflage(ch);
    
    if (IS_SET(ch->affected_by, AFF_SNEAK))
    {
        affect_bit_strip(ch, &affect_flags, AFF_SNEAK, true);
        REMOVE_BIT(ch->affected_by, AFF_SNEAK);
    }
}
