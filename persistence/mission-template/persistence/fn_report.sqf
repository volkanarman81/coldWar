// Send the local player's state to the host (read there by fn_serverTick.sqf).
private ["_state"];
_state = [] call PERS_fnc_playerState;
call format ["PERS_rep_%1 = _state", PERS_mySlot];
publicVariable format ["PERS_rep_%1", PERS_mySlot];
