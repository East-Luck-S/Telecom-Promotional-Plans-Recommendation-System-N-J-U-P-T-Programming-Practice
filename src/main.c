#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "user.h"
#include "system.h"
#include "admin.h"

int main()
{
	printf("应用初始化");
	if (!load_admin_accounts(admins, &adminCount)) {
            printf("初始化失败：未找到管理员账号文件。\n");
    }// 从文件加载管理员账号
	if(!load_packages(allPackages,&pkgCount)){
			printf("初始化失败：未找到套餐数据文件。\n");
	}// 从文件加载套餐
	mainMenu();
	return 0;
}