/**
 * @date 2019
 * Comprehensive tests for libsolidity/modelcheck/BlockConversionVisitor.{h,cpp}.
 */

#include <libsolidity/modelcheck/BlockConversionVisitor.h>

#include <test/libsolidity/AnalysisFramework.h>
#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace std;
using langutil::SourceLocation;

namespace dev
{
namespace solidity
{
namespace modelcheck
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(BlockConversion, ::dev::solidity::test::AnalysisFramework)

// Tests that input parameters are registered as declarations.
BOOST_AUTO_TEST_CASE(argument_registration)
{
    char const* text = R"(
		contract A {
			function f(int a, int b) public {
                a;
                b;
            }
		}
	)";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& func = *ctrt.definedFunctions()[0];

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, TypeTranslator());
    visitor.print(actual);
    expected << "{" << endl
             << "a;" << endl
             << "b;" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

// Tests that else statements and bodies are optional and that branch bodies are
// properly scoped
BOOST_AUTO_TEST_CASE(if_statement)
{
    char const* text = R"(
		contract A {
            int a;
			function if_stmt() public {
                if (a == 1) { }
                if (a == 1) { int a; }
                a;
            }
			function if_else_stmt() public {
                if (a == 1) { }
                else { }
                if (a == 1) { int a; }
                else { int a; }
                a;
            }
		}
	)";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& fncs = ctrt.definedFunctions();

    auto if_stmt = (fncs[0]->name() == "if_stmt") ? fncs[0] : fncs[1];
    auto else_stmt = (fncs[0]->name() == "if_stmt") ? fncs[1] : fncs[0];

    TypeTranslator translator;
    translator.enter_scope(ctrt);

    ostringstream actual_if;
    BlockConversionVisitor if_visitor(*if_stmt, translator);
    if_visitor.print(actual_if);
    ostringstream expected_if;
    expected_if << "{" << endl
                << "if ((self->d_a)==(1))" << endl
                << "{" << endl
                << "}" << endl
                << "if ((self->d_a)==(1))" << endl
                << "{" << endl
                << "int a;" << endl
                << "}" << endl
                << "self->d_a;" << endl
                << "}";
    BOOST_CHECK_EQUAL(actual_if.str(), expected_if.str());

    ostringstream actual_else;
    BlockConversionVisitor else_visitor(*else_stmt, translator);
    else_visitor.print(actual_else);
    ostringstream expected_else;
    expected_else << "{" << endl
                  << "if ((self->d_a)==(1))" << endl
                  << "{" << endl
                  << "}" << endl
                  << "else" << endl
                  << "{" << endl
                  << "}" << endl
                  << "if ((self->d_a)==(1))" << endl
                  << "{" << endl
                  << "int a;" << endl
                  << "}" << endl
                  << "else" << endl
                  << "{" << endl
                  << "int a;" << endl
                  << "}" << endl
                  << "self->d_a;" << endl
                  << "}";
    BOOST_CHECK_EQUAL(actual_else.str(), expected_else.str());
}

// Tests that while and for loops work in general, that expressions of a for
// loop are optional, and that loops are correctly scoped.
BOOST_AUTO_TEST_CASE(loop_statement)
{
    char const* text = R"(
		contract A {
            uint a;
            uint i;
			function while_stmt() public {
                while (a != a) { }
                while (a != a) { int i; }
                i;
            }
            function for_stmt() public {
                for (; a < 10; ++a) { int i; }
                for (int i = 0; ; ++i) { i; }
                for (int i = 0; i < 10; ) { ++i; }
                for (int i = 0; i < 10; ++i) { }
                i;
            }
        }
    )";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& fncs = ctrt.definedFunctions();

    TypeTranslator translator;
    translator.enter_scope(ctrt);

    auto while_stmt = (fncs[0]->name() == "while_stmt") ? fncs[0] : fncs[1];
    ostringstream actual_while, expected_while;
    BlockConversionVisitor while_visitor(*while_stmt, translator);
    while_visitor.print(actual_while);
    expected_while << "{" << endl
                   << "while ((self->d_a)!=(self->d_a))" << endl
                   << "{" << endl
                   << "}" << endl
                   << "while ((self->d_a)!=(self->d_a))" << endl
                   << "{" << endl
                   << "int i;" << endl
                   << "}" << endl
                   << "self->d_i;" << endl
                   << "}";
    BOOST_CHECK_EQUAL(actual_while.str(), expected_while.str());

    auto for_stmt = (fncs[0]->name() == "while_stmt") ? fncs[1] : fncs[0];
    ostringstream actual_for, expected_for;
    BlockConversionVisitor for_visitor(*for_stmt, translator);
    for_visitor.print(actual_for);
    expected_for << "{" << endl
                 << "for (; (self->d_a)<(10); ++(self->d_a))" << endl
                 << "{" << endl
                 << "int i;" << endl
                 << "}" << endl
                 << "for (int i = 0; ; ++(i))" << endl
                 << "{" << endl
                 << "i;" << endl
                 << "}" << endl
                 << "for (int i = 0; (i)<(10); )" << endl
                 << "{" << endl
                 << "++(i);" << endl
                 << "}" << endl
                 << "for (int i = 0; (i)<(10); ++(i))" << endl
                 << "{" << endl
                 << "}" << endl
                 << "self->d_i;" << endl
                 << "}";
    BOOST_CHECK_EQUAL(actual_for.str(), expected_for.str());
}

