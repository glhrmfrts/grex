#include "grex.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define GREX_EOF_CHECK(p) do { \
  if (p->parsing_offset >= p->input_length) { \
    return GREX_EOF; \
  } \
} while (0)

#define NEXT(p) if (!next(p)) { return GREX_EOF; }

static inline int cur(grex_parser_t* p) {
  return p->input[p->parsing_offset];
}

static inline int next(grex_parser_t* p) {
  if (p->parsing_offset+1 >= p->input_length) {
    return 0;
  }
  return p->input[++p->parsing_offset];
}

static int includes(const char* set, int c) {
  unsigned len = strlen(set);
  for (unsigned i = 0; i < len; i++) {
    if (c == set[i]) { return 1; }
  }
  return 0;
}

static void report_error(grex_parser_t* p, const char* where) {
  if (p->error_callback) {
    p->error_callback(p, where, p->error_callback_arg);
  }
  else {
    //fprintf(stderr, "grex error: %s - %s\n", where, &p->input[p->parsing_offset]);
  }
}

void grex_parser_init(grex_parser_t* p, const char* input, unsigned length) {
  p->input = input;
  p->input_length = length;
  p->parsing_offset = 0;
}

void grex_parser_destroy(grex_parser_t* p) {
}

void grex_parser_set_error_callback(grex_parser_t* p, grex_error_callback_t cb, void* arg) {
  p->error_callback = cb;
  p->error_callback_arg = arg;
}

int grex_whitespace(grex_parser_t* p) {
  return grex_set(p, " \t\n\r");
}

int grex_whitespace_no_line(grex_parser_t* p) {
  return grex_set(p, " \t");
}

int grex_char(grex_parser_t* p, int c) {
  GREX_EOF_CHECK(p);
  if (cur(p) == c) {
    next(p);
    return GREX_OK;
  }
  report_error(p, "grex_char");
  return GREX_NO_MATCH;
}

int grex_set(grex_parser_t* p, const char* set) {
  GREX_EOF_CHECK(p);
  int n = 0;
  while (includes(set, cur(p))) {
    NEXT(p);
    n++;
  }
  if (n) {
    return GREX_OK;
  }

  report_error(p, "grex_set");
  return GREX_NO_MATCH;
}

int grex_range(grex_parser_t* p, const char* range) {
  // TODO: handle multiple ranges
  if (strlen(range) < 2) return GREX_NO_MATCH;

  GREX_EOF_CHECK(p);
  int c = cur(p);
  if (c >= range[0] && c <= range[1]) {
    next(p);
    return GREX_OK;
  }

  report_error(p, "grex_range");
  return GREX_NO_MATCH;
}

int grex_integer(grex_parser_t* p, long long *value) {
  GREX_EOF_CHECK(p);

  unsigned prev_offset = p->parsing_offset;
  char* endptr = NULL;
  long long res = strtoll(&p->input[p->parsing_offset], &endptr, 10);

  if (endptr > &p->input[p->parsing_offset]) {
    *value = res;
    p->parsing_offset = endptr - p->input;
    return GREX_OK;
  }

  p->parsing_offset = prev_offset;
  report_error(p, "grex_integer");
  return GREX_NO_MATCH;
}

int grex_uinteger(grex_parser_t* p, unsigned long long* value) {
  GREX_EOF_CHECK(p);

  unsigned prev_offset = p->parsing_offset;
  char* endptr = NULL;
  unsigned long long res = strtoull(&p->input[p->parsing_offset], &endptr, 10);

  if (endptr > &p->input[p->parsing_offset]) {
    *value = res;
    p->parsing_offset = endptr - p->input;
    return GREX_OK;
  }

  p->parsing_offset = prev_offset;
  report_error(p, "grex_uinteger");
  return GREX_NO_MATCH;
}

int grex_float(grex_parser_t* p, double* value) {
  GREX_EOF_CHECK(p);

  unsigned prev_offset = p->parsing_offset;
  char* endptr = NULL;
  double res = strtod(&p->input[p->parsing_offset], &endptr);

  if (endptr > &p->input[p->parsing_offset]) {
    *value = res;
    p->parsing_offset = endptr - p->input;
    return GREX_OK;
  }

  p->parsing_offset = prev_offset;
  report_error(p, "grex_float");
  return GREX_NO_MATCH;
}

static int identifier_starter(grex_parser_t* p) {
  int c = cur(p);
  return isalpha(c) || (c == '_');
}

static int identifier_component(grex_parser_t* p) {
  int c = cur(p);
  return isalnum(c) || (c == '_');
}

int grex_identifier(grex_parser_t* p, char* buf, unsigned size) {
  GREX_EOF_CHECK(p);

  const char *begin = &p->input[p->parsing_offset];
  int n = 0;

  if (!identifier_starter(p)) {
    return GREX_NO_MATCH;
  }

  NEXT(p);
  n++;

  while (identifier_component(p)) {
    NEXT(p);
    n++;
  }

  if (n >= size) {
    n = size - 1;
  }
  if (n > 0) {
    memset(buf, 0, size);
    strncpy(buf, begin, n);
  }

  return GREX_OK;
}

int grex_string(grex_parser_t* p, char* buf, unsigned size) {
  int result = grex_double_quoted_string(p, buf, size);
  if (result == GREX_OK) {
    return result;
  }

  result = grex_single_quoted_string(p, buf, size);
  return result;
}

static int parse_string(grex_parser_t* p, int delim, char* buf, unsigned size) {
  GREX_EOF_CHECK(p);

  if (cur(p) != delim) {
    return GREX_NO_MATCH;
  }

  NEXT(p);

  const char *begin = &p->input[p->parsing_offset];
  int n = 0;

  // TODO: escaped strings

  while (cur(p) != delim) {
    NEXT(p);
    n++;
  }

  // consume end delimieter
  NEXT(p);

  if (n >= size) {
    n = size - 1;
  }
  if (n > 0) {
    memset(buf, 0, size);
    strncpy(buf, begin, n);
  }

  return GREX_OK;
}

int grex_delimited_string(grex_parser_t* p, int delim, char* buf, unsigned size) {
  unsigned prev_offset = p->parsing_offset;

  int result = parse_string(p, delim, buf, size);

  if (result != GREX_OK) {
    p->parsing_offset = prev_offset;
  }

  return result;
}

int grex_single_quoted_string(grex_parser_t* p, char* buf, unsigned size) {
  return grex_delimited_string(p, '\'', buf, size);
}

int grex_double_quoted_string(grex_parser_t* p, char* buf, unsigned size) {
  return grex_delimited_string(p, '"', buf, size);
}