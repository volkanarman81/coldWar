// Persistence settings. Edit these for your mission.

// Save file name, stored on the host in <profile>/Saves/.
// 1-64 characters from A-Z a-z 0-9 _ - . ; use a different name per mission.
PERS_saveName = "cwr_persist.txt";

// Editor names of the playable units. Every player slot must be listed here,
// otherwise that player's progress is not saved.
PERS_slots = ["p1", "p2", "p3", "p4"];

// Editor names of the vehicles to keep (position, direction, damage, fuel).
// A vehicle destroyed during play stays gone in later sessions.
PERS_vehicles = [];

// Global variables to keep, e.g. ["money", "townsCaptured"].
// Values may be numbers, strings, booleans and arrays of those (not units or vehicles).
PERS_vars = [];

// Keep date/time and weather between sessions.
PERS_keepWorld = true;

// Seconds between automatic saves on the host.
PERS_saveInterval = 60;

// Seconds between each player's state reports to the host.
PERS_reportInterval = 15;
