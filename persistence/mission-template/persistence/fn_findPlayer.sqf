// Host: saved state of the named player, or [] if there is none. _this = [playerName]
private ["_name", "_result"];
_name = _this select 0;
_result = [];
{ if ((_x select 0) == _name) then { _result = _x } } forEach PERS_players;
_result
