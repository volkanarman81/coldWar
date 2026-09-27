// [[varName, value], ...] for every defined variable in PERS_vars
private ["_list"];
_list = [];
{ if (!(isNil _x)) then { _list = _list + [[_x, call _x]] } } forEach PERS_vars;
_list
