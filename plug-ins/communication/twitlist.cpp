/* $Id: twitlist.cpp,v 1.1.2.1.8.2 2009/08/16 02:50:30 rufina Exp $
 *
 * ruffina, 2005
 */

#include <ostream>

#include "twitlist.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "arg_utils.h"

COMMAND(CTwit, "twit")
{
    PCharacter *pch;
    DLString cmd;
    DLString arguments = constArguments;

    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (ch->is_npc( )) {
        ch->pecho("Тебе нельзя.");
        return;
    }
    
    pch = ch->getPC( );

    if (cmd.empty( ))
        doUsage( pch );
    else if (arg_oneof( cmd, "add", "добавить" ))
        doAdd( pch, arguments );
    else if (arg_is_list( cmd ))
        doList( pch );
    else if (arg_oneof( cmd, "remove", "удалить" ))
        doRemove( pch, arguments );
    else
        doUsage( pch );
}

void CTwit::doAdd( PCharacter *ch, DLString &arg )
{
    XMLAttributeTwitList::Pointer attr;
    DLString name;
    PCMemoryInterface *pci;

    if (arg.empty( )) {
        ch->pecho("Использование: {lRжаба добавить{lEtwit add{lx <имя>");
        return;
    }

    if (( pci = PCharacterManager::find( arg ) ))
        name = pci->getName( );
    else {
        ch->pecho( "Жертва с таким именем не обнаружена." );
        return;
    }
    
    attr = ch->getAttributes( ).getAttr<XMLAttributeTwitList>( "twit" );

    if (attr->isAvailable( name )) {
        ch->pecho( "Имя \"%s\" уже в списке.", name.c_str( ) );
        return;
    }
    
    attr->push_back( name );
    ch->pecho( "Имя \"%s\" добавлено в список.", name.c_str( ) );
}

void CTwit::doRemove( PCharacter *ch, DLString &arg )
{
    XMLAttributeTwitList::Pointer attr;
    XMLAttributeTwitList::iterator iter;

    if (arg.empty( )) {
        ch->pecho("Использование: {lRжаба удалить{lEtwit rem{lx <имя> ");
        return;
    }
    
    attr = ch->getAttributes( ).findAttr<XMLAttributeTwitList>( "twit" );

    if (!attr || attr->size( ) == 0) {
        ch->pecho("Список жаб и так пуст.");
        return;
    }
    
    for (iter = attr->begin( ); iter != attr->end( ); iter++)
        if (arg ^ iter->getValue( )) {
            ch->pecho( "Имя %s удалено из списка.", iter->getValue( ).c_str( ) );
            attr->erase( iter );
            return;
        }
            
    ch->pecho( "Но имени \"%s\" нет в списке.", arg.c_str( ) );
}

void CTwit::doList( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributeTwitList::Pointer attr;
    XMLAttributeTwitList::iterator iter;

    attr = ch->getAttributes( ).findAttr<XMLAttributeTwitList>( "twit" );
    
    if (!attr || attr->size( ) == 0)
        buf << "Список пуст." << endl;
    else {
        buf << "Список людей, которых Вы не желаете слышать: " << endl;
        
        for (iter = attr->begin( ); iter != attr->end( ); iter++)
            buf << *iter << endl;
    }

    ch->send_to( buf );
}

void CTwit::doUsage( PCharacter *ch )
{
    ch->pecho( "twit add <имя> \r\n"
                 "twit rem <имя> \r\n"
                 "twit list" );
}

const DLString XMLAttributeTwitList::TYPE= "XMLAttributeTwitList";

XMLAttributeTwitList::XMLAttributeTwitList( )
{
}

XMLAttributeTwitList::~XMLAttributeTwitList( )
{
}

bool XMLAttributeTwitList::isAvailable( const DLString &arg ) const
{
    const_iterator iter;
    
    for (iter = begin( ); iter != end( ); iter++)
        if (arg ^ iter->getValue( ))
            return true;

    return false;
}

