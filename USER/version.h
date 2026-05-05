#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thesis_meta.h"

/*
 * 版本号怎么读（毕设里写进论文也说得过去）：
 * - MAJOR：大改版，比如换 MCU、换屏驱、任务架构重写；
 * - MINOR：加功能模块、改外设驱动接口；
 * - PATCH：修 bug、调参、改文案，不动整体结构。
 */
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
