// Host: store a player state reported by fn_report.sqf, replacing the older
// state with the same player name. _this = [playerState]
private ["_state", "_name", "_i", "_found"];
_state = _this select 0;
_name = _state select 0;
_found = -1;
for "_i" from 0 to ((count PERS_players) - 1) do
{
    if (((PERS_players select _i) select 0) == _name) then { _found = _i };
};
if (_found < 0) then
{
    PERS_players = PERS_players + [_state];
}
else
{
    PERS_players set [_found, _state];
};
