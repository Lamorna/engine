#include <iostream>
#include <Windows.h>

/*
==================
==================
*/
void JIT_test(void) {

	//char* filename = "test.tgn";
	char* filename;


	FILE *file_handle;
	fopen_s(&file_handle, filename, "rb");		// txt mode
	if (!file_handle) {
		printf_s("FAILED to open %s \n", filename);
		exit(0);
	}

	const __int32 size = 128;
	unsigned __int8 buffer[size];
	memset(buffer, 0, size);

	for (__int32 i_char = 0; i_char < size; i_char++) {

		__int32 c = fgetc(file_handle);

		if (c == EOF) {
			break;
		}

		buffer[i_char] = (unsigned __int8)c;
	}

	//fgets(buffer, size, file_handle);

	__int32 n_chars = 0;
	__int32 inc = 1;
	for (__int32 i_char = 0; i_char < size; i_char++) {

		inc = buffer[i_char] == '\0' ? 0 : inc;
		n_chars += inc;
	}

	fclose(file_handle);

	printf_s("\n________________________________________________________________________\n");

	for (__int32 i_char = 0; i_char < n_chars; i_char++) {

		printf_s(" %02x ", buffer[i_char]);
	}

	printf_s("\n________________________________________________________________________\n");

	const __int32 buffer_size = 512;

	byte* code_buffer = (byte *)VirtualAllocEx(GetCurrentProcess(), 0, buffer_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	for (__int32 i_char = 0; i_char < size; i_char++) {
		code_buffer[i_char] = buffer[i_char];
	}

	typedef int(*Func)(const float* in, float* out);

	Func test_func = (Func)code_buffer;

	const float array_in[] = {

		0.0f, 1.0f, 2.0f, 3.0f ,
		0.0f, 1.0f, 2.0f, 3.0f ,
	};

	float array_out[] = {

		0.0f, 0.0f, 0.0f, 0.0f ,
		0.0f, 0.0f, 0.0f, 0.0f ,
	};

	int result = test_func(array_in, array_out);

	for (__int32 i_result = 0; i_result < 4; i_result++) {

		printf_s(" %f ", array_out[i_result]);
	}
}