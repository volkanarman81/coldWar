// Runs the persistence framework's .sqf files unchanged in the real SQF evaluator,
// with game commands replaced by in-memory mocks. Objects are represented as strings.
#include <Evaluator/express.hpp>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace Poseidon;

static std::string GDir;
static int fails = 0;  // Usage: framework_test <mission-template-dir>  (see run.sh)
#define CHECK(x) do { if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); fails++; } } while (0)

struct Unit { std::string name; float pos[3] = {0, 0, 0}; float dir = 0, damage = 0, fuel = 1; std::vector<std::string> mags, weapons; std::string primary, vehicle; bool alive = true, deleted = false; bool setPosAboveGround = false; };
static std::map<std::string, Unit> GUnits;
static std::map<std::string, std::string> GFiles;
static struct { float date[5] = {1985, 6, 1, 8, 0}; float overcast = 0.3f, fog = 0.1f, rain = 0; } GWorldMock;
static std::string GPlayer;
static std::vector<std::pair<std::string, GameValue>> GRemoteQueue;
static std::vector<std::string> GPublished;

static std::string S(GameValuePar v) { RString t = v; const char* p = t; return p ? p : ""; }
static GameValue Str(const std::string& s) { return GameValue(RString(s.c_str())); }
static GameValue Arr(const std::vector<GameValue>& items) { GameValue v = GGameState.CreateGameValue(GameArray); GameArrayType& a = v; a.Resize(items.size()); for (size_t i = 0; i < items.size(); i++) a[(int)i] = items[i]; return v; }
static GameValue StrArr(const std::vector<std::string>& items) { std::vector<GameValue> v; for (auto& s : items) v.push_back(Str(s)); return Arr(v); }
static std::vector<std::string> ToStrs(GameValuePar v) { std::vector<std::string> r; const GameArrayType& a = v; for (int i = 0; i < a.Size(); i++) r.push_back(S(a[i])); return r; }
static Unit& U(GameValuePar v) { return GUnits[S(v)]; }

static std::string ReadScript(const std::string& name) {
    std::string path = name; for (auto& c : path) if (c == '\\') c = '/';
    std::ifstream in(GDir + "/" + path); std::stringstream ss; ss << in.rdbuf(); std::string text = ss.str();
    if (text.empty()) printf("missing script %s\n", path.c_str());
    // strip // comments (the real preprocessor does this too)
    std::string out; bool inStr = false;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '"') inStr = !inStr;
        if (!inStr && text[i] == '/' && i + 1 < text.size() && text[i + 1] == '/') { while (i < text.size() && text[i] != '\n') i++; out += '\n'; continue; }
        out += text[i];
    }
    return out;
}

