/* $Id: scenario.h,v 1.1.2.7.6.2 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __KIDNAPSCENARIO_H__
#define __KIDNAPSCENARIO_H__

#include "questscenario.h"

class NPCharacter;
class PCharacter;
class Character;
class Object;
class KidnapQuest;

class KSPrinceData : public QuestMobileAppearence {
XML_OBJECT
public:
    typedef ::Pointer<KSPrinceData> Pointer;
    
    void dress( NPCharacter *, NPCharacter * ) const;
};


class KidnapScenario : public QuestScenario, public virtual XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE KSPrinceData kid;
    XML_VARIABLE VnumList refuges;
    XML_VARIABLE VnumList kings;
    XML_VARIABLE QuestItemAppearence mark;
    XML_VARIABLE QuestMobileAppearence bandit;

    virtual void onQuestStart( PCharacter *, NPCharacter *, NPCharacter * ) const;
    
    virtual void msgRemoteReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const = 0;
    virtual void msgKingDeath( NPCharacter *king, Character *killer, PCharacter *hero ) const = 0;
    virtual void msgKidDeath( NPCharacter *kid, Character *killer, PCharacter *hero ) const = 0;

    virtual void actAttackHero( NPCharacter *bandit, PCharacter *hero ) const = 0;
    virtual void actBeginKidnap( NPCharacter *bandit, NPCharacter *kid ) const = 0;
    virtual void actHuntStep( NPCharacter *bandit ) const = 0;
    virtual void actKidnapStep( NPCharacter *bandit, NPCharacter *kid ) const = 0;
    virtual void actEmptyPath( NPCharacter *bandit, NPCharacter *kid ) const = 0;

    virtual void actLegend( NPCharacter *king, PCharacter *hero, ::Pointer<KidnapQuest> quest ) const = 0;
    virtual void actGiveMark( NPCharacter *king, PCharacter *hero, Object *mark, int time ) const = 0;
    virtual void actMarkLost( NPCharacter *king, PCharacter *hero, Object *mark ) const = 0;
    virtual void actAckWaitComplete( NPCharacter *king, PCharacter *hero ) const = 0;
   
    virtual void actHeroWait( NPCharacter *kid ) const = 0;
    virtual void actNoHero( NPCharacter *kid, PCharacter *hero ) const = 0;
    virtual void actHeroDetach( NPCharacter *kid, PCharacter *hero ) const = 0;
    virtual void actWrongGiver( NPCharacter *kid, Character *victim, Object *obj ) const = 0;
    virtual void actWrongMark( NPCharacter *kid, Character *victim, Object *obj ) const = 0;
    virtual void actGoodMark( NPCharacter *kid, Character *victim, Object *obj ) const = 0;
    virtual void actReunion( NPCharacter *kid, NPCharacter *king, PCharacter *hero ) const = 0;
    virtual void actBanditsUnleash( NPCharacter *kid, PCharacter *hero, NPCharacter *bandit ) const = 0;
};



#endif
