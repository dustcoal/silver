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

static void get_comment_name(char buf[100], const char *prop_name) {
	static const char *prefix = "#comment [";
	static const int prefixlen = strlen("#comment [");
	memcpy(buf, prefix, prefixlen);
	unsigned long copy_len = ulmin(50, strlen(prop_name));
	memcpy(buf + prefixlen, prop_name, copy_len);
	buf[prefixlen + copy_len] = ']';
	buf[prefixlen + copy_len + 1] = 0;
	buf[999] = 0;
}

static int add_comment(jsonConfig *cfg, char *prop_name, char *comment) {
	char comment_name[1000];
	get_comment_name(comment_name, prop_name);
	cJSON_DeleteItemFromObjectCaseSensitive(cfg, comment_name);
	if (!cJSON_AddStringToObject(cfg, comment_name, comment)) {
		return (0);
	}
	return (1);
}

static char *get_final_comment_int(char *comment, int default_val, int min_val, int max_val) {
	char *res = NULL;
	char *dv = itoa(default_val, 10);
	char *miv = itoa(min_val, 10);
	char *mav = itoa(max_val, 10);

	if (!dv || !miv || !mav) {
		goto end;
	}

	vec_str tmp = vector_create();
	if (!tmp) {
		goto end;
	}
	if(!vector_add(&tmp, "[Default value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, dv)) {
		goto end;
	}
	if(!vector_add(&tmp, ", min value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, miv)) {
		goto end;
	}
	if(!vector_add(&tmp, ", max value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, mav)) {
		goto end;
	}
	if(!vector_add(&tmp, "] Comment: ")) {
		goto end;
	}
	if(!vector_add(&tmp, comment)) {
		goto end;
	}
	res = vec_str_join(tmp);

	end:
	if (tmp) {
		vector_free(tmp);
	}
	if (dv) {
		free(dv);
	}
	if (miv) {
		free(miv);
	}
	if (mav) {
		free(mav);
	}
	return (res);
}

static char *get_final_comment_float(char *comment, float default_val, float min_val, float max_val) {
	char *res = NULL;
	char *dv = ftoa(default_val, 5);
	char *miv = ftoa(min_val, 5);
	char *mav = ftoa(max_val, 5);

	if (!dv || !miv || !mav) {
		goto end;
	}

	vec_str tmp = vector_create();
	if (!tmp) {
		goto end;
	}
	if(!vector_add(&tmp, "[Default value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, dv)) {
		goto end;
	}
	if(!vector_add(&tmp, ", min value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, miv)) {
		goto end;
	}
	if(!vector_add(&tmp, ", max value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, mav)) {
		goto end;
	}
	if(!vector_add(&tmp, "] Comment: ")) {
		goto end;
	}
	if(!vector_add(&tmp, comment)) {
		goto end;
	}
	res = vec_str_join(tmp);

	end:
	if (tmp) {
		vector_free(tmp);
	}
	if (dv) {
		free(dv);
	}
	if (miv) {
		free(miv);
	}
	if (mav) {
		free(mav);
	}
	return (res);
}

static char *get_final_comment_bool(char *comment, int default_val) {
	char *res = NULL;

	vec_str tmp = vector_create();
	if (!tmp) {
		goto end;
	}
	if(!vector_add(&tmp, "[Default value: ")) {
		goto end;
	}
	if (default_val) {
		if(!vector_add(&tmp, "true")) {
			goto end;
		}
	} else {
		if(!vector_add(&tmp, "false")) {
			goto end;
		}
	}
	if(!vector_add(&tmp, "] Comment: ")) {
		goto end;
	}
	if(!vector_add(&tmp, comment)) {
		goto end;
	}
	res = vec_str_join(tmp);

	end:
	if (tmp) {
		vector_free(tmp);
	}
	return (res);
}

static char *get_final_comment_str(char *comment, const char *default_val) {
	char *res = NULL;

	vec_str tmp = vector_create();
	if (!tmp) {
		goto end;
	}
	if(!vector_add(&tmp, "[Default value: ")) {
		goto end;
	}
	if(!vector_add(&tmp, (char *)default_val)) {
		goto end;
	}
	if(!vector_add(&tmp, "] Comment: ")) {
		goto end;
	}
	if(!vector_add(&tmp, comment)) {
		goto end;
	}
	res = vec_str_join(tmp);

	end:
	if (tmp) {
		vector_free(tmp);
	}
	return (res);
}

static char *get_final_comment_intvec(char *comment, vec_int default_val) {
	char *res = NULL;
	vec_str tmp = NULL;
	vec_str tmp2 = NULL;
	char *tmp4 = NULL;

	tmp = vector_create();
	if (!tmp) {
		goto end;
	}
	if(!vector_add(&tmp, "[Default value: [")) {
		goto end;
	}

	tmp2 = vector_create();
	if (!tmp2) {
		goto end;
	}
	for (int i = 0; i < vector_size(default_val); ++i) {
		char *tmp3 = itoa(default_val[i], 10);
		if (!tmp3) {
			goto end;
		}
		vector_add(&tmp2, tmp3);
	}
	tmp4 = vec_str_join_sep(tmp2, ", ");
	if (!tmp4) {
		goto end;
	}
	if(!vector_add(&tmp, tmp4)) {
		goto end;
	}

	if(!vector_add(&tmp, "]] Comment: ")) {
		goto end;
	}
	if(!vector_add(&tmp, comment)) {
		goto end;
	}
	res = vec_str_join(tmp);

	end:
	if (tmp) {
		vector_free(tmp);
	}
	if (tmp2) {
		vector_free(tmp2);
	}
	if (tmp4) {
		free(tmp4);
	}
	return (res);
}

int get_int_val(jsonConfig *cfg, char *name, char *comment, int default_val, int min_val, int max_val) {
	cJSON *nameval = NULL;
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("Failed to get value '%s': couldn't find section name\n", name);
		goto end;
	}

	finalcomment = get_final_comment_int(comment, default_val, min_val, max_val);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		log_debug("1\n");
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
		log_debug("2\n");
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
	cJSON *nameval = NULL;
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("Failed to get value '%s': couldn't find section name\n", name);
		goto end;
	}

	finalcomment = get_final_comment_float(comment, default_val, min_val, max_val);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		log_debug("1\n");
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
		log_debug("2\n");
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
	cJSON *nameval = NULL;
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("Failed to get value '%s': couldn't find section name\n", name);
		goto end;
	}

	finalcomment = get_final_comment_bool(comment, default_val);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		log_debug("1\n");
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
		log_debug("2\n");
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
	cJSON *nameval = NULL;
	cJSON *newobj = NULL;
	cJSON *oldobj = NULL;
	cJSON *infoobj = NULL;
	char *finalcomment = NULL;

	nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("Failed to get value '%s': couldn't find section name\n", name);
		goto end;
	}

	finalcomment = get_final_comment_str(comment, default_val);
	if (!finalcomment) {
		goto end;
	}

	oldobj = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!oldobj) { // in case we don't find the key
		log_debug("1\n");
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
		log_debug("2\n");
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

vec_int get_int_vec(jsonConfig *cfg, char *name, char *comment, int *default_val) {
	/*
	int *res = vector_create();
	for (int i = 0; i < 10; ++i) {
		vector_add(&res, i);
	}
	return (res);
	*/


	cJSON *nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("Failed to get value '%s': couldn't find section name\n", name);
		if (nameval) {
			cJSON_Delete(nameval);
		}
		return (vector_copy(default_val));
	}
	cJSON *objval = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!objval || !cJSON_IsObject(objval)) {
		log_warn("Failed to get value '%s' for config/section '%s'\n", name, nameval->valuestring);
		cJSON_Delete(nameval);
		if (objval) {
			cJSON_Delete(objval);
			cJSON_DeleteItemFromObjectCaseSensitive(cfg, name);
		}
		if (!add_comment(cfg, name, comment)) {
			return (vector_copy(default_val));
		}
		/* add the vect to the obj */
		cJSON *new_array = cJSON_AddArrayToObject(cfg, name);
		if (new_array) {
			for (int i = 0; i < vector_size(default_val); ++i) {
				
			}
		}
		/*if (!cJSON_AddStringToObject(cfg, name, default_val)) {
			return (vector_copy(default_val));
		}*/



		return (vector_copy(default_val));
	}
	cJSON_Delete(nameval);
	add_comment(cfg, name, comment); //updating comment in case it changed in the code
	//char *res = strdup(objval->valuestring);
	cJSON_Delete(objval);
	return (NULL);
}


