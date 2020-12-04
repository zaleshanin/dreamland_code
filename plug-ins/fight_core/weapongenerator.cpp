#include "weapongenerator.h"
#include "weaponcalculator.h"
#include "weapontier.h"
#include "weaponaffixes.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill.h"
#include "skillreference.h"
#include "core/object.h"
#include "character.h"
#include "affect.h"

#include "damageflags.h"
#include "morphology.h"
#include "material-table.h"
#include "attacks.h"
#include "loadsave.h"
#include "dl_math.h"
#include "merc.h"
#include "def.h"

GSN(none);
WEARLOC(wield);
WEARLOC(second_wield);

Json::Value weapon_classes;
CONFIGURABLE_LOADED(fight, weapon_classes)
{
    weapon_classes = value;
}

Json::Value weapon_names;
CONFIGURABLE_LOADED(fight, weapon_names)
{
    weapon_names = value;
}

/*--------------------------------------------------------------------------
 * WeaponGenerator
 *-------------------------------------------------------------------------*/
WeaponGenerator::WeaponGenerator()
        : extraFlags(0, &extra_flags),
          weaponFlags(0, &weapon_type2)
{
    sn = gsn_none;
    valTier = hrTier = drTier = 5;
    hrCoef = drCoef = 0;
    hrMinValue = drMinValue = 0;
    hrIndexBonus = drIndexBonus = 0;
    align = ALIGN_NONE;
}

WeaponGenerator::~WeaponGenerator()
{

}

WeaponGenerator & WeaponGenerator::item(Object *obj)
{ 
    this->obj = obj; 
    wclass = weapon_class.name(obj->value0());
    wclassConfig = weapon_classes[wclass];
    if (wclass.empty())
        warn("Weapon generator: no configuration defined for weapon class %s.", wclass.c_str());
     return *this; 
}

const WeaponGenerator & WeaponGenerator::assignValues() const
{    
    WeaponCalculator calc(valTier, obj->level, obj->value0());
    obj->value1(calc.getValue1());
    obj->value2(calc.getValue2());
    return *this;
}

int WeaponGenerator::maxDamroll() const
{
    return WeaponCalculator(drTier, obj->level, obj->value0(), drIndexBonus).getDamroll();
}

int WeaponGenerator::maxHitroll() const
{
    return WeaponCalculator(hrTier, obj->level, obj->value0(), hrIndexBonus).getDamroll();
}

int WeaponGenerator::minDamroll() const
{
    return max( drMinValue, (int)(drCoef * maxDamroll()));
}

int WeaponGenerator::minHitroll() const
{
    return max( hrMinValue, (int)(hrCoef * maxHitroll()));
}

