/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          object.cpp  -  description
                             -------------------
    begin                : Fri Apr 27 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "class.h"
#include "logstream.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "ru_pronouns.h"

#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"

#include "objectbehavior.h"
#include "object.h"
#include "affect.h"
#include "character.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(none);

const DLString Object::TYPE = "Object";

Object::Object( ) :
                ID( 0 ),
                name( 0 ), owner( 0 ), 
                short_descr( 0 ), description( 0 ), material( 0 ), 
                next( 0 ), prev( 0 ),
                next_content( 0 ), contains( 0 ), in_obj( 0 ), on( 0 ),
                carried_by( 0 ), extra_descr( 0 ), affected( 0 ), pIndexData( 0 ),
                in_room( 0 ), enchanted( 0 ),
                item_type( 0 ),
                extra_flags( 0 ), wear_flags( 0 ), 
                wear_loc( wear_none ),
                weight( 0 ), cost( 0 ), level( 0 ), condition( 0 ),
                timer( 0 ), timestamp( -1 ),
                from( &str_empty[0] ),
                extracted( 0 ), water_float( 0 ),
                killer( &str_empty[0] ), count( 0 ),
                behavior( ObjectBehavior::NODE_NAME )
{
        for( int i = 0; i < 5; i++ ) value[i] = 0;
}

void Object::extract( )
{
        Affect* paf_next;
        EXTRA_DESCR_DATA* ed_next;
        
        for( Affect* paf = affected; paf != 0; paf = paf_next )
        {
                paf_next = paf->next;
                ddeallocate( paf );
        }

        for( EXTRA_DESCR_DATA* ed = extra_descr; ed != 0; ed = ed_next )
        {
                ed_next = ed->next;
                free_extra_descr(ed);
        }
        
        free_string( material );
        free_string( name );
        free_string( description );
        free_string( short_descr );
        free_string( owner );
        free_string( killer );
        free_string( from );
        
        next = 0;
        prev = 0;
        next_content = 0;
        contains = 0;
        in_obj = 0;
        on = 0;
        carried_by = 0;
        extra_descr = 0;
        affected = 0;
        pIndexData = 0;
        in_room = 0;
        enchanted = 0;
        pocket = "";
        owner = 0;
        name = 0;
        short_descr = 0;
        description = 0;
        cachedNoun.clear( );
        item_type = 0;
        extra_flags = 0;
        wear_flags = 0;
        wear_loc.assign( wear_none );
        weight = 0;
        cost = 0;
        level = 0;
        condition = 0;
        material = 0;
        timer = 0;
        timestamp = -1;
        from = &str_empty[0];
        extracted = 0;
        water_float = 0;
        killer = &str_empty[0];
        count = 0;
        properties.clear();

        wrapper = 0;
        behavior.clear( );
        ID = 0;
        
        for( int i = 0; i < 5; i++ ) value[i] = 0;
}

Object::~Object( )
{
    Affect *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;

        for (paf = affected; paf != 0; paf = paf_next)
        {
                paf_next = paf->next;
                ddeallocate( paf );
        }

        for (ed = extra_descr; ed != 0; ed = ed_next )
        {
                ed_next = ed->next;
                free_extra_descr(ed);
        }
    
    free_string( material );
    free_string( name );
    free_string( description );
    free_string( short_descr );

    free_string( owner );
    free_string( killer );
    free_string( from );
}

Character * Object::getCarrier( ) {
    if (!carried_by)
        if (in_obj)
            return in_obj->getCarrier( );
        else
            return NULL;
    else
        return carried_by;
}

Room * Object::getRoom( ) {
    if (carried_by)
        return carried_by->in_room;
    else if (in_room)
        return in_room;
    else if (in_obj)
        return in_obj->getRoom( );
    else
        return NULL;
}

/* object condition aliases */
const char *Object::get_cond_alias( void )
{
 const char *stat;

 if      ( condition >  99 ) stat = "{Cотл.{x";
 else if ( condition >= 80 ) stat = "{cхор.{x";
 else if ( condition >= 60 ) stat = "{Yнорм.{x";
 else if ( condition >= 40 ) stat = "{yср.{x";
 else if ( condition >= 20 ) stat = "{Rпл.{x";
 else                        stat = "{rуж.{x";

 return stat;
}

int Object::floating_time( )
{
 int  ftime;

 ftime = 0;
 switch( item_type )
 {
    default: break;
    case ITEM_KEY       : ftime = 1;    break;
    case ITEM_LOCKPICK  : ftime = 1;    break;
    case ITEM_ARMOR     : ftime = 2;    break;
    case ITEM_TREASURE  : ftime = 2;    break;
    case ITEM_PILL      : ftime = 2;    break;
    case ITEM_POTION    : ftime = 3;    break;
    case ITEM_TRASH     : ftime = 3;    break;
    case ITEM_PARCHMENT : ftime = 3;    break;
    case ITEM_SPELLBOOK : ftime = 3;    break;
    case ITEM_FOOD      : ftime = 4;    break;
    case ITEM_CONTAINER : ftime = 5;    break;
    case ITEM_KEYRING   : ftime = 5;    break;
    case ITEM_CORPSE_NPC: ftime = 10;   break;
    case ITEM_CORPSE_PC : ftime = 10;   break;
 }
 ftime = number_fuzzy( ftime ) ;

 return max( 0, ftime );
}

