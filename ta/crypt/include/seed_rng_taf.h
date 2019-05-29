/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2018, Linaro Limited
 */

#ifndef SEED_RNG_TAF_H
#define SEED_RNG_TAF_H

#include <pta_system.h>

TEE_Result seed_rng_pool(uint32_t param_types, TEE_Param params[4]);
TEE_Result derive_ta_unique_key_test(uint32_t param_types,
				     TEE_Param params[4]);
TEE_Result derive_ta_unique_key_test_shm(uint32_t param_types,
					 TEE_Param params[4]);
#endif /* SEED_RNG_TAF_H */
