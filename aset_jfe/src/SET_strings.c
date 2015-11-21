/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
/* Implementation of SET_strings.h. */

#include <stdio.h>
#include <string.h>
typedef void an_il_to_str_output_control_block;
typedef int a_boolean;
#include "SET_strings.h"
#include "SET_jfe_msg.h"

#define TRUE 1
#define FALSE 0

/* The following variable is the file to which the text in output_buf
 * is to be sent.
 */

static FILE* IF_file;

/* The following function initializes a string buffer by allocating
 * the buffer storage; if the existing buffer has a partial string in
 * it, the partial string is copied to the new buffer storage.  The
 * current string pointer is adjusted to point to the newly-allocated
 * buffer.  This routine can thus be used either to set up a string
 * buffer for the first time or to recover from overflow.  If there
 * is already a buffer and the current string begins at that location,
 * the buffer is extended before being replaced.
 */

static void init_string_buffer(string_buffer_ptr bp) {
    char* new_buf;
    a_boolean free_old_buf = FALSE;
    size_t cur_str_len = bp->next_free_byte - bp->cur_str;
    if (!bp->buf) {
	bp->buf_size = STRING_BUFFER_SIZE;
    }
    else if (bp->cur_str == bp->buf) {
	/* This is the case where a single string won't fit in the
	 * buffer; we need to allocate a bigger one, copy the entire
	 * old contents (the single string under construction), and
	 * free the old one.  (Normally we leave the old one around
	 * since there will be pointers into it, but there are no
	 * pointers to the buffer contents created until the current
	 * string is terminated, which hasn't happened yet.
	 */
	bp->buf_size *= 2;
	free_old_buf = TRUE;
    }
    else if (cur_str_len < STRING_BUFFER_SIZE / 2) {
	/* No need to use a large buffer size for every buffer just
	 * because one string needed it.
	 */
	bp->buf_size = STRING_BUFFER_SIZE;
    }
    new_buf = (char*) malloc(bp->buf_size);
    if (cur_str_len) {
	memcpy(new_buf, bp->cur_str, cur_str_len);
    }
    if (free_old_buf) {
	free(bp->buf);
    }
    bp->buf = new_buf;
    bp->cur_str = new_buf;
    bp->next_free_byte = new_buf + cur_str_len;
    bp->last_byte_of_buf = bp->buf + bp->buf_size - 1;
}

/* The following declarations implement the string buffer for doing
 * buffered output; overflow handling consists of dumping the contents
 * to the IF file (including any partial contents of the cur_str) and
 * resetting the cur_str pointer to the beginning of the buffer.  The
 * end-of-string character is a newline.
 */

static void handle_output_overflow(string_buffer_ptr bp) {
    if (write(fileno(IF_file), bp->buf, bp->next_free_byte - bp->buf) == -1) {
	complain(catastrophe_csev, "Failure writing to IF file.");
        term_compilation(catastrophe_csev);
    }
    bp->cur_str = bp->buf;
    bp->next_free_byte = bp->buf;
}

string_buffer output_buf = { NULL, 0, NULL, NULL, NULL,
			     &handle_output_overflow,
			     NULL, '\n' };

/* The following declarations implement a string buffer for trial
 * strings; it is just like the output_buf except that the termination
 * character is a NUL instead of a newline and the overflow handling
 * just wraps around to the beginning of the buffer again.  It is used
 * to construct a string that might or might not be output or saved
 * (e.g., an ATR line when it is not known if any attributes will be
 * added).
 */

static void handle_trial_overflow(string_buffer_ptr bp) {
    if (!bp || bp->cur_str == bp->buf) {
	init_string_buffer(bp);
    }
    else {
	size_t cur_str_len = bp->next_free_byte - bp->cur_str;
	if (cur_str_len) {
	    memcpy(bp->buf, bp->cur_str, cur_str_len);
	}
	bp->cur_str = bp->buf;
	bp->next_free_byte = bp->buf + cur_str_len;
    }
}

string_buffer trial_buf = { NULL, 0, NULL, NULL, NULL,
			    &handle_trial_overflow,
			    NULL, '\0' };

/* The following declarations implement the string buffer for doing
 * string heap management; overflow handling consists of allocating a
 * new buffer and copying the string currently under construction to
 * the beginning of the new buffer.  The end-of-string character is a
 * NUL.
 */

static void handle_string_overflow(string_buffer_ptr bp) {
    init_string_buffer(bp);
}

