
#include "array.h"

void dl_array_init(dl_array_t *array, dl_memoryAllocation_t *memoryAllocation, dl_size_t element_size, dl_array_strategy_t strategy) {
	array->memoryAllocation = memoryAllocation;
	array->element_size = element_size;
	array->elements_length = 0;
	array->elements_memorySize = 0;
	array->elements = dl_null;
	array->strategy = strategy;
}

dl_error_t dl_array_quit(dl_array_t *array) {
	dl_error_t e = dl_error_ok;
	
	if (array->elements != dl_null) {
		e = dl_free(array->memoryAllocation, &array->elements);
		if (e) {
			goto l_cleanup;
		}
	}
	array->elements_length = 0;
	array->memoryAllocation = dl_null;
	array->element_size = 0;
	array->elements_memorySize = 0;
	array->strategy = 0;
	
	l_cleanup:
	
	return e;
}

dl_error_t dl_array_pushElement(dl_array_t *array, void *element) {
	dl_error_t e = dl_error_ok;
	
	// Add space for a new element.
	switch (array->strategy) {
	case dl_array_strategy_fit:
		e = dl_realloc(array->memoryAllocation, &array->elements, (array->elements_length + 1) * array->element_size);
		if (e) {
			goto l_cleanup;
		}
		array->elements_memorySize = (array->elements_length + 1) * array->element_size;
		break;
	case dl_array_strategy_double:
		while ((array->elements_length + 1) * array->element_size > array->elements_memorySize) {
			e = dl_realloc(array->memoryAllocation, &array->elements, 2 * (array->elements_length + 1) * array->element_size);
			if (e) {
				goto l_cleanup;
			}
			array->elements_memorySize = 2 * (array->elements_length + 1) * array->element_size;
		}
		break;
	default:
		e = dl_error_shouldntHappen;
		goto l_cleanup;
	}
	
	if (element == dl_null) {
		// Create an empty element.
		/**/ dl_memclear(&((unsigned char *) array->elements)[array->element_size * array->elements_length], array->element_size);
	} else {
		// Copy given element into array.
		e = dl_memcopy(&((unsigned char *) array->elements)[array->element_size * array->elements_length], element, array->element_size);
		if (e) {
			goto l_cleanup;
		}
	}
	
	array->elements_length++;
	
	l_cleanup:
	
	return e;
}

dl_error_t dl_array_pushElements(dl_array_t *array, const void *elements, dl_size_t elements_length) {
	dl_error_t e = dl_error_ok;
	
	dl_bool_t wasNull = (elements == dl_null);
	
	if (elements_length == 0) {
		goto l_cleanup;
	}
	
	// Add space for new elements.
	switch (array->strategy) {
	case dl_array_strategy_fit:
		e = dl_realloc(array->memoryAllocation, &array->elements, (array->elements_length + elements_length) * array->element_size);
		if (e) {
			goto l_cleanup;
		}
		array->elements_memorySize = (array->elements_length + elements_length) * array->element_size;
		break;
	case dl_array_strategy_double:
		while ((array->elements_length + elements_length) * array->element_size > array->elements_memorySize) {
			e = dl_realloc(array->memoryAllocation, &array->elements, 2 * (array->elements_length + elements_length) * array->element_size);
			if (e) {
				goto l_cleanup;
			}
			array->elements_memorySize = 2 * (array->elements_length + elements_length) * array->element_size;
		}
		break;
	default:
		e = dl_error_shouldntHappen;
		goto l_cleanup;
	}
	
	if (wasNull) {
		/**/ dl_memclear(&((unsigned char *) array->elements)[array->element_size * array->elements_length],
		                 elements_length * array->element_size);
	}
	else {
		// Copy given element into array.
		e = dl_memcopy(&((unsigned char *) array->elements)[array->element_size * array->elements_length], elements,
		               elements_length * array->element_size);
		if (e) {
			goto l_cleanup;
		}
	}
	
	array->elements_length += elements_length;
	
	l_cleanup:
	
	return e;
}

// dl_error_t dl_array_popElement(dl_array_t *array, void **element, dl_ptrdiff_t index) {
// 	dl_error_t e = dl_error_ok;
	
// 	*element = (unsigned char *) array->elements + index;
	
// 	l_cleanup:
	
// 	return e;
// }

dl_error_t dl_array_popElement(dl_array_t *array, void *element) {
	dl_error_t e = dl_error_ok;
	
	if (array->elements_length > 0) {
		if (element != dl_null) {
			e = dl_memcopy(element,
			               (char*)array->elements + (array->elements_length - 1) * array->element_size,
			               array->element_size);
			if (e) goto l_cleanup;
		}
		--array->elements_length;
	}
	else e = dl_error_bufferUnderflow;
	
	l_cleanup:
	
	return e;
}

dl_error_t dl_array_popElements(dl_array_t *array, void *elements, dl_size_t count) {
	dl_error_t e = dl_error_ok;

	if (count > 0) {
		if (array->elements_length >= count) {
			if (elements != dl_null) {
				e = dl_memcopy(elements, (char*)array->elements + (array->elements_length - count) * array->element_size, count * array->element_size);
				if (e) {
					goto l_cleanup;
				}
			}
			array->elements_length -= count;
		}
		else {
			e = dl_error_bufferUnderflow;
			goto l_cleanup;
		}
	}
	
	l_cleanup:
	
	return e;
}

