// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2018, Linaro Limited
 */

#include <tee_internal_api.h>
#include <string.h>

#include "seed_rng_taf.h"

#define TA_DERIVED_KEY_MIN_SIZE 16
#define TA_DERIVED_KEY_MAX_SIZE 32

static const TEE_UUID system_uuid = PTA_SYSTEM_UUID;

TEE_Result seed_rng_pool(uint32_t param_types, TEE_Param params[4])
{
	TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_orig = 0;

	if (param_types !=
	    TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
			    TEE_PARAM_TYPE_NONE,
			    TEE_PARAM_TYPE_NONE,
			    TEE_PARAM_TYPE_NONE)) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (!params[0].memref.size)
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_OpenTASession(&system_uuid, 0, 0, NULL, &sess, &ret_orig);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_OpenTASession failed");
		goto cleanup_return;
	}

	res = TEE_InvokeTACommand(sess, 0, PTA_SYSTEM_ADD_RNG_ENTROPY,
				  param_types, params, &ret_orig);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_InvokeTACommand failed");
		goto cleanup_return;
	}

cleanup_return:
	TEE_CloseTASession(sess);
	return res;
}

/*
 * Helper function that just derives a key.
 */
static TEE_Result derive_unique_key(TEE_TASessionHandle session,
				    uint8_t *key, uint16_t key_size,
				    uint8_t *extra, uint16_t extra_size)
{
	TEE_Param params[4] = { 0 };
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
	uint32_t param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
					       TEE_PARAM_TYPE_MEMREF_OUTPUT,
					       TEE_PARAM_TYPE_NONE,
					       TEE_PARAM_TYPE_NONE);
	if (extra && extra_size > 0) {
		params[0].memref.buffer = extra;
		params[0].memref.size = extra_size;
	}

	params[1].memref.buffer = key;
	params[1].memref.size = key_size;

	res = TEE_InvokeTACommand(session, 0, PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY,
				  param_types, params, &ret_origin);
	if (res != TEE_SUCCESS)
		EMSG("Failure when calling PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY");

	return res;
}

TEE_Result derive_ta_unique_key_test(uint32_t param_types,
				     TEE_Param params[4] __unused)
{
	size_t i = 0;
	TEE_Result res_final = TEE_SUCCESS;
	TEE_Result res = TEE_ERROR_GENERIC;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	uint8_t big_key[64] = { };
	uint8_t extra_key_data[] = { "My dummy data" };
	uint8_t key1[32] = { };
	uint8_t key2[32] = { };
	uint32_t ret_origin = 0;

	if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_OpenTASession(&system_uuid, 0, 0, NULL, &session,
				&ret_origin);
	if (res != TEE_SUCCESS) {
		res_final = TEE_ERROR_GENERIC;
		goto err;
	}

	/*
	 * Testing for successful calls to the pTA and that two calls with same
	 * input data generates the same output data (keys).
	 */
	res = derive_unique_key(session, key1, sizeof(key1), NULL, 0);
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	res = derive_unique_key(session, key2, sizeof(key2), NULL, 0);
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	if (TEE_MemCompare(key1, key2, sizeof(key1)) != 0)
		res_final = TEE_ERROR_GENERIC;

	memset(key1, 0, sizeof(key1));
	memset(key2, 0, sizeof(key2));

	/*
	 * Testing for successful calls to the pTA and that two calls with same
	 * input data generates the same output data (keys). Here we are using
	 * extra data also.
	 */
	res = derive_unique_key(session, key1, sizeof(key1), extra_key_data,
				sizeof(extra_key_data));
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	res = derive_unique_key(session, key2, sizeof(key2), extra_key_data,
				sizeof(extra_key_data));
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	if (TEE_MemCompare(key1, key2, sizeof(key1)) != 0)
		res_final = TEE_ERROR_GENERIC;

	memset(key1, 0, sizeof(key1));
	memset(key2, 0, sizeof(key2));

	/*
	 * Testing for successful calls to the pTA and that two calls with
	 * different input data do not generate the same output data (keys). We
	 * do that by using one key with and one key without extra data.
	 */
	res = derive_unique_key(session, key1, sizeof(key1), extra_key_data,
				sizeof(extra_key_data));
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	res = derive_unique_key(session, key2, sizeof(key2), NULL, 0);
	if (res != TEE_SUCCESS)
		res_final = TEE_ERROR_GENERIC;

	/* They should not be equal */
	if (TEE_MemCompare(key1, key2, sizeof(key1)) == 0)
		res_final = TEE_ERROR_GENERIC;

	memset(key1, 0, sizeof(key1));
	memset(key2, 0, sizeof(key2));

	/*
	 * Testing limits.
	 */
	for (i = 0; i < sizeof(big_key); i++) {
		uint8_t *extra = NULL;
		uint8_t extra_size = 0;

		memset(big_key, 0, sizeof(big_key));

		/* Alternate between using and not using extra data. */
		if (i % 2) {
			extra = extra_key_data;
			extra_size = sizeof(extra_key_data);
		}

		res = derive_unique_key(session, big_key, i, extra, extra_size);
		if (i < TA_DERIVED_KEY_MIN_SIZE) {
			if (res == TEE_SUCCESS) {
				EMSG("Small key test iteration %d failed", i);
				res_final = TEE_ERROR_GENERIC;
				break;
			} else {
				continue;
			}
		}

		if (i > TA_DERIVED_KEY_MAX_SIZE) {
			if (res == TEE_SUCCESS) {
				EMSG("Big key test iteration %d failed", i);
				res_final = TEE_ERROR_GENERIC;
				break;
			} else {
				continue;
			}
		}

		if (res != TEE_SUCCESS) {
			res_final = TEE_ERROR_GENERIC;
			break;
		}
	}
err:
	TEE_CloseTASession(session);

	return res_final;
}
