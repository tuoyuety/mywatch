#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thesis_meta.h"

/* 我：主版本 / 次版本 / 小修补 */
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_INFO  "thesis-build"

#define VERSION_AUTHOR_ENG_NAME    "Thesis-Author"
#define VERSION_AUTHOR_CN_NAME     THESIS_AUTHOR_NAME
#define VERSION_PROJECT_LINK       "https://example.com/thesis-placeholder"

static inline int watch_version_major(void)
{
    return VERSION_MAJOR;
}

static inline int watch_version_minor(void)
{
    return VERSION_MINOR;
}

static inline int watch_version_patch(void)
{
    return VERSION_PATCH;
}

static inline const char *watch_version_info(void)
{
    return VERSION_INFO;
}

#ifdef __cplusplus
}
#endif

#endif
