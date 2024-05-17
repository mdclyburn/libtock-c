#ifndef EVALUATION_H
#define EVALUATION_H

void eval_setup(void);

/** Check error code and halt on error.
 */
void eval_check_return_code(const int rc,
                            const char* const note);

void eval_usprng_init(const uint32_t seed);
uint32_t eval_usprng_next(void);

#endif
