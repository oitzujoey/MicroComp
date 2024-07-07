
#include "string.h"

void dl_string_isDigit(dl_bool_t *result, const char character) {
	*result = (character >= '0') && (character <= '9');
}

void dl_string_isHexadecimalDigit(dl_bool_t *result, const char character) {
	*result = (((character >= '0') && (character <= '9'))
	           || ((character >= 'a') && (character <= 'f'))
	           || ((character >= 'A') && (character <= 'F')));
}

void dl_string_isAlpha(dl_bool_t *result, const char character) {
	*result = ((character >= 'a') && (character <= 'z')) || ((character >= 'A') && (character <= 'Z'));
}

void dl_string_isSpace(dl_bool_t *result, const char character) {
	*result = (character <= ' ') && (character >= '\0');
}


dl_error_t dl_string_toBool(dl_bool_t *result, const char *string, const dl_size_t string_length) {
	dl_error_t e = dl_error_ok;

	dl_string_compare(result, string, string_length, DL_STR("true"));
	if (!*result) {
		dl_string_compare(result, string, string_length, DL_STR("false"));
		if (!*result) {
			*result = dl_false;
			e = dl_error_invalidValue;
			goto cleanup;
		}
		else {
			*result = dl_false;
		}
	}
	else {
		*result = dl_true;
	}

 cleanup:
	return e;
}

