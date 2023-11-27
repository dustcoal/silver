#include <stdio.h>

#include "cJSON.h"

#include "world.h"
#include "common/common.h"
#include "common/logging.h"
#include "server/config_main.h"

/* testing */
#include "cwalk.h"
#include "common/qoi_utils.h"
#include "common/util/pointer_util.h"
/* end testing */

//static const int CHUNK_SIZE = 32;
ServerConfig server_config;

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	common_init(SERVER);
	logging_init();

	/* Testing cwalk */
	const char *basename;
	size_t length;
	cwk_path_get_basename("/my/sneed.txt", &basename, &length);
	log_debug("", 0);
	if (DEBUG_MODE)
		printf("filename of /my/path.txt is %s and its length is %zu\n", basename, length);
	/* End testing cwalk */

	/* Testing qoi */
	QoiImage *img = read_qoi_image("textures/sneed.qoi");
	if (img) {
		log_debug("", 0);
		if (DEBUG_MODE)
			printf("successfully read test qoi image - width: %d, height: %d\n", img->desc->width, img->desc->height);
		free_QoiImage(img);
	} else {
		log_debug("failed to read test qoi image", 1);
	}
	/* End testing qoi */

	/* testing ptr */
	int a = 42;
	char *ptr = ptr_to_str(&a);
	int b = *(int *)str_to_ptr(ptr);
	log_debug("", 0);
	if (DEBUG_MODE)
		printf("b: %d\n", b);
	/* endtesting ptr */

	log_info("Server starting", 1);
	if (!read_server_config(&server_config)) {
		log_fatal("Failed to parse config", 1);
		return (1);
	}
	return (0);
}
