#include "select-demo.h"
 
/* Enable entropy for devices with TRNG. This means entropy is disabled for all other targets. */
/* Do **NOT** deploy this code in production on other targets! */
/* See https://tls.mbed.org/kb/how-to/add-entropy-sources-to-entropy-pool */
#if defined(DEVICE_TRNG)
#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#undef MBEDTLS_TEST_NULL_ENTROPY
#endif
 