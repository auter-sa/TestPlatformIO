# TestPlatformIO
Test project for PlatformIO and Arduino. To be compiled using Visual Studio Code.

***Notes:***

1. The action buffer **ledValue** is defined as an array of 8 integer values, but only the first one is being used, to turn on/off DIMAC led lights according to its bit values:
   - Bits 1, 4, 7 -> First led
   - Bits 2, 5, 8 -> Second led
   - Bits 3, 6    -> Third led

2. The only Schedule function meant to interact with such buffer is `void Schedule::triggerAction(ScheduleEntry *action)`.

3. Not using seasons from the `.hex` file yet. To be done.
