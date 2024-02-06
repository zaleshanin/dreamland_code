#include <algorithm>
#include "feniatriggers.h"
#include "wrapperbase.h"
#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "plugininitializer.h"
#include "wrappermanager.h"
#include "iconvmap.h"
#include "defaultspell.h"
#include "defaultaffecthandler.h"
#include "defaultskillcommand.h"
#include "feniaskillaction.h"
#include "wrappedcommand.h"
#include "websocketrpc.h"
#include "dlfileloader.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "damageflags.h"
#include "arg_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

static IconvMap utf2koi("utf-8", "koi8-r");

static const DLString EXAMPLE_FOLDER = "fenia.examples";

using namespace Scripting;

FeniaTriggerLoader *feniaTriggers = 0;

FeniaTriggerLoader::FeniaTriggerLoader()
{
    checkDuplicate( feniaTriggers );
    feniaTriggers = this;
}
FeniaTriggerLoader::~FeniaTriggerLoader()
{
    feniaTriggers = 0;
}

void FeniaTriggerLoader::initialization()
{
    loadFolder("obj");
    loadFolder("mob");
    loadFolder("room");
    loadFolder("spell");
    loadFolder("affect");
    loadFolder("skillcommand");
    loadFolder("command");
    loadFolder("queststep");
    loadFolder("areaquest");
}

void FeniaTriggerLoader::destruction()
{

}

// Populate index trigger map based of a content of fenia.examples subfolder.
// Folder 'mob' with 'onGive', 'onGreet', 'onSpeech' triggers becomes
// indexTriggers["mob"] = map of
//     ["onGive"] = <file content>
//     ["onGreet"] = <file content>
//     ["onSpeech"] = <file content>
void FeniaTriggerLoader::loadFolder(const DLString &indexType)
{
    DLFileLoader loader(EXAMPLE_FOLDER + "/" + indexType, "");
    loader.loadAll();

    DLFileLoader::Files::const_iterator f;
    DLFileLoader::Files files = loader.getAll();

    TriggerContent triggers;
    for (f = files.begin(); f != files.end(); f++) {
        // Assume that the source file is in utf-8.
        triggers[f->first] = DLString(utf2koi(f->second.content));
    }

    indexTriggers[indexType] = triggers;

    LogStream::sendNotice() << "OLC Fenia loaded " << triggers.size() << " triggers of type " << indexType << endl;
}

// Return 'Use' for 'onUse' or 'postUse'.
DLString triggerType(const DLString &name) 
{
    DLString key = name;
    static const DLString ON("on");
    static const DLString POST("post");

    if (ON.strPrefix(key))
        key = key.substr(ON.size());
    else if (POST.strPrefix(key))
        key = key.substr(POST.size());

    return key;
}

// Return true for strings like Aaaaa.
bool stringIsCapitalized(const DLString &str)
{
    return !str.empty() && dl_isupper(str.at(0));
}

/**
 * Clear a single runtime field (aka trigger) if exists on this wrapper.
 */
bool FeniaTriggerLoader::clearTrigger(Scripting::Object *wrapper, const DLString &trigName) const
{
    if (!wrapper)
        return false;

    WrapperBase *base = get_wrapper(wrapper);
    if (!base)
        return false;

    Scripting::IdRef methodId(trigName);
    if (base->getField(methodId).type == Register::NONE)
        return false;

    Register self = base->getSelf();
    base->setField(methodId, Register());
    return true;
}

/**
 * Retrieve and clear all runtime fields assigned to this wrapper.
 */
bool FeniaTriggerLoader::clearTriggers(Scripting::Object *wrapper) const
{
    if (!wrapper)
        return false;

    WrapperBase *base = get_wrapper(wrapper);
    if (!base)
        return false;

    StringSet triggers, misc;
    base->collectTriggers(triggers, misc);

    if (triggers.empty() && misc.empty())
        return false;

    for (auto &trigName: triggers)
        clearTrigger(wrapper, trigName);

    for (auto &miscName: misc)
        clearTrigger(wrapper, miscName);

    return true;
}

static void show_one_trigger(PCharacter *ch, DefaultSpell *spell, const char *trigName, bitnumber_t target_mask, ostringstream &bufActive, ostringstream &bufAvailable)
{
    bool hasTrigger = FeniaSkillActionHelper::spellHasTrigger(spell, trigName);
    bool hasTarget = spell->target.isSet(target_mask);
    DLString trigLink = web_cmd(ch, "fenia $1", trigName);

    if (hasTrigger)
        bufActive << trigLink << " ";
    else if (hasTarget)
        bufAvailable << trigLink << " ";
}

