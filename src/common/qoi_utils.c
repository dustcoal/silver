#include "common/qoi_utils.h"

/* Load qoi image from path */
QoiImage *read_qoi_image(char *path) {
	QoiImage *res;
	res = malloc(sizeof(*res));
	if (!res) {
		return (NULL);
	}
	res->desc = malloc(sizeof(*res->desc));
	if (!res->desc) {
		free (res);
		return (NULL);
	}
	res->pixels = qoi_read(path, res->desc, 0);
	if (!res->pixels) {
		free(res->desc);
		free(res);
		return (NULL);
	}
	return (res);
}

void free_QoiImage(QoiImage *qoiImage) {
	free(qoiImage->pixels);
	free(qoiImage->desc);
	free(qoiImage);
}
