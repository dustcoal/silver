#include <stdio.h>

#include "cJSON.h"
#include "log.h"

#include "world.h"
#include "common/common.h"
#include "server/config_main.h"

/* testing */
#include "cwalk.h"
#include "common/qoi_utils.h"
#include "common/util/pointer_util.h"
#include "common/util/vec_util.h"
#include "common/config.h"
/* end testing */

//static const int CHUNK_SIZE = 32;
ServerConfig server_config;

int main(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	logging_init(stdout);
	common_init(SERVER);

	log_error("This is a sample error\n");
	log_warn("This is a sample warning\n");

	/* Testing cwalk */
	const char *basename;
	size_t length;
	cwk_path_get_basename("/my/sneed.txt", &basename, &length);
	log_debug("filename of /my/path.txt is %s and its length is %zu\n", basename, length);
	/* End testing cwalk */

	/* Testing qoi */
	QoiImage *img = read_qoi_image("textures/sneed.qoi");
	if (img) {
		log_debug("successfully read test qoi image - width: %d, height: %d\n", img->desc->width, img->desc->height);
		free_QoiImage(img);
	} else {
		log_debug("failed to read test qoi image\n");
	}
	/* End testing qoi */

	/* testing ptr */
	int a = 42;
	char *ptr = ptr_to_str(&a);
	int b = *(int *)str_to_ptr(ptr);
	log_debug("b: %d\n", b);
	free(ptr);
	/* endtesting ptr */

	jsonConfig *conf = new_conf("my_server_config.json", "server_main","Server Main", "The main config for the server.", NULL);
	if (conf) {
		jsonConfig *section1 = get_section(conf, "section1", "The first section", "This is the first section, I guess.");
		if (section1) {
			int apple_amount = get_int_val(section1, "number_of_apples", "The amount of apples.", 5, 0, 999);
			printf("there are %d apples\n", apple_amount);
			float brightness = get_float_val(section1, "brightness", "Screen brightness.", 3.4f, 0.0f, 10.0f);
			printf("brightness: %f\n", brightness);

			char *str = cJSON_Print(conf);
			printf("%s\n", str);

			free(str);
		}
		save_conf(conf);
		cJSON_Delete(conf);
	}

	log_info("Server starting\n");
	if (!read_server_config(&server_config)) {
		log_fatal("Failed to parse config\n");
		return (1);
	}
	return (0);
}