/**
 * Show all triggers on a spell. Trigger names shown depend on the spell target type.
 */
void FeniaTriggerLoader::showTriggers(PCharacter *ch, DefaultSpell *spell) const
{
    ostringstream buf, bufActive, bufAvailable;
        
    show_one_trigger(ch, spell, "runVict", TAR_CHAR_ROOM|TAR_CHAR_SELF|TAR_CHAR_WORLD, bufActive, bufAvailable);
    show_one_trigger(ch, spell, "runObj", TAR_OBJ_EQUIP|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_WORLD, bufActive, bufAvailable);
    show_one_trigger(ch, spell, "runRoom", TAR_PEOPLE|TAR_ROOM, bufActive, bufAvailable);
    show_one_trigger(ch, spell, "runArg", TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ, bufActive, bufAvailable);

    if (bufActive.str().empty() && bufAvailable.str().empty())
        bufAvailable << "(укажи цель заклинания)";
    else
        bufAvailable << "{D(fenia <trig> [clear]){x";
    
    buf << "{gАктивные триггеры{x:    " << bufActive.str() << endl;
    if (!bufAvailable.str().empty())
        buf << "Доступные триггеры:   " << bufAvailable.str() << endl;

    ch->send_to(buf);
}

/**
 * Show all available and active triggers for a wrapper (skill command, mob index etc).
 */
void FeniaTriggerLoader::showTriggers(PCharacter *ch, WrapperBase *wrapper, const DLString &indexType) const
{
    ostringstream buf;
    
    // Collect a set of all trigger names defined in the examples.
    StringSet availableTriggers;
    auto myTriggers = indexTriggers.find(indexType);
    if (myTriggers != indexTriggers.end())
        for (auto &t: myTriggers->second)
            availableTriggers.insert(t.first);

    // Collect a set of all active triggers and other methods on the wrapper.
    StringSet activeTriggers, miscMethods;    
    if (wrapper)
        wrapper->collectTriggers(activeTriggers, miscMethods);

    // Display active ones.
    if (!activeTriggers.empty()) {
        buf << "{gАктивные триггеры{x:    ";
        for (auto &t: activeTriggers)
            buf << web_cmd(ch, "fenia $1", t) << " ";
        buf << "{D(fenia <trig> [clear]){x" << endl;
    }

     if (!miscMethods.empty()) {
        buf << "{gАктивные методы{x:      ";
        for (auto &t: miscMethods)
            buf << web_cmd(ch, "fenia $1", t) << " ";
        buf << endl;    
    }
    
    // Display remaining available triggers.
    if (!availableTriggers.empty()) {
        ostringstream abuf;
        
        for (auto &t: availableTriggers)
            if (activeTriggers.count(t) == 0 && miscMethods.count(t) == 0)
                abuf << web_cmd(ch, "fenia $1", t) << " ";

        if (!abuf.str().empty())
            buf << "Доступные триггеры:   " << abuf.str() << "{D(fenia <trig>){x" << endl;
    }

    ch->send_to(buf);
}



Register get_wrapper_for_index_data(int vnum, const DLString &type)
{
    Register w;
    
    if (type == "obj") {
        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        if (pObj)
            w = WrapperManager::getThis()->getWrapper(pObj);

    } else if (type == "room") {
        Room *pRoom = get_room_instance(vnum);
        if (pRoom)
            w = WrapperManager::getThis()->getWrapper(pRoom);

    } else if (type == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (pMob)
            w = WrapperManager::getThis()->getWrapper(pMob);
    }

    return w;
}