string_buffer string_buf = { NULL, 0, NULL, NULL, NULL,
			     &handle_string_overflow,
			     NULL, '\0' };

/* The following function copies the input argument to the specified
 * string buffer, handling overflow as necessary.
 */

void add_to_string_with_len(const char* text, size_t len,
			    string_buffer_ptr bp) {
    register char* new_next_free_byte = bp->next_free_byte + len;
    if (new_next_free_byte > bp->last_byte_of_buf) {
	(*bp->handle_overflow)(bp);
	while (len > bp->buf_size) {
	    init_string_buffer(bp);
	}
	new_next_free_byte = bp->next_free_byte + len;
    }
    memcpy(bp->next_free_byte, text, len);
    bp->next_free_byte = new_next_free_byte;
}

/* The following functions are fixed-length versions of the preceding
 * (it's more efficient to copy a known number of characters than to
 * call memcpy, at least for a small number of characters, and many
 * of the calls are for 1, 2, or 3 characters).
 */

void add_1_char_to_string(char ch, string_buffer_ptr bp) {
    register char* free_byte = bp->next_free_byte;
    if (free_byte + 1 > bp->last_byte_of_buf) {
	(*bp->handle_overflow)(bp);
	free_byte = bp->next_free_byte;
    }
    free_byte[0] = ch;
    bp->next_free_byte = free_byte + 1;
}

void add_2_chars_to_string(const char* str, string_buffer_ptr bp) {
    register char* free_byte = bp->next_free_byte;
    if (free_byte + 2 > bp->last_byte_of_buf) {
	(*bp->handle_overflow)(bp);
	free_byte = bp->next_free_byte;
    }
    free_byte[0] = str[0];
    free_byte[1] = str[1];
    bp->next_free_byte = free_byte + 2;
}

void add_3_chars_to_string(const char* str, string_buffer_ptr bp) {
    register char* free_byte = bp->next_free_byte;
    if (free_byte + 3 > bp->last_byte_of_buf) {
	(*bp->handle_overflow)(bp);
	free_byte = bp->next_free_byte;
    }
    free_byte[0] = str[0];
    free_byte[1] = str[1];
    free_byte[2] = str[2];
    bp->next_free_byte = free_byte + 3;
}

/* The following function adds a NUL-terminated string to the
 * specified buffer.
 */

void add_to_string(const char* text, string_buffer_ptr bp) {
    add_to_string_with_len(text, strlen(text), bp);
}

/* The following function adds a number to the string (with optional
 * bracketing strings).  Because metering showed sprintf to be the single
 * largest time sink in IF dumping, an alternative method is provided here.
 * It assumes that unsigned longs are at least 32 bits long and that no value
 * greater than 0xffffffff will ever be printed (if this is run on
 * an architecture where unsigned long is >32 bits).  If these
 * assumptions are not correct, use of sprintf can be restored by
 * defining the macro USE_SPRINTF_FOR_NUMS.  (The assumption is also made
 * that the total length of the bracketing strings and the converted
 * number will be <= 32.)
 */

