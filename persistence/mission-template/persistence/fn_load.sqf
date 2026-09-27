// Host: read the save file into PERS_loaded ([] when there is none) and
// PERS_players. A save that reads correctly is also kept as <name>.bak, so a bad
// session can be rolled back by copying the .bak over the save.
private ["_text", "_data"];
PERS_loaded = [];
PERS_players = [];
_text = loadString PERS_saveName;
if (_text != "") then
{
    _data = call _text;
    if (isNil "_data") then
    {
        logInfo format ["Persistence: %1 could not be read, starting fresh (the previous .bak is kept)", PERS_saveName];
    }
    else
    {
        (PERS_saveName + ".bak") saveString _text;
        PERS_loaded = _data;
        PERS_players = _data select 2;
    };
};