// --- nulars
static GameValue MPlayer(const GameState*) { return Str(GPlayer); }
static GameValue MTime(const GameState*) { return 0.0f; }
static GameValue MIsServer(const GameState*) { return true; }
static GameValue MDate(const GameState*) { std::vector<GameValue> v; for (float f : GWorldMock.date) v.push_back(f); return Arr(v); }
static GameValue MOvercast(const GameState*) { return GWorldMock.overcast; }
static GameValue MFog(const GameState*) { return GWorldMock.fog; }
static GameValue MRain(const GameState*) { return GWorldMock.rain; }
// --- unary
static GameValue MName(const GameState*, GameValuePar u) { return Str(U(u).name); }
static GameValue MGetPosASL(const GameState*, GameValuePar u) { auto& x = U(u); return Arr({x.pos[0], x.pos[1], x.pos[2]}); }
static GameValue MGetDir(const GameState*, GameValuePar u) { return U(u).dir; }
static GameValue MDamage(const GameState*, GameValuePar u) { return U(u).damage; }
static GameValue MFuel(const GameState*, GameValuePar u) { return U(u).fuel; }
static GameValue MMags(const GameState*, GameValuePar u) { return StrArr(U(u).mags); }
static GameValue MWeapons(const GameState*, GameValuePar u) { return StrArr(U(u).weapons); }
static GameValue MPrimary(const GameState*, GameValuePar u) { return Str(U(u).primary); }
static GameValue MVehicle(const GameState*, GameValuePar u) { auto& x = U(u); return Str(x.vehicle.empty() ? S(u) : x.vehicle); }
static GameValue MAlive(const GameState*, GameValuePar u) { auto& x = U(u); return x.alive && !x.deleted; }
static GameValue MIsNull(const GameState*, GameValuePar u) { return S(u).empty() || U(u).deleted; }
static GameValue MRemoveAll(const GameState*, GameValuePar u) { auto& x = U(u); x.mags.clear(); x.weapons.clear(); x.primary = ""; return GameValue(); }
static GameValue MDelete(const GameState*, GameValuePar u) { U(u).deleted = true; return GameValue(); }
static GameValue MPublicVariable(const GameState*, GameValuePar n) { GPublished.push_back(S(n)); return GameValue(); }
static GameValue MLog(const GameState*, GameValuePar t) { printf("  [log] %s\n", S(t).c_str()); return GameValue(); }
static GameValue MHint(const GameState*, GameValuePar t) { printf("  [hint] %s\n", S(t).c_str()); return GameValue(); }
static GameValue MLoadString(const GameState*, GameValuePar n) { return Str(GFiles[S(n)]); }
static GameValue MPreprocess(const GameState*, GameValuePar n) { return Str(ReadScript(S(n))); }
static GameValue MSetDate(const GameState*, GameValuePar a) { const GameArrayType& d = a; for (int i = 0; i < 5; i++) GWorldMock.date[i] = (float)d[i]; return GameValue(); }
// copy of StrFormat from engine/Poseidon/Game/Commands/GameStateExtWorld.cpp
static GameValue MFormat(const GameState*, GameValuePar oper1) {
    const GameArrayType& array = oper1;
    if (array.Size() < 1 || array[0].GetType() != GameString) return "";
    RString format = array[0]; int nParams = array.Size() - 1; std::string result; const char* src = format;
    while (char c = *src) {
        if (c == '%') { src++; int index = 0; while (isdigit((unsigned char)*src)) { index = index * 10 + (*src - '0'); src++; }
            if (index < 1 || index > nParams) continue;
            const GameValue& value = array[index];
            RString text = (value.GetType() == GameString) ? value.GetData()->GetString() : value.GetText();
            result.append((const char*)text, text.GetLength()); }
        else { result.push_back(c); src++; }
    }
    return RString(result.c_str());
}
// --- binary
static GameValue MSaveString(const GameState*, GameValuePar n, GameValuePar t) { GFiles[S(n)] = S(t); return true; }
static GameValue MAddMag(const GameState*, GameValuePar u, GameValuePar m) { U(u).mags.push_back(S(m)); return GameValue(); }
static GameValue MAddWeapon(const GameState*, GameValuePar u, GameValuePar w) { U(u).weapons.push_back(S(w)); return GameValue(); }
static GameValue MSelectWeapon(const GameState*, GameValuePar u, GameValuePar w) { U(u).primary = S(w); return GameValue(); }
static GameValue MSetDir(const GameState*, GameValuePar u, GameValuePar d) { U(u).dir = d; return GameValue(); }
static GameValue MSetPosASL(const GameState*, GameValuePar u, GameValuePar p) { const GameArrayType& a = p; auto& x = U(u); for (int i = 0; i < 3; i++) x.pos[i] = a[i]; x.setPosAboveGround = false; return GameValue(); }
static GameValue MSetPos(const GameState*, GameValuePar u, GameValuePar p) { const GameArrayType& a = p; auto& x = U(u); for (int i = 0; i < 3; i++) x.pos[i] = a[i]; x.setPosAboveGround = true; return GameValue(); }
static GameValue MSetDamage(const GameState*, GameValuePar u, GameValuePar d) { U(u).damage = d; return GameValue(); }
static GameValue MSetFuel(const GameState*, GameValuePar u, GameValuePar f) { U(u).fuel = f; return GameValue(); }
static GameValue MSetOvercast(const GameState*, GameValuePar, GameValuePar v) { GWorldMock.overcast = v; return GameValue(); }
static GameValue MSetFog(const GameState*, GameValuePar, GameValuePar v) { GWorldMock.fog = v; return GameValue(); }
static GameValue MSetRain(const GameState*, GameValuePar, GameValuePar v) { GWorldMock.rain = v; return GameValue(); }
static GameValue MRemoteExec(const GameState*, GameValuePar params, GameValuePar spec) { const GameArrayType& a = spec; GRemoteQueue.push_back({S(a[0]), params}); return GameValue(); }

