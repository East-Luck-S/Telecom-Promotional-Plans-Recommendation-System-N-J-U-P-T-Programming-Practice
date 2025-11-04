#include "user.h"
#include "system.h"

// 全局变量定义
Package* packageList = NULL;
int totalPackages = 0;
User* userList = NULL;
int totalUsers = 0;
User currentUser;

// 清空输入缓冲区
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 判断字符串是否为空（全空格或空）
int isStrEmpty(const char* str) {
    if (!str) return 1;
    while (*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

// 去除字符串首尾空格
void trimStr(char* str) {
    if (!str) return;
    // 去除开头空格
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    // 去除结尾空格
    char* end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    // 重新赋值
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

// 从文本文件加载套餐（格式：id,name,monthly_fee,data_mb,voice_minutes,sms,contract_months,start_date,end_date,is_active,description）
int loadPackagesFromText() {
    // 释放旧数据
    if (packageList) {
        free(packageList);
        packageList = NULL;
    }
    totalPackages = 0;

    FILE* fp = fopen(PKG_FILE, "r");
    if (!fp) {
        // 首次运行创建空文件
        fp = fopen(PKG_FILE, "w");
        fclose(fp);
        return 1;
    }

    // 先统计行数（套餐数量）
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        trimStr(line);
        if (!isStrEmpty(line)) totalPackages++;
    }
    rewind(fp);

    // 分配内存
    if (totalPackages > 0) {
        packageList = (Package*)malloc(totalPackages * sizeof(Package));
        if (!packageList) {
            fclose(fp);
            return 0;
        }

        // 解析每行数据
        int i = 0;
        while (fgets(line, sizeof(line), fp)) {
            trimStr(line);
            if (isStrEmpty(line)) continue;

            // 拆分字段（逗号分隔）
            char* token = strtok(line, ",");
            if (token) packageList[i].id = atoi(token);

            token = strtok(NULL, ",");
            if (token) strncpy(packageList[i].name, token, 63);

            token = strtok(NULL, ",");
            if (token) packageList[i].monthly_fee = atof(token);

            token = strtok(NULL, ",");
            if (token) packageList[i].data_mb = atoi(token);

            token = strtok(NULL, ",");
            if (token) packageList[i].voice_minutes = atoi(token);

            token = strtok(NULL, ",");
            if (token) packageList[i].sms = atoi(token);

            token = strtok(NULL, ",");
            if (token) packageList[i].contract_months = atoi(token);

            token = strtok(NULL, ",");
            if (token) strncpy(packageList[i].start_date, token, 10);

            token = strtok(NULL, ",");
            if (token) strncpy(packageList[i].end_date, token, 10);

            token = strtok(NULL, ",");
            if (token) packageList[i].is_active = atoi(token);

            token = strtok(NULL, ",");
            if (token) strncpy(packageList[i].description, token, 255);

            i++;
        }
    }

    fclose(fp);
    return 1;
}

// 保存套餐到文本文件
int savePackagesToText() {
    FILE* fp = fopen(PKG_FILE, "w");
    if (!fp) return 0;

    for (int i = 0; i < totalPackages; i++) {
        fprintf(fp, "%d,%s,%.2f,%d,%d,%d,%d,%s,%s,%d,%s\n",
                packageList[i].id,
                packageList[i].name,
                packageList[i].monthly_fee,
                packageList[i].data_mb,
                packageList[i].voice_minutes,
                packageList[i].sms,
                packageList[i].contract_months,
                packageList[i].start_date,
                packageList[i].end_date,
                packageList[i].is_active,
                packageList[i].description);
    }

    fclose(fp);
    return 1;
}

// 从文本文件加载用户（格式：userId,userPwd,userName,selectedPkg,useYears,totalCost,userStar）
int loadUsersFromText() {
    // 释放旧数据
    if (userList) {
        free(userList);
        userList = NULL;
    }
    totalUsers = 0;

    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) {
        // 首次运行创建空文件
        fp = fopen(USER_FILE, "w");
        fclose(fp);
        return 1;
    }

    // 先统计行数（用户数量）
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        trimStr(line);
        if (!isStrEmpty(line)) totalUsers++;
    }
    rewind(fp);

    // 分配内存
    if (totalUsers > 0) {
        userList = (User*)malloc(totalUsers * sizeof(User));
        if (!userList) {
            fclose(fp);
            return 0;
        }

        // 解析每行数据
        int i = 0;
        while (fgets(line, sizeof(line), fp)) {
            trimStr(line);
            if (isStrEmpty(line)) continue;

            // 拆分字段（逗号分隔）
            char* token = strtok(line, ",");
            if (token) strncpy(userList[i].userId, token, 19);

            token = strtok(NULL, ",");
            if (token) strncpy(userList[i].userPwd, token, 19);

            token = strtok(NULL, ",");
            if (token) strncpy(userList[i].userName, token, 19);

            token = strtok(NULL, ",");
            if (token) strncpy(userList[i].selectedPkg, token, 19);

            token = strtok(NULL, ",");
            if (token) userList[i].useYears = atoi(token);

            token = strtok(NULL, ",");
            if (token) userList[i].totalCost = atof(token);

            token = strtok(NULL, ",");
            if (token) userList[i].userStar = atoi(token);

            i++;
        }
    }

    fclose(fp);
    return 1;
}

