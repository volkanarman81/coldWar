// Host: state of every vehicle in PERS_vehicles:
// [name, true, positionASL, direction, damage, fuel] while it exists,
// [name, false] once it is destroyed or deleted.
private ["_list", "_obj"];
_list = [];
{
    if (!(isNil _x)) then
    {
        _obj = call _x;
        if (isNull _obj) then
        {
            _list = _list + [[_x, false]];
        }
        else
        {
            if (alive _obj) then
            {
                _list = _list + [[_x, true, getPosASL _obj, getDir _obj, damage _obj, fuel _obj]];
            }
            else
            {
                _list = _list + [[_x, false]];
            };
        };
    };
} forEach PERS_vehicles;
_list