void add_num_to_string(unsigned long num, const char* pre_str,
			      const char* post_str, string_buffer_ptr bp) {
#ifdef USE_SPRINTF_FOR_NUMS
    char buff[32];
    size_t len;
#ifdef __BSD__
    sprintf(buff, "%s%lu%s", (pre_str) ? pre_str : "", num,
	    (post_str) ? post_str : "");
    len = strlen(buff);
#else /* !defined(__BSD__) */
    len = sprintf(buff, "%s%lu%s", (pre_str) ? pre_str : "", num,
		  (post_str) ? post_str : "");
#endif /* __BSD__ */
    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
  register char* buf = bp->next_free_byte;
  if (buf + 32 > bp->last_byte_of_buf) {
    (*bp->handle_overflow)(bp);
    buf = bp->next_free_byte;
  }
  if (pre_str) {
    while (*pre_str) {
      *buf++ = *pre_str++;
    }
  }
  if (num >= 10UL) {
    if (num >= 100UL) {
      if (num >= 1000UL) {
	if (num >= 10000UL) {
	  if (num >= 100000UL) {
	    if (num >= 1000000UL) {
	      if (num >= 10000000UL) {
		if (num >= 100000000UL) {
		  if (num >= 1000000000UL) {
		    if (num >= 3000000000UL) {
		      if (num >= 4000000000UL) {
			num -= 4000000000UL;
			*buf++ = '4';
		      }
		      else {
			num -= 3000000000UL;
			*buf++ = '3';
		      }
		    }
		    else if (num >= 2000000000UL) {
		      num -= 2000000000UL;
		      *buf++ = '2';
		    }
		    else {
		      num -= 1000000000UL;
		      *buf++ = '1';
		    }
		  }
		  if (num >= 500000000UL) {
		    if (num >= 800000000UL) {
		      if (num >= 900000000UL) {
			num -= 900000000UL;
			*buf++ = '9';
		      }
		      else {
			num -= 800000000UL;
		        *buf++ = '8';
		      }
		    }
		    else if (num >= 600000000UL) {
		      if (num >= 700000000UL) {
			num -= 700000000UL;
			*buf++ = '7';
		      }
		      else {
			num -= 600000000UL;
			*buf++ = '6';
		      }
		    }
		    else {
		      num -= 500000000UL;
		      *buf++ = '5';
		    }
		  }
		  else if (num >= 200000000UL) {
		    if (num >= 400000000UL) {
		      num -= 400000000UL;
		      *buf++ = '4';
		    }
		    else if (num >= 300000000UL) {
		      num -= 300000000UL;
		      *buf++ = '3';
		    }
		    else {
		      num -= 200000000UL;
		      *buf++ = '2';
		    }
		  }
		  else if (num >= 100000000UL) {
		    num -= 100000000UL;
		    *buf++ = '1';
		  }
		  else *buf++ = '0';
		}
		if (num >= 50000000UL) {
		  if (num >= 80000000UL) {
		    if (num >= 90000000UL) {
		      num -= 90000000UL;
		      *buf++ = '9';
		    }
		    else {
		      num -= 80000000UL;
		      *buf++ = '8';
		    }
		  }
		  else if (num >= 60000000UL) {
		    if (num >= 70000000UL) {
		      num -= 70000000UL;
		      *buf++ = '7';
		    }
		    else {
		      num -= 60000000UL;
		      *buf++ = '6';
		    }
		  }
		  else {
		    num -= 50000000UL;
		    *buf++ = '5';
		  }
		}
		else if (num >= 20000000UL) {
		  if (num >= 40000000UL) {
		    num -= 40000000UL;
		    *buf++ = '4';
		  }
		  else if (num >= 30000000UL) {
		    num -= 30000000UL;
		    *buf++ = '3';
		  }
		  else {
		    num -= 20000000UL;
		    *buf++ = '2';
		  }
		}
		else if (num >= 10000000UL) {
		  num -= 10000000UL;
		  *buf++ = '1';
		}
		else *buf++ = '0';
	      }
	      if (num >= 5000000UL) {
		if (num >= 8000000UL) {
		  if (num >= 9000000UL) {
		    num -= 9000000UL;
		    *buf++ = '9';
		  }
		  else {
		    num -= 8000000UL;
		    *buf++ = '8';
		  }
		}
		else if (num >= 6000000UL) {
		  if (num >= 7000000UL) {
		    num -= 7000000UL;
		    *buf++ = '7';
		  }
		  else {
		    num -= 6000000UL;
		    *buf++ = '6';
		  }
		}
		else {
		  num -= 5000000UL;
		  *buf++ = '5';
		}
	      }
	      else if (num >= 2000000UL) {
		if (num >= 4000000UL) {
		  num -= 4000000UL;
		  *buf++ = '4';
		}
		else if (num >= 3000000UL) {
		  num -= 3000000UL;
		  *buf++ = '3';
		}
		else {
		  num -= 2000000UL;
		  *buf++ = '2';
		}
	      }
	      else if (num >= 1000000UL) {
		num -= 1000000UL;
		*buf++ = '1';
	      }
	      else *buf++ = '0';
	    }
	    if (num >= 500000UL) {
	      if (num >= 800000UL) {
		if (num >= 900000UL) {
		  num -= 900000UL;
		  *buf++ = '9';
		}
		else {
		  num -= 800000UL;
		  *buf++ = '8';
		}
	      }
	      else if (num >= 600000UL) {
		if (num >= 700000UL) {
		  num -= 700000UL;
		  *buf++ = '7';
		}
		else {
		  num -= 600000UL;
		  *buf++ = '6';
		}
	      }
	      else {
		num -= 500000UL;
		*buf++ = '5';
	      }
	    }
	    else if (num >= 200000UL) {
	      if (num >= 400000UL) {
		num -= 400000UL;
		*buf++ = '4';
	      }
	      else if (num >= 300000UL) {
		num -= 300000UL;
		*buf++ = '3';
	      }
	      else {
		num -= 200000UL;
		*buf++ = '2';
	      }
	    }
	    else if (num >= 100000UL) {
	      num -= 100000UL;
	      *buf++ = '1';
	    }
	    else *buf++ = '0';
	  }
	  if (num >= 50000UL) {
	    if (num >= 80000UL) {
	      if (num >= 90000UL) {
		num -= 90000UL;
		*buf++ = '9';
	      }
	      else {
		num -= 80000UL;
		*buf++ = '8';
	      }
	    }
	    else if (num >= 60000UL) {
	      if (num >= 70000UL) {
		num -= 70000UL;
		*buf++ = '7';
	      }
	      else {
		num -= 60000UL;
		*buf++ = '6';
	      }
	    }
	    else {
	      num -= 50000UL;
	      *buf++ = '5';
	    }
	  }
	  else if (num >= 20000UL) {
	    if (num >= 40000UL) {
	      num -= 40000UL;
	      *buf++ = '4';
	    }
	    else if (num >= 30000UL) {
	      num -= 30000UL;
	      *buf++ = '3';
	    }
	    else {
	      num -= 20000UL;
	      *buf++ = '2';
	    }
	  }
	  else if (num >= 10000UL) {
	    num -= 10000UL;
	    *buf++ = '1';
	  }
	  else *buf++ = '0';
	}
	if (num >= 5000UL) {
	  if (num >= 8000UL) {
	    if (num >= 9000UL) {
	      num -= 9000UL;
	      *buf++ = '9';
	    }
	    else {
	      num -= 8000UL;
	      *buf++ = '8';
	    }
	  }
	  else if (num >= 6000UL) {
	    if (num >= 7000UL) {
	      num -= 7000UL;
	      *buf++ = '7';
	    }
	    else {
	      num -= 6000UL;
	      *buf++ = '6';
	    }
	  }
	  else {
	    num -= 5000UL;
	    *buf++ = '5';
	  }
	}
	else if (num >= 2000UL) {
	  if (num >= 4000UL) {
	    num -= 4000UL;
	    *buf++ = '4';
	  }
	  else if (num >= 3000UL) {
	    num -= 3000UL;
	    *buf++ = '3';
	  }
	  else {
	    num -= 2000UL;
	    *buf++ = '2';
	  }
	}
	else if (num >= 1000UL) {
	  num -= 1000UL;
	  *buf++ = '1';
	}
	else *buf++ = '0';
      }
      if (num >= 500UL) {
	if (num >= 800UL) {
	  if (num >= 900UL) {
	    num -= 900UL;
	    *buf++ = '9';
	  }
	  else {
	    num -= 800UL;
	    *buf++ = '8';
	  }
	}
	else if (num >= 600UL) {
	  if (num >= 700UL) {
	    num -= 700UL;
	    *buf++ = '7';
	  }
	  else {
	    num -= 600UL;
	    *buf++ = '6';
	  }
	}
	else {
	  num -= 500UL;
	  *buf++ = '5';
	}
      }
      else if (num >= 200UL) {
	if (num >= 400UL) {
	  num -= 400UL;
	  *buf++ = '4';
	}
	else if (num >= 300UL) {
	  num -= 300UL;
	  *buf++ = '3';
	}
	else {
	  num -= 200UL;
	  *buf++ = '2';
	}
      }
      else if (num >= 100UL) {
	num -= 100UL;
	*buf++ = '1';
      }
      else *buf++ = '0';
    }
    if (num >= 50UL) {
      if (num >= 80UL) {
	if (num >= 90UL) {
	  num -= 90UL;
	  *buf++ = '9';
	}
	else {
	  num -= 80UL;
	  *buf++ = '8';
	}
      }
      else if (num >= 60UL) {
	if (num >= 70UL) {
	  num -= 70UL;
	  *buf++ = '7';
	}
	else {
	  num -= 60UL;
	  *buf++ = '6';
	}
      }
      else {
	num -= 50UL;
	*buf++ = '5';
      }
    }
    else if (num >= 20UL) {
      if (num >= 40UL) {
	num -= 40UL;
	*buf++ = '4';
      }
      else if (num >= 30UL) {
	num -= 30UL;
	*buf++ = '3';
      }
      else {
	num -= 20UL;
	*buf++ = '2';
      }
    }
    else if (num >= 10UL) {
      num -= 10UL;
      *buf++ = '1';
    }
    else *buf++ = '0';
  }
  if (num >= 5) {
    if (num >= 8) {
      if (num >= 9) {
	*buf++ = '9';
      }
      else {
	*buf++ = '8';
      }
    }
    else if (num >= 6) {
      if (num >= 7) {
	*buf++ = '7';
      }
      else {
	*buf++ = '6';
      }
    }
    else {
      *buf++ = '5';
    }
  }
  else if (num >= 2) {
    if (num >= 4) {
      *buf++ = '4';
    }
    else if (num >= 3) {
      *buf++ = '3';
    }
    else {
      *buf++ = '2';
    }
  }
  else if (num >= 1) {
    *buf++ = '1';
  }
  else *buf++ = '0';
  if (post_str) {
    while (*post_str) {
      *buf++ = *post_str++;
    }
  }
  bp->next_free_byte = buf;
#endif /* USE_SPRINTF_FOR_NUMS */
}

