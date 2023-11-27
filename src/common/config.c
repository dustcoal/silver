/* Home of the high lvl config API */

#include "common/config.h"

jsonConfig  *new_conf(char *path, char *name, char *displayname, char *description, void *(*populate_func)(jsonConfig *)) {
	jsonConfig *conf = cJSON_CreateObject();
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
		return (NULL);
	}
	char *path_ = strdup(path);
	if (!path_) {
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "path", path_) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	char *name_ = strdup(name);
	if (!name_) {
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "name", name_) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	char *displayname_ = strdup(displayname);
	if (!displayname_) {
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "displayname", displayname_) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	char *description_ = strdup(description);
	if (!description_) {
		cJSON_Delete(conf);
		return (NULL);
	}
	if (cJSON_AddStringToObject(conf, "description", description_) == NULL)
	{
		cJSON_Delete(conf);
		return (NULL);
	}
	return (conf);
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

configSection *add_section(configSection *target_section, char *name, char *displayname, char *description) {
	jsonConfig *new_section = cJSON_AddObjectToObject(target_section, name);
	if (!new_section) {
		return (NULL);
	}
	char *name_ = strdup(name);
	if (!name_) {
		cJSON_Delete(new_section);
		return (NULL);
	}
	if (cJSON_AddStringToObject(new_section, "name", name_) == NULL)
	{
		cJSON_Delete(new_section);
		return (NULL);
	}
	char *displayname_ = strdup(displayname);
	if (!displayname_) {
		cJSON_Delete(new_section);
		return (NULL);
	}
	if (cJSON_AddStringToObject(new_section, "displayname", displayname_) == NULL)
	{
		cJSON_Delete(new_section);
		return (NULL);
	}
	char *description_ = strdup(description);
	if (!description_) {
		cJSON_Delete(new_section);
		return (NULL);
	}
	if (cJSON_AddStringToObject(new_section, "description", description_) == NULL)
	{
		cJSON_Delete(new_section);
		return (NULL);
	}
	return (new_section);
}

static void get_comment_name(char buf[100], const char *prop_name) {
	static const char *prefix = "#comment [";
	static const int prefixlen = strlen("#comment [");
	memcpy(buf, prefix, prefixlen);
	unsigned long copy_len = ulmin(50, strlen(prop_name));
	memcpy(buf + prefixlen, prop_name, copy_len);
	buf[prefixlen + copy_len] = ']';
	buf[prefixlen + copy_len + 1] = 0;
	buf[99] = 0;
}

static int add_comment(jsonConfig *cfg, char *prop_name, char *comment) {
	char comment_name[100];
	get_comment_name(comment_name, prop_name);
	cJSON_DeleteItemFromObjectCaseSensitive(cfg, comment_name);
	if (!cJSON_AddStringToObject(cfg, comment_name, comment)) {
		return (0);
	}
	return (1);
}

int get_int_val(jsonConfig *cfg, char *name, char *comment, int default_val, int min_val, int max_val) {
	cJSON *nameval = cJSON_GetObjectItemCaseSensitive(cfg, "name");
	if (!nameval || !cJSON_IsString(nameval)) {
		log_warn("", 0);
		printf("Failed to get value '%s': couldn't find section name\n", name);
		if (nameval) {
			cJSON_Delete(nameval);
		}
		return (default_val);
	}
	cJSON *intval = cJSON_GetObjectItemCaseSensitive(cfg, name);
	if (!intval || !cJSON_IsNumber(intval)) {
		log_warn("", 0);
		printf("Failed to get value '%s' for config/section '%s'\n", name, nameval->valuestring);
		cJSON_Delete(nameval);
		if (intval) {
			cJSON_Delete(intval);
			cJSON_DeleteItemFromObjectCaseSensitive(cfg, name);
		}
		if (!add_comment(cfg, name, comment)) {
			return (default_val);
		}
		if (!cJSON_AddNumberToObject(cfg, name, default_val)) {
			return (default_val);
		}
		return (default_val);
	}
	cJSON_Delete(nameval);
	add_comment(cfg, name, comment); //updating comment in case it changed in the code
	if (intval->valueint < min_val || intval->valueint > max_val) {
		cJSON_AddNumberToObject(cfg, name, default_val);
		cJSON_Delete(intval);
		return (default_val);
	}
	int res = intval->valueint;
	cJSON_Delete(intval);
	return (res);
}


