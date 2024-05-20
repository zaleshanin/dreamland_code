#include "lasthost.h"
#include "logstream.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

void XMLAttributeLastHost::addHost(const DLString &host)
{
    hosts[host]++;
}

bool XMLAttributeLastHost::hasHost(const DLString &host) const
{
    return hosts.isAvailable(host);
}

const DLString &XMLAttributeLastHost::getMatchingHost(const DLString &hostPrefix) const
{
    Hosts::const_iterator h;
    for (h = hosts.begin(); h != hosts.end(); h++)    
        if (hostPrefix.strPrefix(h->first))
            return h->first;

    return DLString::emptyString;
}

bool XMLAttributeLastHost::isUnique(const DLString &playerName, const DLString &host)
{
    for (auto &p: PCharacterManager::getPCM()) {
        if (playerName != p.first) {
            XMLAttributeLastHost::Pointer attr = p.second->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
            if (attr && attr->hasHost(host))
                return false;
        }
    }

    return true;
}

/**
 *  Outputs host/usage pairs sorted by usage in descending order.
 */
void XMLAttributeLastHost::showHosts(ostringstream &buf) const
{
    Hosts::ValueList list = hosts.toSortedList();

    for (Hosts::ValueList::const_iterator v = list.begin(); v != list.end(); v++) 
        buf << fmt(0, "%16s %4d", v->first.c_str(), v->second) << endl;
}

void XMLAttributeLastHostListenerPlugin::run(int oldState, int newState, Descriptor *d)
{
    Character *ch = d->character;

    if (!ch)
        return;
    
    if (newState != CON_PLAYING) 
        return;
   
    PCharacter *pch = ch->getPC(); 
    DLString host = d->getRealHost();

    // Don't increment usage counter if reconnecting with the same IP address.
    if (pch->getLastAccessHost() == host && oldState == CON_BREAK_CONNECT)
        return;
    
    // Remember current host and increment usage counter.
    pch->setLastAccessHost(host);
    XMLAttributeLastHost::Pointer attr = pch->getAttributes( ).getAttr<XMLAttributeLastHost>("lasthost");
    attr->addHost(host);
    pch->save();
}