/* The following function copies a string to the specified string
 * buffer, optionally enclosing it in quotes, and escaping embedded
 * quotes along the way.
 *
 * Since the reader of the resulting string, when interpreting
 * escaped quotes (i.e. \" as ") will also interpret \\ as \,
 * it is necessary to escape backslashes as well.
 */

void add_quoted_str_to_string(const char* str,
				     a_boolean add_quotes,
				     string_buffer_ptr bp) {
    const char* p = str;
    const char* escapee;
    if (add_quotes) {
	add_1_char_to_string('\"', bp);
    }
    for (escapee = p + strcspn(p, "\"\\"); *escapee;
         escapee = p + strcspn(p, "\"\\")) {
	const char* q = escapee - 1;
	add_to_string_with_len(p, escapee - p, bp);
	add_1_char_to_string('\\', bp);
	add_1_char_to_string(*escapee, bp);
	p = escapee + 1;
    }
    add_to_string(p, bp);
    if (add_quotes) {
	add_1_char_to_string('\"', bp);
    }
}

/* The following function adds a symbol id to the string, using the
 * syntax required for the IF.
 */

void add_symid_to_string(unsigned long id, string_buffer_ptr bp) {
#ifdef USE_SPRINTF_FOR_NUMS
    char buff[32];
    size_t len;
#ifdef __BSD__
    sprintf(buff, " [%lu] ", id);
    len = strlen(buff);
#else
    len = sprintf(buff, " [%lu] ", id);
#endif
    add_to_string_with_len(buff, len, bp);
#else /* !defined(USE_SPRINTF_FOR_NUMS) */
    add_num_to_string(id, " [", "] ", bp);
#endif /* USE_SPRINTF_FOR_NUMS */
}

