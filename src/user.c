#include "user.h"
#include "system.h"
#include <math.h>

// 全局变量定义
Package* packageList = NULL;
User* userList = NULL;
int totalUsers = 0;
User* currentUser;

//需求结构体
typedef struct {
    int data_mb;         // 流量需求(MB)
    int voice_minutes;   // 通话需求(分钟)
    int sms;             // 短信需求(条)
    int valid;           // 需求有效性标记(1:有效)
} Demand;
static Demand userDemand = {0, 0, 0, 0};  //全局需求变量

float userPkgMatrix[100][50] = {0}; //用户-套餐交互矩阵（记录用户对套餐的行为评分，1-5分），行：用户索引，列：套餐索引，值：评分（0表示无交互）

//判断字符串是否为空（全空格或空）
int isStrEmpty(const char* str) {
    if (!str) return 1;
    while (*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}

//去除字符串首尾空格
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

//检查用户ID是否已存在
static int isUserIdExists(const char* userId) {
    for (int i = 0; i < totalUsers; i++) {
        if (strcmp(userList[i].userId, userId) == 0) {
            return 1; // 存在
        }
    }
    return 0; // 不存在
}

//检查输入是否为回退指令（q/Q）
static int isQuitInput(const char* input) {
    return (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0);
}

//查找用户（返回指针，未找到返回NULL）
User* findUser(const char* userId) {
    for (int i = 0; i < totalUsers; i++) {
        if (strcmp(userList[i].userId, userId) == 0) {
            return &userList[i];
        }
    }
    return NULL;
}

//交换套餐（排序用）
static void swapPackages(Package* a, Package* b) {
    Package temp = *a;
    *a = *b;
    *b = temp;
}

//从文本文件加载套餐（格式：id,name,monthly_fee,data_mb,voice_minutes,sms,contract_months,start_date,end_date,is_active,description）
int loadPackagesFromText() {
    // 释放旧数据
    if (packageList) {
        free(packageList);
        packageList = NULL;
    }
    pkgCount = 0;

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
        if (!isStrEmpty(line)) pkgCount++;
    }
    rewind(fp);

    // 分配内存
    if (pkgCount > 0) {
        packageList = (Package*)malloc(pkgCount * sizeof(Package));
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

//保存套餐到文本文件
int savePackagesToText() {
    FILE* fp = fopen(PKG_FILE, "w");
    if (!fp) return 0;

    for (int i = 0; i < pkgCount; i++) {
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

//从文本文件加载用户（格式：userId,userPwd,userName,selectedPkg,useYears,totalCost,userStar）
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

//保存用户到文本文件
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

//保存用户需求到文本文件 (格式：userId,data_mb,voice_minutes,sms,valid)
int saveDemandToText(const char* userId) {
    FILE* fp = fopen("user_demand.txt", "a");  // 以追加模式打开文件
    if (fp == NULL) {
        printf("[错误] 打开需求文件失败，无法保存！\n");
        return 0;
    }

    // 按格式：userId,data_mb,voice_minutes,sms,valid 写入文件
    fprintf(fp, "%s,%d,%d,%d,%d\n", 
            userId, 
            userDemand.data_mb, 
            userDemand.voice_minutes, 
            userDemand.sms, 
            userDemand.valid);

    fclose(fp);
    printf("[成功] 用户需求已保存到文件！\n");
    return 1;
}

//用户注册函数
int userRegister() {
    //加载已有用户数据,防止新用户被覆盖掉
    if (!loadUsersFromText()) {
        printf("[错误] 加载用户数据失败，无法注册！\n");
        return 0;
    }

    //定义新用户变量
    char input[20];
    User newUser = {0};
    char password1[20], password2[20];

    //输入并校验用户ID
    while (1) {
        printf("\n===== 用户注册(输入q/Q可取消注册) =====");
        printf("\n请输入用户ID（1-20位，字母/数字）：");
        
        if (scanf("%19s", input) != 1) {
            printf("[错误] 输入格式错误，请重新输入！\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        //回退判断
        if (isQuitInput(input)) {
            printf("[提示] 已取消注册流程！\n");
            return 0;
        }
       
        int idLen = strlen(input);
        if (idLen < 1 || idLen > 20) {
            printf("[错误] 用户ID长度需在1-20位之间！\n");
            continue;
        }

        int valid = 1;
        for (int i = 0; i < idLen; i++) {
            if (!isalnum((unsigned char)input[i])) {
                valid = 0;
                break;
            }
        }
        if (!valid) {
            printf("[错误] 用户ID只能包含字母和数字！\n");
            continue;
        }

        if (isUserIdExists(input)) {
            printf("[错误] 该用户ID已被注册，请更换！\n");
            continue;
        }

        //确认用户ID有效，保存到新用户信息
        strcpy(newUser.userId, input);

        break;
    }

    //输入并校验密码
    while (1) {
        printf("请设置密码（3-16位,输入q/Q可取消注册）：");
        if (scanf("%19s", input) != 1) {
            printf("[错误] 输入格式错误，请重新输入！\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        //检查是否回退
        if (isQuitInput(input)) {
            printf("[提示] 已取消注册流程！\n");
            return 0;
        }

        if (strlen(input) < 3 || strlen(input) > 16) {
            printf("[错误] 密码长度需在3-16位之间！\n");
            continue;
        }

        strcpy(password1, input); // 保存第一次输入的密码

        printf("请确认密码（输入q/Q可取消注册）：");
        if (scanf("%19s", input) != 1) {
            printf("[错误] 输入格式错误，请重新输入！\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

         // 检查确认密码时是否回退
        if (isQuitInput(input)) {
            printf("[提示] 已取消注册流程！\n");
            return 0;
        }

        strcpy(password2, input);

        if (strcmp(password1, password2) != 0) {
            printf("[错误] 两次输入的密码不一致！\n");
            continue;
        }

        strcpy(newUser.userPwd, password1);
        break;
    }

    //输入用户名（支持回退）
    printf("请输入用户名（输入q/Q可取消）：");
    if (scanf("%19s", input) != 1) {
        printf("[错误] 输入格式错误，使用默认用户名！\n");
        strcpy(newUser.userName, "默认用户");
    } else {
        clearInputBuffer();
        //检查是否回退
        if (isQuitInput(input)) {
            printf("[提示] 已取消注册流程！\n");
            return 0;
        }
        strcpy(newUser.userName, input);
    }

    //初始化其他字段
    newUser.selectedPkg[0] = '0';  //未选套餐
    newUser.useYears = 0;           //使用年限0
    newUser.totalCost = 0.0;        //累计消费0
    newUser.userStar = 1;           //初始星级1星

    //扩容用户列表并添加新用户
    User* tempList = (User*)realloc(userList, (totalUsers + 1) * sizeof(User));
    if (!tempList) {
        printf("[错误] 内存分配失败，注册失败！\n");
        return 0;
    }
    userList = tempList;
    userList[totalUsers] = newUser;
    totalUsers++;

    //保存到文件
    if (!saveUsersToText()) {
        printf("[错误] 保存用户数据失败，但注册流程已完成！\n");
        return 0;
    }

    printf("\n[成功] 用户注册完成！用户ID：%s，密码：%s\n", 
           newUser.userId, newUser.userPwd);
    return 1;
}

//用户登录
void loginUser() {
   if(loadUsersFromText() != 1)
   {
     printf("error\n");
     return;
   }

    char id[20], pwd[20];

    printf("\n===== 用户登录 =====\n");
    printf("请输入用户ID: ");
    if (scanf("%19s", id) != 1) {
        printf("[错误] 输入格式错误！\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    printf("请输入密码: ");
    if (scanf("%19s", pwd) != 1) {
        printf("[错误] 输入格式错误！\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    //查找用户
    User* user = findUser(id);
    if (!user) {
        printf("[错误] 用户ID不存在！\n");
        currentUser = NULL;
        return;
    }

    //校验密码
    if (strcmp(user->userPwd, pwd) == 0) {
        currentUser = user;
        printf("[成功] 用户 %s 登录成功！\n", currentUser->userName);
    } else {
        printf("[错误] 密码错误！\n");
        currentUser = NULL;
    }
}

//填写需求调查
void inputDemandByForm() {
    //检查登录状态
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }

    //重置需求状态
    userDemand.valid = 0;

    //输入流量需求（带校验）
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

    //输入通话需求（带校验）
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

    //输入短信需求（带校验）
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

    //调用保存函数，将需求写入文本文件
    if (saveDemandToText(currentUser->userId)) {
        printf("需求已成功保存！\n");
    } else {
        printf("需求保存失败，请重试！\n");
    }
}

//计算用户星级
void calcUserStar() {
    //检查登录状态
    if (currentUser == NULL || strlen(currentUser->userName) == 0) {
        printf("[错误] 请先登录系统！\n");
        return;
    }

    //基础星级（基于累计消费）
    int baseStar = 1;
    if (currentUser->totalCost >= 10000)      baseStar = 5;
    else if (currentUser->totalCost >= 5000)  baseStar = 4;
    else if (currentUser->totalCost >= 2000)  baseStar = 3;
    else if (currentUser->totalCost >= 500)   baseStar = 2;

    //年限加成（最高5星）
    int yearBonus = 0;
    if (currentUser->useYears >= 10)  yearBonus = 2;
    else if (currentUser->useYears >= 5)  yearBonus = 1;

    //最终星级（不超过5星）
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

//优先判断用户是否有套餐
void recommendPackages() {
    if (currentUser == NULL) {
        printf("[错误] 请先登录！\n");
        return;
    }

    // 区分“有套餐用户”和“无套餐用户”
    if (strcmp(currentUser->selectedPkg, "0") == 0) {
        recommendForNewUser();  //新用户推荐
    } else {
        itemBasedCFRecommendation();  //老用户：基于已选套餐的相似推荐
    }
}

//计算套餐匹配得分
static float calcMatchScore(const Package* pkg, const Demand* demand, int userStar) {
    if (!pkg || !demand || !demand->valid) return -1.0f;

    //基础需求不满足直接淘汰
    if (pkg->data_mb < demand->data_mb || 
        pkg->voice_minutes < demand->voice_minutes || 
        pkg->sms < demand->sms) {
        return -1.0f;
    }

    //资源匹配度计算（1.0为最优）
    float dataScore = (demand->data_mb == 0) ? 1.0f : 
                     (1.0f - (pkg->data_mb - demand->data_mb) / (float)(pkg->data_mb + 1));
    float voiceScore = (demand->voice_minutes == 0) ? 1.0f : 
                      (1.0f - (pkg->voice_minutes - demand->voice_minutes) / (float)(pkg->voice_minutes + 1));
    float smsScore = (demand->sms == 0) ? 1.0f : 
                    (1.0f - (pkg->sms - demand->sms) / (float)(pkg->sms + 1));

    //价格优势计算（星级越高，价格敏感度越低）
    float priceScore = 1.0f - (pkg->monthly_fee / (float)(100 + userStar * 20));

    //综合得分（加权求和）
    return dataScore * 0.4 + voiceScore * 0.3 + smsScore * 0.2 + priceScore * 0.1;
}

//根据需求匹配套餐
void matchPackagesByDemand() {
    matchedPkgCount = 0; // 重置匹配结果
    
    // 检查需求是否有效
    if (!userDemand.valid) {
        printf("[错误] 请先填写有效的需求调查！\n");
        return;
    }
    
    // 遍历所有套餐，计算匹配得分
    for (int i = 0; i < pkgCount; i++) {
        float score = calcMatchScore(&packageList[i], &userDemand, currentUser->userStar);
        if (score > 0) { // 得分大于0视为有效匹配
            matchedPackages[matchedPkgCount++] = packageList[i];
        }
    }
}

//针对未选择套餐用户的推荐函数
void recommendForNewUser() {
    matchedPkgCount = 0;  // 重置推荐结果

    //前置检查：用户必须登录且未选择套餐
    if (currentUser == NULL) {
        printf("[错误] 请先登录系统！\n");
        return;
    }
    if (strcmp(currentUser->selectedPkg, "0") != 0) {
        printf("[提示] 该用户已选择套餐，使用常规相似套餐推荐！\n");
        itemBasedCFRecommendation();  //调用原有基于物品的推荐
        return;
    }

    //使用用户填写的需求匹配
    if (userDemand.valid) {  // 若用户填写了需求调查
        printf("\n===== 基于用户需求的推荐（新用户）=====\n");
        matchPackagesByDemand();  
        if (matchedPkgCount > 0) {
            showMatchedPackages();
            return;
        }
    }
    else {
        printf("[提示] 用户未填写需求调查，无法基于需求推荐套餐！\n");
        return;
    }
    showMatchedPackages();
}

//计算两个套餐的属性相似度（基于余弦相似度）
static float calcPackageSimilarity(const Package* pkgA, const Package* pkgB) {
    if (!pkgA || !pkgB) return 0.0f;

    // 提取套餐属性作为向量（月费、流量、通话、短信）
    // 注意：月费需取倒数（价格越低，与"性价比高"的套餐相似度越高）
    float vecA[4] = {
        1.0f / (pkgA->monthly_fee + 1),  // 月费（倒数处理）
        (float)pkgA->data_mb,
        (float)pkgA->voice_minutes,
        (float)pkgA->sms
    };
    float vecB[4] = {
        1.0f / (pkgB->monthly_fee + 1),
        (float)pkgB->data_mb,
        (float)pkgB->voice_minutes,
        (float)pkgB->sms
    };

    // 计算余弦相似度：cosθ = (A·B) / (|A|·|B|)
    float dotProduct = 0.0f;  // 点积
    float normA = 0.0f, normB = 0.0f;  // 模长
    for (int i = 0; i < 4; i++) {
        dotProduct += vecA[i] * vecB[i];
        normA += vecA[i] * vecA[i];
        normB += vecB[i] * vecB[i];
    }
    if (normA == 0 || normB == 0) return 0.0f;  // 避免除零

    return dotProduct / (sqrt(normA) * sqrt(normB));
}

//获取用户对套餐的评分（从交互矩阵中）
static float getUserRatingForPackage(int userIndex, int pkgIndex) {
    if (userIndex < 0 || userIndex >= totalUsers) return 0.0f;
    if (pkgIndex < 0 || pkgIndex >= pkgCount) return 0.0f;
    return userPkgMatrix[userIndex][pkgIndex];
}

//针对已选择套餐用户的推荐函数
void itemBasedCFRecommendation() {
    matchedPkgCount = 0;  // 重置推荐结果

    //前置检查
    if (currentUser == NULL) {
        printf("[错误] 请先登录系统！\n");
        return;
    }
    if (pkgCount == 0) {
        printf("[错误] 暂无套餐数据！\n");
        return;
    }

    //找到当前用户有交互的套餐（评分>0的套餐）
    int userIndex = -1;
    for (int i = 0; i < totalUsers; i++) {
        if (strcmp(userList[i].userId, currentUser->userId) == 0) {
            userIndex = i;
            break;
        }
    }
    if (userIndex == -1) {
        printf("[错误] 用户数据不存在！\n");
        return;
    }

    //收集用户已交互的套餐及评分
    typedef struct {
        int pkgIndex;
        float rating;
    } UserInteraction;
    UserInteraction interactions[10] = {0};  // 最多10个交互套餐
    int interactionCount = 0;

    for (int i = 0; i < pkgCount; i++) {
        float rating = getUserRatingForPackage(userIndex, i);
        if (rating > 0) {
            interactions[interactionCount].pkgIndex = i;
            interactions[interactionCount].rating = rating;
            interactionCount++;
        }
    }

    //如果用户无交互历史，基于已选套餐推荐（若有）
    if (interactionCount == 0) {
        if (strlen(currentUser->selectedPkg) == 0) {
            printf("[提示] 暂无用户交互数据，无法推荐相似套餐！\n");
            return;
        }
        // 将已选套餐作为唯一交互项
        int selectedPkgId = atoi(currentUser->selectedPkg);
        for (int i = 0; i < pkgCount; i++) {
            if (packageList[i].id == selectedPkgId) {
                interactions[0].pkgIndex = i;
                interactions[0].rating = 4.0f;  // 假设对已选套餐评分4分
                interactionCount = 1;
                break;
            }
        }
    }

    //计算待推荐套餐的得分
    float pkgScores[50] = {0};  // 套餐推荐得分
    float similaritySums[50] = {0};  // 相似度总和（用于归一化）

    for (int i = 0; i < interactionCount; i++) {
        int srcPkgIndex = interactions[i].pkgIndex;  // 用户交互过的套餐
        float userRating = interactions[i].rating;   // 用户对该套餐的评分

        //计算该套餐与其他所有套餐的相似度
        for (int j = 0; j < pkgCount; j++) {
            if (j == srcPkgIndex) continue;  // 跳过自身

            float sim = calcPackageSimilarity(&packageList[srcPkgIndex], &packageList[j]);
            if (sim <= 0) continue;  // 只考虑正相似

            // 得分 = 相似度 * 用户评分（累加）
            pkgScores[j] += sim * userRating;
            similaritySums[j] += sim;  // 累计相似度
        }
    }

    //归一化得分并筛选推荐结果（排除用户已交互的套餐）
    for (int j = 0; j < pkgCount; j++) {
        // 跳过用户已交互的套餐
        int isInteracted = 0;
        for (int i = 0; i < interactionCount; i++) {
            if (j == interactions[i].pkgIndex) {
                isInteracted = 1;
                break;
            }
        }
        if (isInteracted) continue;

        // 归一化得分（除以相似度总和）
        if (similaritySums[j] > 0) {
            float finalScore = pkgScores[j] / similaritySums[j];
            if (finalScore >= 2.5f) {  // 得分阈值（可调整）
                matchedPackages[matchedPkgCount++] = packageList[j];
            }
        }
    }

    //按得分排序推荐结果（从高到低）
    for (int i = 0; i < matchedPkgCount - 1; i++) {
        for (int j = 0; j < matchedPkgCount - i - 1; j++) {
            // 重新计算临时得分用于排序
            float scoreJ = 0, scoreJ1 = 0;
            for (int k = 0; k < interactionCount; k++) {
                int srcIdx = interactions[k].pkgIndex;
                scoreJ += calcPackageSimilarity(&packageList[srcIdx], &matchedPackages[j]) * interactions[k].rating;
                scoreJ1 += calcPackageSimilarity(&packageList[srcIdx], &matchedPackages[j+1]) * interactions[k].rating;
            }
            if (scoreJ < scoreJ1) {
                Package temp = matchedPackages[j];
                matchedPackages[j] = matchedPackages[j+1];
                matchedPackages[j+1] = temp;
            }
        }
    }

    printf("\n===== 基于套餐相似性的推荐结果 =====\n");
    showMatchedPackages();  // 复用原显示函数
}

//显示推荐套餐
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

        // 表头
printf("+------+----------------+------------+------------+------------+------------+\n");
printf("|  ID  |    套餐名称     |  月费(元)  |  流量(MB)  | 通话(分钟) |  短信(条)  |\n");
printf("+------+----------------+------------+------------+------------+------------+\n");

// 数据行循环
for (int i = start; i < end; i++) {
    const Package* pkg = &matchedPackages[i];
    char buf[32];
    int len, pad, left, right;

    // ID 居中
    sprintf(buf, "%d", pkg->id);
    len = strlen(buf); pad = 6 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", buf);
    for (int s=0; s<right; s++) printf(" ");

    // 套餐名称居中
    len = strlen(pkg->name); pad = 16 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", pkg->name);
    for (int s=0; s<right; s++) printf(" ");

    // 月费居中
    sprintf(buf, "%.2f", pkg->monthly_fee);
    len = strlen(buf); pad = 12 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", buf);
    for (int s=0; s<right; s++) printf(" ");

    // 流量居中
    sprintf(buf, "%d", pkg->data_mb);
    len = strlen(buf); pad = 12 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", buf);
    for (int s=0; s<right; s++) printf(" ");

    // 通话居中
    sprintf(buf, "%d", pkg->voice_minutes);
    len = strlen(buf); pad = 12 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", buf);
    for (int s=0; s<right; s++) printf(" ");

    // 短信居中
    sprintf(buf, "%d", pkg->sms);
    len = strlen(buf); pad = 12 - len; left = pad/2; right = pad - left;
    printf("|");
    for (int s=0; s<left; s++) printf(" ");
    printf("%s", buf);
    for (int s=0; s<right; s++) printf(" ");
    printf("|\n");   // ✅ 每行最后换行
}

// 表尾
printf("+------+----------------+------------+------------+------------+------------+\n");

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
    printf("用户编号：%s\n", currentUser->userId);
    printf("用户名：%s\n", currentUser->userName);
    printf("已选套餐编号：%s\n", currentUser->selectedPkg);

    // 查找套餐详情
    int found = 0;
    for (int i = 0; i < pkgCount; i++) {
        if (packageList[i].id == atoi(currentUser->selectedPkg)) { // 假设套餐编号为数字ID
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
    strncpy(currentUser->selectedPkg, pkgId, 19);

    // 更新用户列表并保存
    User* user = findUser(currentUser->userId);
    if (user) {
        strncpy(user->selectedPkg, pkgId, 19);
    } 
    if (saveUsersToText()) {
        printf("套餐变更成功！\n");
    } else {
        printf("套餐变更失败！\n");
    }
}