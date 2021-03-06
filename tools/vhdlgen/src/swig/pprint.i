%module pprint

%{
#include "pprint.h"
#include "pprint_builder.h"

#include <sstream>

using boost::shared_ptr;
%}

%include "std_string.i"
%include "std_vector.i"
%include "std_set.i"
%include "std_map.i"
%include <boost_shared_ptr.i>

%shared_ptr(pprint::PrettyPrintable)
%shared_ptr(pprint::PrettyPrinted)

%shared_ptr(pprint::Empty)
%shared_ptr(pprint::Text)
%shared_ptr(pprint::Keyword)
%shared_ptr(pprint::PrettyPrintedWithChildren)
%shared_ptr(pprint::VCat)
%shared_ptr(pprint::HCat)
%shared_ptr(pprint::VCatOverlapping)
%shared_ptr(pprint::Indent)

%shared_ptr(std::vector< pprint::PrettyPrinted_p >)
%shared_ptr(std::set< std::string >)

namespace std {
   %template(pprintedvector) vector<pprint::PrettyPrinted_p>;
   %template(stringset) set<std::string>;
};

%rename(print_line) print;

// only available for tests
%ignore pprint::VCat::_measured;
%ignore pprint::HCat::_measured;

// shadowed, so it would be ignored anyway -> ignore it to avoid a warning
//TODO The warning is printed, anyway.
%ignore pprint::PrettyPrintBuilder::add(pprint::PrettyPrintable const *);

%extend pprint::PrettyPrinted {
	const std::string __str__() const {
		std::stringstream stream;
		stream << *$self;
		return stream.str();
	}
}

%extend pprint::PrettyPrintable {
	const std::string __str__() const {
		std::stringstream stream;
		stream << $self->prettyPrint();
		return stream.str();
	}
}

%newobject pprint::PrettyPrinted::lines();
%delobject PrettyPrintedWithChildren::add;
%delobject PrettyPrintBuilder::add;
%delobject PrettyPrintBuilder::addAndSelect;
%delobject PrettyPrintBuilder::seperateBy;

%include "pprint.h"
%include "pprint_builder.h"
