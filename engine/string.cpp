#include "string.h"

/*
==================
==================
*/
__int32 string_length(

	const char read[]

) {

	__int32 n_chars = 0;
	for (; read[n_chars] != '\0'; n_chars++) {
	}
	return n_chars;
}


/*
==================
==================
*/
void string_copy(

	const char read[],
	char write[]

) {

	const __int32 length = string_length(read);

	for (__int32 i_char = 0; i_char < length; i_char++) {
		write[i_char] = read[i_char];
	}
	write[length] = '\0';
}

/*
==================
==================
*/
void string_concatenate(

	const char string_a[],
	char string_b[]

) {

	__int32 length_a = string_length(string_a);
	__int32 length_b = string_length(string_b);

	for (__int32 i_char = 0; i_char < length_a; i_char++) {
		string_b[length_b + i_char] = string_a[i_char];
	}
	string_b[length_b + length_a] = '\0';
}


/*
==================
==================
*/
void string_concatenate(

	const char string_a[],
	const char string_b[],
	char string_out[]

) {

	__int32 n_chars = 0;
	{
		const __int32 length = string_length(string_a);
		for (__int32 i_char = 0; i_char < length; i_char++) {
			string_out[n_chars++] = string_a[i_char];
		}
	}
	{
		const __int32 length = string_length(string_b);
		for (__int32 i_char = 0; i_char < length; i_char++) {
			string_out[n_chars++] = string_b[i_char];
		}
	}
	string_out[n_chars] = '\0';
}

/*
==================
==================
*/
__int32 string_compare(

	const char string_a[],
	const char string_b[]

) {

	__int32 result = 0x1;
	const __int32 length = string_length(string_a);
	for (__int32 i_char = 0; i_char < length; i_char++) {
		result &= string_a[i_char] == string_b[i_char];
	}
	result &= length == string_length(string_b);
	return result;
}