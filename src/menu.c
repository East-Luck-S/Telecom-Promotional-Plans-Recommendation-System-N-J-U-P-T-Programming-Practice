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
        printf("\n========================= 系统主菜单 =========================\n");
        printf("                       1. [用户登录]\n");
        printf("                       2. [用户注册]\n");
        printf("                       3. [管理员登录]\n");
        printf("                       4. [退出系统]\n");
        printf("                       请选择操作（1-4）：");
        printf("\n==============================================================\n");

        int choice;
        printf("                       您的选择是：");
        if (scanf("%d", &choice) != 1) {
            printf("                     输入错误！\n");
            clearInputBuffer();
            continue;
        }
        
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                loadPackagesFromText();
                loginUser();
                if (currentUser != NULL && strlen(currentUser->userId) > 0) {
                    userFunctionMenu(); // 进入用户菜单
                } else {
                    printf("                      回到主菜单...\n");
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
                printf("                     程序退出。\n");
                break;
            case 2:
                if (userRegister()) {
                    printf("               请使用新注册的账号登录。\n");
                }
                break;
            default:
                printf("              无效选项，请重新输入！\n");
        }
        if (choice == 4) {
            break;
    }
    }
}

/*管理员菜单*/
void adminMenu(int index) {
    while (1) {
        printf("\n========================= 管理员菜单 =========================\n");
        // 公共选项（普通/超级管理员都可见）
        printf("                      1. [显示所有套餐]\n");
        printf("                      2. [添加全新套餐]\n");
        printf("                      3. [修改当前套餐]\n");
        printf("                      4. [修改用户标签]\n");
        printf("                      5. [用户套餐修改]\n");
        // 超级管理员特有选项
        if (admins[index].is_super) {
            printf("                      6. [管理员账户修改]\n");
            printf("                      7. [返回主菜单]");
            printf("\n==============================================================\n");

        } else {
            printf("                      6. [返回主菜单]");
            printf("\n==============================================================\n");

        }
        printf("                       请选择操作：");
        
        int choice;
        scanf("%d", &choice);
        clearInputBuffer();
        
        // 处理公共选项
        switch (choice) {
            case 1: list_packages(allPackages, pkgCount); break;
            case 2: add_package(allPackages, &pkgCount); break;
            case 3: modify_package(allPackages, pkgCount); break;
            case 4: modify_user_tag_menu(); break;  // 调用带菜单的标签修改函数
            case 5: modify_user_package_menu(); break;  // 调用带菜单的套餐修改函数
            default: break;
        }
        // 处理超级管理员特有选项
        if (admins[index].is_super) {
            if (choice == 6) manage_admins(admins, &adminCount);
            else if (choice == 7) { printf("                      返回主菜单...\n"); return; }
            else if (choice <1 || choice>7) printf("                      无效选项\n");
        } else {
            if (choice == 6) { printf("                      返回主菜单...\n"); return; }
            else if (choice <1 || choice>6) printf("                      无效选项\n");
        }
    }
}

/*用户菜单*/
void userFunctionMenu() {
    while (1) {
        printf("\n========================= 用户功能菜单 =========================\n");
        printf("                      1. [填写需求调查]\n");
        printf("                      2. [查看用户星级]\n");
        printf("                      3. [查看推荐套餐]\n");
        printf("                      4. [查询个人套餐]\n");
        printf("                      5. [变更套餐]\n");
        printf("                      6. [返回主菜单]\n");
        printf("                       请选择操作（1-6）：");
        printf("\n==============================================================\n");
        
        int choice;
        printf("                       您的选择是：");

        if (scanf("%d", &choice) != 1) {
            printf("                      输入错误！\n");
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
                printf("                      返回主菜单...\n");
                return;
            default:
                printf("                      无效选项，请重新输入！\n");
        }
    }
}