#ifndef ADMIN_H
#define ADMIN_H

#include <stdio.h>
#include "user.h"
#include "system.h"

#define KEYWORD_LEN 10

// 用户历史消费数据文件路径
#define USER_HISTORY_FILE "user_history.txt"
#define USER_TAG_FILE "user_tags.txt"

// 用户需求与行为分区
typedef struct user_history {
	int user_id;
	double monthly_fees[12]; // 最近12个月月消费
	int data_usage[12];      // 最近12个月流量使用（MB）
	int voice_usage[12];     // 最近12个月语音分钟
	int sms_usage[12];       // 最近12个月短信条数
} UserHistory;

typedef struct user_tag {
	int user_id;
	char tags[128]; // 逗号分隔的标签，如“重度流量,低通话,学生”
} UserTag;


/* 管理员相关函数 */
int load_admin_accounts(AdminAccount admins[], int *count);// 从文件加载管理员账号
void read_password(char *buf, int buflen); // 从控制台读取密码并用 '*' 代替显示
int save_admin_accounts(AdminAccount admins[], int count);// 保存管理员账号到文件
int admin_login(AdminAccount admins[], int count, int *login_idx);// 管理员登录
void manage_admins(AdminAccount admins[], int *count); // 仅全局管理员可用

/* 套餐管理 */
int load_packages(Package pkgs[], int *count);// 从文件加载套餐
int save_packages(Package pkgs[], int count);// 保存套餐到文件
void add_package(Package pkgs[], int *count);// 添加新套餐
void modify_package(Package pkgs[], int count);// 修改套餐
void list_packages(Package pkgs[], int count);// 列出所有套餐

/* 用户需求信息分区（动态管理） */
UserHistory* load_user_history(int *count); // 动态读取所有用户历史消费数据，返回指针和条数
int save_user_history(UserHistory *histories, int count); // 保存所有用户历史消费数据到文件
UserTag* load_user_tags(int *count); // 动态读取所有用户标签数据，返回指针和条数
int save_user_tags(UserTag *tags, int count); // 保存所有用户标签数据到文件
void tag_user(int user_id, const char *tag); // 给指定用户添加标签（如“重度流量”），自动合并
void list_user_tags(int user_id); // 列出指定用户的所有标签
void analyze_user_behavior(int user_id); // 分析指定用户的历史行为，自动生成标签或画像

#endif /* ADMIN_H */

