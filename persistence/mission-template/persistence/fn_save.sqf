// Host: write everything to the save file. Called by the autosave loop; you can
// also call it yourself, e.g. from an end-of-mission trigger:  [] call PERS_fnc_save
// Save layout: [version, world, players, vehicles, vars]
private ["_world", "_data", "_ok"];
_ok = false;
if (PERS_serverReady) then
{
    _world = [];
    if (PERS_keepWorld) then { _world = [] call PERS_fnc_worldState };
    _data = [1, _world, PERS_players, [] call PERS_fnc_vehiclesState, [] call PERS_fnc_varsState];
    _ok = PERS_saveName saveString (str _data);
    if (!_ok) then { logInfo format ["Persistence: could not write %1", PERS_saveName] };
};
_ok