bool FeniaTriggerLoader::checkWebsock(Character *ch) const
{
    if (!is_websock(ch)) {
        ch->pecho("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    return true;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const
{
    if (!checkWebsock(ch))
        return false;

    Register w = get_wrapper_for_index_data(indexData.getVnum(), indexData.getIndexType());
    if (w.type == Register::NONE)
        return false;
        
    WrapperBase *base = get_wrapper(w.toObject());
    if (!base) {
        ch->pecho("Не могу найти wrapper base, всё плохо.");
        return false;
    }

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Scripting::IdRef methodId(methodName);
    Register retval = base->getField(methodId);

    // fenia <trigName> clear
    if (arg_is_clear(args)) {
        if (feniaTriggers->clearTrigger(w.toObject(), methodName))
            ch->pecho("Триггер %s успешно удален.\r\n", methodName.c_str());
        else
            ch->pecho("Триггер %s не найден.\r\n", methodName.c_str());        

        return true;
    }

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        DLString trigType = triggerType(methodName);
        if (trigType == methodName) {
            ch->pecho("Название триггера должно начинаться с 'on' или 'post'.");
            return false;
        }

        if (!stringIsCapitalized(trigType)) {
            ch->pecho("Название триггера должно выглядеть так: onFight, postSpeech, onWear и т.п.");
            return false;
        }

        std::vector<DLString> parms(2);
        // Create codesource subject.
        parms[0] = dlprintf("areas/%s/%s/%d.%s", 
                        indexData.getArea()->area_file->file_name, 
                        indexData.getIndexType(),
                        indexData.getVnum(),
                        methodName.c_str());   

        // Create codesource body with example code.
        DLString tmpl;
        if (!findExample(ch, methodName, indexData.getIndexType(), tmpl))
            return false;

        tmpl.replaces("@vnum@", indexData.getVnum());
        tmpl.replaces("@trig@", methodName);
        parms[1] = tmpl;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для нового сценария, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}

// Look up trigger example for given index type and method name.
bool FeniaTriggerLoader::findExample(Character *ch, const DLString &methodName, const DLString &indexType, DLString &tmpl) const
{
    IndexTriggers::const_iterator i = indexTriggers.find(indexType);
    if (i == indexTriggers.end()) {
        ch->pecho("Нет ни одного триггера, попросите богов принять меры.");
        return false;
    }

    TriggerContent::const_iterator t = i->second.find(methodName);
    if (t == i->second.end()) {
        ch->printf("Триггер %s не найден, проверьте написание или попросите богов добавить его.\r\n", methodName.c_str());
        return false;
    }

    tmpl = t->second;
    return true;
}

// Launch web editor with the codesource for this function.
bool FeniaTriggerLoader::editExisting(Character *ch, Register &retval) const
{
    // Fenia field is not a function.
    if (retval.type != Register::FUNCTION) {
        ch->pecho("Это поле уже задано, но это не функция. Попробуйте что-то еще.");
        return false;
    }

    Scripting::CodeSourceRef csRef = retval.toFunction()->getFunction()->source;

    // Construct cs_edit arguments: subject, editor content and line number.
    std::vector<DLString> parms(3);
    parms[0] = csRef.source->name;
    parms[1] = csRef.source->content;
    parms[2] = csRef.line; // TODO line number at the end of the function

    // Open the editor.
    ch->desc->writeWSCommand("cs_edit", parms);
    ch->printf("Запускаю веб-редактор для сценария %s, строка %d.\r\n", csRef.source->name.c_str(), csRef.line);
    return true;
}

vector<DLString> FeniaTriggerLoader::createSkillActionParams(
    Character *ch, const DLString &actionType, SkillAction *action, const DLString &methodName) const
{
    std::vector<DLString> parms;

    // Create codesource body with example code.
    DLString tmpl;
    if (!findExample(ch, methodName, actionType, tmpl))
        return parms;

    parms.resize(2);
    tmpl.replaces("@name@", DLString("\"") + action->getSkill()->getName() + "\"");
    tmpl.replaces("@trig@", methodName);
    parms[1] = tmpl;

    // Create codesource subject.
    parms[0] = dlprintf("%s/%s/%s",
                    actionType.c_str(),
                    action->getSkill()->getName().c_str(),
                    methodName.c_str());   

    return parms;
}

vector<DLString> FeniaTriggerLoader::createCommandParams(
    Character *ch, WrappedCommand *cmd, const DLString &methodName) const
{
    std::vector<DLString> parms;
    const DLString indexType = "command";

    // Create codesource body with example code.
    DLString tmpl;
    if (!findExample(ch, methodName, indexType, tmpl))
        return parms;

    parms.resize(2);
    tmpl.replaces("@name@", DLString("\"") + cmd->getName() + "\"");
    tmpl.replaces("@trig@", methodName);
    parms[1] = tmpl;

    // Create codesource subject.
    parms[0] = dlprintf("%s/%s/%s",
                    indexType.c_str(),
                    cmd->getName().c_str(),
                    methodName.c_str());   

    return parms;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, DefaultSpell *spell, const DLString &constArguments) const
{
    if (!checkWebsock(ch))
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Register retval = getMethodForName<Spell>(spell, methodName);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        vector<DLString> parms = createSkillActionParams(ch, "spell", spell, methodName);
        if (parms.empty())
            return false;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для заклинания, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, DefaultAffectHandler *ah, const DLString &constArguments) const
{
    if (!checkWebsock(ch))
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Register retval = getMethodForName<AffectHandler>(ah, methodName);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        vector<DLString> parms = createSkillActionParams(ch, "affect", ah, methodName);
        if (parms.empty())
            return false;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для аффекта, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, DefaultSkillCommand *cmd, const DLString &constArguments) const
{
    if (!checkWebsock(ch))
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Register retval = getMethodForName<SkillCommand>(cmd, methodName);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        vector<DLString> parms = createSkillActionParams(ch, "skillcommand", cmd, methodName);
        if (parms.empty())
            return false;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для команды, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}
    

bool FeniaTriggerLoader::openEditor(PCharacter *ch, WrappedCommand *cmd, const DLString &constArguments) const
{
    if (!checkWebsock(ch))
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Register retval = getMethodForName<WrappedCommand>(cmd, methodName);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        vector<DLString> parms = createCommandParams(ch, cmd, methodName);
        if (parms.empty())
            return false;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для команды, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}
    
// For given step type (mob/obj/room) return a list of all triggers that begin with this
// prefix, e.g. 'mob:onGreet', 'mob:onSpeech', with "mob:" prefix removed.
StringSet FeniaTriggerLoader::getQuestTriggers(const DLString &stepType) const
{
    StringSet stepTriggers;
    DLString trigPrefix = stepType + ":";

    auto t = indexTriggers.find("queststep");
    if (t == indexTriggers.end())
        return stepTriggers;

    for (auto &trig: t->second) {
        const DLString &trigName = trig.first;
        if (trigPrefix.strPrefix(trigName))
            stepTriggers.insert(trigName.substr(trigPrefix.length()));
    }

    return stepTriggers;
}

Register find_function(const DLString &type, const Integer &vnum, const DLString &trigName);
DLString aquest_method_id(AreaQuest *q, int step, bool isBegin, const DLString &trigName);


vector<DLString> FeniaTriggerLoader::createQuestStepParams(
    Character *ch, AreaQuest *q, const DLString &type, const DLString &vnum, const DLString &trigName, const Integer &s, const DLString &methodId) const
{
    std::vector<DLString> parms;
    const DLString indexType = "queststep";
    DLString methodName = type + ":" + trigName;

    // Create codesource body with example code.
    DLString tmpl;
    if (!findExample(ch, methodName, indexType, tmpl))
        return parms;

    parms.resize(2);
    tmpl.replaces("@vnum@", vnum);
    tmpl.replaces("@trig@", methodId);
    tmpl.replaces("@quest.vnum@", q->vnum.toString());
    tmpl.replaces("@quest.step@", s.toString());
    parms[1] = tmpl;

    AreaIndexData *pArea;
    if (type == "mob" && get_mob_index(vnum.toInt()))
        pArea = get_mob_index(vnum.toInt())->area;
    else if (type == "obj" && get_obj_index(vnum.toInt()))
        pArea = get_obj_index(vnum.toInt())->area;
    else if (type == "room" && get_room_index(vnum.toInt()))
        pArea = get_room_index(vnum.toInt())->areaIndex;
    else
        return parms;

    // Create codesource subject.
    parms[0] = dlprintf("areas/%s/%s/%s.%s", 
                    pArea->area_file->file_name, 
                    type.c_str(),
                    vnum.c_str(),
                    methodId.c_str());   

    return parms;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, AreaQuest *q, const Integer &s, bool isBegin)
{
    const QuestStep::XMLPointer &thisStep = q->steps[s];
    DLString type = isBegin ? thisStep->beginType : thisStep->endType;
    DLString vnum = isBegin ? thisStep->beginValue : thisStep->endValue;
    DLString trigName = isBegin ? thisStep->beginTrigger : thisStep->endTrigger;

    DLString methodId = aquest_method_id(q, s, isBegin, trigName);
    Register method = find_function(type, vnum, methodId);

    if (method.type == Register::NONE) {
        // No trigger defined yet, create new from a template
        vector<DLString> parms = createQuestStepParams(ch, q, type, vnum, trigName, s, methodId);
        if (parms.empty())
            return false;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->pecho("Запускаю веб-редактор для шага %d квеста %d, триггер %s.\r\n", 
                    s.getValue(), q->vnum.getValue(), trigName.c_str());
        return true;

    } else {
        // Open existing trigger as is
        editExisting(ch, method);
    }

    return true;
}

PluginInitializer<FeniaTriggerLoader> initFeniaTriggerLoader;