// 保存用户到文本文件
int saveUsersToText() {
    FILE* fp = fopen(USER_FILE, "w");
    if (!fp) return 0;

    for (int i = 0; i < totalUsers; i++) {
        fprintf(fp, "%s,%s,%s,%s,%d,%.2f,%d\n",
                userList[i].userId,
                userList[i].userPwd,
                userList[i].userName,
                userList[i].selectedPkg,
                userList[i].useYears,
                userList[i].totalCost,
                userList[i].userStar);
    }

    fclose(fp);
    return 1;
}

// 查找用户（返回指针，未找到返回NULL）
User* findUser(const char* userId) {
    for (int i = 0; i < totalUsers; i++) {
        if (strcmp(userList[i].userId, userId) == 0) {
            return &userList[i];
        }
    }
    return NULL;
}

// 填写需求调查
void inputDemandByForm() {
    printf("\n===== 填写需求调查 =====\n");
    printf("请输入每月通话需求（分钟）：");
    scanf("%d", &currentUser.callDemand);
    clearInputBuffer();

    printf("请输入每月流量需求（MB）：");
    scanf("%d", &currentUser.flowDemand);
    clearInputBuffer();

    printf("是否需要宽带（1=需要，0=不需要）：");
    scanf("%d", &currentUser.broadbandDemand);
    clearInputBuffer();

    printf("请输入套餐使用年限（年）：");
    scanf("%d", &currentUser.useYears);
    clearInputBuffer();

    printf("请输入累计消费金额（元）：");
    scanf("%f", &currentUser.totalCost);
    clearInputBuffer();

    User* user = findUser(currentUser.userId);
    if (user) {
        user->callDemand = currentUser.callDemand;
        user->flowDemand = currentUser.flowDemand;
        user->broadbandDemand = currentUser.broadbandDemand;
        user->useYears = currentUser.useYears;
        user->totalCost = currentUser.totalCost;
    }
    saveUsersToText();
    printf("需求提交成功！\n");
}

// 计算用户星级
void calcUserStar() {
    int star = 1;
    if (currentUser.totalCost >= 800) star = 5;
    else if (currentUser.totalCost >= 600) star = 4;
    else if (currentUser.totalCost >= 400) star = 3;
    else if (currentUser.totalCost >= 200) star = 2;

    printf("\n===== 用户星级 =====\n");
    printf("用户：%s（%s）\n", currentUser.userName, currentUser.userId);
    printf("累计消费金额：%.2f元\n", currentUser.totalCost);
    printf("星级评定：%d星\n", star);
    currentUser.userStar = star;

    User* user = findUser(currentUser.userId);
    if (user) {
        user->userStar = star;
    }
    saveUsersToText();
}

// 匹配套餐
void matchPackagesByDemand() {
    matchedCount = 0;
    if (totalPackages == 0) {
        printf("系统中暂无套餐数据！\n");
        return;
    }

    for (int i = 0; i < totalPackages; i++) {
        int callMatch = (packageList[i].voice_minutes >= currentUser.callDemand) ? 1 : 0;
        int flowMatch = (packageList[i].data_mb >= currentUser.flowDemand) ? 1 : 0;
        int broadbandMatch = 0;
        if (currentUser.broadbandDemand == 1) {
            broadbandMatch = (packageList[i].broadband > 0) ? 1 : 0;
        } else {
            broadbandMatch = (packageList[i].broadband == 0) ? 1 : 0;
        }

        if (callMatch && flowMatch && broadbandMatch) {
            if (matchedCount < 10) {
                matchedPackages[matchedCount++] = packageList[i];
            }
        }
    }
}

// 显示推荐套餐
void showMatchedPackages() {
    if (matchedCount == 0) {
        printf("\n暂无匹配的套餐，请调整需求后重试！\n");
        return;
    }

    printf("\n===== 推荐套餐列表 =====\n");
    printf("序号\t套餐ID\t套餐名称\t月资费\t通话分钟\t流量(MB)\t宽带(M)\n");
    for (int i = 0; i < matchedCount; i++) {
        printf("%d\t%d\t%s\t%.2f\t%d\t%d\t%d\n",
               i+1,
               matchedPackages[i].id,
               matchedPackages[i].name,
               matchedPackages[i].monthly_fee,
               matchedPackages[i].voice_minutes,
               matchedPackages[i].data_mb,
               matchedPackages[i].broadband);
    }

    int choice;
    printf("请选择套餐序号（1-%d）办理，或输入0返回：", matchedCount);
    scanf("%d", &choice);
    clearInputBuffer();

    if (choice >= 1 && choice <= matchedCount) {
        sprintf(currentUser.selectedPkg, "%d", matchedPackages[choice-1].id);
        User* user = findUser(currentUser.userId);
        if (user) {
            strcpy(user->selectedPkg, currentUser.selectedPkg);
        }
        saveUsersToText();
        printf("套餐办理成功！当前套餐：%s\n", currentUser.selectedPkg);
    } else {
        printf("已取消办理\n");
    }
}

