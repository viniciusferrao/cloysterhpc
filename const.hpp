/* Copyright to be added
 * Description to be added
 * Authores to be added
 */

#ifndef _H_CONST_
#define _H_CONST_

#if 0 /* Port to C++ */
extern const char* const ibStacks[];
extern const char* const queueSystems[];
extern const char* const pbsDefaultPlace[];
extern const char* const postfixProfiles[];

const char* const ibStacks[] = {
    "None",
    "Inbox",
    "Mellanox",
    NULL
};

const char* const queueSystems[] = {
    "None",
    "SLURM",
    "PBS Professional",
    NULL
};

const char* const pbsDefaultPlace[] = {
    "Shared",
    "Scatter",
    NULL
};

const char* const postfixProfiles[] = {
    "Local",
    "Relay",
    "SASL",
    NULL
};
#endif

#endif /* _H_CONST_ */
