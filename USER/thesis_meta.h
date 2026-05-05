/**
 * @file thesis_meta.h
 * @brief 毕设「门面信息」集中配置。
 *
 * 注意（Keil ARMCC5 容易踩坑）：
 * 含中文的字符串若直接写在宏里，一旦文件编码/引号不是纯 ASCII，会报 #8 missing closing quote。
 * 下面中文统一用 UTF-8 字节的 \x 转义，和 ui_AboutPage 里 lv_label 的写法一致，编译最稳。
 */
#ifndef THESIS_META_H
#define THESIS_META_H

/* 姓名：谢志坚 */
#define THESIS_AUTHOR_NAME \
  "\xe8\xb0\xa2\xe5\xbf\x97\xe5\x9d\x9a"

#define THESIS_AUTHOR_STUDENT_ID  "B20220302130"

/* 题目：低功耗智能手表的设计与实现（当前若未在界面使用，可仅作论文/文档引用） */
#define THESIS_PROJECT_TAGLINE \
  "\xe4\xbd\x8e\xe5\x8a\x9f\xe8\x80\x97\xe6\x99\xba\xe8\x83\xbd\xe6\x89\x8b\xe8\xa1\xa8\xe7\x9a\x84\xe8\xae\xbe\xe8\xae\xa1\xe4\xb8\x8e\xe5\xae\x9e\xe7\x8e\xb0"

/* 长沙学院毕设·智能腕表（中间点为 U+00B7，UTF-8 C2 B7）；论文/其它文案仍可用 */
#define THESIS_PRODUCT_NAME \
  "\xe9\x95\xbf\xe6\xb2\x99\xe5\xad\xa6\xe9\x99\xa2\xe6\xaf\x95\xe8\xae\xbe\xc2\xb7\xe6\x99\xba\xe8\x83\xbd\xe8\x85\x95\xe8\xa1\xa8"

/* 关于页「手表型号」一行 */
#define THESIS_WATCH_MODEL \
  "\xe6\xaf\x95\xe4\xb8\x9a\xe8\xae\xbe\xe8\xae\xa1"

/* 关于页「软件开发者 / 界面设计者」：字形在 ui_font_thesis_about20 */

/* 需要「姓名 + 学号」拼接时可用（例如其它界面） */
#define THESIS_AUTHOR_AND_ID \
  THESIS_AUTHOR_NAME "  " THESIS_AUTHOR_STUDENT_ID

#define THESIS_CREDIT_SW          THESIS_AUTHOR_NAME
#define THESIS_CREDIT_UI          THESIS_AUTHOR_NAME

#endif /* THESIS_META_H */
