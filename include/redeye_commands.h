#ifndef REDEYE_COMMANDS_H
#define REDEYE_COMMANDS_H

/* RedEye protocol command table */
#define REDEYE_ESCAPE 27
#define REDEYE_CR     4
#define REDEYE_LF     10
#define REDEYE_FILLER 158 
#define REDEYE_RESET  255
#define REDEYE_TEST   254
#define REDEYE_SETWC  253
#define REDEYE_ENDWC  252
#define REDEYE_SETUL  251
#define REDEYE_ENDUL  250
#define REDEYE_ECMA94 249 // ISO8859-1 [Latin-1]
#define REDEYE_ROMAN8 248

#define LINEBREAKERS_COUNT 1
static uint8_t linebreakers[LINEBREAKERS_COUNT] = {0x20};

#endif // REDEYE_COMMANDS_H

