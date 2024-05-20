/* $Id: pcskilldata.h,v 1.1.2.3.18.2 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __PCSKILLDATA_H__
#define __PCSKILLDATA_H__

#include <vector>

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmllong.h"
#include "xmlmap.h"
#include "xmlenumeration.h"
#include "globalprofilearray.h"

class PCSkillData : public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<PCSkillData> Pointer;
        
        PCSkillData( );

        XML_VARIABLE XMLInteger learned;
        XML_VARIABLE XMLIntegerNoEmpty timer;
        XML_VARIABLE XMLBooleanNoFalse forgetting;
        XML_VARIABLE XMLBooleanNoFalse temporary; // to be removed once all profiles loaded & converted
        XML_VARIABLE XMLEnumeration origin;
        XML_VARIABLE XMLLongNoEmpty start;
        XML_VARIABLE XMLLongNoEmpty end;
    
        bool isValid() const;
        bool isTemporary() const;
        void clear();
        static PCSkillData empty;

        static long END_NEVER;
};

class PCSkills : public GlobalProfileArray<PCSkillData> {
XML_OBJECT
public:
        typedef ::Pointer<PCSkills> Pointer;
        typedef GlobalProfileArray<PCSkillData> Base;

        PCSkills();
};

#endif
