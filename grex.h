#pragma once

#define GREX_OK 0
#define GREX_NO_MATCH 1
#define GREX_EOF -1

struct grex_parser;

/// @brief Error callback for handling parser errors
typedef void (*grex_error_callback_t)(struct grex_parser*, const char* msg, void* arg);

/// @brief The parser structure
typedef struct grex_parser {
  const char* input;
  unsigned input_length;
  unsigned parsing_offset;
  grex_error_callback_t error_callback;
  void* error_callback_arg;
} grex_parser_t;

/// @brief Initialize the parser with the input
/// @param p
/// @param input
/// @param length
void grex_parser_init(grex_parser_t* p, const char* input, unsigned length);

/// @brief Releases all resources used by the parser
/// @param p
void grex_parser_destroy(grex_parser_t* p);

/// @brief Sets an error callback for the parser
/// @param p
/// @param cb
/// @param arg
void grex_parser_set_error_callback(grex_parser_t* p, grex_error_callback_t cb, void* arg);

/// @brief Matches on whitespace, including line breaks and carriage returns
/// @param p
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_whitespace(grex_parser_t* p);

/// @brief Matches on whitespace, NOT including line breaks and carriage returns
/// @param p
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_whitespace_no_line(grex_parser_t* p);

/// @brief Matches on specific character
/// @param p
/// @param c
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_char(grex_parser_t* p, int c);

/// @brief Matches a set of characters
/// @param p
/// @param set
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_set(grex_parser_t* p, const char* set);

/// @brief Matches an inclusive range of characters
/// @param p
/// @param range
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_range(grex_parser_t* p, const char* range);

/// @brief Matches an integer number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_integer(grex_parser_t* p, long long *value);

/// @brief Matches an unsigned integer number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_uinteger(grex_parser_t* p, unsigned long long* value);

/// @brief Matches a floating-point number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_float(grex_parser_t* p, double* value);

/// @brief Matches a C-like identifier sequence of characters
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_identifier(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches either a single or double quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_string(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches a single-quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_single_quoted_string(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches a double-quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
int grex_double_quoted_string(grex_parser_t* p, char* buf, unsigned size);