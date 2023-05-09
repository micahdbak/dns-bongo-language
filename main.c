#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ARGS  10
#define MAX_VARS  10
#define ARG_LEN   16

// 256 instructions by default
#define DEFAULT_PROG_SIZE  8

#define COMPILER_PREFIX  "bongo: "

const char *i_name[] = {
	"define",
	"set",
	"add",
	"subtract",
	"multiply",
	"return",
	"nil"
};

// Modified hashing algorithm from http://www.cse.yorku.ca/~oz/hash.html
unsigned long djb2_hash(const char *str, int size) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++) != '\0')
		hash = ((hash << 5) + hash) + c;

	return hash % size;
}

struct instruction {
	enum i_type {
		I_DEFINE,
		I_SET,
		I_ADD,
		I_SUB,
		I_MUL,
		I_RETURN,
		I_NONE
	} type;
	struct i_argument {
		char v[ARG_LEN];
	} arg[MAX_ARGS];
	int nargs;
};

enum i_type get_ins_type(char *arr) {
	for (int i = 0; i < I_NONE; ++i)
		if (strcmp(arr, i_name[i]) == 0)
			return i;

	return I_NONE;
}

int read_field(char *line, char *dst) {
	int i;

	for (i = 0; !isspace(*line) && *line != '\0'; ++line, ++dst, ++i)
		*dst = *line;

	*dst = '\0';

	return i;
}

struct instruction **bongo_compile(FILE *bongo_file, int *nins_ptr) {
	struct instruction **bongo_prog, *instruction;
	enum i_type ins_type;
	char line[256];
	int bongo_capacity,
	    c, i,
	    nargs,
	    nins;

	bongo_capacity = DEFAULT_PROG_SIZE;
	bongo_prog = (struct instruction **)malloc(sizeof(struct instruction *) * bongo_capacity);

	nins = 0;

	do {
		c = getc(bongo_file);

		for (i = 0; c != '\n' && c != EOF; c = getc(bongo_file))
			line[i++] = c;

		if (c == EOF)
			break;

		// empty line; skip
		if (i == 0)
			continue;

		line[i] = '\0';

		switch (line[0]) {
		case ':':
			// comment
			if (line[1] == ':')
				// skip the rest of this line
				continue;

			break;
		// instruction
		default:
			for (i = 0; line[i] != ':' && line[i] != '\0'; ++i)
				;

			// split line at instruction terminator
			line[i] = '\0';

			ins_type = get_ins_type(line);

			// invalid instruction
			if (ins_type == I_NONE) {
				printf(COMPILER_PREFIX "Invalid instruction \"%s\".\n", line);

				// skip line
				continue;
			}

			// valid instruction

			instruction = (struct instruction *)malloc(sizeof(struct instruction));

			if (instruction == NULL) {
				printf(COMPILER_PREFIX "malloc error.\n");

				// crash
				exit(1);
			}

			instruction->type = ins_type;
			nargs = 0;

			++i;

			while (isspace(line[i]) && line[i] != '\0')
				++i;

			i += read_field(line + i, instruction->arg[nargs++].v);

			while (line[i] != '\0') {
				while (isspace(line[i]) && line[i] != '\0')
					++i;

				i += read_field(line + i, instruction->arg[nargs++].v);
			}

			instruction->nargs = nargs;

			bongo_prog[nins++] = instruction;

			if (nins == bongo_capacity) {
				bongo_capacity *= 2;

				bongo_prog = (struct instruction **)realloc(bongo_prog,
				             sizeof(struct instruction *) * bongo_capacity);
			}

			break;
		}
	}
	while (c != EOF);

	*nins_ptr = nins;

	return bongo_prog;
}

int main(int argc, char **argv) {
	struct instruction **bongo_prog;
	int prog_capacity = DEFAULT_PROG_SIZE,
	    nins,
	    store[MAX_VARS],
	    var_id, res_id, a_id, b_id, val,
	    prog_end,
	    flag_i;
	FILE *bongo_file;

	if (argc == 1) {
		printf(COMPILER_PREFIX "No file provided. "
		       "Please use %s <file-name> to interpret a file.\n", argv[0]);

		return 1;
	}

	bongo_file = fopen(argv[1], "r");

	if (bongo_file == NULL) {
		printf(COMPILER_PREFIX "Could not open the file provided.\n");

		return 1;
	}

	// load instructions
	bongo_prog = bongo_compile(bongo_file, &nins);

	prog_end = 0;

	for (int i = 0; i < nins && !prog_end; ++i) {

		printf("%s: ", i_name[bongo_prog[i]->type]);

		switch (bongo_prog[i]->type) {
		case I_DEFINE:
			var_id = djb2_hash(bongo_prog[i]->arg[0].v, MAX_VARS);
			val = atoi(bongo_prog[i]->arg[1].v);

			store[var_id] = val;

			printf("%s (%d) to %d\n", bongo_prog[i]->arg[0].v, var_id, val);

			break;
		case I_SET:
			a_id = djb2_hash(bongo_prog[i]->arg[0].v, MAX_VARS);
			b_id = djb2_hash(bongo_prog[i]->arg[1].v, MAX_VARS);

			store[a_id] = store[b_id];

			printf("%s (%d) to %s (%d); %d\n",
			       bongo_prog[i]->arg[0].v, a_id,
			       bongo_prog[i]->arg[1].v, b_id,
			       store[b_id]);

			break;
		case I_RETURN:
			var_id = djb2_hash(bongo_prog[i]->arg[0].v, MAX_VARS);

			printf("%d\n", store[var_id]);

			prog_end = 1;

			break;
		case I_ADD:
		case I_SUB:
		case I_MUL:
			res_id = djb2_hash(bongo_prog[i]->arg[0].v, MAX_VARS);
			a_id = djb2_hash(bongo_prog[i]->arg[1].v, MAX_VARS);
			b_id = djb2_hash(bongo_prog[i]->arg[2].v, MAX_VARS);

			switch (bongo_prog[i]->type) {
			case I_ADD:
				store[res_id] = store[a_id] + store[b_id];

				break;
			case I_SUB:
				store[res_id] = store[a_id] - store[b_id];

				break;
			case I_MUL:
				store[res_id] = store[a_id] * store[b_id];

				break;
			default:
				break;
			}

			printf("%s (%d) to %s %s (%d), %s (%d); %s %d, %d = %d\n",
			       bongo_prog[i]->arg[0].v, res_id,
			       i_name[bongo_prog[i]->type],
			       bongo_prog[i]->arg[1].v, a_id,
			       bongo_prog[i]->arg[2].v, b_id,
			       i_name[bongo_prog[i]->type],
			       store[a_id], store[b_id], store[res_id]);

			break;
		default:
			break;
		}
	}

	return 0;
}
