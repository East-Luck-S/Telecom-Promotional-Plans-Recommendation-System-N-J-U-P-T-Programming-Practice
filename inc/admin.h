#ifndef ADMIN_H
#define ADMIN_H

#include <stdio.h>
#include "user.h"
#include "system.h"

#define KEYWORD_LEN 10

// 用户标签文件路径
#define USER_TAG_FILE "user_tags.txt"

typedef struct user_tag {
	int user_id;
	char tags[128]; // 逗号分隔的标签，如“重度流量,低通话,学生”
} UserTag;

extern char* tag_buf;

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

/* 用户需求信息分区 */
// 在 admin.h 中确保已有以下声明
int load_user_tags(UserTag tags[], int *count);
int save_user_tags(UserTag tags[], int count);
void generate_user_tag(User* user, Demand* demand, char* tag_buf);// 为用户生成标签
void show_user_tag(int user_id);
void match_packages_by_tag(int user_id, Package matched[], int* matched_count);
int modify_user_tag(int user_id, const char* new_tags);
void admin_modify_user_package();
void modify_user_tag_menu();       // 带菜单的用户标签修改
void modify_user_package_menu();   // 带菜单的用户套餐修改
#endif /* ADMIN_H */