/*
Fetches element on the top of the stack.
Returns bufferUnderflow if the stack is empty.
*/
dl_error_t dl_array_getTop(dl_array_t *array, void *element) {
	dl_error_t e = dl_error_ok;
	
	if (array->elements_length > 0) {
		e = dl_memcopy(element, (char*)array->elements + (array->elements_length - 1) * array->element_size, array->element_size);
		if (e) {
			goto l_cleanup;
		}
	}
	else {
		e = dl_error_bufferUnderflow;
		goto l_cleanup;
	}
	
	l_cleanup:
	
	return e;
}

// dl_error_t dl_array_popElements(dl_array_t *array, void *element, dl_size_t number) {
// 	dl_error_t e = dl_error_ok;
	
// 	for (dl_size_t i = 0; i < number; i++) {
// 		if (array->elements_length > 0) {
// 			e = dl_memcopy(element, array->elements + (array->elements_length - number) * array->element_size, number * array->element_size);
// 			if (e) {
// 				goto l_cleanup;
// 			}
// 		}
// 		else {
// 			e = dl_error_bufferUnderflow;
// 			goto l_cleanup;
// 		}
// 	}
	
// 	array->elements_length -= number;
	
// 	l_cleanup:
	
// 	return e;
// }

dl_error_t dl_array_get(dl_array_t *array, void *element, dl_ptrdiff_t index) {
	dl_error_t e = dl_error_ok;
	
	if ((index >= 0) && ((dl_size_t) index < array->elements_length)) {
		e = dl_memcopy(element, (char*)array->elements + index * array->element_size, array->element_size);
		if (e) {
			goto l_cleanup;
		}
	}
	else {
		e = dl_error_invalidValue;
		goto l_cleanup;
	}
	
	l_cleanup:
	
	return e;
}

dl_error_t dl_array_set(dl_array_t *array, const void *element, dl_ptrdiff_t index) {
	dl_error_t e = dl_error_ok;
	
	if ((index >= 0) && ((dl_size_t) index < array->elements_length)) {
		e = dl_memcopy( (char*)array->elements + index * array->element_size, element, array->element_size);
		if (e) {
			goto l_cleanup;
		}
	}
	else {
		e = dl_error_invalidValue;
		goto l_cleanup;
	}
	
	l_cleanup:
	
	return e;
}

/*
Keeps arrayDestination's allocator the same. After all, it's easy enough to copy the allocator yourself if needed.
*/
dl_error_t dl_array_copy(dl_array_t *arrayDestination, dl_array_t arraySource) {
	dl_error_t e = dl_error_ok;
	
	e = dl_free(arrayDestination->memoryAllocation, &arrayDestination->elements);
	if (e) {
		goto l_cleanup;
	}
	e = dl_malloc(arrayDestination->memoryAllocation, &arrayDestination->elements, arraySource.elements_memorySize);
	if (e) {
		goto l_cleanup;
	}
	
	arrayDestination->elements_length = arraySource.elements_length;
	// arrayDestination->memoryAllocation = dl_null;
	arrayDestination->element_size = arraySource.element_size;
	arrayDestination->elements_memorySize = arraySource.elements_memorySize;
	arrayDestination->strategy = arraySource.strategy;
	
	e = dl_memcopy(arrayDestination->elements, arraySource.elements, arraySource.elements_length * arraySource.element_size);
	if (e) {
		goto l_cleanup;
	}
	
	*arrayDestination = arraySource;
	
	l_cleanup:
	
	return e;
}

// Just `array_push` but for arrays.
dl_error_t dl_array_append(dl_array_t *arrayDestination, dl_array_t *arraySource) {
	dl_error_t e = dl_error_ok;

	// Add space for new elements.
	switch (arrayDestination->strategy) {
	case dl_array_strategy_fit:
		e = dl_realloc(arrayDestination->memoryAllocation, &arrayDestination->elements, (arrayDestination->elements_length + arraySource->elements_length) * arrayDestination->element_size);
		if (e) {
			goto l_cleanup;
		}
		arrayDestination->elements_memorySize = (arrayDestination->elements_length + arraySource->elements_length) * arrayDestination->element_size;
		break;
	case dl_array_strategy_double:
		while ((arrayDestination->elements_length + arraySource->elements_length) * arrayDestination->element_size > arrayDestination->elements_memorySize) {
			e = dl_realloc(arrayDestination->memoryAllocation, &arrayDestination->elements, 2 * (arrayDestination->elements_length + arraySource->elements_length) * arrayDestination->element_size);
			if (e) {
				goto l_cleanup;
			}
			arrayDestination->elements_memorySize = 2 * (arrayDestination->elements_length + arraySource->elements_length) * arrayDestination->element_size;
		}
		break;
	default:
		e = dl_error_shouldntHappen;
		goto l_cleanup;
	}
	
	// Copy given element into array.
	e = dl_memcopy(&((unsigned char *) arrayDestination->elements)[arrayDestination->element_size * arrayDestination->elements_length], arraySource->elements,
	               arraySource->elements_length * arrayDestination->element_size);
	if (e) {
		goto l_cleanup;
	}
	
	arrayDestination->elements_length += arraySource->elements_length;

	l_cleanup:

	return e;
}

dl_error_t dl_array_clear(dl_array_t *array) {
	dl_error_t e = dl_error_ok;
	
	if (array->elements_length == 0) {
		return e;
	}
	
	array->elements_length = 0;
	array->elements_memorySize = 0;
	e = dl_free(array->memoryAllocation, (void **) &array->elements);
	if (e) {
		goto l_cleanup;
	}
	
	l_cleanup:
	
	return e;
}
