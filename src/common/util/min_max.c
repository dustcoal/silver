#include "common/util/min_max.h"

int min(int a, int b) {
	if (a < b) {
		return (a);
	}
	return (b);
}

int max(int a, int b) {
	if (a > b) {
		return (a);
	}
	return (b);
}

unsigned long ulmin(unsigned long a, unsigned long b) {
	if (a < b) {
		return (a);
	}
	return (b);
}

unsigned long ulmax(unsigned long a, unsigned long b) {
	if (a > b) {
		return (a);
	}
	return (b);
}
