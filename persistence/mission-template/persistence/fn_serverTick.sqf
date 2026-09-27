// Host, once a second: take in new player reports and answer players that are ready.
// PERS_rep_<slot> and PERS_hello_<slot> arrive from players through publicVariable.
// Only values that changed since the last tick are used, so a report left behind by
// a player who has since moved to another slot is never applied twice.
private ["_i", "_var", "_value", "_text", "_unitName", "_unit"];
for "_i" from 0 to ((count PERS_slots) - 1) do
{
    _var = format ["PERS_rep_%1", _i];
    if (!(isNil _var)) then
    {
        _value = call _var;
        _text = str _value;
        if (_text != (PERS_repSeen select _i)) then
        {
            PERS_repSeen set [_i, _text];
            [_value] call PERS_fnc_mergePlayer;
        };
    };

    _var = format ["PERS_hello_%1", _i];
    if (!(isNil _var)) then
    {
        _value = call _var;
        _text = str _value;
        _unitName = PERS_slots select _i;
        if ((_text != (PERS_helloSeen select _i)) && !(isNil _unitName)) then
        {
            _unit = call _unitName;
            if (!(isNull _unit)) then
            {
                PERS_helloSeen set [_i, _text];
                [_unit, _value select 0] call PERS_fnc_sendRestore;
            };
        };
    };
};
