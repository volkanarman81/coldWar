#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Foundation/Framework/DebugLog.hpp>
#include <Evaluator/express.hpp>
#include "evaluator_fixture.hpp"
#include <string>

// str: text that reads back as the same value (used to write persistent saves)
// isNil: variable / expression nil check

namespace
{
std::string AsText(const GameValue& value)
{
    RString text = value;
    const char* data = text;
    return data ? data : "";
}

std::string EvalText(const char* expression)
{
    return AsText(GGameState.Evaluate(expression));
}

// true when str of the value parses back to a value with identical str
bool RoundTrips(const std::string& literal)
{
    GameVarSpace local(GGameState.GetContext());
    GGameState.BeginContext(&local);
    std::string code = "call {_v = " + literal + "; _s = str _v; (str (call _s)) == _s}";
    GameValue result = GGameState.Evaluate(code.c_str());
    GGameState.EndContext();
    return result.GetType() == GameBool && (bool)result;
}
} // namespace

TEST_CASE_METHOD(EvaluatorFixture, "str formats values as readable literals", "[evaluator][str]")
{
    CHECK(EvalText("str 1.5") == "1.5");
    CHECK(EvalText("str -3") == "-3");
    CHECK(EvalText("str 0.1") == "0.1");
    CHECK(EvalText("str true") == "true");
    CHECK(EvalText("str false") == "false");
    CHECK(EvalText("str \"abc\"") == "\"abc\"");
    CHECK(EvalText("str []") == "[]");
    CHECK(EvalText("str [1,[2,\"x\"],false]") == "[1,[2,\"x\"],false]");
    CHECK(EvalText("str [1,nil]") == "[1,nil]");
}

TEST_CASE_METHOD(EvaluatorFixture, "str keeps full float precision", "[evaluator][str]")
{
    // format / GetText use %g and would print 1.23457e+06
    CHECK(EvalText("str 1234567") == "1234567");
    CHECK(EvalText("str 12345.678") == "12345.678");
}

TEST_CASE_METHOD(EvaluatorFixture, "str doubles quotes inside strings", "[evaluator][str]")
{
    CHECK(EvalText("str \"say \"\"hi\"\"\"") == "\"say \"\"hi\"\"\"");
    CHECK(EvalText("str [\"a\"\"b\"]") == "[\"a\"\"b\"]");
}

TEST_CASE_METHOD(EvaluatorFixture, "str output round-trips through call", "[evaluator][str]")
{
    CHECK(RoundTrips("[1234567, 0.1, -0.000123, 6543.21, 1e-10]"));
    CHECK(RoundTrips("[\"a \"\"quoted\"\" word\", \"\", \"[x],{y}\"]"));
    CHECK(RoundTrips("[[1,[2,[3,[]]]], true, false]"));
    CHECK(RoundTrips("\"\"\"\""));
}

TEST_CASE_METHOD(EvaluatorFixture, "isNil checks variables and expressions", "[evaluator][isNil]")
{
    GGameState.Execute("test_isnil_defined = 5");
    CHECK((bool)GGameState.Evaluate("isNil \"test_isnil_undefined\""));
    CHECK_FALSE((bool)GGameState.Evaluate("isNil \"test_isnil_defined\""));
    CHECK((bool)GGameState.Evaluate("isNil {nil}"));
    CHECK_FALSE((bool)GGameState.Evaluate("isNil {1 + 1}"));
    CHECK((bool)GGameState.Evaluate("isNil \"\""));
    // commands are not variables
    CHECK((bool)GGameState.Evaluate("isNil \"pi\""));

    GameVarSpace local(GGameState.GetContext());
    GGameState.BeginContext(&local);
    CHECK((bool)GGameState.Evaluate("call {isNil \"_test_nope\"}"));
    CHECK_FALSE((bool)GGameState.Evaluate("call {_test_yes = 1; isNil \"_test_yes\"}"));
    GGameState.EndContext();
}
