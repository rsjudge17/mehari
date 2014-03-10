#ifndef __HEADER_PPRINT__
#define __HEADER_PPRINT__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace pprint {

class PrettyPrinted;

typedef boost::shared_ptr<PrettyPrinted> PrettyPrinted_p;

class PrettyPrintable {
public:
	virtual const PrettyPrinted_p prettyPrint() const =0;
};

std::ostream& operator <<(std::ostream& stream, const PrettyPrintable& data);


class PrettyPrintStatus {
public:
	PrettyPrintStatus();
	~PrettyPrintStatus();
};

class LineIterator {
public:
	virtual ~LineIterator();

	virtual bool next() =0;
	virtual bool last() =0;

	virtual const std::string text() const =0;
	virtual int width() const =0;

	// print current line without newline, return width
	virtual int print(std::ostream& stream, int width, PrettyPrintStatus& status) const =0;
};

class PrettyPrinted {
public:
	virtual ~PrettyPrinted();

	virtual LineIterator* lines() const =0;

	virtual int width()  const =0;
	virtual int height() const =0;
};

std::ostream& operator <<(std::ostream& stream, const PrettyPrinted&  data);
std::ostream& operator <<(std::ostream& stream, const PrettyPrinted_p data);

class Empty : public PrettyPrinted {
public:
	Empty();
	virtual ~Empty();

	virtual LineIterator* lines() const;

	virtual int width()  const;
	virtual int height() const;
};

class Text : public PrettyPrinted {
	std::string _text;
public:
	// use Text::create unless you are sure that the string doesn't have any newlines
	Text(std::string text);
	virtual ~Text();

	static PrettyPrinted_p create(std::string text);

	const std::string text() const;

	virtual LineIterator* lines() const;

	virtual int width()  const;
	virtual int height() const;
};

class Keyword : public Text { };

class PrettyPrintedWithChildren;
typedef boost::shared_ptr<PrettyPrinted> PrettyPrintedWithChildren_p;

class PrettyPrintedWithChildren : public PrettyPrinted {
protected:
	virtual void _add(PrettyPrinted_p item) =0;

public:
	template<typename PP>
	inline boost::shared_ptr<PP> add(boost::shared_ptr<PP> item);

	template<typename PP>
	inline boost::shared_ptr<PP> add(PP* item);

	inline PrettyPrinted_p add(std::string text);
};

class VCat : public PrettyPrintedWithChildren, public std::vector<PrettyPrinted_p> {
	bool measured;
	int _width;
	int _height;

protected:
	virtual void _add(PrettyPrinted_p item);

public:
	VCat();
	virtual ~VCat();

	void measure();

	virtual LineIterator* lines() const;

	virtual int width()  const;
	virtual int height() const;

	// only for testing
	bool _measured();
};

class HCat : public PrettyPrintedWithChildren, public std::vector<PrettyPrinted_p> {
	bool measured;
	int _width;
	int _height;

protected:
	virtual void _add(PrettyPrinted_p item);

public:
	HCat();
	virtual ~HCat();

	void measure();

	virtual LineIterator* lines() const;

	virtual int width()  const;
	virtual int height() const;

	// only for testing
	bool _measured();
};


template<typename PP>
inline boost::shared_ptr<PP> PrettyPrintedWithChildren::add(boost::shared_ptr<PP> item) {
	_add(item);
	return item;
}

template<typename PP>
inline boost::shared_ptr<PP> PrettyPrintedWithChildren::add(PP* item) {
	return add(boost::shared_ptr<PP>(item));
}

inline PrettyPrinted_p PrettyPrintedWithChildren::add(std::string text) {
	return add(Text::create(text));
}

} // end of namespace pprint

#endif // not defined __HEADER_PPRINT__
