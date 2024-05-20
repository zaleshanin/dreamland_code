/* $Id: notethread.h,v 1.1.2.10.6.11 2009/09/19 22:37:33 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __NOTETHREAD_H__
#define __NOTETHREAD_H__

#include <sstream>
#include <list>

#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmllist.h"
#include "xmlenumeration.h"
#include "xmlshort.h"

#include "command.h"
#include "so.h"
#include "dlxmlloader.h"

#include "note.h"

class PCharacter;
class XMLNoteData;
class XMLAttributeNoteData;
class NoteThread;

class NoteHelp : public CommandHelp {
public:
    typedef ::Pointer<NoteHelp> Pointer;

    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
};

inline const DLString & NoteHelp::getType( ) const
{
    return TYPE;
}

class NoteCommand : public Command {
XML_OBJECT
public:
    typedef ::Pointer<NoteCommand> Pointer;
    typedef ::Pointer<XMLAttributeNoteData> Attribute;    

    virtual bool saveCommand() const;
    virtual void run( Character *, const DLString & );

    void setThread(::Pointer<NoteThread> thread);
    void unsetThread();
    ::Pointer<NoteThread> getThread() const;

protected:    
    void doFlag(PCharacter *, DLString & );
    bool doShow( PCharacter *, Attribute ) const;
    bool doClear( PCharacter *, Attribute ) const;
    void doSubject( PCharacter *, Attribute, const DLString & ) const;
    void doTo( PCharacter *, Attribute, const DLString & ) const;
    void doLinePlus( PCharacter *, Attribute, const DLString & ) const;
    void doLineMinus( PCharacter *, Attribute ) const;
    void doPaste( PCharacter *, Attribute ) const;
    void doCatchup( PCharacter *ch ) const;
    void doCopy( PCharacter *, DLString & ) const;
    void doRead( PCharacter *, DLString & ) const;
    void doList( PCharacter *, DLString &argument ) const;
    void doRemove( PCharacter *, DLString & );
    void doUncatchup( PCharacter *, DLString & ) const;
    bool doPost( PCharacter *, Attribute );
    bool doFrom( PCharacter *, Attribute, const DLString & ) const;
    void doForward( PCharacter *, Attribute, DLString & ) const;

    void echo( PCharacter *, const DLString &, const DLString &, const DLString & = "" ) const;

    ::Pointer<NoteThread> thread;
};

 
/*
 * NoteThread
 */