const WeaponGenerator & WeaponGenerator::assignHitroll() const
{
    setAffect(APPLY_HITROLL, maxHitroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamroll() const
{
    setAffect(APPLY_DAMROLL, maxDamroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignStartingHitroll() const
{
    setAffect(APPLY_HITROLL, minHitroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignStartingDamroll() const
{
    setAffect(APPLY_DAMROLL, minDamroll());
    return *this;
}

const WeaponGenerator & WeaponGenerator::incrementHitroll() const
{
    Affect *paf_hr = obj->affected.find( sn, APPLY_HITROLL );
    if (paf_hr) {
        int oldMod = paf_hr->modifier;
        int min_hr = minHitroll();
        int max_hr = maxHitroll();
        paf_hr->modifier = URANGE( min_hr, oldMod + 1, max_hr );

        if (obj->carried_by && (obj->wear_loc == wear_wield || obj->wear_loc == wear_second_wield)) {
            obj->carried_by->hitroll += paf_hr->modifier - oldMod;
        }
    }

    return *this;
}

const WeaponGenerator & WeaponGenerator::incrementDamroll() const
{
    Affect *paf_dr = obj->affected.find( sn, APPLY_DAMROLL );
    if (paf_dr) {
        int oldMod = paf_dr->modifier;
        int min_dr = minDamroll();
        int max_dr = maxDamroll();
        paf_dr->modifier = URANGE( min_dr, oldMod + 1, max_dr );

        if (obj->carried_by && (obj->wear_loc == wear_wield || obj->wear_loc == wear_second_wield)) {
            obj->carried_by->damroll += paf_dr->modifier - oldMod;
        }
    }
    
    return *this;
}

void WeaponGenerator::setAffect(int location, int modifier) const
{
    int skill = sn < 0 ? gsn_none : sn;
    Affect *paf = obj->affected.find(sn, location);

    if (!paf) {
        Affect af;

        af.type = skill;
        af.level = obj->level;
        af.duration = -1;
        af.location = location;
        affect_to_obj(obj, &af);

        paf = obj->affected.front();
    }

    paf->modifier = modifier;
}

WeaponGenerator & WeaponGenerator::randomNames()
{
    const Json::Value &configs = weapon_names[wclass];

    if (configs.empty()) {
        warn("Weapon generator: no names defined for type %s.", wclass.c_str());
        return *this;
    }

    int index = number_range(0, configs.size() - 1);
    nameConfig = configs[index];
    return *this;
}

WeaponGenerator & WeaponGenerator::randomAffixes()
{
    affix_generator gen(valTier);

    // Set exclusions or requirements based on chosen names and weapon flags.
    for (auto const &affixName: wclassConfig["forbids"])
        gen.addForbidden(affixName.asString());

    for (auto const &affixName: wclassConfig["requires"])
        gen.addRequired(affixName.asString());

    for (auto const &affixName: nameConfig["forbids"])
        gen.addForbidden(affixName.asString());

    for (auto const &affixName: nameConfig["requires"])
        gen.addRequired(affixName.asString());

    // TODO: affix preferences setup and align bonus setup happens here.

    gen.setAlign(align);

    // Generate all combinations of affixes.
    gen.run();

    if (gen.getResultSize() == 0) {
        warn("Weapon generator: no affixes found for tier %d.", valTier);
        return *this;
    }    

    // Collect all configurations mandated by given set of affixes: flags, material, affects.
    auto result = gen.getSingleResult();
    int minPrice = result.front().price;
    int maxPrice = result.back().price;

    for (auto &pinfo: result) {
        const Json::Value &affix = pinfo.entry;
        const DLString &section = pinfo.section;
        obj->carried_by->pecho("{DAffix %s [%d]", affix["value"].asCString(), pinfo.price);

        if (section == "flag") {
            extraFlags.setBits(affix["extra"].asString());
            weaponFlags.setBits(affix["value"].asString());
        } else if (section == "extra") {
            extraFlags.setBits(affix["value"].asString());
        } else if (section == "material") {
            materialName = affix["value"].asString();
        }

        // TODO collect data for suffixes, including hr/dr/ave tier bonuses.

        // Each adjective or noun has a chance to be chosen, but the most expensive get an advantage.
        for (auto &adj: affix["adjectives"])
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price)
                adjectives.push_back(adj.asString());

        for (auto &noun: affix["nouns"])
            if (number_range(minPrice - 10, maxPrice) <= pinfo.price)
                nouns.push_back(noun.asString());
    }

    obj->carried_by->pecho("{DExtras %s, weapon flags %s, material %s{x", 
        extraFlags.names().c_str(), weaponFlags.names().c_str(), materialName.c_str());

    return *this;
}

void WeaponGenerator::setName() const
{
    StringList mynames(nameConfig["name"].asString());
    mynames.addUnique(wclass);
    mynames.addUnique(weapon_class.message(obj->value0()));
    obj->setName(mynames.join(" ").c_str());
}

void WeaponGenerator::setShortDescr() const
{
    DLString randomAdjective, randomNoun; 

    obj->gram_gender = MultiGender(nameConfig["gender"].asCString());

    if (!adjectives.empty()) {
        int a = number_range(0, adjectives.size() - 1);
        randomAdjective = adjectives[a];
    }

    if (!nouns.empty()) {
        int n = number_range(0, nouns.size() - 1);
        randomNoun = nouns[n];
    }

    DLString colour = weapon_tier_table[valTier-1].colour;
    DLString myshort;

    if (!colour.empty())
        myshort = "{" + colour;

    if (!randomAdjective.empty())
        myshort += Morphology::adjective(randomAdjective, obj->gram_gender) + " "; // леденящий

    myshort += nameConfig["short"].asString(); // буздыган

    if (!randomNoun.empty())
        myshort += " " + randomNoun; // боли

    if (!colour.empty())
        myshort += "{x";

    obj->setShortDescr(myshort.c_str());
}

const WeaponGenerator & WeaponGenerator::assignNames() const
{
    // Config item names and gram gender. 
    setName();
    setShortDescr();
    obj->setDescription(nameConfig["long"].asCString());

    // Set up provided material or default.
    obj->setMaterial(findMaterial().c_str());

    obj->properties["tier"] = valTier;
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignAffects() const
{
    // TODO apply affects from suffixes.
    return *this;
}

const WeaponGenerator & WeaponGenerator::assignFlags() const
{
    SET_BIT(obj->extra_flags, extraFlags.getValue());
    SET_BIT(obj->extra_flags, weapon_tier_table[valTier-1].extra.getValue());
    obj->value4(obj->value4() | weaponFlags.getValue());

    // Set weight: 0.4 kg by default in OLC, 2kg for two hand.
    // TODO: Weight is very approximate, doesn't depend on weapon type.
    if (IS_SET(obj->value4(), WEAPON_TWO_HANDS))
        obj->weight *= 5;

    return *this;
}

const WeaponGenerator & WeaponGenerator::assignDamageType() const
{
    StringSet attacks = StringSet(wclassConfig["attacks"].asString()); // frbite, divine, etc
    StringSet damtypes = StringSet(wclassConfig["damtypes"].asString()); // bash, pierce, etc
    bool any = damtypes.count("any") > 0;
    vector<int> result;

    for (int a = 0; attack_table[a].name != 0; a++) {
        const attack_type &attack = attack_table[a];
        if (any 
            || attacks.count(attack.name) > 0
            || damtypes.count(damage_table.name(attack.damage)) > 0)
        {
            result.push_back(a);
        }
    }

    if (result.empty()) {
        warn("Weapon generator: no matching damtype found for %s.", wclass.c_str());
        return *this;
    }

    obj->value3(
        result.at(number_range(0, result.size() - 1)));

    return *this;
}

/** Look up material based on suggested names or types. 
 *  Return 'metal' if nothing found.
 */
DLString WeaponGenerator::findMaterial() const
{
    // First analyze prefix requirements for material.
    if (!materialName.empty())
        return materialName;

    // Find by exact name, e.g. "fish".
    DLString mname = nameConfig["material"].asString();
    const material_t *material = material_by_name(mname);
    if (material)
        return material->name;

    // Find a random material name for each of requested types.
    StringList materials;
    for (auto &mtype: nameConfig["mtypes"]) {
        bitstring_t type = material_types.bitstring(mtype.asString());
        auto withType = materials_by_type(type);

        if (!withType.empty())
            materials.push_back(
                withType.at(number_range(0, withType.size() - 1))->name);
    }

    // Concatenate two or more material names, e.g. "pine, steel".
    if (!materials.empty())
        return materials.join(", ");

    return "metal";
}

