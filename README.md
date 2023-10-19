# grex

Minimal general-purpose parsing library for C.

## Usage

Simply include/copy the **grex.h** and **grex.c** file into your project.

## Examples

Parsing a Unix-like config file (key-value pairs):

```c
grex_parser_t p = {0};
grex_parser_init(&p, input, strlen(input));

static char key_buf[1024];
static char value_buf[1024];
static long long value_num;
static double value_double;

while (grex_whitespace(&p) != GREX_EOF) {
  if (grex_identifier(&p, key_buf, sizeof(key_buf))) break;
  if (grex_whitespace(&p) == GREX_EOF) break;

  if (grex_char(&p, '=')) break;
  if (grex_whitespace(&p) == GREX_EOF) break;

  if (!grex_float(&p, &value_double)) {
    printf("float prop: %s = %f\n", key_buf, value_double);
    continue;
  }

  if (!grex_integer(&p, &value_num)) {
    printf("integer prop: %s = %lld\n", key_buf, value_num);
    continue;
  }

  if (!grex_identifier(&p, value_buf, sizeof(value_buf))) {
    printf("ident prop: %s = %s\n", key_buf, value_buf);
    continue;
  }

  if (!grex_string(&p, value_buf, sizeof(value_buf))) {
    printf("string prop: %s = %s\n", key_buf, value_buf);
    continue;
  }

  // If we got here, it's an invalid property
  fprintf(stderr, "Invalid property: %s\n", key_buf);
  break;
}

grex_parser_destroy(&p);
```

## LICENSE

MIT