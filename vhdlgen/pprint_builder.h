#ifndef __HEADER_PPRINT_BUILDER__
#define __HEADER_PPRINT_BUILDER__

#include "pprint.h"
#include <stack>

namespace pprint {

struct _PrettyPrintBuilderStackItem;

class PrettyPrintBuilder {
	std::stack<_PrettyPrintBuilderStackItem> containerStack;
	PrettyPrinted_p _recentItem;
	PrettyPrintedWithChildren_p _root;

	_PrettyPrintBuilderStackItem& getOrCreateCurrent();
public:
	PrettyPrintBuilder();
	~PrettyPrintBuilder();

	bool hasCurrentContainer();
	unsigned int depth();
	PrettyPrintedWithChildren_p currentContainer();
	PrettyPrinted_p recentItem();

	PrettyPrintBuilder& add(const PrettyPrintable& item);
	PrettyPrintBuilder& add(const PrettyPrintable* item);

	PrettyPrintBuilder& add(PrettyPrinted_p item);
	PrettyPrintBuilder& add(PrettyPrinted* item);

	PrettyPrintBuilder& add(std::string text);

	PrettyPrintBuilder& addAndSelect(PrettyPrintedWithChildren_p container);
	PrettyPrintBuilder& addAndSelect(PrettyPrintedWithChildren* container);

	PrettyPrintBuilder& columns();
	PrettyPrintBuilder& append();

	PrettyPrintBuilder& seperateBy(PrettyPrinted_p item);
	PrettyPrintBuilder& seperateBy(std::string text);

	PrettyPrintBuilder& up();

	PrettyPrinted_p build();
};

PrettyPrintBuilder& builder();

} // end of namespace pprint

#endif // not defined __HEADER_PPRINT_BUILDER__
