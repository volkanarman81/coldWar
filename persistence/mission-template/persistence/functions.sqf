// Loads every persistence function. Functions are strings of code (OFP style):
// call them with  [args] call PERS_fnc_name.

// shared
PERS_fnc_applyWorld = preprocessFile "persistence\fn_applyWorld.sqf";
PERS_fnc_applyPlayer = preprocessFile "persistence\fn_applyPlayer.sqf";
PERS_fnc_restoreClient = preprocessFile "persistence\fn_restoreClient.sqf";

// client
PERS_fnc_findSlot = preprocessFile "persistence\fn_findSlot.sqf";
PERS_fnc_playerState = preprocessFile "persistence\fn_playerState.sqf";
PERS_fnc_hello = preprocessFile "persistence\fn_hello.sqf";
PERS_fnc_report = preprocessFile "persistence\fn_report.sqf";

// server
PERS_fnc_load = preprocessFile "persistence\fn_load.sqf";
PERS_fnc_restoreServer = preprocessFile "persistence\fn_restoreServer.sqf";
PERS_fnc_save = preprocessFile "persistence\fn_save.sqf";
PERS_fnc_serverTick = preprocessFile "persistence\fn_serverTick.sqf";
PERS_fnc_sendRestore = preprocessFile "persistence\fn_sendRestore.sqf";
PERS_fnc_mergePlayer = preprocessFile "persistence\fn_mergePlayer.sqf";
PERS_fnc_findPlayer = preprocessFile "persistence\fn_findPlayer.sqf";
PERS_fnc_worldState = preprocessFile "persistence\fn_worldState.sqf";
PERS_fnc_vehiclesState = preprocessFile "persistence\fn_vehiclesState.sqf";
PERS_fnc_applyVehicles = preprocessFile "persistence\fn_applyVehicles.sqf";
PERS_fnc_varsState = preprocessFile "persistence\fn_varsState.sqf";
