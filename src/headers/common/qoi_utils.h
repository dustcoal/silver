#ifndef CRAFT_QOI_UTILS_H
#define CRAFT_QOI_UTILS_H

#include <stdlib.h>

#include "qoi.h"

struct QoiImage {
	qoi_desc *desc;
	void *pixels;
} typedef QoiImage;

/* Load qoi image from path */
QoiImage *read_qoi_image(char *path);

/* free QoiImage struct */
void free_QoiImage(QoiImage *qoiImage);

#endif

