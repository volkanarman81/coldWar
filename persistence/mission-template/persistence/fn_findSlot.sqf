// Sets PERS_mySlot to the index in PERS_slots of the local player's unit, or -1.
private ["_i", "_name"];
PERS_mySlot = -1;
for "_i" from 0 to ((count PERS_slots) - 1) do
{
    _name = PERS_slots select _i;
    if (!(isNil _name)) then
    {
        if ((call _name) == player) then { PERS_mySlot = _i };
    };
};
PERS_mySlot
