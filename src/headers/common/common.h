#ifndef CRAFT_COMMON_H
#define CRAFT_COMMON_H

enum Enum_Side {
	CLIENT,
	SERVER,
	NONE
};

int	common_init(enum Enum_Side side);

extern int NET_SIDE;
extern int DEBUG_MODE;

#endif
