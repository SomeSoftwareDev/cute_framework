/*
	Cute Framework
	Copyright (C) 2019 Randy Gaul https://randygaul.net

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

#include <cute_kv.h>
using namespace cute;

struct thing_t
{
	int a = 5;
	float b = 10.3f;
	const char* str = "Hello.";
	struct
	{
		int a = 5;
		int die_pls = 5;
		struct
		{
			int hi = 5;
			const char* geez = "Hello.";
		} interior_thing;
	} sub_thing;
	float x = 5;
	float y = 10.3f;
	char blob_data[24] = "Some blob input.";
	int array_of_ints[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	int array_of_array_of_ints[2][3] = {
		{ 0, 1, 2 },
		{ 0, 1, 2 },
	};
	struct
	{
		int some_integer = 5;
		const char* some_string = "Hello.";
	} array_of_objects[3];
};

error_t do_serialize(kv_t* kv, thing_t* thing)
{
	size_t len;
	CUTE_RETURN_IF_ERROR(kv_object_begin(kv));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "a")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->a));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "b")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->b));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "str")); len = 6; CUTE_RETURN_IF_ERROR(kv_val_string(kv, &thing->str, &len));
		CUTE_RETURN_IF_ERROR(kv_key(kv, "sub_thing")); CUTE_RETURN_IF_ERROR(kv_object_begin(kv));
		CUTE_RETURN_IF_ERROR(kv_key(kv, "a")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->sub_thing.a));
		CUTE_RETURN_IF_ERROR(kv_key(kv, "die_pls")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->sub_thing.die_pls));
			CUTE_RETURN_IF_ERROR(kv_key(kv, "interior_thing")); CUTE_RETURN_IF_ERROR(kv_object_begin(kv));
			CUTE_RETURN_IF_ERROR(kv_key(kv, "hi")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->sub_thing.interior_thing.hi));
			CUTE_RETURN_IF_ERROR(kv_key(kv, "geez")); len = 6; CUTE_RETURN_IF_ERROR(kv_val_string(kv, &thing->sub_thing.interior_thing.geez, &len));
			CUTE_RETURN_IF_ERROR(kv_object_end(kv));
		CUTE_RETURN_IF_ERROR(kv_object_end(kv));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "x")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->x));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "y")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->y));
	int blob_size = 17;
	CUTE_RETURN_IF_ERROR(kv_key(kv, "blob_data")); CUTE_RETURN_IF_ERROR(kv_val_blob(kv, thing->blob_data, &blob_size, sizeof(thing->blob_data)));
	int int_count = 8;
	CUTE_RETURN_IF_ERROR(kv_key(kv, "array_of_ints")); CUTE_RETURN_IF_ERROR(kv_array_begin(kv, &int_count));
		for (int i = 0; i < int_count; ++i) CUTE_RETURN_IF_ERROR(kv_val(kv, thing->array_of_ints + i));
	CUTE_RETURN_IF_ERROR(kv_array_end(kv));
	int_count = 2;
	CUTE_RETURN_IF_ERROR(kv_key(kv, "array_of_array_of_ints")); CUTE_RETURN_IF_ERROR(kv_array_begin(kv, &int_count));
		int_count = 3;
		CUTE_RETURN_IF_ERROR(kv_array_begin(kv, &int_count));
		for (int i = 0; i < int_count; ++i)
		{
			CUTE_RETURN_IF_ERROR(kv_val(kv, thing->array_of_array_of_ints[0] + i));
		}
		CUTE_RETURN_IF_ERROR(kv_array_end(kv));
		CUTE_RETURN_IF_ERROR(kv_array_begin(kv, &int_count));
		for (int i = 0; i < int_count; ++i)
		{
			CUTE_RETURN_IF_ERROR(kv_val(kv, thing->array_of_array_of_ints[1] + i));
		}
		CUTE_RETURN_IF_ERROR(kv_array_end(kv));
	CUTE_RETURN_IF_ERROR(kv_array_end(kv));
	CUTE_RETURN_IF_ERROR(kv_key(kv, "array_of_objects")); CUTE_RETURN_IF_ERROR(kv_array_begin(kv, &int_count));
	for (int i = 0; i < int_count; ++i)
	{
		CUTE_RETURN_IF_ERROR(kv_object_begin(kv));
		CUTE_RETURN_IF_ERROR(kv_key(kv, "some_integer")); CUTE_RETURN_IF_ERROR(kv_val(kv, &thing->array_of_objects[i].some_integer));
		CUTE_RETURN_IF_ERROR(kv_key(kv, "some_string")); len = 6; CUTE_RETURN_IF_ERROR(kv_val_string(kv, &thing->array_of_objects[i].some_string, &len));
		CUTE_RETURN_IF_ERROR(kv_object_end(kv));
	}
	CUTE_RETURN_IF_ERROR(kv_array_end(kv));
	CUTE_RETURN_IF_ERROR(kv_object_end(kv));
	return error_success();
}

CUTE_TEST_CASE(test_kv_basic, "Fairly comprehensive test for basic kv to and from buffer.");
int test_kv_basic()
{
	kv_t* kv = kv_make();

	char buffer[1024];
	kv_reset_io(kv, buffer, sizeof(buffer), CUTE_KV_MODE_WRITE);

	thing_t thing;
	thing.a = 5;
	thing.b = 10.3f;
	thing.str = "Hello.";

	CUTE_TEST_ASSERT(!do_serialize(kv, &thing).is_error());

	const char* expected =
	"{\n"
	"	a = 5,\n"
	"	b = 10.300000,\n"
	"	str = \"Hello.\",\n"
	"	sub_thing = {\n"
	"		a = 5,\n"
	"		die_pls = 5,\n"
	"		interior_thing = {\n"
	"			hi = 5,\n"
	"			geez = \"Hello.\",\n"
	"		},\n"
	"	},\n"
	"	x = 5.000000,\n"
	"	y = 10.300000,\n"
	"	blob_data = \"U29tZSBibG9iIGlucHV0LgA=\",\n"
	"	array_of_ints = [8] {\n"
	"		0, 1, 2, 3, 4, 5, 6, 7,\n"
	"	},\n"
	"	array_of_array_of_ints = [2] {\n"
	"		[3] {\n"
	"			0, 1, 2,\n"
	"		},\n"
	"		[3] {\n"
	"			0, 1, 2,\n"
	"		},\n"
	"	},\n"
	"	array_of_objects = [3] {\n"
	"		{\n"
	"			some_integer = 5,\n"
	"			some_string = \"Hello.\",\n"
	"		},\n"
	"		{\n"
	"			some_integer = 5,\n"
	"			some_string = \"Hello.\",\n"
	"		},\n"
	"		{\n"
	"			some_integer = 5,\n"
	"			some_string = \"Hello.\",\n"
	"		},\n"
	"	},\n"
	"},\n"
	;

	int size = kv_size_written(kv);
	CUTE_TEST_ASSERT(!CUTE_STRNCMP(buffer, expected, size));

	error_t err = kv_reset_io(kv, buffer, size, CUTE_KV_MODE_READ);
	CUTE_TEST_ASSERT(!err.is_error());

	CUTE_MEMSET(&thing, 0, sizeof(thing_t));

	CUTE_TEST_ASSERT(!do_serialize(kv, &thing).is_error());

	kv_destroy(kv);

	return 0;
}

#include <string>

CUTE_TEST_CASE(test_kv_std_string_to_disk, "Testing kv utility for c-string to disk, std::string from disk.");
int test_kv_std_string_to_disk()
{
	std::string s0;
	const char* s1 = "Alice in Wonderland.";
	size_t s1_len = CUTE_STRLEN(s1);

	kv_t* kv = kv_make();
	CUTE_TEST_CHECK_POINTER(kv);

	char buffer[1024];
	kv_reset_io(kv, buffer, sizeof(buffer), CUTE_KV_MODE_WRITE);

	kv_object_begin(kv);
	kv_key(kv, "book_title");
	kv_val_string(kv, &s1, &s1_len);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(!kv_error_state(kv).is_error());
	int size = kv_size_written(kv);
	CUTE_TEST_ASSERT(!kv_reset_io(kv, buffer, size, CUTE_KV_MODE_READ).is_error());

	kv_object_begin(kv);
	kv_key(kv, "book_title");
	kv_val(kv, &s0);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(!kv_error_state(kv).is_error());
	CUTE_TEST_ASSERT(s0.length() == s1_len);
	CUTE_TEST_ASSERT(!CUTE_STRNCMP(s0.data(), s1, s1_len));

	kv_destroy(kv);

	return 0;
}

CUTE_TEST_CASE(test_kv_std_string_from_disk, "Testing kv utility for std::string to disk, c-string from disk.");
int test_kv_std_string_from_disk()
{
	// std::string from disk, c-string to disk
	const char* s0 = NULL;
	size_t s0_len = 0;
	std::string s1 = "Alice in Wonderland.";
	size_t s1_len = s1.length();

	kv_t* kv = kv_make();
	CUTE_TEST_CHECK_POINTER(kv);

	char buffer[1024];
	kv_reset_io(kv, buffer, sizeof(buffer), CUTE_KV_MODE_WRITE);

	kv_object_begin(kv);
	kv_key(kv, "book_title");
	kv_val(kv, &s1);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(!kv_error_state(kv).is_error());
	int size = kv_size_written(kv);
	CUTE_TEST_ASSERT(!kv_reset_io(kv, buffer, size, CUTE_KV_MODE_READ).is_error());

	kv_object_begin(kv);
	kv_key(kv, "book_title");
	kv_val_string(kv, &s0, &s0_len);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(!kv_error_state(kv).is_error());
	CUTE_TEST_ASSERT((int)s1.length() == s0_len);
	CUTE_TEST_ASSERT(!CUTE_STRNCMP(s1.data(), s0, s0_len));

	kv_destroy(kv);

	return 0;
}

CUTE_TEST_CASE(test_kv_std_vector, "Testing kv utility for std::vector support.");
int test_kv_std_vector()
{
	kv_t* kv = kv_make();
	CUTE_TEST_CHECK_POINTER(kv);

	char buffer[1024];
	kv_reset_io(kv, buffer, sizeof(buffer), CUTE_KV_MODE_WRITE);

	std::vector<int> v;
	v.push_back(10);
	v.push_back(-3);
	v.push_back(17);
	v.push_back(5);
	v.push_back(0);
	v.push_back(100);
	v.push_back(6);
	v.push_back(-2);

	kv_object_begin(kv);
	kv_key(kv, "vector_of_ints");
	kv_val(kv, &v);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(!kv_error_state(kv).is_error());
	int size = kv_size_written(kv);
	CUTE_TEST_ASSERT(!kv_reset_io(kv, buffer, size, CUTE_KV_MODE_READ).is_error());

	v.clear();
	kv_object_begin(kv);
	kv_key(kv, "vector_of_ints");
	kv_val(kv, &v);
	kv_object_end(kv);

	CUTE_TEST_ASSERT(v.size() == 8);
	CUTE_TEST_ASSERT(v[0] == 10);
	CUTE_TEST_ASSERT(v[1] == -3);
	CUTE_TEST_ASSERT(v[2] == 17);
	CUTE_TEST_ASSERT(v[3] == 5);
	CUTE_TEST_ASSERT(v[4] == 0);
	CUTE_TEST_ASSERT(v[5] == 100);
	CUTE_TEST_ASSERT(v[6] == 6);
	CUTE_TEST_ASSERT(v[7] == -2);

	kv_destroy(kv);

	return 0;
}