BOOST_AUTO_TEST_CASE(continue_statement)
{
    char const* text = R"(
		contract A {
			function void_func() public {
                while (false) { continue; }
            }
        }
    )";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& func = *ctrt.definedFunctions()[0];

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, TypeTranslator());
    visitor.print(actual);
    expected << "{" << endl
             << "while (0)" << endl
             << "{" << endl
             << "continue;" << endl
             << "}" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

BOOST_AUTO_TEST_CASE(break_statement)
{
    char const* text = R"(
		contract A {
			function void_func() public {
                while (false) { break; }
            }
        }
    )";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& func = *ctrt.definedFunctions()[0];

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, TypeTranslator());
    visitor.print(actual);
    expected << "{" << endl
             << "while (0)" << endl
             << "{" << endl
             << "break;" << endl
             << "}" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

BOOST_AUTO_TEST_CASE(return_statement)
{
    char const* text = R"(
		contract A {
			function void_func() public { return; }
            function int_func() public returns (int) { return 10 + 5; }
        }
    )";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& fncs = ctrt.definedFunctions();

    TypeTranslator translator;
    translator.enter_scope(ctrt);

    auto void_func = (fncs[0]->name() == "void_func") ? fncs[0] : fncs[1];
    ostringstream actual_void, expected_void;
    BlockConversionVisitor void_visitor(*void_func, translator);
    void_visitor.print(actual_void);
    expected_void << "{" << endl
                  << "return;" << endl
                  << "}";
    BOOST_CHECK_EQUAL(actual_void.str(), expected_void.str());

    auto int_func = (fncs[0]->name() == "void_func") ? fncs[1] : fncs[0];
    ostringstream actual_int, expected_int;
    BlockConversionVisitor int_visitor(*int_func, translator);
    int_visitor.print(actual_int);
    expected_int << "{" << endl
                 << "return (10)+(5);" << endl
                 << "}";
    BOOST_CHECK_EQUAL(actual_int.str(), expected_int.str());
}

BOOST_AUTO_TEST_CASE(variable_declaration_statement)
{
    char const* text = R"(
		contract A {
            int a;
            int c;
			function f() public {
                int b;
                {
                    int c;
                    a; b; c;
                }
                { a; b; c; }
                a; b; c;
            }
		}
	)";

    const auto &unit = *parseAndAnalyse(text);
    const auto &ctrt = *retrieveContractByName(unit, "A");
    const auto &func = *ctrt.definedFunctions()[0];

    TypeTranslator translator;
    translator.enter_scope(ctrt);

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, translator);
    visitor.print(actual);
    expected << "{" << endl
             << "int b;" << endl
             << "{" << endl
             << "int c;" << endl
             << "self->d_a;" << endl
             << "b;" << endl
             << "c;" << endl
             << "}" << endl
             << "{" << endl
             << "self->d_a;" << endl
             << "b;" << endl
             << "self->d_c;" << endl
             << "}" << endl
             << "self->d_a;" << endl
             << "b;" << endl
             << "self->d_c;" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

BOOST_AUTO_TEST_CASE(named_function_retvars)
{
    char const* text = R"(
		contract A {
			function f() public returns (int) { return 5; }
			function g() public returns (int a) { a = 5; }
		}
	)";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& fncs = ctrt.definedFunctions();

    TypeTranslator translator;
    translator.enter_scope(ctrt);

    auto unnamed = (fncs[0]->name() == "f") ? fncs[0] : fncs[1];
    ostringstream actual_unnamed, expected_unnamed;
    BlockConversionVisitor unnamed_visitor(*unnamed, translator);
    unnamed_visitor.print(actual_unnamed);
    expected_unnamed << "{" << endl
                     << "return 5;" << endl
                     << "}";
    BOOST_CHECK_EQUAL(actual_unnamed.str(), expected_unnamed.str());

    auto named = (fncs[0]->name() == "f") ? fncs[1] : fncs[0];
    ostringstream actual_named, expected_named;
    BlockConversionVisitor named_visitor(*named, translator);
    named_visitor.print(actual_named);
    expected_named << "{" << endl
                   << "int a;" << endl
                   << "(a)=(5);" << endl
                   << "return a;" << endl
                   << "}";
    BOOST_CHECK_EQUAL(actual_named.str(), expected_named.str());
}

// Tests conversion of transfer/send into _pay. This is tested on the block
// level due to the complexity of annotated FunctionCall expressions.
BOOST_AUTO_TEST_CASE(payment_function_calls)
{
    char const* text = R"(
		contract A {
			function f(address payable dst) public {
                dst.transfer(5);
                dst.send(10);
                (dst.send)(15);
            }
		}
	)";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& func = *ctrt.definedFunctions()[0];

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, TypeTranslator());
    visitor.print(actual);
    expected << "{" << endl
             << "_pay(state, dst, 5);" << endl
             << "_pay(state, dst, 10);" << endl
             << "_pay(state, dst, 15);" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

// Tests conversion of assert/require into C-horn. This is tested on the block
// level due to the complexity of annotated FunctionCall expressions.
BOOST_AUTO_TEST_CASE(verification_function_calls)
{
    char const* text = R"(
		contract A {
			function f(address payable dst) public {
                require(true);
                require(true, "test");
                assert(true);
            }
		}
	)";

    const auto& unit = *parseAndAnalyse(text);
    const auto& ctrt = *retrieveContractByName(unit, "A");
    const auto& func = *ctrt.definedFunctions()[0];

    ostringstream actual, expected;
    BlockConversionVisitor visitor(func, TypeTranslator());
    visitor.print(actual);
    expected << "{" << endl
             << "assume(1);" << endl
             << "assume(1);" << endl
             << "assert(1);" << endl
             << "}";
    BOOST_CHECK_EQUAL(actual.str(), expected.str());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
}