static void RegisterMocks() {
    GGameState.NewNularOp(GameNular(GameString, "player", MPlayer));
    GGameState.NewNularOp(GameNular(GameScalar, "time", MTime));
    GGameState.NewNularOp(GameNular(GameBool, "isServer", MIsServer));
    GGameState.NewNularOp(GameNular(GameArray, "date", MDate));
    GGameState.NewNularOp(GameNular(GameScalar, "overcast", MOvercast));
    GGameState.NewNularOp(GameNular(GameScalar, "fog", MFog));
    GGameState.NewNularOp(GameNular(GameScalar, "rain", MRain));
    struct { const char* n; ProcUnary p; GameType r, a; } un[] = {
        {"name", MName, GameString, GameString}, {"getPosASL", MGetPosASL, GameArray, GameString}, {"getDir", MGetDir, GameScalar, GameString},
        {"damage", MDamage, GameScalar, GameString}, {"fuel", MFuel, GameScalar, GameString}, {"magazines", MMags, GameArray, GameString},
        {"weapons", MWeapons, GameArray, GameString}, {"primaryWeapon", MPrimary, GameString, GameString}, {"vehicle", MVehicle, GameString, GameString},
        {"alive", MAlive, GameBool, GameString}, {"isNull", MIsNull, GameBool, GameString}, {"removeAllWeapons", MRemoveAll, GameNothing, GameString},
        {"deleteVehicle", MDelete, GameNothing, GameString}, {"publicVariable", MPublicVariable, GameNothing, GameString}, {"logInfo", MLog, GameNothing, GameVoid},
        {"hint", MHint, GameNothing, GameString}, {"loadString", MLoadString, GameString, GameString}, {"preprocessFile", MPreprocess, GameString, GameString},
        {"setDate", MSetDate, GameNothing, GameArray}, {"format", MFormat, GameString, GameArray}};
    for (auto& f : un) GGameState.NewFunction(GameFunction(f.r, f.n, f.p, f.a));
    struct { const char* n; ProcBinary p; GameType r, a, b; } bin[] = {
        {"saveString", MSaveString, GameBool, GameString, GameString}, {"addMagazine", MAddMag, GameNothing, GameString, GameString},
        {"addWeapon", MAddWeapon, GameNothing, GameString, GameString}, {"selectWeapon", MSelectWeapon, GameNothing, GameString, GameString},
        {"setDir", MSetDir, GameNothing, GameString, GameScalar}, {"setPosASL", MSetPosASL, GameNothing, GameString, GameArray},
        {"setPos", MSetPos, GameNothing, GameString, GameArray}, {"setDamage", MSetDamage, GameNothing, GameString, GameScalar},
        {"setFuel", MSetFuel, GameNothing, GameString, GameScalar}, {"setOvercast", MSetOvercast, GameNothing, GameScalar, GameScalar},
        {"setFog", MSetFog, GameNothing, GameScalar, GameScalar}, {"setRain", MSetRain, GameNothing, GameScalar, GameScalar},
        {"remoteExec", MRemoteExec, GameNothing, GameAny, GameArray}};
    for (auto& f : bin) GGameState.NewOperator(GameOperator(f.r, f.n, function, f.p, f.a, f.b));
}

// Run code like a script line: in its own local context.
static GameValue Run(const std::string& code) {
    GameVarSpace local(GGameState.GetContext());
    GGameState.BeginContext(&local);
    GameValue r = GGameState.Evaluate(("call {" + code + "}").c_str());
    if (GGameState.GetLastError() != EvalOK) printf("  [eval error] %s in: %.120s\n", (const char*)GGameState.GetLastErrorText(), code.c_str());
    GGameState.EndContext();
    // deliver remoteExec messages the way the engine does (ExecuteNamedRemoteExec)
    while (!GRemoteQueue.empty()) {
        auto msg = GRemoteQueue.front(); GRemoteQueue.erase(GRemoteQueue.begin());
        GameVarSpace l2(GGameState.GetContext());
        GGameState.BeginContext(&l2);
        GGameState.VarSetLocal("_this", msg.second, true);
        GameValue fn = GGameState.VarGet(msg.first.c_str());
        if (fn.GetType() == GameString) { GGameState.Execute((RString)(GameStringType)fn); if (GGameState.GetLastError() != EvalOK) printf("  [eval error] %s in remote %s\n", (const char*)GGameState.GetLastErrorText(), msg.first.c_str()); }
        else printf("remoteExec target function %s is not defined\n", msg.first.c_str());
        GGameState.EndContext();
    }
    return r;
}
static bool B(const std::string& code) { GameValue v = Run(code); return v.GetType() == GameBool && (bool)v; }
static std::string T(const std::string& code) { return S(Run(code)); }

