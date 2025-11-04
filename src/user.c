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
// 需求结构体（需在system.h中声明为extern）
typedef struct {
    int data_mb;         // 流量需求(MB)
    int voice_minutes;   // 通话需求(分钟)
    int sms;             // 短信需求(条)
    int valid;           // 需求有效性标记(1:有效)
} Demand;
static Demand userDemand = {0, 0, 0, 0};  // 全局需求变量

// 1. 填写需求调查
void inputDemandByForm() {
    // 检查登录状态
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }

    // 重置需求状态
    userDemand.valid = 0;

    // 输入流量需求（带校验）
    printf("\n===== 需求调查 =====");
    while (1) {
        printf("\n请输入每月流量需求(MB，0-10000)：");
        if (scanf("%d", &userDemand.data_mb) != 1) {
            printf("[错误] 输入格式错误，请重新输入数字！");
            clearInputBuffer();
            continue;
        }
        if (userDemand.data_mb >= 0 && userDemand.data_mb <= 10000) break;
        printf("[错误] 流量范围需在0-10000MB之间！");
    }
    clearInputBuffer();

    // 输入通话需求（带校验）
    while (1) {
        printf("请输入每月通话时长需求(分钟，0-3000)：");
        if (scanf("%d", &userDemand.voice_minutes) != 1) {
            printf("[错误] 输入格式错误，请重新输入数字！");
            clearInputBuffer();
            continue;
        }
        if (userDemand.voice_minutes >= 0 && userDemand.voice_minutes <= 3000) break;
        printf("[错误] 通话时长需在0-3000分钟之间！");
    }
    clearInputBuffer();

    // 输入短信需求（带校验）
    while (1) {
        printf("请输入每月短信需求(条，0-1000)：");
        if (scanf("%d", &userDemand.sms) != 1) {
            printf("[错误] 输入格式错误，请重新输入数字！");
            clearInputBuffer();
            continue;
        }
        if (userDemand.sms >= 0 && userDemand.sms <= 1000) break;
        printf("[错误] 短信数量需在0-1000条之间！");
    }
    clearInputBuffer();

    userDemand.valid = 1;
    printf("\n[成功] 需求调查已保存！\n");
}

// 2. 计算用户星级
void calcUserStar() {
    // 检查登录状态
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }

    // 基础星级（基于累计消费）
    int baseStar = 1;
    if (currentUser->totalCost >= 10000)      baseStar = 5;
    else if (currentUser->totalCost >= 5000)  baseStar = 4;
    else if (currentUser->totalCost >= 2000)  baseStar = 3;
    else if (currentUser->totalCost >= 500)   baseStar = 2;

    // 年限加成（最高5星）
    int yearBonus = 0;
    if (currentUser->useYears >= 10)  yearBonus = 2;
    else if (currentUser->useYears >= 5)  yearBonus = 1;

    // 最终星级（不超过5星）
    int finalStar = baseStar + yearBonus;
    finalStar = (finalStar > 5) ? 5 : finalStar;

    // 更新用户星级
    currentUser->userStar = finalStar;

    // 输出结果
    printf("\n[星级计算结果]\n");
    printf("累计消费：%.2f元 → 基础星级：%d星\n", currentUser->totalCost, baseStar);
    printf("使用年限：%d年 → 年限加成：%d星\n", currentUser->useYears, yearBonus);
    printf("当前星级：%d星\n", finalStar);
}

// 工具函数：计算套餐匹配得分
static float calcMatchScore(const Package* pkg, const Demand* demand, int userStar) {
    if (!pkg || !demand || !demand->valid) return -1.0f;

    // 基础需求不满足直接淘汰
    if (pkg->data_mb < demand->data_mb || 
        pkg->voice_minutes < demand->voice_minutes || 
        pkg->sms < demand->sms) {
        return -1.0f;
    }

    // 资源匹配度计算（1.0为最优）
    float dataScore = (demand->data_mb == 0) ? 1.0f : 
                     (1.0f - (pkg->data_mb - demand->data_mb) / (float)(pkg->data_mb + 1));
    float voiceScore = (demand->voice_minutes == 0) ? 1.0f : 
                      (1.0f - (pkg->voice_minutes - demand->voice_minutes) / (float)(pkg->voice_minutes + 1));
    float smsScore = (demand->sms == 0) ? 1.0f : 
                    (1.0f - (pkg->sms - demand->sms) / (float)(pkg->sms + 1));

    // 价格优势计算（星级越高，价格敏感度越低）
    float priceScore = 1.0f - (pkg->monthly_fee / (float)(100 + userStar * 20));

    // 综合得分（加权求和）
    return dataScore * 0.4 + voiceScore * 0.3 + smsScore * 0.2 + priceScore * 0.1;
}

