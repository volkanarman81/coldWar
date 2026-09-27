// Host: apply saved vehicle states (see fn_vehiclesState.sqf). Vehicles that were
// destroyed are removed, so a wreck does not explode again at mission start.
// Vehicles that are no longer listed in PERS_vehicles are ignored.
private ["_name", "_obj"];
{
    _name = _x select 0;
    if ((_name in PERS_vehicles) && !(isNil _name)) then
    {
        _obj = call _name;
        if (!(isNull _obj)) then
        {
            if (_x select 1) then
            {
                _obj setDir (_x select 3);
                _obj setPosASL (_x select 2);
                _obj setDamage (_x select 4);
                _obj setFuel (_x select 5);
            }
            else
            {
                deleteVehicle _obj;
            };
        };
    };
} forEach _this;