static void StartSession(const char* configExtra) {
    GGameState.Reset();
    GGameState.Init();
    RegisterMocks();
    // init.sqs, host part
    Run("call preprocessFile \"persistence\\config.sqf\"");
    Run(configExtra);
    Run("call preprocessFile \"persistence\\functions.sqf\"");
    Run("p1 = \"unitA\"; p2 = \"unitB\"; truck1 = \"veh1\"; tank1 = \"veh2\"");
    // server.sqs start
    Run("PERS_serverReady = false; [] call PERS_fnc_load; [] call PERS_fnc_restoreServer; PERS_serverReady = true");
}

static void ResetWorldObjects() {
    GUnits.clear();
    GUnits["unitA"] = Unit{"Alice", {100, 200, 5}, 0, 0, 1, {"M16", "M16"}, {"M16"}, "M16"};
    GUnits["unitB"] = Unit{"Bob", {110, 200, 5}, 0, 0, 1, {"AK74"}, {"AK74"}, "AK74"};
    GUnits["veh1"] = Unit{"truck", {300, 300, 7}, 90, 0, 1};
    GUnits["veh2"] = Unit{"tank", {400, 400, 9}, 180, 0, 1};
    GWorldMock.date[0] = 1985; GWorldMock.date[1] = 6; GWorldMock.date[2] = 1; GWorldMock.date[3] = 8; GWorldMock.date[4] = 0;
    GWorldMock.overcast = 0.3f; GWorldMock.fog = 0.1f; GWorldMock.rain = 0;
}

// client.sqs steps for the local player
static void ClientJoin(const char* unitId) {
    GPlayer = unitId;
    Run("[] call PERS_fnc_findSlot; PERS_helloCount = 0; [] call PERS_fnc_hello");
}

