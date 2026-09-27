// Host: apply the loaded save to the world. Players are restored one by one
// when they report ready (fn_serverTick.sqf).
// Save layout: [version, world, players, vehicles, vars]
private ["_i"];
PERS_helloSeen = [];
PERS_repSeen = [];
for "_i" from 1 to (count PERS_slots) do
{
    PERS_helloSeen = PERS_helloSeen + [""];
    PERS_repSeen = PERS_repSeen + [""];
};

if ((count PERS_loaded) > 0) then
{
    if (PERS_keepWorld && ((count (PERS_loaded select 1)) > 0)) then { (PERS_loaded select 1) call PERS_fnc_applyWorld };
    (PERS_loaded select 3) call PERS_fnc_applyVehicles;
    { call format ["%1 = _x select 1", _x select 0] } forEach (PERS_loaded select 4);
};
