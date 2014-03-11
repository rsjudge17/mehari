#ifndef __HEADER_VHDLGEN__
#define __HEADER_VHDLGEN__

#include <string>
#include <vector>
#include <set>
#include <map>

#include "pprint_builder.h"

namespace vhdl {

using boost::shared_ptr;

class PrettyPrintable : public virtual pprint::PrettyPrintable { };

class ToplevelDeclaration : public PrettyPrintable { };

class LocalDeclaration : public PrettyPrintable { };


class UsedLibrary : public std::set<std::string>, public pprint::PrettyPrintable {
	std::string name;
public:
	UsedLibrary(std::string name = "");

	UsedLibrary& operator << (std::string element);

	const std::string& getName();

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class UsedLibraries : public std::map<std::string, UsedLibrary>, public ToplevelDeclaration {
public:
	UsedLibrary& add(std::string name);

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

enum _Direction {
	Direction_Default = 0,
	Direction_In,
	Direction_Out,
	Direction_InOut
};

class Direction : public pprint::PrettyPrintable {
	_Direction _direction;
	std::string text;

	Direction(_Direction _direction, std::string text);
public:
	const static Direction Default;
	const static Direction In;
	const static Direction Out;
	const static Direction InOut;

	virtual const pprint::PrettyPrinted_p prettyPrint() const;

	_Direction direction() const;

	std::string str() const;

	bool operator ==(const Direction& dir) const;
	bool operator !=(const Direction& dir) const;
};

//TODO
class Type : public pprint::PrettyPrintable {
	std::string _type;
public:
	Type(std::string type);
	~Type();

	std::string type() const;

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

//TODO
class Value : public pprint::PrettyPrintable {
	std::string _value;
public:
	Value(std::string value);
	~Value();

	std::string value() const;

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class ValueDeclaration : public PrettyPrintable {
protected:
	virtual void build(pprint::PrettyPrintBuilder& builder) const;

	const std::string _name;
	const Direction _direction;
	const Type _type;

	ValueDeclaration(std::string name, Direction direction, Type type);
public:
	virtual ~ValueDeclaration();

	virtual const pprint::PrettyPrinted_p prettyPrint() const;

	const std::string& name() const;
	const Direction& direction() const;
	const Type& type() const;
};

class ValueDeclarationWithOptionalDefault : public ValueDeclaration {
protected:
	virtual void build(pprint::PrettyPrintBuilder& builder) const;

	const Value _value;

	ValueDeclarationWithOptionalDefault(std::string name, Direction direction, Type type);
	ValueDeclarationWithOptionalDefault(std::string name, Direction direction, Type type, Value value);
public:
	virtual ~ValueDeclarationWithOptionalDefault();

	bool hasValue() const;
	const Value& value() const;
};

class Pin : public ValueDeclaration {
public:
	Pin(std::string name, Direction direction, Type type);
};

class Port : public pprint::PrettyPrintable {
	std::map<std::string, boost::shared_ptr<Pin> > _byName;
	std::vector<boost::shared_ptr<Pin> > _order;
public:
	Port& add(const Pin& pin);
	Port& add(boost::shared_ptr<Pin> pin);

	const std::map<std::string, boost::shared_ptr<Pin> >& pinsByName() const;
	const std::vector<boost::shared_ptr<Pin> >& pins() const;

	bool contains(const std::string& name) const;
	const Pin& operator[](const std::string& name) const;

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Comment : public ToplevelDeclaration, public LocalDeclaration {
	std::string _text;
public:
	//TODO accept any PrettyPrintable or PrettyPrinted object
	Comment(const std::string& text);

	const std::string& text() const;

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Entity : public ToplevelDeclaration {
	std::string _name;
	Port _port;
public:
	Entity(const std::string& name);

	const std::string& name() const;
	Port& port();

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Architecture : public ToplevelDeclaration {
	std::string _name, _entityName;
	shared_ptr<Entity> _entity;
public:
	Architecture(const std::string& name, const std::string& entityName);
	Architecture(const std::string& name, shared_ptr<Entity> entity);

	const std::string& name() const;
	const std::string& entityName() const;
	shared_ptr<Entity> entity();

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class CompilationUnit : std::vector<boost::shared_ptr<ToplevelDeclaration> > {
	boost::shared_ptr<UsedLibraries> _libraries;
public:
	CompilationUnit();

	boost::shared_ptr<UsedLibraries> libraries();

	CompilationUnit& add(ToplevelDeclaration* decl);
	CompilationUnit& add(boost::shared_ptr<ToplevelDeclaration> decl);

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

}

#endif // not defined __HEADER_VHDLGEN__
