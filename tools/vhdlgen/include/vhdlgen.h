#ifndef __HEADER_VHDLGEN__
#define __HEADER_VHDLGEN__

#include <string>
#include <vector>
#include <set>
#include <map>

#include "pprint_builder.h"

namespace vhdl {

using boost::shared_ptr;

class PrettyPrintableV : public virtual pprint::PrettyPrintable { };

class ToplevelDeclaration : public virtual PrettyPrintableV { };

class LocalDeclaration : public virtual PrettyPrintableV { };


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

#ifndef SWIG
enum _Direction {
	Direction_Default = 0,
	Direction_In,
	Direction_Out,
	Direction_InOut
};
#else
typedef int _Direction;
#endif

class Direction : public pprint::PrettyPrintable {
	_Direction _direction;
	std::string text;

	Direction(_Direction _direction, std::string text);
public:
	static const Direction Default;
	static const Direction In;
	static const Direction Out;
	static const Direction InOut;

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

class ValueDeclaration : public virtual PrettyPrintableV {
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
	virtual std::string prefix() const =0;

	const Value _value;
public:
	ValueDeclarationWithOptionalDefault(std::string name, Type type);
	ValueDeclarationWithOptionalDefault(std::string name, Type type, Value value);
	ValueDeclarationWithOptionalDefault(std::string name, Direction direction, Type type);
	ValueDeclarationWithOptionalDefault(std::string name, Direction direction, Type type, Value value);
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

class Statement : public virtual PrettyPrintableV { };

class Comment : public ToplevelDeclaration, public LocalDeclaration, public Statement {
	pprint::PrettyPrinted_p _content;
	std::string _prefix;
public:
	Comment(pprint::PrettyPrintable_p content, const std::string& prefix = " ");
	Comment(pprint::PrettyPrinted_p content, const std::string& prefix = " ");
	Comment(const std::string& text, const std::string& prefix = " ");

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Signal : public ValueDeclarationWithOptionalDefault {
protected:
	virtual std::string prefix() const;
public:
	Signal(std::string name, Type type);
	Signal(std::string name, Type type, Value value);
	Signal(std::string name, Direction direction, Type type);
	Signal(std::string name, Direction direction, Type type, Value value);
};

class Constant : public ValueDeclarationWithOptionalDefault {
protected:
	virtual std::string prefix() const;
public:
	Constant(std::string name, Type type);
	Constant(std::string name, Type type, Value value);
	Constant(std::string name, Direction direction, Type type);
	Constant(std::string name, Direction direction, Type type, Value value);
};

class Variable : public ValueDeclarationWithOptionalDefault {
protected:
	virtual std::string prefix() const;
public:
	Variable(std::string name, Type type);
	Variable(std::string name, Type type, Value value);
	Variable(std::string name, Direction direction, Type type);
	Variable(std::string name, Direction direction, Type type, Value value);
};

class LocalValueDeclaration : public LocalDeclaration {
	boost::shared_ptr<ValueDeclaration> _inner;
public:
	LocalValueDeclaration(boost::shared_ptr<ValueDeclaration> inner);

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

class Component : public LocalDeclaration {
	std::string _name;
	Port _port;
public:
	Component(const std::string& name);

	const std::string& name() const;
	Port& port();

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Instance : public Statement {
	std::string _label;
	boost::shared_ptr<Component> _component;
	std::vector< std::pair<std::string, std::string> > _portMapOrder;
	std::map<std::string, std::string> _portMap;
	std::map<std::string, boost::shared_ptr<Signal> > _portMapWithSignals;

	boost::shared_ptr<Signal> createDummySignal(const std::string& signal_name) const;
public:
	Instance(const std::string& label, boost::shared_ptr<Component> component);

	Instance& mapPin(const Pin& pin, boost::shared_ptr<Signal> signal);
	Instance& mapPin(const Pin& pin, std::string signal);
	Instance& mapPin(const std::string& pin, boost::shared_ptr<Signal> signal);
	Instance& mapPin(const std::string& pin, const std::string& signal);

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class Architecture : public ToplevelDeclaration {
	std::string _name, _entityName;
	shared_ptr<Entity> _entity;
	std::vector<boost::shared_ptr<LocalDeclaration> > _declarations;
	std::vector<boost::shared_ptr<Statement> > _statements;
public:
	Architecture(const std::string& name, const std::string& entityName);
	Architecture(const std::string& name, shared_ptr<Entity> entity);

	const std::string& name() const;
	const std::string& entityName() const;
	shared_ptr<Entity> entity();
	const std::vector<boost::shared_ptr<LocalDeclaration> >& declarations() const;

	Architecture& addDeclaration(const boost::shared_ptr<ValueDeclaration>& declaration);
	Architecture& addDeclaration(const boost::shared_ptr<LocalDeclaration>& declaration);

	Architecture& addStatement(const boost::shared_ptr<Statement>& declaration);

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

class CompilationUnit : public std::vector<boost::shared_ptr<ToplevelDeclaration> > {
	boost::shared_ptr<UsedLibraries> _libraries;
public:
	CompilationUnit();

	boost::shared_ptr<UsedLibraries> libraries();

#ifndef SWIG
	CompilationUnit& add(ToplevelDeclaration* decl);
#endif
	CompilationUnit& add(boost::shared_ptr<ToplevelDeclaration> decl);

	virtual const pprint::PrettyPrinted_p prettyPrint() const;
};

template<typename T>
inline boost::shared_ptr<T> ptr(const T& item) {
	return boost::shared_ptr<T>(new T(item));
}

}

#endif // not defined __HEADER_VHDLGEN__
