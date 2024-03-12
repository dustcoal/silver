/* Home of the high lvl config API */


#include "common/config.h"
#include "common/common.h"
#include "common/util/itoa.h"
#include "common/util/vec_util.h"
#include "common/util/file_util.h"
#include "common/util/ftoa.h"

jsonConfig  *new_conf(char *path, char *name, char *displayname, char *description, void *(*populate_func)(jsonConfig *)) {
	jsonConfig *conf = NULL;
	if (file_readable(path)) {
		char *json_content = read_file(path);
		if (json_content) {
			conf = cJSON_Parse(json_content);
			free(json_content);
			if (!conf) {
				return (NULL);
			}
			/* still update the path, displayname, description, pointer */
			char *populate_func_ptr = ptr_to_str(populate_func);
			if (populate_func_ptr) {
				cJSON_AddStringToObject(conf, "populate_func_ptr", populate_func_ptr);
				free(populate_func_ptr);
			}
			cJSON *pathobj = cJSON_CreateString(path);
			if (pathobj) {
				cJSON_ReplaceItemInObjectCaseSensitive(conf, "path", pathobj);
			}
			cJSON *displaynameobj = cJSON_CreateString(displayname);
			if (pathobj) {
				cJSON_ReplaceItemInObjectCaseSensitive(conf, "displayname", displaynameobj);
			}
			cJSON *descriptionobj = cJSON_CreateString(description);
			if (pathobj) {
				cJSON_ReplaceItemInObjectCaseSensitive(conf, "description", descriptionobj);
			}
			return (conf);
		}
	}
	conf = cJSON_CreateObject();
	if (!conf) {
		return (NULL);
	}
	char *populate_func_ptr = ptr_to_str(populate_func);
	if (!populate_func_ptr) {
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "populate_func_ptr", populate_func_ptr) == NULL)
	{
		cJSON_Delete(conf);
		free(populate_func_ptr);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "path", path) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "name", name) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "displayname", displayname) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "description", description) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	free(populate_func_ptr);
	return (conf);
}

void save_conf(jsonConfig *cfg) {
	cJSON_DeleteItemFromObject(cfg, "populate_func_ptr");
	cJSON  *nameobj = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameobj || !cJSON_IsString(nameobj)) {
		log_warn("Failed to serialize config '%s'\n");
		return ;
	}
	char *name = cJSON_GetStringValue(nameobj);
	if (!name) {
		log_warn("Failed to serialize config '%s'\n");
		return ;
	}

	cJSON  *pathobj = cJSON_GetObjectItemCaseSensitive(cfg, "path");
	if (!pathobj || !cJSON_IsString(pathobj)) {
		log_warn("Failed to serialize config '%s'\n");
		return ;
	}
	char *path = cJSON_GetStringValue(pathobj);
	if (!path) {
		log_warn("Failed to serialize config '%s'\n");
		return ;
	}

	char *text = cJSON_Print(cfg);
	if (!text) {
		log_warn("Failed to serialize config '%s'\n", name);
		return ;
	}

	writeFile(path, text);
	free(text);
}

/* todo: move this elsewhere */
void *populate_config_struct(jsonConfig *cfg) {
	cJSON *ptr_json = cJSON_GetObjectItemCaseSensitive(cfg, "populate_func_ptr");
	if (!ptr_json) {
		return (NULL);
	}
	if (!cJSON_IsString(ptr_json)) {
		cJSON_Delete(ptr_json);
		return (NULL);
	}
	void *(*populate_func)(jsonConfig *) = (void *(*)(jsonConfig*)) str_to_ptr(ptr_json->valuestring);
	if (populate_func == NULL) {
		cJSON_Delete(ptr_json);
		return (NULL);
	}
	void *res = populate_func(cfg);
	cJSON_Delete(ptr_json);
	return (res);
}

static int check_int(int val, int min, int max) {
	return (val <= max && val >= min);
}

static int check_float(float val, float min, float max) {
	return (val <= max && val >= min);
}

static int check_int_vec(vec_int val, int min, int max) {
	for (int i = 0; i < vector_size(val); ++i) {
		if (!check_int(val[i], min, max)) {
			return (0);
		}
	}
	return (1);
}

