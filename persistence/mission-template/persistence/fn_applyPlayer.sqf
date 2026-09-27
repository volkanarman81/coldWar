// Apply a saved state to a unit. Runs on the machine that owns the unit, because
// setDir / addWeapon / addMagazine only take effect there.
// _this = [unit, [name, positionASL, direction, damage, magazines, weapons, primaryWeapon, inVehicle] or []]
private ["_unit", "_s", "_pos"];
_unit = _this select 0;
_s = _this select 1;

if ((count _s) > 0) then
{
    removeAllWeapons _unit;
    { _unit addMagazine _x } forEach (_s select 4);
    { _unit addWeapon _x } forEach (_s select 5);
    if ((_s select 6) != "") then { _unit selectWeapon (_s select 6) };

    _unit setDir (_s select 2);
    _pos = _s select 1;
    if (_s select 7) then
    {
        // was in a vehicle: stand next to it (the vehicle is restored separately)
        _unit setPos [(_pos select 0) + 3, (_pos select 1), 0];
    }
    else
    {
        _unit setPosASL _pos;
    };
    _unit setDamage (_s select 3);
};

if (_unit == player) then { PERS_restored = true };
