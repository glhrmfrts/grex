/**
MIT License

Copyright (c) 2023 Guilherme Freitas Nemeth

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "grex.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

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
  p->input_end = input + length;
  p->input_length = length;
  p->parsing_offset = 0;
}

void grex_parser_destroy(grex_parser_t* p) {
}

void grex_parser_set_error_callback(grex_parser_t* p, grex_error_callback_t cb, void* arg) {
  p->error_callback = cb;
  p->error_callback_arg = arg;
}

grex_result_t grex_whitespace(grex_parser_t* p) {
  return grex_set(p, " \t\n\r\f");
}

grex_result_t grex_whitespace_no_line(grex_parser_t* p) {
  return grex_set(p, " \t\f");
}

grex_result_t grex_char(grex_parser_t* p, int c) {
  GREX_EOF_CHECK(p);
  if (cur(p) == c) {
    next(p);
    return GREX_OK;
  }
  report_error(p, "grex_char");
  return GREX_NO_MATCH;
}

grex_result_t grex_set(grex_parser_t* p, const char* set) {
  GREX_EOF_CHECK(p);

  int n = 0;

  while (strchr(set, cur(p))) {
    NEXT(p);
    n++;
  }

  if (n) {
    return GREX_OK;
  }

  report_error(p, "grex_set");
  return GREX_NO_MATCH;
}

grex_result_t grex_range(grex_parser_t* p, const char* range) {
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

grex_result_t grex_sequence(grex_parser_t* p, const char* seq) {
  GREX_EOF_CHECK(p);

  unsigned n = strlen(seq);
  if (p->parsing_offset+n > p->input_length) {
    report_error(p, "grex_sequence");
    return GREX_NO_MATCH;
  }

  if (memcmp(&p->input[p->parsing_offset], seq, n)) {
    report_error(p, "grex_sequence");
    return GREX_NO_MATCH;
  }

  p->parsing_offset += n;
  return GREX_OK;
}

grex_result_t grex_while(grex_parser_t* p, unsigned c) {
  GREX_EOF_CHECK(p);

  int n = 0;

  while (cur(p) == c) {
    NEXT(p);
    n++;
  }

  if (n) {
    return GREX_OK;
  }

  report_error(p, "grex_while");
  return GREX_NO_MATCH;
}

grex_result_t grex_until(grex_parser_t* p, unsigned c) {
  GREX_EOF_CHECK(p);

  int n = 0;

  while (cur(p) != c) {
    NEXT(p);
    n++;
  }

  if (n) {
    return GREX_OK;
  }

  report_error(p, "grex_until");
  return GREX_NO_MATCH;
}

grex_result_t grex_while_sequence(grex_parser_t* p, const char* seq) {
  int result = grex_sequence(p, seq);

  while (result == GREX_OK) {
    result = grex_sequence(p, seq);
  }

  return result;
}

grex_result_t grex_until_sequence(grex_parser_t* p, const char* seq) {
  int result = grex_sequence(p, seq);

  while (result == GREX_NO_MATCH) {
    NEXT(p);
    result = grex_sequence(p, seq);
  }

  return result;
}

grex_result_t grex_integer(grex_parser_t* p, int base, long long *value) {
  GREX_EOF_CHECK(p);

  const char* str = &p->input[p->parsing_offset];
  char* endptr = NULL;
  unsigned long long res = strtoll(str, &endptr, base);

  if (endptr > str) {

    if (endptr < p->input_end && *endptr == '.') {
      // Possibly decimal number
      if (endptr+1 < p->input_end && isdigit(endptr[1])) {
        return GREX_NO_MATCH;
      }
    }

    *value = res;
    p->parsing_offset = endptr - p->input;
    if (errno == ERANGE) {
      return GREX_RANGE_ERR;
    }
    return GREX_OK;
  }

  report_error(p, "grex_uinteger");
  return GREX_NO_MATCH;
}

grex_result_t grex_uinteger(grex_parser_t* p, int base, unsigned long long* value) {
  GREX_EOF_CHECK(p);

  const char* str = &p->input[p->parsing_offset];
  char* endptr = NULL;
  unsigned long long res = strtoull(str, &endptr, base);

  if (endptr > str) {

    if (endptr < p->input_end && *endptr == '.') {
      // Possibly decimal number
      if (endptr+1 < p->input_end && isdigit(endptr[1])) {
        return GREX_NO_MATCH;
      }
    }

    *value = res;
    p->parsing_offset = endptr - p->input;
    if (errno == ERANGE) {
      return GREX_RANGE_ERR;
    }
    return GREX_OK;
  }

  report_error(p, "grex_uinteger");
  return GREX_NO_MATCH;
}

grex_result_t grex_float(grex_parser_t* p, double* value) {
  GREX_EOF_CHECK(p);

  const char* str = &p->input[p->parsing_offset];
  char* endptr = NULL;
  double res = strtod(str, &endptr);

  if (endptr > str) {
    *value = res;
    p->parsing_offset = endptr - p->input;
    return GREX_OK;
  }

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

grex_result_t grex_identifier(grex_parser_t* p, char* buf, unsigned size) {
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
    memcpy(buf, begin, n);
  }

  return GREX_OK;
}

grex_result_t grex_string(grex_parser_t* p, char* buf, unsigned size) {
  int result = grex_double_quoted_string(p, buf, size);
  if (result == GREX_OK) {
    return result;
  }

  result = grex_single_quoted_string(p, buf, size);
  return result;
}

static grex_result_t parse_string(grex_parser_t* p, int delim, char* buf, unsigned size) {
  GREX_EOF_CHECK(p);

  if (cur(p) != delim) {
    return GREX_NO_MATCH;
  }

  NEXT(p);

  const char *begin = &p->input[p->parsing_offset];
  int n = 0;

  while (cur(p) != delim) {
    if (cur(p) == '\\') {
      NEXT(p);
      n++;
      if (cur(p) == delim) {
        NEXT(p);
        n++;
      }
    }
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

    int di = 0;
    for (int i = 0; i < n; i++) {
      if (begin[i] == '\\') {
        if (i+1 < n && begin[i + 1] == delim) {
          buf[di++] = delim;
          i++;
        }
        else {
          buf[di++] = '\\';
        }
      }
      else {
        buf[di++] = begin[i];
      }
    }
  }

  return GREX_OK;
}

grex_result_t grex_delimited_string(grex_parser_t* p, int delim, char* buf, unsigned size) {
  unsigned prev_offset = p->parsing_offset;

  int result = parse_string(p, delim, buf, size);

  if (result != GREX_OK) {
    p->parsing_offset = prev_offset;
  }

  return result;
}

grex_result_t grex_single_quoted_string(grex_parser_t* p, char* buf, unsigned size) {
  return grex_delimited_string(p, '\'', buf, size);
}

grex_result_t grex_double_quoted_string(grex_parser_t* p, char* buf, unsigned size) {
  return grex_delimited_string(p, '"', buf, size);
}