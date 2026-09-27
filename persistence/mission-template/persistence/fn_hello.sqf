// Tell the host this player is ready to be restored. The counter makes every
// retry a new value, so the host answers again if an earlier answer was missed.
PERS_helloCount = PERS_helloCount + 1;
call format ["PERS_hello_%1 = [name player, PERS_helloCount]", PERS_mySlot];
publicVariable format ["PERS_hello_%1", PERS_mySlot];