// 查询个人套餐
void queryUserPackage() {
    printf("\n===== 个人套餐信息 =====\n");
    printf("用户编号：%s\n", currentUser.userId);
    printf("用户名：%s\n", currentUser.userName);
    printf("已选套餐编号：%s\n", currentUser.selectedPkg);

    // 查找套餐详情
    int found = 0;
    for (int i = 0; i < totalPackages; i++) {
        if (packageList[i].id == atoi(currentUser.selectedPkg)) { // 假设套餐编号为数字ID
            printf("套餐详情：\n");
            printf("  套餐名称：%s\n", packageList[i].name);
            printf("  月资费：%.2f元\n", packageList[i].monthly_fee);
            printf("  包含流量：%d MB\n", packageList[i].data_mb);
            printf("  包含语音：%d 分钟\n", packageList[i].voice_minutes);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("提示：当前套餐信息未找到（可能已下架）\n");
    }
}

// 申请变更套餐
void applyPackageChange() {
    printf("\n===== 套餐变更申请 =====\n");
    printf("请输入新套餐编号：");
    char pkgId[20];
    scanf("%s", pkgId);
    clearInputBuffer();
    strncpy(currentUser.selectedPkg, pkgId, 19);

    // 更新用户列表并保存
    User* user = findUser(currentUser.userId);
    if (user) {
        strncpy(user->selectedPkg, pkgId, 19);
    }
    if (saveUsersToText()) {
        printf("套餐变更成功！\n");
    } else {
        printf("套餐变更失败！\n");
    }
}

// 用户功能菜单
void userFunctionMenu() {
    // 初始化：加载数据
    if (!loadPackagesFromText() || !loadUsersFromText()) {
        printf("数据加载失败，无法启动用户功能！\n");
        return;
    }

    // 用户登录
    printf("\n===== 用户登录 =====\n");
    char userId[20];
    char userPwd[20];
    printf("请输入用户编号：");
    fgets(userId, sizeof(userId), stdin);
    userId[strcspn(userId, "\n")] = '\0';
    trimStr(userId);

    printf("请输入用户密码：");
    fgets(userPwd, sizeof(userPwd), stdin);
    userPwd[strcspn(userPwd, "\n")] = '\0';
    trimStr(userPwd);

    User* loginUser = NULL;
    for (int i = 0; i < totalUsers; i++) {
        if (strcmp(userList[i].userId, userId) == 0 && strcmp(userList[i].userPwd, userPwd) == 0) {
            loginUser = &userList[i];
            break;
        }
    }

    if (loginUser) {
        currentUser = *loginUser;
        printf("登录成功，欢迎 %s！\n", currentUser.userName);
    } else {
        // 新用户注册
        printf("用户不存在或密码错误，是否注册新用户（y/n）：");
        char reg;
        scanf("%c", &reg);
        clearInputBuffer();
        if (reg != 'y' && reg != 'Y') {
            printf("登录失败，返回主菜单！\n");
            return;
        }

        // 初始化新用户
        strncpy(currentUser.userId, userId, 19);
        strncpy(currentUser.userPwd, userPwd, 19);
        printf("请输入用户名：");
        fgets(currentUser.userName, sizeof(currentUser.userName), stdin);
        currentUser.userName[strcspn(currentUser.userName, "\n")] = '\0';
        trimStr(currentUser.userName);
        strcpy(currentUser.selectedPkg, "未选择");
        currentUser.useYears = 0;
        currentUser.totalCost = 0.0;
        currentUser.userStar = 1;

        // 添加到用户列表
        totalUsers++;
        userList = (User*)realloc(userList, totalUsers * sizeof(User));
        if (!userList) {
            printf("内存分配失败，注册失败！\n");
            totalUsers--;
            return;
        }
        userList[totalUsers - 1] = currentUser;

        // 保存新用户
        if (saveUsersToText()) {
            printf("注册成功，已登录！\n");
        } else {
            printf("注册失败，返回主菜单！\n");
            return;
        }
    }

    // 菜单循环
    while (1) {
        printf("\n===== 用户功能菜单 =====\n");
        printf("1. 录入用户需求（示例）\n");
        printf("2. 查看用户星级\n");
        printf("3. 查看推荐套餐（示例）\n");
        printf("4. 查询个人套餐\n");
        printf("5. 变更套餐\n");
        printf("6. 返回主菜单\n");
        printf("请选择：");

        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("输入错误，请输入数字！\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:
                inputUserDemand();
                break;
            case 2:
                calcUserStar();
                break;
            case 3:
                matchPackagesByDemand();
                showMatchedPackages();
                break;
            case 4:
                queryUserPackage();
                break;
            case 5:
                applyPackageChange();
                break;
            case 6:
                printf("返回主菜单...\n");
                // 释放内存
                if (packageList) free(packageList);
                if (userList) free(userList);
                return;
            default:
                printf("无效选项，请输入1-6！\n");
        }
    }
}