configSection *get_section(configSection *target_section, char *name, char *displayname, char *description) {
	cJSON *gotten_section = cJSON_GetObjectItemCaseSensitive(target_section, name);
	if (gotten_section) {
		/* still updating displayname and description */
		cJSON *displaynameobj = cJSON_CreateString(displayname);
		if (displaynameobj) {
			cJSON_ReplaceItemInObjectCaseSensitive(gotten_section, "displayname", displaynameobj);
		}
		cJSON *descriptionobj = cJSON_CreateString(description);
		if (descriptionobj) {
			cJSON_ReplaceItemInObjectCaseSensitive(gotten_section, "description", descriptionobj);
		}
		return (gotten_section);
	} else {
		jsonConfig *new_section = cJSON_AddObjectToObject(target_section, name);
		if (!new_section) {
			return (NULL);
		}
		if (cJSON_AddStringToObject(new_section, "name", name) == NULL)
		{
			cJSON_Delete(new_section);
			return (NULL);
		}
		if (cJSON_AddStringToObject(new_section, "displayname", displayname) == NULL)
		{
			cJSON_Delete(new_section);
			return (NULL);
		}
		if (cJSON_AddStringToObject(new_section, "description", description) == NULL)
		{
			cJSON_Delete(new_section);
			return (NULL);
		}
		return (new_section);
	}
}

static char *get_comment(const char *comment) {
	return (strjoin("Comment: ", comment));
}

int get_int_val(jsonConfig *cfg, char *name, char *comment, int default_val, int min_val, int max_val) {
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	if (!check_int(default_val, min_val, max_val)) {
		log_fatal("Default value for config element '%s' is out of allowd min-max range", name);
		shutdown(1);
	}

	finalcomment = get_comment(comment);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		newobj = cJSON_AddObjectToObject(cfg, name);
		if (!newobj) {
			goto end;
		}
		log_debug("adding info: '%s'\n", finalcomment);
		if (!cJSON_AddStringToObject(newobj, "info", finalcomment)) {
			goto end;
		}
		if (!cJSON_AddNumberToObject(newobj, "value", default_val)) {
			goto end;
		}

	} else { // in case we find it, we still update the comment
		infoobj = cJSON_CreateString(finalcomment);
		if (infoobj) {
			if (!cJSON_ReplaceItemInObjectCaseSensitive(oldobj, "info", infoobj)) {
				cJSON_Delete(infoobj);
				goto end;
			}
		}
		cJSON *valobj = cJSON_GetObjectItemCaseSensitive(oldobj, "value");
		if (!valobj || !cJSON_IsNumber(valobj)) {
			goto end;
		}
		free(finalcomment);
		return ((int) cJSON_GetNumberValue(valobj));
	}

	end:
	if (finalcomment) {
		free(finalcomment);
	}
	return (default_val);
}

float get_float_val(jsonConfig *cfg, char *name, char *comment, float default_val, float min_val, float max_val) {
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	if (!check_float(default_val, min_val, max_val)) {
		log_fatal("Default value for config element '%s' is out of allowd min-max range", name);
		shutdown(1);
	}

	finalcomment = get_comment(comment);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		newobj = cJSON_AddObjectToObject(cfg, name);
		if (!newobj) {
			goto end;
		}
		log_debug("adding info: '%s'\n", finalcomment);
		if (!cJSON_AddStringToObject(newobj, "info", finalcomment)) {
			goto end;
		}
		if (!cJSON_AddNumberToObject(newobj, "value", default_val)) {
			goto end;
		}
	} else { // in case we find it, we still update the comment
		infoobj = cJSON_CreateString(finalcomment);
		if (infoobj) {
			if (!cJSON_ReplaceItemInObjectCaseSensitive(oldobj, "info", infoobj)) {
				cJSON_Delete(infoobj);
				goto end;
			}
		}
		cJSON *valobj = cJSON_GetObjectItemCaseSensitive(oldobj, "value");
		if (!valobj || !cJSON_IsNumber(valobj)) {
			goto end;
		}
		free(finalcomment);
		return ((float) cJSON_GetNumberValue(valobj));
	}
	end:
	if (finalcomment) {
		free(finalcomment);
	}
	return (default_val);
}

int get_bool_val(jsonConfig *cfg, char *name, char *comment, int default_val) {
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	finalcomment = get_comment(comment);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		newobj = cJSON_AddObjectToObject(cfg, name);
		if (!newobj) {
			goto end;
		}
		log_debug("adding info: '%s'\n", finalcomment);
		if (!cJSON_AddStringToObject(newobj, "info", finalcomment)) {
			goto end;
		}
		if (!cJSON_AddBoolToObject(newobj, "value", default_val)) {
			goto end;
		}
	} else { // in case we find it, we still update the comment
		infoobj = cJSON_CreateString(finalcomment);
		if (infoobj) {
			if (!cJSON_ReplaceItemInObjectCaseSensitive(oldobj, "info", infoobj)) {
				cJSON_Delete(infoobj);
				goto end;
			}
		}
		cJSON *valobj = cJSON_GetObjectItemCaseSensitive(oldobj, "value");
		if (!valobj || !cJSON_IsNumber(valobj)) {
			goto end;
		}
		free(finalcomment);
		return ((int) cJSON_GetNumberValue(valobj));
	}
	end:
	if (finalcomment) {
		free(finalcomment);
	}
	return (default_val);
}