// 工具函数：交换套餐（排序用）
static void swapPackages(Package* a, Package* b) {
    Package temp = *a;
    *a = *b;
    *b = temp;
}

// 3. 匹配套餐
void matchPackagesByDemand() {
    // 前置条件检查
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }
    if (!userDemand.valid) {
        printf("[错误] 请先填写需求调查！\n");
        return;
    }
    if (totalPackages <= 0 || !allPackages) {
        printf("[错误] 系统无套餐数据，请联系管理员！\n");
        return;
    }

    // 重置匹配结果
    matchedPkgCount = 0;
    memset(matchedPackages, 0, sizeof(matchedPackages));

    // 筛选有效套餐并计算得分
    for (int i = 0; i < totalPackages; i++) {
        const Package* pkg = &allPackages[i];
        if (!pkg->is_active || pkg->is_deleted) continue;  // 跳过无效套餐

        float score = calcMatchScore(pkg, &userDemand, currentUser->userStar);
        if (score <= 0) continue;  // 过滤不匹配套餐

        // 保存匹配结果（最多20个）
        if (matchedPkgCount < 20) {
            matchedPackages[matchedPkgCount] = *pkg;
            matchedPackages[matchedPkgCount].match_score = score;
            matchedPkgCount++;
        }
    }

    // 按匹配度降序排序
    for (int i = 0; i < matchedPkgCount - 1; i++) {
        for (int j = 0; j < matchedPkgCount - i - 1; j++) {
            if (matchedPackages[j].match_score < matchedPackages[j+1].match_score) {
                swapPackages(&matchedPackages[j], &matchedPackages[j+1]);
            }
        }
    }

    printf("\n[匹配完成] 共找到 %d 个符合需求的套餐\n", matchedPkgCount);
}

// 4. 显示推荐套餐
void showMatchedPackages() {
    // 前置条件检查
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }
    if (matchedPkgCount == 0) {
        printf("[提示] 暂无匹配套餐，建议降低需求或联系客服！\n");
        return;
    }

    // 分页显示（每页5条）
    int pageSize = 5;
    int totalPages = (matchedPkgCount + pageSize - 1) / pageSize;
    int currentPage = 1;
    char input[10];

    while (1) {
        int start = (currentPage - 1) * pageSize;
        int end = (currentPage * pageSize < matchedPkgCount) ? currentPage * pageSize : matchedPkgCount;

        // 打印当前页套餐
        printf("\n===== 推荐套餐（%d星用户）- 第%d/%d页 =====\n", 
               currentUser->userStar, currentPage, totalPages);
        printf("+----+----------------+----------+----------+----------+----------+------------+\n");
        printf("| ID | 套餐名称       | 月费(元) | 流量(MB) | 通话(分钟)| 短信(条) | 匹配度(%%)  |\n");
        printf("+----+----------------+----------+----------+----------+----------+------------+\n");
        
        for (int i = start; i < end; i++) {
            const Package* pkg = &matchedPackages[i];
            printf("| %2d | %-14s | %8.2f | %8d | %8d | %8d | %8.1f |\n",
                   pkg->id, pkg->name, pkg->monthly_fee,
                   pkg->data_mb, pkg->voice_minutes, pkg->sms,
                   pkg->match_score * 100);
        }
        
        printf("+----+----------------+----------+----------+----------+----------+------------+\n");

        // 分页控制
        if (totalPages <= 1) break;
        printf("\n输入页码(1-%d)或q退出：", totalPages);
        fgets(input, sizeof(input), stdin);
        if (input[0] == 'q' || input[0] == 'Q') break;
        
        int page = atoi(input);
        if (page >= 1 && page <= totalPages) currentPage = page;
        else printf("无效页码，请重新输入！\n");
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