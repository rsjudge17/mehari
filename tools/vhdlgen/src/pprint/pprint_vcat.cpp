
#include <assert.h>
#include <boost/scoped_ptr.hpp>
#include "pprint.h"

namespace pprint {

typedef std::vector<PrettyPrinted_p>::const_iterator iter_t;

VCat::VCat() {
	measured = false;
}

VCat::~VCat() { }

void VCat::measure() {
	_width  = 0;
	_height = 0;

	for (iter_t it = this->begin(); it != this->end(); ++it) {
		unsigned int width = (*it)->width();
		if (width > _width)
			_width = width;

		_height += (*it)->height();
	}

	measured = true;
}

unsigned int VCat::width()  const {
	assert(measured);

	return _width;
}

unsigned int VCat::height() const {
	assert(measured);

	return _height;
}

class VCatLines : public LineIterator {
	iter_t begin, current, end;
	boost::scoped_ptr<LineIterator> nested_current;
	bool valid;
public:
	VCatLines(iter_t begin, iter_t end) : begin(begin), current(begin), end(end) {
		this->valid = false;

		if (current != end)
			nested_current.reset((*current)->lines());
	}

	~VCatLines() {
	}

	virtual bool next() {
		if (!nested_current)
			return false;

		while (!nested_current->next()) {
			nested_current.reset();

			++current;
			if (current == end) {
				// no more items
				nested_current.reset();
				valid = false;
				return false;
			}

			nested_current.reset((*current)->lines());
		}

		valid = true;
		return true;
	}

	virtual bool last() {
		current = end;
		while (current != begin) {
			--current;

			nested_current.reset((*current)->lines());

			if (nested_current->last()) {
				valid = true;
				return true;
			}
		}

		valid = false;
		return false;
	}

	virtual bool isLast() const {
		assert(valid);
		if (!valid || !nested_current)
			return false;
		if (!nested_current)
			return false;

		if (!nested_current->isLast())
			return false;

		iter_t current = this->current;
		++current;
		while (current != end) {
			boost::scoped_ptr<LineIterator> lines((*current)->lines());
			if (lines->next())
				return false;

			++current;
		}

		return true;
	}

	virtual const std::string text() const {
		assert(valid);
		return nested_current->text();
	}

	virtual unsigned int width() const {
		assert(valid);
		return nested_current->width();
	}

	virtual unsigned int print(std::ostream& stream, unsigned int width, PrettyPrintStatus& status) const {
		assert(valid);
		return nested_current->print(stream, width, status);
	}
};

LineIterator* VCat::lines() const {
	return new VCatLines(this->begin(), this->end());
}

void VCat::_add(PrettyPrinted_p item) {
	this->push_back(item);
}

} // end of namespace pprint
