#ifndef _UMLCLASSMEMBER_H
#define _UMLCLASSMEMBER_H


#include <QByteArray>
#include "UmlBaseClassMember.h"
#include "aVisibility.h"

class FileOut;
class UmlTypeSpec;

// This class globaly manages class's relations, attributes, operations
// and sub-classes
//
// You can modify it as you want (except the constructor)
class UmlClassMember : public UmlBaseClassMember
{
public:
    UmlClassMember(void * id, const QByteArray & n) : UmlBaseClassMember(id, n) {};

    void write_scope(FileOut & out, const char * who = "owner");

    void write_visibility(FileOut & out);

    void write_visibility(FileOut & out, aVisibility v);

    void write_annotation(FileOut & out);

    //  return the true name of the item

    QByteArray true_name(QByteArray decl);

    static bool identChar(char c);

    //The keyword link to the language are removed/replaced
    //Have to remove the name and type keywords given in argument,
    //and to remove the initial value.
    //t.type != 0 or t.explicit_type != ""
    static void write_type(FileOut & out, const UmlTypeSpec & t, QByteArray s, const char * k_name, const char * k_type);

    static void remove_comments(QByteArray & s);

};

#endif