int main(int argc, char** argv) {
    GDir = argv[1];
    const char* cfg = "PERS_slots = [\"p1\",\"p2\"]; PERS_vehicles = [\"truck1\",\"tank1\"]; PERS_vars = [\"money\",\"towns\"]; PERS_saveName = \"test.txt\"";

    printf("== session 1: fresh start, play, save\n");
    ResetWorldObjects();
    StartSession(cfg);
    CHECK(T("str PERS_loaded") == "[]");
    Run("money = 1234567; towns = [\"Le Port\", \"say \"\"hi\"\"\"]");
    ClientJoin("unitA");                       // host player in slot p1
    CHECK(T("str PERS_mySlot") == "0");
    CHECK(B("isNil \"PERS_restored\""));
    Run("[] call PERS_fnc_serverTick");        // host answers hello
    CHECK(B("!(isNil \"PERS_restored\")"));
    CHECK(GUnits["unitA"].weapons.size() == 1); // no save: gear untouched

    // play: Alice moves, swaps gear, gets hurt; truck moves; tank destroyed; weather changes
    auto& a = GUnits["unitA"]; a.pos[0] = 1234.5f; a.pos[1] = 5678.25f; a.pos[2] = 12; a.dir = 45; a.damage = 0.25f;
    a.mags = {"30Rnd_556x45_Stanag", "HandGrenade"}; a.weapons = {"M4", "Binocular"}; a.primary = "M4";
    GUnits["veh1"].pos[0] = 999; GUnits["veh1"].fuel = 0.4f; GUnits["veh1"].damage = 0.3f;
    GUnits["veh2"].alive = false;
    GWorldMock.date[3] = 17; GWorldMock.date[4] = 45; GWorldMock.overcast = 0.8f; GWorldMock.rain = 0.6f;
    Run("[] call PERS_fnc_report");
    Run("[] call PERS_fnc_serverTick");
    CHECK(T("str (count PERS_players)") == "1");
    CHECK(B("[] call PERS_fnc_save"));
    printf("  save file: %s\n", GFiles["test.txt"].c_str());

    // stale report must not be merged again
    Run("PERS_players = []");
    Run("[] call PERS_fnc_serverTick");
    CHECK(T("str (count PERS_players)") == "0");
    // a new report (state changed) is merged again
    a.dir = 46;
    Run("[] call PERS_fnc_report; [] call PERS_fnc_serverTick");
    a.dir = 45;
    Run("[] call PERS_fnc_report; [] call PERS_fnc_serverTick");
    CHECK(T("str (count PERS_players)") == "1");
    CHECK(B("(count (PERS_players select 0)) == 8"));
    CHECK(B("[] call PERS_fnc_save"));
    std::string saved = GFiles["test.txt"];

    printf("== session 2: restart, everything comes back\n");
    ResetWorldObjects();
    StartSession(cfg);
    CHECK(GFiles["test.txt.bak"] == saved);
    // world restored on host
    CHECK(GWorldMock.date[3] == 17 && GWorldMock.date[4] == 45);
    CHECK(GWorldMock.overcast > 0.79f && GWorldMock.overcast < 0.81f && GWorldMock.rain > 0.59f && GWorldMock.rain < 0.61f);
    // vehicles
    CHECK(GUnits["veh1"].pos[0] == 999 && GUnits["veh1"].fuel > 0.39f && GUnits["veh1"].fuel < 0.41f && GUnits["veh1"].damage > 0.29f);
    CHECK(GUnits["veh2"].deleted);
    // variables, exact
    CHECK(T("str money") == "1234567");
    CHECK(T("towns select 1") == "say \"hi\"");
    // Alice rejoins in slot p2 this time (unitB now named Alice), Bob in p1
    GUnits["unitB"].name = "Alice"; GUnits["unitA"].name = "Bob";
    ClientJoin("unitB");
    Run("[] call PERS_fnc_serverTick");
    auto& b = GUnits["unitB"];
    CHECK(b.pos[0] == 1234.5f && b.pos[1] == 5678.25f && b.pos[2] == 12);
    CHECK(b.dir == 45 && b.damage == 0.25f);
    CHECK((b.weapons == std::vector<std::string>{"M4", "Binocular"}));
    CHECK((b.mags == std::vector<std::string>{"30Rnd_556x45_Stanag", "HandGrenade"}));
    CHECK(b.primary == "M4");
    CHECK(B("!(isNil \"PERS_restored\")"));
    // Bob (no save) keeps his default gear
    Run("PERS_restored = nil");
    ClientJoin("unitA");
    Run("[] call PERS_fnc_serverTick");
    CHECK((GUnits["unitA"].weapons == std::vector<std::string>{"AK74"} || GUnits["unitA"].weapons == std::vector<std::string>{"M16"}));
    CHECK(B("!(isNil \"PERS_restored\")"));

    printf("== session 3: saved in a vehicle\n");
    ResetWorldObjects();
    StartSession(cfg);
    ClientJoin("unitA");
    Run("[] call PERS_fnc_serverTick");
    GUnits["unitA"].vehicle = "veh1";
    Run("[] call PERS_fnc_report; [] call PERS_fnc_serverTick; [] call PERS_fnc_save");
    ResetWorldObjects();
    StartSession(cfg);
    ClientJoin("unitA");
    Run("[] call PERS_fnc_serverTick");
    CHECK(GUnits["unitA"].setPosAboveGround && GUnits["unitA"].pos[0] == 1002 && GUnits["unitA"].pos[2] == 0);

    printf("== corrupt save file\n");
    std::string goodBak = GFiles["test.txt.bak"];
    CHECK(!goodBak.empty());
    GFiles["test.txt"] = "[1, [broken";
    ResetWorldObjects();
    StartSession(cfg);
    CHECK(T("str PERS_loaded") == "[]");
    CHECK(GFiles["test.txt.bak"] == goodBak);   // corrupt file must not replace the backup

    printf("== client.sqs wait condition\n");
    CHECK(B("_time = 5; _retry = 10; PERS_restored = true; !(isNil \"PERS_restored\") || _time > _retry"));
    CHECK(!B("_time = 5; _retry = 10; PERS_restored = nil; !(isNil \"PERS_restored\") || _time > _retry"));
    CHECK(B("_time = 11; _retry = 10; PERS_restored = nil; !(isNil \"PERS_restored\") || _time > _retry"));

    printf(fails ? "%d FAILURES\n" : "ALL PASSED\n", fails);
    return fails != 0;
}