class NoteThread : public virtual Plugin, 
                   public DLXMLRuntimeLoader, 
                   public XMLVariableContainer 
{
XML_OBJECT    
friend class NoteCommand;
public:
    class NoteBucket : public XMLListBase<Note> {
    public:
        typedef XMLListBase<Note> Base;        
        
        NoteBucket( );
        virtual bool nodeFromXML( const XMLNode::Pointer& child );
        inline void setThread( NoteThread * );

    private:
        NoteThread *thread;
    };

    static const DLString NODE_NAME;

    typedef ::Pointer<NoteThread> Pointer;
    typedef list<Note *> NoteList;
    typedef vector<NoteBucket> NoteHash;
    typedef ::Pointer<XMLAttributeNoteData> Attribute;

    NoteThread( const DLString & );
    
    virtual void initialization( );
    virtual void destruction( );

    virtual const DLString &getName() const;
    
    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;
    
    virtual bool canWrite( const PCharacter * ) const;
    virtual bool canRead( const PCharacter * ) const;
    virtual bool isExpired( const Note * ) const;
    virtual void getUnreadMessage( int, ostringstream & ) const { };
    inline const Enumeration &getGender() const;
    inline const DLString &getRussianThreadName( ) const;
    inline const DLString &getRussianMltName() const;

    int countSpool( PCharacter * ) const;
    const Note * getNextUnreadNote( PCharacter * ) const;
    void showNoteToChar( PCharacter *, const Note * ) const;
    const Note * findNote( time_t ) const;
    Note * findNote( time_t );
    const Note * findNextNote( PCharacter *, time_t ) const;
    const NoteList & getNoteList( ) const;

protected:
    bool isNoteHidden( const Note *, PCharacter *, time_t ) const;
    const Note * getNoteAtPosition( PCharacter *, int  ) const;
    int getNoteNumber( PCharacter *, const Note * ) const;
    time_t getStamp( PCharacter * ) const;

protected:
    void loadAllBuckets( );
    void saveAllBuckets( ) const;
    void saveBucket( time_t ) const;

    void attach( const Note * );
    void remove( const Note * );
    void dump( ) const;
    
    XML_VARIABLE XMLPointerNoEmpty<NoteCommand> command;
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString nameMlt;
    XML_VARIABLE XMLString rusName;
    XML_VARIABLE XMLString rusNameMlt;
    XML_VARIABLE XMLEnumeration gender;
    XML_VARIABLE XMLInteger keepDays;
    XML_VARIABLE XMLShort readLevel;
    XML_VARIABLE XMLShort writeLevel;
    XML_VARIABLE XMLBooleanNoFalse godsSeeAlways;
    
    XML_VARIABLE XMLStringNoEmpty msgToOk, msgToError;
    XML_VARIABLE XMLStringNoEmpty msgNoCurrent, msgTooLong;
    XML_VARIABLE XMLStringNoEmpty msgNoRecepient;
    XML_VARIABLE XMLStringNoEmpty msgSent;

    NoteList xnotes;
    NoteHash hash;

    static const int HASH_LEN;
};

inline void NoteThread::NoteBucket::setThread( NoteThread *thread )
{
    this->thread = thread;
}

inline const DLString & NoteThread::getName( ) const
{
    return name.getValue( );
}

inline const DLString &NoteThread::getRussianThreadName( ) const
{
    return rusName;
}

inline const DLString &NoteThread::getRussianMltName( ) const
{
    return rusNameMlt;
}

inline const Enumeration &NoteThread::getGender() const
{
    return gender;
}

/*
 * NoteThreadRegistrator
 */
class NoteThreadRegistrator {
public:    
    NoteThreadRegistrator( )
    {
        next = first;
        first = this;
    }

    virtual void registrate( SO::PluginList & ) = 0;

    static void registrateAll( SO::PluginList &ppl )
    {
        for (; first; first=first->next)
            first->registrate( ppl );
    }

    NoteThreadRegistrator *next;
    static NoteThreadRegistrator *first;
};

/*
 * NoteThreadTemplate
 */
template <typename T>
class NoteThreadTemplate : public NoteThread {
public:
    typedef ::Pointer<NoteThreadTemplate> Pointer;
    
    NoteThreadTemplate( ) : NoteThread( THREAD_NAME )
    {
    }
    virtual const DLString& getName( ) const 
    {
        return THREAD_NAME;
    }
    virtual void getUnreadMessage( int count, ostringstream &buf ) const 
    {
        return NoteThread::getUnreadMessage( count, buf );
    }
    virtual bool canWrite( const PCharacter *ch ) const 
    {
        return NoteThread::canWrite( ch );
    }

    struct Registrator : public ::NoteThreadRegistrator
    {
        virtual void registrate( SO::PluginList &ppl )
        {
            Plugin::registerPlugin<NoteThreadTemplate>( ppl );
        }
    };

protected:
    static const DLString THREAD_NAME;    
};

#define NOTE_STRUCT(x) note_decl_ ## x
#define NOTE(x) NoteThreadTemplate<NOTE_STRUCT(x)>

#define TYPE_NOTE(type, x) template <> type NOTE(x)
#define VOID_NOTE(x) TYPE_NOTE(void, x)

#define NOTE_DECL(x) \
struct NOTE_STRUCT(x); \
template <> const DLString NOTE(x)::THREAD_NAME = #x; \
NOTE(x)::Registrator note ## x ## Registrator

#endif
