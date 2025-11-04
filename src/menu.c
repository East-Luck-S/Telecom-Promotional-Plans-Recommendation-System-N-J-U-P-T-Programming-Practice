#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.c"
#include "admin.c"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}//清空输入缓存区函数
/*主菜单 实现功能：主菜单信息，执行操作通过调用别的函数内的接口*/
void mainMenu() {
    while (1) {
        printf("\n===== 系统主菜单 =====\n");
        printf("1. 用户登录\n");
        printf("2. 管理员登录\n");
        printf("3. 退出系统\n");
        printf("请选择操作（1-3）：");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("输入错误！\n");
            clearInputBuffer();
            continue;
        }
        
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                userLogin();//用户登入函数
                // 登录成功后跳转到用户功能菜单
                if (strlen(currentUser->userId) > 0) {
                    userFunctionMenu();
                }
                break;
            case 2:
                if (adminLogin()) {//管理员账号登入函数
                    adminMenu();
                }
                break;
            case 3:
                exitUserSystem();//返回主菜单函数
                break;
            default:
                printf("无效选项，请重新输入！\n");
        }
    }
}

/*管理员菜单*/
void adminMenu() {
    while (1) {
        printf("\n===== 管理员菜单 =====\n");
        printf("1. 显示所有套餐\n");
        printf("2. 添加新套餐\n");
        printf("3. 删除套餐\n");
        printf("4. 修改套餐\n");
        printf("5. 统计套餐用户数\n");
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
                showAllPackages();//显示所有套餐
                break;
            case 2:
                addPackage();//添加套餐
                break;
            case 3:
                deletePackage();//删除套餐
                break;
            case 4:
                modifyPackage();//修改套餐
                break;
            case 5:
                statPackageUsers();//统计套餐用户数
                break;
            case 6:
                printf("返回主菜单...\n");
                return;
            default:
                printf("无效选项，请重新输入！\n");
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
                {
                    Package demand = inputDemandByForm();
                    matchPackagesByDemand(demand);
                    showMatchedPackages();
                }
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
