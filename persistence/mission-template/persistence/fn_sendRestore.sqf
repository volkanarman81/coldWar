// Host: send a player's saved state, the current world and the kept variables to
// the machine that owns the player's unit.
// _this = [unit, playerName]
private ["_unit", "_world"];
_unit = _this select 0;
_world = [];
if (PERS_keepWorld) then { _world = [] call PERS_fnc_worldState };
[_unit, [_this select 1] call PERS_fnc_findPlayer, _world, [] call PERS_fnc_varsState] remoteExec ["PERS_fnc_restoreClient", _unit];
