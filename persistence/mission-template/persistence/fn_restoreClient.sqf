// Sent by the host to the machine that owns a player's unit.
// _this = [unit, playerState or [], worldState or [], [[varName, value], ...]]
private ["_unit"];
_unit = _this select 0;

if ((count (_this select 2)) > 0) then { (_this select 2) call PERS_fnc_applyWorld };
{ call format ["%1 = _x select 1", _x select 0] } forEach (_this select 3);
[_unit, _this select 1] call PERS_fnc_applyPlayer;
