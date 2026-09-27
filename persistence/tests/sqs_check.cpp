// Syntax-checks every line of the persistence .sqs scripts with the real SQF evaluator.
// Usage: sqs_check <mission-template-dir> <file.sqs>...  (see run.sh)
#define main frametest_main
#include "framework_test.cpp"
#undef main
static GameValue MNoop1(const GameState*, GameValuePar) { return GameValue(); }
static GameValue MNoop0(const GameState*) { return GameValue(); }
static GameValue MNoop2(const GameState*, GameValuePar, GameValuePar) { return GameValue(); }
int main(int argc, char** argv) {
    GDir = argv[1];
    GGameState.Init(); RegisterMocks();
    GGameState.NewFunction(GameFunction(GameNothing, "goto", MNoop1, GameString));
    GGameState.NewNularOp(GameNular(GameNothing, "exit", MNoop0));
    GGameState.NewOperator(GameOperator(GameNothing, "exec", function, MNoop2, GameAny, GameString));
    int bad = 0, n = 0;
    for (int f = 2; f < argc; f++) {
        std::ifstream in(argv[f]); std::string line; int ln = 0;
        while (std::getline(in, line)) {
            ln++;
            size_t p = line.find_first_not_of(" \t"); if (p == std::string::npos) continue; line = line.substr(p);
            char c = line[0]; std::vector<std::pair<std::string, bool>> parts; // (code, isBoolCondition)
            if (c == ';' || c == '#') continue;
            if (c == 0x7e) parts.push_back({"__waituntil = _time+(" + line.substr(1) + ")", false});
            else if (c == '@') parts.push_back({line.substr(1), true});
            else if (c == '?') { size_t col = line.find(" : "); parts.push_back({line.substr(1, col - 1), true}); parts.push_back({line.substr(col + 3), false}); }
            else parts.push_back({line, false});
            GameVarSpace local(GGameState.GetContext()); GGameState.BeginContext(&local);
            for (auto& part : parts) {
                n++;
                bool ok = part.second ? GGameState.CheckEvaluateBool(part.first.c_str()) : GGameState.CheckExecute(part.first.c_str());
                if (!ok) { bad++; printf("%s:%d: %s  (%s)\n", argv[f], ln, part.first.c_str(), (const char*)GGameState.GetLastErrorText()); }
            }
            GGameState.EndContext();
        }
    }
    printf("%d sqs expressions checked, %d bad\n", n, bad);
    return bad != 0;
}
