#include "client/chat.h"


void add_message(const char *text) {
	printf("%s\n", text);
	snprintf(
			g->messages[g->message_index], MAX_TEXT_LENGTH, "%s", text);
	g->message_index = (g->message_index + 1) % MAX_MESSAGES;
}