dl_error_t dl_string_toPtrdiff(dl_ptrdiff_t *result, const char *string, const dl_size_t string_length) {
	dl_error_t e = dl_error_ok;

	dl_ptrdiff_t index = 0;
	dl_bool_t tempBool;
	dl_bool_t negative = dl_false;
	dl_bool_t hexadecimal = dl_false;

	*result = 0;

	if ((dl_size_t) index >= string_length) {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	if (string[index] == '-') {
		index++;

		if ((dl_size_t) index >= string_length) {
			e = dl_error_invalidValue;
			goto cleanup;
		}

		negative = dl_true;
	}

	/**/ dl_string_isDigit(&tempBool, string[index]);
	if (!tempBool) {
		e = dl_error_invalidValue;
		goto cleanup;
	}

	/* Hexadecimal */
	if ((dl_size_t) index < string_length) {
		if ((string[index] == '0')
		    && ((dl_size_t) index + 1 < string_length)
		    && ((string[index + 1] == 'x')
		        || (string[index + 1] == 'X'))) {
			hexadecimal = dl_true;
			index += 2;
		}
	}

	while ((dl_size_t) index < string_length) {
		if (hexadecimal) {
			/**/ dl_string_isHexadecimalDigit(&tempBool, string[index]);
		}
		else {
			/**/ dl_string_isDigit(&tempBool, string[index]);
		}
		if (!tempBool) {
			e = dl_error_invalidValue;
			goto cleanup;
		}

		if (hexadecimal) {
			*result = (*result * 16
			           + (('0' <= string[index]) && (string[index] <= '9')
			              ? string[index] - '0'
			              : ('a' <= string[index]) && (string[index] <= 'f')
			              ? string[index] - 'a' + 10
			              : (string[index] - 'A' + 10)));
		}
		else {
			*result = *result * 10 + (string[index] - '0');
		}

		index++;
	}

	if (negative) {
		*result = -*result;
	}

 cleanup:
	return e;
}

dl_error_t dl_string_toDouble(double *result, const char *string, const dl_size_t string_length) {
	dl_error_t e = dl_error_ok;

	dl_ptrdiff_t index = 0;
	dl_bool_t tempBool;
	dl_bool_t negative = dl_false;
	dl_ptrdiff_t power = 0;
	dl_bool_t power_negative = dl_false;
	
	*result = 0.0;
	
	if (string[index] == '-') {
		index++;
		
		if ((dl_size_t) index >= string_length) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		negative = dl_true;
	}
	
	/* Try .1 */
	if (string[index] == '.') {
		index++;
		
		if ((dl_size_t) index >= string_length) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		power = 1;
		
		/* No error */ dl_string_isDigit(&tempBool, string[index]);
		if (!tempBool) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		*result += (double) (string[index] - '0') / (10.0 * (double) power);
		
		index++;
		
		while (((dl_size_t) index < string_length) && (dl_string_toLower((unsigned char) string[index]) != 'e')) {
			
			power++;
			
			/* No error */ dl_string_isDigit(&tempBool, string[index]);
			if (!tempBool) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			*result += (double) (string[index] - '0') / (10.0 * (double) power);
			
			index++;
		}
	}
	// Try 1.2, 1., and 1
	else {
		/* No error */ dl_string_isDigit(&tempBool, string[index]);
		if (!tempBool) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		*result = string[index] - '0';
		
		index++;
		
		while (((dl_size_t) index < string_length) && (dl_string_toLower(string[index]) != 'e') && (string[index] != '.')) {
			
			/* No error */ dl_string_isDigit(&tempBool, string[index]);
			if (!tempBool) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			*result = *result * 10.0 + (double) (string[index] - '0');
			
			index++;
		}
		
		if (string[index] == '.') {
			index++;
			
			if ((dl_size_t) index >= string_length) {
				// eError = duckLisp_error_push(duckLisp, DL_STR("Expected a digit after decimal point."), index);
				// e = eError ? eError : dl_error_bufferOverflow;
				// This is expected. 1. 234.e61  435. for example.
				goto l_cleanup;
			}
			
			power = 1;
		}
		
		while (((dl_size_t) index < string_length) && (dl_string_toLower(string[index]) != 'e')) {
			
			/* No error */ dl_string_isDigit(&tempBool, string[index]);
			if (!tempBool) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			*result += (double) (string[index] - '0') / (10.0 * (double) power);
			
			index++;
		}
	}
	
	// …e3
	if (dl_string_toLower(string[index]) == 'e') {
		index++;
		
		if ((dl_size_t) index >= string_length) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		if (string[index] == '-') {
			index++;
			
			if ((dl_size_t) index >= string_length) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			power_negative = dl_true;
		}
		
		/* No error */ dl_string_isDigit(&tempBool, string[index]);
		if (!tempBool) {
			e = dl_error_invalidValue;
			goto l_cleanup;
		}
		
		power = string[index] - '0';
		
		index++;
		
		while ((dl_size_t) index < string_length) {
			/* No error */ dl_string_isDigit(&tempBool, string[index]);
			if (!tempBool) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			power = power * 10 + string[index] - '0';
			
			index++;
		}
		
		if (power_negative) {
			if (power == 0) {
				e = dl_error_invalidValue;
				goto l_cleanup;
			}
			
			for (dl_ptrdiff_t i = 0; i < power; i++) {
				*result /= 10.0;
			}
		}
		else {
			for (dl_ptrdiff_t i = 0; i < power; i++) {
				*result *= 10.0;
			}
		}
	}
	
	if ((dl_size_t) index != string_length) {
		e = dl_error_cantHappen;
		goto l_cleanup;
	}
	
	if (negative) {
		*result = -*result;
	}
	
	l_cleanup:
	
	return e;
}


void dl_string_compare(dl_bool_t *result, const char *str1, const dl_size_t str1_length, const char *str2, const dl_size_t str2_length) {
	*result = dl_true;
	if (str1_length != str2_length) {
		*result = dl_false;
	}
	else for (dl_ptrdiff_t i = 0; (dl_size_t) i < str1_length; i++) {
		if (str1[i] != str2[i]) {
			*result = dl_false;
			break;
		}
	}
}

void dl_string_compare_partial(dl_bool_t *result, const char *str1, const char *str2, const dl_size_t length) {
	*result = dl_true;
	for (dl_ptrdiff_t i = 0; (dl_size_t) i < length; i++) {
		if (str1[i] != str2[i]) {
			*result = dl_false;
			break;
		}
	}
}