/* The following function terminates the current string as
 * appropriate, moves the current string pointer following the
 * termination character, resets the current length to zero, and
 * returns a pointer to the just-terminated string.
 */

const char* terminate_string(string_buffer_ptr bp) {
    char* old_str;
    add_1_char_to_string(bp->string_terminator, bp);
    old_str = bp->cur_str;
    bp->cur_str = bp->next_free_byte;
    return old_str;
}

/* The following function initializes the three string buffers,
 * including opening the output file.  It returns FALSE if the
 * specified file cannot be opened for output, TRUE otherwise.
 */

a_boolean initialize_string_buffers(const char* output_filename,
				    an_il_to_str_output_control_block* output_ocb,
				    an_il_to_str_output_control_block* string_ocb,
				    an_il_to_str_output_control_block* trial_ocb) {
    if (output_filename) {
	if (strcmp(output_filename, "-") == 0) {
	    IF_file = stdout;
	}
	else IF_file = fopen(output_filename, "a");
    }
    output_buf.ocb = output_ocb;
    string_buf.ocb = string_ocb;
    trial_buf.ocb = trial_ocb;
    return IF_file != NULL;
}

/* The following function flushes the output_buf and closes the
 * associated file.
 */

void close_output_buf() {
    handle_output_overflow(&output_buf);
    if (fclose(IF_file) != 0) {
	complain(error_csev, "Failed closing IF file.");
    }
    IF_file = NULL;
}

/* The following function adds a specified number of blanks to the
 * output buffer.
 */

void indent_to_depth(int depth) {
    int i;
    char* sixtyfour_spaces =
	    "                                                                ";
           /*         1         2         3         4         5         6    */
           /*1234567890123456789012345678901234567890123456789012345678901234*/
    for (i = 0; i < depth / 64; i++) {
	add_to_string_with_len(sixtyfour_spaces, 64, &output_buf);
    }
    add_to_string_with_len(sixtyfour_spaces, depth % 64, &output_buf);
}
