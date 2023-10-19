#include "../grex.h"
#include "../grex.c"
#include <stdio.h>

static char input[1024];

static int read_file(const char* filename) {
  FILE* fh = fopen(filename, "rb");
  if (!fh) return 0;

  fread(input, 1, sizeof(input), fh);
  fclose(fh);
  return 1;
}

static void parse_properties(grex_parser_t* p) {
  static char key_buf[1024];
  static char value_buf[1024];
  static long long value_num;
  static double value_double;

  while (grex_whitespace(p) != GREX_EOF) {
    if (!grex_char(p, '#')) {
      // comments
      grex_until(p, '\n');
      grex_whitespace(p);
    }

    if (grex_identifier(p, key_buf, sizeof(key_buf))) break;
    if (grex_whitespace(p) == GREX_EOF) break;

    if (grex_char(p, '=')) break;
    if (grex_whitespace(p) == GREX_EOF) break;

    if (!grex_integer(p, 0, &value_num)) {
      printf("integer prop: %s = %lld\n", key_buf, value_num);
      continue;
    }

    if (!grex_float(p, &value_double)) {
      printf("float prop: %s = %f\n", key_buf, value_double);
      continue;
    }

    if (!grex_identifier(p, value_buf, sizeof(value_buf))) {
      printf("ident prop: %s = %s\n", key_buf, value_buf);
      continue;
    }

    if (!grex_string(p, value_buf, sizeof(value_buf))) {
      printf("string prop: %s = %s\n", key_buf, value_buf);
      continue;
    }

    // If we got here, it's an invalid property
    fprintf(stderr, "Invalid property: %s\n", key_buf);
    break;
  }
}

int main(int argc, const char* argv[]) {
  if (!read_file("test.ini")) {
    return 1;
  }

  grex_parser_t p = {0};
  grex_parser_init(&p, input, strlen(input));

  grex_whitespace(&p);
  if (grex_sequence(&p, "[first_section]")) return 1;
  printf("[first_section]\n");
  parse_properties(&p);

  if (grex_sequence(&p, "[second_section]")) return 1;
  printf("[second_section]\n");
  parse_properties(&p);

  static char capbuf[200];
  grex_parser_reset(&p);
  grex_capture_until(&p, ']', capbuf, sizeof(capbuf));
  printf("captured 1: %s\n", capbuf);

  grex_parser_reset(&p);
  grex_capture_until(&p, '\n', capbuf, sizeof(capbuf));
  printf("captured 2: %s\n", capbuf);
  grex_whitespace(&p);
  grex_capture_until(&p, '\n', capbuf, sizeof(capbuf));
  printf("captured 3: %s\n", capbuf);

  grex_parser_destroy(&p);

  return 0;
}