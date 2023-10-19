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

#pragma once

typedef enum grex_result {
  GREX_OK = 0,
  GREX_NO_MATCH = 1,
  GREX_RANGE_ERR = 2,
  GREX_EOF = -1,
} grex_result_t;

struct grex_parser;

/// @brief Error callback for handling parser errors
typedef void (*grex_error_callback_t)(struct grex_parser*, const char* msg, void* arg);

/// @brief The parser structure
typedef struct grex_parser {
  const char* input;
  const char* input_end;
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

/// @brief Reset the parser's reading head to the start of the input
/// @param p
void grex_parser_reset(grex_parser_t* p);

/// @brief Set the parser's reading head to the end of the input
/// @param p
void grex_parser_end(grex_parser_t* p);

/// @brief Sets an error callback for the parser
/// @param p
/// @param cb
/// @param arg
void grex_parser_set_error_callback(grex_parser_t* p, grex_error_callback_t cb, void* arg);

/// @brief Matches on whitespace, including line breaks and carriage returns
/// @param p
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_whitespace(grex_parser_t* p);

/// @brief Matches on whitespace, NOT including line breaks and carriage returns
/// @param p
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_whitespace_no_line(grex_parser_t* p);

/// @brief Matches on specific character
/// @param p
/// @param c
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_char(grex_parser_t* p, int c);

/// @brief Matches a set of characters
/// @param p
/// @param set
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_set(grex_parser_t* p, const char* set);

/// @brief Matches an inclusive range of characters
/// @param p
/// @param range
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_range(grex_parser_t* p, const char* range);

/// @brief Matches a sequence of characters.
/// When the function returns, the parsing head is at the start of the sequence.
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_sequence(grex_parser_t* p, const char* seq);

/// @brief Matches a sequence of characters, from the end to the beginning.
/// When the function returns, the parsing head is at the start of the sequence.
/// @param p
/// @param seq
/// @return
grex_result_t grex_sequence_reverse(grex_parser_t* p, const char* seq);

/// @brief Keep advancing while the current character is equal
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_while(grex_parser_t* p, unsigned c);

/// @brief Keep advancing while the current character is not equal
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_until(grex_parser_t* p, unsigned c);

/// @brief Keep advancing while the sequence of characters matches
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_while_sequence(grex_parser_t* p, const char* seq);

/// @brief Keep receding while the sequence of characters matches
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_while_sequence_reverse(grex_parser_t* p, const char* seq);

/// @brief Keep advancing while the sequence of characters does not match
/// When the function returns, the parsing head is at the end of the sequence.
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_until_sequence(grex_parser_t* p, const char* seq);

/// @brief Keep receding while the sequence of characters does not match
/// When the function returns, the parsing head is at the start of the sequence.
/// @param p
/// @param seq
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_until_sequence_reverse(grex_parser_t* p, const char* seq);

/// @brief Matches an integer number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_integer(grex_parser_t* p, int base, long long *value);

/// @brief Matches an unsigned integer number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_uinteger(grex_parser_t* p, int base, unsigned long long* value);

/// @brief Matches a floating-point number
/// @param p
/// @param value
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_float(grex_parser_t* p, double* value);

/// @brief Matches a C-like identifier sequence of characters
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_identifier(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches either a single or double quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_string(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches a single-quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_single_quoted_string(grex_parser_t* p, char* buf, unsigned size);

/// @brief Matches a double-quoted string
/// @param p
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_double_quoted_string(grex_parser_t* p, char* buf, unsigned size);

/// @brief Capture all characters until a character matches
/// @param p
/// @param c
/// @param buf
/// @param size
/// @return GREX_OK on success, GREX_NO_MATCH on invalid input, GREX_EOF on eof
grex_result_t grex_capture_until(grex_parser_t* p, int c, char* buf, unsigned size);