bool Object::may_float(void)
{
    if (behavior)
        if (behavior->mayFloat( ))
            return true;
    
    switch (item_type) {
    case ITEM_BOAT: 
    case ITEM_PORTAL:
        return true;
        
    default:
        return false;
    }
}

void Object::addExtraDescr( const DLString &keys, const DLString &value )
{
    EXTRA_DESCR_DATA *ed = 0;
    
    for (ed = extra_descr; ed; ed = ed->next)
        if (keys == ed->keyword)
            break;

    if (ed) {
        free_string( ed->description );
    }
    else {
        ed = new_extra_descr( );
        ed->keyword = str_dup( keys.c_str( ) );
        ed->next = extra_descr;
        extra_descr = ed;
    }

    ed->description = str_dup( value.c_str( ) );
}


/*
 * String fields set/get methods
 */
void Object::setName( const char *s )
{
    if (name)
        free_string( name );

    name = str_dup( s );
}
void Object::setShortDescr( const char *s )
{
    if (short_descr)
        free_string( short_descr );

    short_descr = str_dup( s );
    updateCachedNoun( );
}
void Object::setDescription( const char *s )
{
    if (description)
        free_string( description );

    description = str_dup( s );
}
void Object::setMaterial( const char *s )
{
    if (material)
        free_string( material );

    material = str_dup( s );
}
void Object::setOwner( const char *s )
{
    if (owner)
        free_string( owner );

    owner = str_dup( s );
}

void Object::fmtName( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setName( buf );
}
void Object::fmtShortDescr( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setShortDescr( buf );
}
void Object::fmtDescription( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setDescription( buf );
}
void Object::fmtMaterial( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setMaterial( buf );
}
DLString Object::getFirstName( ) const
{
    return DLString( getName( ) ).getOneArgument( );
}
DLString Object::getShortDescr( char gram_case )
{
    return toNoun( )->decline( gram_case );
}

static bool is_anti_align( Character *ch, int flags )
{
    if (IS_SET(flags, ITEM_ANTI_EVIL) && IS_EVIL( ch ))
        return true;

    if (IS_SET(flags, ITEM_ANTI_GOOD) && IS_GOOD( ch ))
        return true;

    if (IS_SET(flags, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL( ch ))
        return true;
    
    return false;
}

bool Object::isAntiAligned( Character *ch ) const
{
    return is_anti_align( ch, extra_flags );
}

bool Object::wasAntiAligned( Character *ch ) const
{
    return is_anti_align( ch, pIndexData->extra_flags );
}

/*
 * Return # of objects which an object counts as.
 */
int Object::getNumber( ) const
{
    int number;

    if (item_type == ITEM_MONEY)
        number = 0;
    else
        number = 1;

    return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int Object::getWeight( ) const
{
    int sum;
    Object *tobj;

    sum = weight;

    for (tobj = contains; tobj != 0; tobj = tobj->next_content)
        sum += tobj->getWeight( ) * getWeightMultiplier( ) / 100;

    return sum;
}

/*
 * True object weight w/o container multipliers 
 */
int Object::getTrueWeight( ) const
{
    int sum;
    Object *tobj;

    sum = weight;

    for (tobj = contains; tobj != 0; tobj = tobj->next_content)
        sum += tobj->getWeight( );

    return sum;
}


bool Object::isAffected( int sn ) const
{
    Affect *paf;

    for (paf = affected; paf != 0; paf = paf->next) 
        if (paf->type == sn)
            return true;

    return false;
}

bool Object::hasOwner( const Character *ch ) const
{
    if (!getOwner( ))
        return false;

    if (ch->is_npc( ))
        return false;

    return !str_cmp( getOwner( ), ch->getNameP( ) );
}

using namespace Grammar;

Noun::Pointer Object::toNoun( const DLObject *forWhom, int flags ) const
{
    const Character *wch = dynamic_cast<const Character *>(forWhom);
    
    if (IS_SET(flags, FMT_INVIS)) {
        if (wch && !wch->can_see( this )) 
            return something;
    }
    
    return cachedNoun;
}

void Object::updateCachedNoun( )
{
    if (!cachedNoun) { 
        cachedNoun = RussianString::Pointer( NEW, 
                                             getShortDescr( ), 
                                             pIndexData->gram_gender );
    }
    else {
        cachedNoun->setFullForm( getShortDescr( ) );
        cachedNoun->setGender( pIndexData->gram_gender );
    }
}

