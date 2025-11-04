#ifndef SYSTEM_H
#define SYSTEM_H

#define MAX_PACKAGES 100 //最大套餐数量
#define MAX_USERS 100 //最大用户数量

/*套餐结构体*/
typedef struct package
{
    int id;                         // 套餐 ID
    char name[64];                  // 套餐名称，NUL 结尾
    double monthly_fee;             // 月资费（元）
    int data_mb;                    // 每月流量（MB）
    int voice_minutes;              // 语音分钟数
    int sms;                        // 短信条数
    int contract_months;            // 合同月数（0 表示无合约）
    char start_date[11];            // 生效日期，格式 "YYYY-MM-DD"
    char end_date[11];              // 终止日期，格式 "YYYY-MM-DD"（无合约可留空）
    int is_active;                  // 是否启用（0 = 否, 1 = 是）
    char description[256];          // 备注/描述
} Package;

/*用户结构体：包含用户基础信息、套餐使用数据及星级*/
typedef struct User {
    char userId[20];       // 用户ID（与前端登录ID同步）
    char userPwd[20];      // 用户密码（前端校验后同步）
    char userName[20];     // 用户名
    char selectedPkg[20];  // 已选套餐编号
    int useYears;          // 套餐使用年限（年）
    float totalCost;       // 累计消费金额（元）
    int userStar;          // 用户星级（1-5星）
} User;

/*管理员账号结构体*/
typedef struct admin_account
{
	char username[32];
	char password[32];
	int is_super; // 1=全局管理员, 0=普通管理员
} AdminAccount;

//全局变量声明
extern char frontEndBuffer[1024];  // 接收前端数据缓冲区
extern char backEndBuffer[1024];   // 发送前端数据缓冲区
extern int pkgCount;               // 总套餐数量
extern int userCount;              // 用户数量
extern Package allPackages[MAX_PACKAGES];    // 存储管理员维护的所有套餐
extern User currentUser[MAX_USERS];           // 当前登录用户信息
extern int matchedPkgCount;        // 匹配到的套餐数量
extern Package matchedPackages[20];// 匹配结果列表
static const char *PKG_FILE = "packages.txt";// 套餐数据文件
static const char *USER_FILE = "users.txt";// 用户数据文件
static const char *ADMIN_FILE = "admins.txt";// 管理员数据文件

#endif /* SYSTEM_H */