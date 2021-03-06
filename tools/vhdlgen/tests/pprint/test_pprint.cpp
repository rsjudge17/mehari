#include "test_pprint.h"

using namespace pprint;
using namespace testing;

class PPrintTest : public::testing::Test {
protected:
	MockPrettyPrintable pprintable;
	boost::shared_ptr<MockPrettyPrinted> pprinted;
	MockLineIterator* lines;	// freed by operator << (see below)

	PPrintTest() : pprinted(new MockPrettyPrinted()), lines(new MockLineIterator()) { }

	void SetUp() {
		EXPECT_CALL(pprintable, prettyPrint())
			.Times(1)
			.WillOnce(Return(pprinted));

		EXPECT_CALL(*pprinted, lines())
			.Times(1)
			// caller owns the reference and will free it
			.WillOnce(Return(lines));
	}
};

TEST_F(PPrintTest, testStreamOperatorsForEmptyObject) {
	EXPECT_CALL(*lines, next())
		.Times(1)
		.WillOnce(Return(false));

	EXPECT_OUTPUT("", stream, stream << pprintable);
}

TEST_F(PPrintTest, testStreamOperatorsReturnStream) {
	EXPECT_CALL(*lines, next())
		.Times(1)
		.WillOnce(Return(false));

	std::stringstream stream;
	EXPECT_EQ(&stream, &(stream << pprintable));
}

class PrintLine {
	std::string line;
public:
	const Action<unsigned int(std::ostream&, unsigned int, PrettyPrintStatus&)> action;

	PrintLine(std::string line) : line(line), action(Invoke(this, &PrintLine::doIt)) { }

	int doIt(std::ostream& stream, Unused, Unused) {
		stream << line;
		return line.size();
	}
};

TEST_F(PPrintTest, testStreamOperatorsWithOneLine) {
	EXPECT_CALL(*lines, next())
		.Times(2)
		.WillOnce(Return(true))
		.WillOnce(Return(false));

	PrintLine line1("blub");

	EXPECT_CALL(*lines, print(_, 0, _))
		.Times(1)
		.WillOnce(line1.action);

	EXPECT_OUTPUT("blub", stream, stream << pprintable);
}

TEST_F(PPrintTest, testStreamOperatorsWithMultipleLines) {
	EXPECT_CALL(*lines, next())
		.Times(4)
		.WillOnce(Return(true))
		.WillOnce(Return(true))
		.WillOnce(Return(true))
		.WillOnce(Return(false));

	PrintLine line1("blub");
	PrintLine line2("ab");
	PrintLine line3("cde");

	EXPECT_CALL(*lines, print(_, 0, _))
		.Times(3)
		.WillOnce(line1.action)
		.WillOnce(line2.action)
		.WillOnce(line3.action);

	EXPECT_OUTPUT("blub\nab\ncde", stream, stream << pprintable);
}


// methods only for testing
// There are defined here, so only the tests can use them.

bool VCat::_measured() { return measured; }
bool HCat::_measured() { return measured; }