char *get_string_val(jsonConfig *cfg, char *name, char *comment, const char *default_val) {
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	finalcomment = get_comment(comment);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		newobj = cJSON_AddObjectToObject(cfg, name);
		if (!newobj) {
			goto end;
		}
		log_debug("adding info: '%s'\n", finalcomment);
		if (!cJSON_AddStringToObject(newobj, "info", finalcomment)) {
			goto end;
		}
		if (!cJSON_AddStringToObject(newobj, "value", default_val)) {
			goto end;
		}
	} else { // in case we find it, we still update the comment
		infoobj = cJSON_CreateString(finalcomment);
		if (infoobj) {
			if (!cJSON_ReplaceItemInObjectCaseSensitive(oldobj, "info", infoobj)) {
				cJSON_Delete(infoobj);
				goto end;
			}
		}
		cJSON *valobj = cJSON_GetObjectItemCaseSensitive(oldobj, "value");
		if (!valobj || !cJSON_IsString(valobj)) {
			goto end;
		}
		free(finalcomment);
		return (strdup(cJSON_GetStringValue(valobj)));
	}
	end:
	if (finalcomment) {
		free(finalcomment);
	}
	return (strdup(default_val));
}

vec_int get_int_vec(jsonConfig *cfg, char *name, char *comment, int *default_val, int min, int max) {
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;

	if (!check_int_vec(default_val, min, max)) {
		log_debug("hmmmm %s", name);
		log_fatal("Default value for config element '%s' is out of allowd min-max range", name, name);
		shutdown(1);
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		newobj = cJSON_AddObjectToObject(cfg, name);
		if (!newobj) {
			goto end;
		}
		cJSON_AddStringToObject(newobj, "comment", comment);
		cJSON *arr = cJSON_CreateIntArray(default_val, (int)vector_size(default_val));
		if (!arr) {
			goto end;
		}
		cJSON_AddItemToObject(newobj, "default_value", arr);
		cJSON_AddNumberToObject(newobj, "min", min);
		cJSON_AddNumberToObject(newobj, "max", max);
		cJSON *arr_ = cJSON_CreateIntArray(default_val, (int)vector_size(default_val));
		if (!arr_) {
			goto end;
		}
		cJSON_AddItemToObject(newobj, "value", arr_);
	} else { // in case we find it, we still update the comment, default_val, min, max
		/* updating comment */
		cJSON *commobj = cJSON_GetObjectItemCaseSensitive(oldobj, "comment");
		if (commobj && cJSON_IsString(commobj)) {
			cJSON_SetValuestring(commobj, comment);
		} else {
			cJSON_AddStringToObject(oldobj, "comment", comment);
		}

		/* updating default_value */
		cJSON *def_val_obj = cJSON_GetObjectItemCaseSensitive(oldobj, "default_value");
		if (def_val_obj && cJSON_IsArray(def_val_obj)) {
			cJSON *arr = cJSON_CreateIntArray(default_val, (int)vector_size(default_val));
			if (!arr) {
				goto end;
			}
			if (!cJSON_ReplaceItemInObjectCaseSensitive(oldobj, "default_value", arr)) {
				cJSON_Delete(arr);
			}
		} else {
			cJSON *arr = cJSON_CreateIntArray(default_val, (int)vector_size(default_val));
			if (!arr) {
				goto end;
			}
			if (!cJSON_AddItemToObject(oldobj, "default_value", arr)) {
				cJSON_Delete(arr);
			}
		}

		/* updating min */
		cJSON *minobj = cJSON_GetObjectItemCaseSensitive(oldobj, "min");
		if (minobj && cJSON_IsNumber(minobj)) {
			cJSON_SetNumberValue(minobj, min);
		} else {
			cJSON_AddNumberToObject(oldobj, "min", min);
		}

		/* updating max */
		cJSON *maxobj = cJSON_GetObjectItemCaseSensitive(oldobj, "max");
		if (maxobj && cJSON_IsNumber(maxobj)) {
			cJSON_SetNumberValue(maxobj, max);
		} else {
			cJSON_AddNumberToObject(oldobj, "max", max);
		}

		/* returning value */
		cJSON *valobj = cJSON_GetObjectItemCaseSensitive(oldobj, "value");
		if (!valobj || !cJSON_IsArray(valobj)) {
			goto end;
		}
		return (jsonTo_vec_int(valobj));
	}
	end:
	return (vector_copy(default_val));
}
