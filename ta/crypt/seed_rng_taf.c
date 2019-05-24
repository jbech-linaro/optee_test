// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2018, Linaro Limited
 */

#include <tee_internal_api.h>

#include "seed_rng_taf.h"

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

TEE_Result derive_ta_unique_key(uint32_t param_types, TEE_Param params[4])
{
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
	uint8_t ta_unique_key1[32] = { };
	uint8_t ta_unique_key2[32] = { };
	uint8_t extra_key_data[] = { "My dummy data" };

	if (param_types != TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE,
					   TEE_PARAM_TYPE_NONE))
		return TEE_ERROR_BAD_PARAMETERS;

	/* Re-use param types for calls to the pTA. */
	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				      TEE_PARAM_TYPE_MEMREF_OUTPUT,
				      TEE_PARAM_TYPE_NONE,
				      TEE_PARAM_TYPE_NONE);


	res = TEE_OpenTASession(&system_uuid, 0, 0, NULL, &session,
				&ret_origin);
	if (res != TEE_SUCCESS)
		return res;

	/*
	 * Test 1 - Successful calls to the pTA and compared that two calls
	 * creates the same keys.
	 */
	params[0].memref.buffer = extra_key_data;
	params[0].memref.size = sizeof(extra_key_data);

	params[1].memref.buffer = ta_unique_key1;
	params[1].memref.size = sizeof(ta_unique_key1);
	res = TEE_InvokeTACommand(session, 0, PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY,
				  param_types, params, &ret_origin);
	if (res != TEE_SUCCESS) {
		EMSG("Failed calling the pTA:PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY");
		goto err;
	}

	params[1].memref.buffer = ta_unique_key2;
	params[1].memref.size = sizeof(ta_unique_key2);
	res = TEE_InvokeTACommand(session, 0, PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY,
				  param_types, params, &ret_origin);
	if (res != TEE_SUCCESS) {
		EMSG("Failed calling the pTA:PTA_SYSTEM_DERIVE_TA_UNIQUE_KEY");
		goto err;
	}

	if (TEE_MemCompare(ta_unique_key1, ta_unique_key2, sizeof(ta_unique_key1) != 0)) {
		EMSG("Generated keys are not equal");
		goto err;
	}

err:
	TEE_CloseTASession(session);
	return res;

}
