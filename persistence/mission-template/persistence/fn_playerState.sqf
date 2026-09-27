// State of the local player, in the layout fn_applyPlayer.sqf expects:
// [name, positionASL, direction, damage, magazines, weapons, primaryWeapon, inVehicle]
private ["_veh"];
_veh = vehicle player;
[name player, getPosASL _veh, getDir player, damage player, magazines player, weapons player, primaryWeapon player, _veh != player]
