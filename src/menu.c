#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "admin.h"
#include "menu.h"
#include "system.h"

/*主菜单 实现功能：主菜单信息，执行操作通过调用别的函数内的接口*/
void mainMenu() {
    while (1) {
        printf("\n===== 系统主菜单 =====\n");
        printf("1. 用户登录\n");
        printf("2. 用户注册\n");
        printf("3. 管理员登录\n");
        printf("4. 退出系统\n");
        printf("请选择操作（1-4）：");

        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("输入错误！\n");
            clearInputBuffer();
            continue;
        }
        
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                loadPackagesFromText();
                loginUser();//用户登入函数
                // 登录成功后跳转到用户功能菜单
                if (strlen(currentUser->userId) > 0) {
                    userFunctionMenu();
                }
                break;
            case 3:{
                int login_idx = -1;
                if (admin_login(admins, adminCount, &login_idx)) {
                    adminMenu(login_idx);
                }
                break;
            }
            case 4:
                printf("程序退出。\n");
                break;
            case 2:
                if (userRegister()) {
                    printf("请使用新注册的账号登录。\n");
                }
                break;
            default:
                printf("无效选项，请重新输入！\n");
        }
        if (choice == 4) {
            break;
    }
    }
}

/*管理员菜单*/
void adminMenu(int index) {
    while (1) {
        printf("\n===== 管理员菜单 =====\n");
        // 公共选项（普通/超级管理员都可见）
        printf("1. 显示所有套餐\n");
        printf("2. 添加新套餐\n");
        printf("3. 修改套餐\n");
        // 超级管理员特有选项
        if (admins[index].is_super) {
            printf("4. 管理员账户修改\n");
            printf("5. 返回主菜单\n");
        } else {
            printf("4. 返回主菜单\n");
        }
        printf("请选择操作：");
        
        int choice;
        scanf("%d", &choice);
        clearInputBuffer();
        
        // 处理公共选项
        switch (choice) {
            case 1: list_packages(allPackages, pkgCount); break;
            case 2: add_package(allPackages, &pkgCount); break;
            case 3: modify_package(allPackages, pkgCount); break;
            default: break;
        }
        // 处理超级管理员特有选项
        if (admins[index].is_super) {
            if (choice == 4) manage_admins(admins, &adminCount);
            else if (choice == 5) { printf("返回主菜单...\n"); return; }
            else if (choice <1 || choice>5) printf("无效选项\n");
        } else {
            if (choice == 4) { printf("返回主菜单...\n"); return; }
            else if (choice <1 || choice>4) printf("无效选项\n");
        }
    }
}

/*用户菜单*/
void userFunctionMenu() {
    while (1) {
        printf("\n===== 用户功能菜单 =====\n");
        printf("1. 填写需求调查\n");
        printf("2. 查看用户星级\n");
        printf("3. 查看推荐套餐\n");
        printf("4. 查询个人套餐\n");
        printf("5. 变更套餐\n");
        printf("6. 返回主菜单\n");
        printf("请选择操作（1-6）：");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("输入错误！\n");
            clearInputBuffer();
            continue;
        }
        
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                inputDemandByForm();//填写调查需求
                break;
            case 2:
                calcUserStar();//查看客户星级
                break;
            case 3:
                recommendPackages();//查看推荐套餐
                break;
            case 4:
                queryUserPackage();//查询套餐
                break;
            case 5:
                applyPackageChange();//变更套餐
                break;
            case 6:
                printf("返回主菜单...\n");
                return;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}