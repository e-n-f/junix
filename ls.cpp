#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string>
#include <vector>
#include "jsonpull/jsonpull.h"

char **av;

void usage(struct option *long_options) {
	fprintf(stderr, "Usage:\n");
	exit(EXIT_FAILURE);
}

struct flag {
	std::string option;
	bool has_arg;
	std::string arg;
};

void read_flags(std::vector<flag> &flags, const char *fname, bool direct) {
	json_pull *jp;
	json_object *obj;
	FILE *fp = NULL;

	if (direct) {
		jp = json_begin_string(fname);
	} else {
		fp = fopen(fname, "r");
		if (fp == NULL) {
			perror(fname);
			exit(EXIT_FAILURE);
		}

		jp = json_begin_file(fp);
	}

	obj = json_read_tree(jp);
	if (obj == NULL) {
		fprintf(stderr, "%s: %s\n", fname, jp->error);
		exit(EXIT_FAILURE);
	}
	json_disconnect(obj);
	json_end(jp);

	if (fp != NULL) {
		fclose(fp);
	}

	if (obj->type != JSON_HASH) {
		fprintf(stderr, "%s: %s: contents are not a JSON object\n", *av, fname);
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < obj->length; i++) {
		if (obj->keys[i]->type != JSON_STRING) {
			fprintf(stderr, "%s: %s: %s: option is not a string\n", *av, fname, json_stringify(obj->keys[i]));
			exit(EXIT_FAILURE);
		}

		if (obj->values[i]->type != JSON_STRING && obj->values[i]->type != JSON_NUMBER && obj->values[i]->type != JSON_NULL && obj->values[i]->type != JSON_TRUE) {
			fprintf(stderr, "%s: %s: %s: option argument is not a string, number, or null\n", *av, fname, json_stringify(obj->values[i]));
			exit(EXIT_FAILURE);
		}

		flag f;
		f.option = obj->keys[i]->string;

		if (obj->values[i]->type == JSON_STRING || obj->values[i]->type == JSON_NUMBER) {
			f.has_arg = true;
			f.arg = obj->values[i]->string;
		} else {
			f.has_arg = false;
		}

		flags.push_back(f);
	}

	json_free(obj);
}

int main(int argc, char **argv) {
	av = argv;

	struct option long_options[] = {
		{"long", no_argument, 0, 'l'},
		{"recursive", no_argument, 0, 'r'},
		{"json-options", required_argument, 0, 'j'},
		{NULL, 0, 0, '\0'},
	};

	std::string getopt_str;
	for (size_t lo = 0; long_options[lo].name != NULL; lo++) {
		if (long_options[lo].val > ' ') {
			getopt_str.push_back(long_options[lo].val);

			if (long_options[lo].has_arg == required_argument) {
				getopt_str.push_back(':');
			}
		}
	}

	std::vector<flag> flags;
	std::vector<const char *> to_free;

	bool want_long = false;
	bool recursive = false;

	while (true) {
		int i;

		if (flags.size() > 0) {
			flag f = flags[0];
			flags.erase(flags.begin());

			struct option *found = NULL;
			for (size_t j = 0; long_options[j].name != NULL; j++) {
				if (f.option == long_options[j].name) {
					found = &long_options[j];
					break;
				}

				if (f.option.size() == 1 && long_options[j].val == f.option[0]) {
					found = &long_options[j];
					break;
				}
			}

			if (found == NULL) {
				i = '?';
				fprintf(stderr, "%s: %s: No such option\n", *av, f.option.c_str());
			} else {
				if (found->flag != NULL) {
					i = 0;
					*(found->flag) = found->val;
				} else {
					i = found->val;

					if (found->has_arg == required_argument && !f.has_arg) {
						fprintf(stderr, "%s: %s requires an argument but one was not specified\n", *av, found->name);
						exit(EXIT_FAILURE);
					}

					if (found->has_arg == no_argument && f.has_arg) {
						fprintf(stderr, "%s: %s does not require an argument but one was specified\n", *av, found->name);
						exit(EXIT_FAILURE);
					}

					optarg = strdup(f.arg.c_str());
					to_free.push_back(optarg);
				}
			}
		} else {
			i = getopt_long(argc, argv, getopt_str.c_str(), long_options, NULL);
		}

		if (i == -1) {
			break;
		}

		switch (i) {
		case 'l':
			want_long = true;
			break;

		case 'r':
			recursive = true;
			break;

		case 'j':
			read_flags(flags, optarg, true);
			break;

		default:
			usage(long_options);
			break;
		}
	}
}
