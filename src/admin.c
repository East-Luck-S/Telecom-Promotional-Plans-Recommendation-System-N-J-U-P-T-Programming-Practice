#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h> /* getch() 用于密码输入遮掩，实现无回显输入读取 */

#include "admin.h"

int pkgCount=0;			   // 总套餐数量  
int adminCount = 0;		 // 管理员数量

// 新增：检查输入是否为回退指令（q/Q）
static int isQuitInput(const char* input) {
    return (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0);
}

/*以下为对管理员信息的管理*/
// 从文件加载管理员账号
int load_admin_accounts(AdminAccount admins[], int *count)
{
	FILE *fp = fopen(ADMIN_FILE, "r");
	if (!fp) { *count = 0; return 0; }
	int idx = 0;
	char line[128];
	while (fgets(line, sizeof(line), fp))
	{
		if (line[0] == '#' || strlen(line) < 3) continue;
		char *u = strtok(line, ",\n");
		char *p = strtok(NULL, ",\n");
		char *s = strtok(NULL, ",\n");
		if (u && p)
		{
			strncpy(admins[idx].username, u, sizeof(admins[idx].username));
			strncpy(admins[idx].password, p, sizeof(admins[idx].password));
			admins[idx].is_super = (s && strcmp(s, "super") == 0) ? 1 : 0;
			idx++;
			if (idx >= 32) break;
		}
	}
	fclose(fp);
	*count = idx;
	return idx > 0;
}
//管理员信息注册保存到文件
int save_admin_accounts(AdminAccount admins[], int count)
{
	FILE *fp = fopen(ADMIN_FILE, "w");
	if (!fp) return 0;//打开失败，返回0.
	fprintf(fp, "# 用户名,密码,类型(super可选)\n");
	for (int i = 0; i < count; i++)
	{
		fprintf(fp, "%s,%s,%s\n", admins[i].username, admins[i].password, admins[i].is_super ? "super" : "normal");
	}
	fclose(fp);
	return 1;//执行成功，返回1.
}
//从控制台读取密码并用 '*' 代替显示，返回已填充的缓冲区（ caller 提供 buf ）
void read_password(char *buf, int buflen)
{
	int idx = 0;
	char ch;
	while (1)
	{
		ch = getch();
		if (ch == '\r' || ch == '\n')
		{
			putchar('\n');
			break;
		}
		else if (ch == '\b')
		{
			if (idx > 0)
			{
				idx--;
				/* 退格并清除 */
				putchar('\b');
				putchar(' ');
				putchar('\b');
			}
		}
		else if (idx < buflen - 1)
		{
			buf[idx++] = ch;
			putchar('*');
		}
	}
	buf[idx] = '\0';
}
// 管理员登录
int admin_login(AdminAccount admins[], int count, int *login_idx)
{
	char username[32], password[32];
	printf("管理员用户名: ");
	if (scanf("%31s", username) != 1) return 0;
	getchar();
	printf("密码: ");
	read_password(password, sizeof(password));
	for (int i = 0; i < count; i++)
	{
		if (strcmp(admins[i].username, username) == 0 && strcmp(admins[i].password, password) == 0)
		{
			*login_idx = i;
			printf("登录成功，类型: %s\n", admins[i].is_super ? "全局管理员" : "普通管理员");
			return 1;
		}
	}
	printf("用户名或密码错误。\n");
	return 0;
}
// 管理员批量管理（仅全局管理员可用）
void manage_admins(AdminAccount admins[], int *count) {
    while (1) {
        printf("\n--- 管理员批量管理 ---\n");
        printf("1. 列出所有管理员\n2. 添加管理员\n3. 删除管理员\n4. 修改管理员密码\n0. 返回\n选择: ");
        char cmdInput[16];
        if (fgets(cmdInput, sizeof(cmdInput), stdin) == NULL) {
            printf("输入错误，返回上一级...\n");
            clearInputBuffer();
            return;
        }
        cmdInput[strcspn(cmdInput, "\n")] = '\0';
        if (isQuitInput(cmdInput)) {  // 支持q/Q直接返回
            printf("返回上一级菜单...\n");
            return;
        }
        int cmd = atoi(cmdInput);
        if (cmd < 0 || cmd > 4) {
            printf("无效选项，请重新输入！\n");
            continue;
        }

        switch (cmd) {
            case 0:
                return;
            case 1:
                printf("\n所有管理员：\n");
                for (int i = 0; i < *count; i++) {
                    printf("[%d] 用户名:%s 类型:%s\n", i+1, admins[i].username, admins[i].is_super ? "全局" : "普通");
                }
                break;
            case 2: {
                if (*count >= 32) { printf("管理员数量已达上限。\n"); break; }
                AdminAccount newa;
                char input[32];

                // 输入用户名（支持取消）
                printf("输入新管理员用户名 (输入q/Q取消): ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    printf("输入错误，取消添加...\n");
                    clearInputBuffer();
                    break;
                }
                input[strcspn(input, "\n")] = '\0';
                if (isQuitInput(input)) {
                    printf("取消添加管理员...\n");
                    break;
                }
                strncpy(newa.username, input, sizeof(newa.username)-1);

                // 输入密码（支持取消）
                printf("输入新管理员密码 (输入q/Q取消): ");
                read_password(input, sizeof(input));  // 复用现有密码输入函数
                if (isQuitInput(input)) {
                    printf("取消添加管理员...\n");
                    break;
                }
                strncpy(newa.password, input, sizeof(newa.password)-1);

                // 输入类型（支持取消）
                printf("类型 (1=全局, 0=普通, 输入q/Q取消): ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    printf("输入错误，取消添加...\n");
                    clearInputBuffer();
                    break;
                }
                input[strcspn(input, "\n")] = '\0';
                if (isQuitInput(input)) {
                    printf("取消添加管理员...\n");
                    break;
                }
                newa.is_super = atoi(input);

                // 确认添加
                admins[*count] = newa;
                (*count)++;
                save_admin_accounts(admins, *count);
                printf("添加成功。\n");
                break;
            }
            case 3: {
                char uname[32];
                printf("输入要删除的管理员用户名 (输入q/Q取消): ");
                if (fgets(uname, sizeof(uname), stdin) == NULL) {
                    printf("输入错误，取消删除...\n");
                    clearInputBuffer();
                    break;
                }
                uname[strcspn(uname, "\n")] = '\0';
                if (isQuitInput(uname)) {
                    printf("取消删除管理员...\n");
                    break;
                }

                int found = -1;
                for (int i = 0; i < *count; i++) {
                    if (strcmp(admins[i].username, uname) == 0) { found = i; break; }
                }
                if (found == -1) { printf("未找到该管理员。\n"); break; }

                // 二次确认（防止误操作）
                printf("确定要删除管理员 %s 吗？(y=确认, 其他=取消): ", uname);
                char confirm[8];
                fgets(confirm, sizeof(confirm), stdin);
                confirm[strcspn(confirm, "\n")] = '\0';
                if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0) {
                    printf("已取消删除...\n");
                    break;
                }

                // 执行删除
                for (int i = found; i < *count-1; i++) admins[i] = admins[i+1];
                (*count)--;
                save_admin_accounts(admins, *count);
                printf("删除成功。\n");
                break;
            }
            case 4: {
                char uname2[32];
                printf("输入要修改密码的管理员用户名 (输入q/Q取消): ");
                if (fgets(uname2, sizeof(uname2), stdin) == NULL) {
                    printf("输入错误，取消修改...\n");
                    clearInputBuffer();
                    break;
                }
                uname2[strcspn(uname2, "\n")] = '\0';
                if (isQuitInput(uname2)) {
                    printf("取消修改密码...\n");
                    break;
                }

                int found2 = -1;
                for (int i = 0; i < *count; i++) {
                    if (strcmp(admins[i].username, uname2) == 0) { found2 = i; break; }
                }
                if (found2 == -1) { printf("未找到该管理员。\n"); break; }

                // 输入新密码（支持取消）
                printf("输入新密码 (输入q/Q取消): ");
                char newPwd[32];
                read_password(newPwd, sizeof(newPwd));
                if (isQuitInput(newPwd)) {
                    printf("取消修改密码...\n");
                    break;
                }
                strncpy(admins[found2].password, newPwd, sizeof(admins[found2].password)-1);

                save_admin_accounts(admins, *count);
                printf("密码修改成功。\n");
                break;
            }
            default:
                printf("无效选项。\n");
                break;
        }
    }
}

/*以下为对套餐信息的录入和修改*/
// 从文本文件加载套餐数据
int load_packages(Package pkgs[], int *count) {
    FILE *fp = fopen(PKG_FILE, "r");  // 文本模式读取
    if (!fp) {
        *count = 0;
        return 0;
    }

	*count = 0;
    // 读取第一行的套餐总数
    char line[512];

    // 检查数量是否超过上限
    if (*count > MAX_PACKAGES) {
        *count = MAX_PACKAGES;
    }

    while (*count < MAX_PACKAGES && fgets(line, sizeof(line), fp)) {
        // 跳过空行（去除首尾空格后为空）
        trimStr(line);  // 使用user.c中定义的trimStr函数去除空格
        if (isStrEmpty(line)) {  // 使用user.c中定义的isStrEmpty判断空行
            continue;
        }

        // 去除行尾换行符（防止解析时包含换行符）
        line[strcspn(line, "\n")] = '\0';

        // 按逗号分割字段
        char *token = strtok(line, ",");
        if (token) pkgs[*count].id = atoi(token);  // 套餐ID

        token = strtok(NULL, ",");
        if (token) strncpy(pkgs[*count].name, token, sizeof(pkgs[*count].name) - 1);  // 名称

        token = strtok(NULL, ",");
        if (token) pkgs[*count].monthly_fee = atof(token);  // 月费

        token = strtok(NULL, ",");
        if (token) pkgs[*count].data_mb = atoi(token);  // 流量

        token = strtok(NULL, ",");
        if (token) pkgs[*count].voice_minutes = atoi(token);  // 语音分钟

        token = strtok(NULL, ",");
        if (token) pkgs[*count].sms = atoi(token);  // 短信条数

        token = strtok(NULL, ",");
        if (token) pkgs[*count].contract_months = atoi(token);  // 合约月数

        token = strtok(NULL, ",");
        if (token) strncpy(pkgs[*count].start_date, token, sizeof(pkgs[*count].start_date) - 1);  // 生效日期

        token = strtok(NULL, ",");
        if (token) strncpy(pkgs[*count].end_date, token, sizeof(pkgs[*count].end_date) - 1);  // 终止日期

        token = strtok(NULL, ",");
        if (token) pkgs[*count].is_active = atoi(token);  // 是否启用

        token = strtok(NULL, ",");
        if (token) strncpy(pkgs[*count].description, token, sizeof(pkgs[*count].description) - 1);  // 描述

        (*count)++;  // 成功解析一行，数量+1
    }

    fclose(fp);
    return 1;
}
// 套餐数据保存为文本文件
int save_packages(Package pkgs[], int count) {
    FILE *fp = fopen(PKG_FILE, "w");  // 文本模式写入
    if (!fp) return 0;

    // 循环写入每个套餐的字段（逗号分隔）
    for (int i = 0; i < count; i++) {
        Package *p = &pkgs[i];
        // 注意：字符串字段（如name、start_date等）若包含逗号会导致解析错误，这里假设无逗号
        fprintf(fp, "%d,%s,%.2f,%d,%d,%d,%d,%s,%s,%d,%s\n",
                p->id,
                p->name,
                p->monthly_fee,
                p->data_mb,
                p->voice_minutes,
                p->sms,
                p->contract_months,
                p->start_date,
                p->end_date,
                p->is_active,
                p->description);
    }

    fclose(fp);
    return 1;
}
// 列出所有套餐
void list_packages(Package pkgs[], int count) {
	printf("\n当前套餐列表（%d）:\n", count);
	for (int i = 0; i < count; i++) {
		Package *p = &pkgs[i];
		printf("ID:%d 名称:%s 资费:%.2f元 流量:%dMB 通话:%d分 短信:%d 合约:%d月 启用:%d\n备注:%s\n",
			p->id, p->name, p->monthly_fee, p->data_mb, p->voice_minutes, p->sms, p->contract_months, p->is_active, p->description);
	}
}
// 添加新套餐
void add_package(Package pkgs[], int *count) {
    if (*count >= MAX_PACKAGES) {
        printf("已达到最大套餐数 %d，无法添加。\n", MAX_PACKAGES);
        return;
    }

    Package tempPkg;  // 临时存储，避免中途取消污染原数组
    tempPkg.id = (*count == 0) ? 1 : (pkgs[*count - 1].id + 1);
    char input[256];  // 通用输入缓冲区

    // 输入套餐名称（支持取消）
    printf("输入套餐名称 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.name, input, sizeof(tempPkg.name)-1);

    // 输入月资费（支持取消）
    printf("月资费(元) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.monthly_fee = atof(input);

    // 输入每月流量（支持取消）
    printf("每月流量(MB) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.data_mb = atoi(input);

    // 输入语音分钟数（支持取消）
    printf("语音分钟数 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.voice_minutes = atoi(input);

    // 输入短信条数（支持取消）
    printf("短信条数 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.sms = atoi(input);

    // 输入合约月数（支持取消）
    printf("合约月数(0表示无合约) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.contract_months = atoi(input);

    // 输入生效日期（支持取消）
    printf("生效日期 (YYYY-MM-DD) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.start_date, input, sizeof(tempPkg.start_date)-1);

    // 输入终止日期（支持取消）
    printf("终止日期 (YYYY-MM-DD, 可空) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.end_date, input, sizeof(tempPkg.end_date)-1);

    // 输入是否启用（支持取消）
    printf("是否启用 (1=是,0=否) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    tempPkg.is_active = atoi(input);

    // 输入备注（支持取消）
    printf("备注 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.description, input, sizeof(tempPkg.description)-1);

    // 所有输入完成，确认添加
    pkgs[*count] = tempPkg;
    (*count)++;
    save_packages(pkgs, *count);
    printf("添加成功（ID=%d）。\n", tempPkg.id);
}
// 修改或删除套餐
void modify_package(Package pkgs[], int count) {
    if (count == 0) {
        printf("没有套餐可供修改。\n");
        return;
    }

    char input[64];
    int id;

    // 输入要修改的套餐ID（支持取消）
    printf("输入要修改的套餐 ID (输入q/Q返回上一级): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，返回上一级...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("返回上一级菜单...\n");
        return;
    }
    id = atoi(input);
    if (id <= 0) {
        printf("无效的套餐ID！\n");
        return;
    }

    // 查找套餐
    int found = -1;
    for (int i = 0; i < count; i++) {
        if (pkgs[i].id == id) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        printf("未找到ID为 %d 的套餐！\n", id);
        return;
    }

    Package *p = &pkgs[found];
    printf("\n当前套餐信息：\n");
    printf("ID: %d\n名称: %s\n月资费: %.2f元\n每月流量: %dMB\n"
           "语音分钟数: %d\n短信条数: %d\n合约月数: %d\n"
           "生效日期: %s\n终止日期: %s\n是否启用: %d（1=是，0=否）\n备注: %s\n",
           p->id, p->name, p->monthly_fee, p->data_mb, p->voice_minutes,
           p->sms, p->contract_months, p->start_date, p->end_date,
           p->is_active, p->description);

    // 选择操作（修改/删除，支持取消）
    printf("\n请选择操作：1-修改  2-删除  (输入q/Q返回上一级): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("输入错误，返回上一级...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("返回上一级菜单...\n");
        return;
    }
    int op = atoi(input);
    if (op != 1 && op != 2) {
        printf("无效操作！\n");
        return;
    }

    if (op == 1) {  // 修改套餐
        printf("\n===== 开始修改套餐（输入q/Q可跳过当前项保留原值）=====\n");

        // 修改名称
        printf("输入新名称（原：%s）: ", p->name);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原名称...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {  // 非退出且非空才更新
                strncpy(p->name, input, sizeof(p->name)-1);
            }
        }

        // 修改月资费
        printf("输入新月资费（元，原：%.2f）: ", p->monthly_fee);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原资费...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->monthly_fee = atof(input);
            }
        }

        // 其他字段（流量、语音、短信等）采用相同逻辑，支持q/Q跳过
        printf("输入新每月流量（MB，原：%d）: ", p->data_mb);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原流量...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->data_mb = atoi(input);
            }
        }

        printf("输入新语音分钟数（原：%d）: ", p->voice_minutes);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原语音分钟数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->voice_minutes = atoi(input);
            }
        }

        printf("输入新短信条数（原：%d）: ", p->sms);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原短信条数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->sms = atoi(input);
            }
        }

        printf("输入新合约月数（0表示无合约，原：%d）: ", p->contract_months);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原合约月数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->contract_months = atoi(input);
            }
        }

        printf("输入新生效日期（YYYY-MM-DD，原：%s）: ", p->start_date);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原生效日期...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->start_date, input, sizeof(p->start_date)-1);
            }
        }

        printf("输入新终止日期（YYYY-MM-DD，原：%s）: ", p->end_date);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原终止日期...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->end_date, input, sizeof(p->end_date)-1);
            }
        }

        printf("是否启用（1=是，0=否，原：%d）: ", p->is_active);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原状态...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->is_active = atoi(input);
            }
        }

        printf("输入新备注（原：%s）: ", p->description);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("输入错误，保留原备注...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->description, input, sizeof(p->description)-1);
            }
        }

        save_packages(pkgs, count);
        printf("套餐修改成功！\n");
    } else {  // 删除套餐（支持二次确认）
        printf("确定要删除ID为 %d 的套餐吗？(y=确认, 其他=取消): ", id);
        char confirm[8];
        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = '\0';
        if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0) {
            printf("已取消删除...\n");
            return;
        }

        // 执行删除
        for (int i = found; i < count - 1; i++) {
            pkgs[i] = pkgs[i + 1];
        }
        pkgCount--;
        save_packages(pkgs, pkgCount);
        printf("套餐 ID=%d 已删除！\n", id);
    }
}


/*以下为对用户所享受套餐政策，优惠的管理*/



// 读取所有用户历史消费数据，填充 histories 数组，返回实际条数
int load_user_history(UserHistory histories[], int *count) {
	FILE *fp = fopen(USER_HISTORY_FILE, "r");
	if (!fp) { *count = 0; return 0; }
	int idx = 0;
	while (!feof(fp)) {
		UserHistory h;
		// 读取用户ID
		int n = fscanf(fp, "%d", &h.user_id);
		if (n != 1) break;
		// 读取12个月的月消费
		for (int i = 0; i < 12; i++) fscanf(fp, "%lf", &h.monthly_fees[i]);
		// 读取12个月的流量使用
		for (int i = 0; i < 12; i++) fscanf(fp, "%d", &h.data_usage[i]);
		// 读取12个月的语音分钟
		for (int i = 0; i < 12; i++) fscanf(fp, "%d", &h.voice_usage[i]);
		// 读取12个月的短信条数
		for (int i = 0; i < 12; i++) fscanf(fp, "%d", &h.sms_usage[i]);
		// 存入数组
		histories[idx++] = h;
		if (idx >= 128) break;
	}
	fclose(fp);
	*count = idx;
	return idx;
}

// 保存所有用户历史消费数据到文件
int save_user_history(UserHistory histories[], int count) {
	FILE *fp = fopen(USER_HISTORY_FILE, "w");
	if (!fp) return 0;
	for (int i = 0; i < count; i++) {
		UserHistory *h = &histories[i];
		// 写入用户ID
		fprintf(fp, "%d ", h->user_id);
		// 写入12个月的月消费
		for (int j = 0; j < 12; j++) fprintf(fp, "%lf ", h->monthly_fees[j]);
		// 写入12个月的流量使用
		for (int j = 0; j < 12; j++) fprintf(fp, "%d ", h->data_usage[j]);
		// 写入12个月的语音分钟
		for (int j = 0; j < 12; j++) fprintf(fp, "%d ", h->voice_usage[j]);
		// 写入12个月的短信条数
		for (int j = 0; j < 12; j++) fprintf(fp, "%d ", h->sms_usage[j]);
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 1;
}

// 读取所有用户标签数据，填充 tags 数组，返回实际条数
int load_user_tags(UserTag tags[], int *count) {
	FILE *fp = fopen(USER_TAG_FILE, "r");
	if (!fp) { *count = 0; return 0; }
	int idx = 0;
	while (!feof(fp)) {
		UserTag t;
		// 读取用户ID和标签字符串（逗号分隔）
		int n = fscanf(fp, "%d,%127[^\n]\n", &t.user_id, t.tags);
		if (n != 2) break;
		tags[idx++] = t;
		if (idx >= 128) break;
	}
	fclose(fp);
	*count = idx;
	return idx;
}

// 保存所有用户标签数据到文件
int save_user_tags(UserTag tags[], int count) {
	FILE *fp = fopen(USER_TAG_FILE, "w");
	if (!fp) return 0;
	for (int i = 0; i < count; i++) {
		// 写入用户ID和标签字符串
		fprintf(fp, "%d,%s\n", tags[i].user_id, tags[i].tags);
	}
	fclose(fp);
	return 1;
}

// 给指定用户添加标签（如“重度流量”），自动合并，避免重复
void tag_user(int user_id, const char *tag) {
	UserTag tags[128]; int count = 0;
	load_user_tags(tags, &count);
	int found = -1;
	// 查找用户标签记录
	for (int i = 0; i < count; i++) {
		if (tags[i].user_id == user_id) { found = i; break; }
	}
	if (found == -1) {
		// 用户无标签，新建一条
		tags[count].user_id = user_id;
		strncpy(tags[count].tags, tag, sizeof(tags[count].tags));
		count++;
	} else {
		// 用户已有标签，若未包含则追加
		if (strstr(tags[found].tags, tag) == NULL) {
			if (strlen(tags[found].tags) + strlen(tag) + 2 < sizeof(tags[found].tags)) {
				strcat(tags[found].tags, ",");
				strcat(tags[found].tags, tag);
			}
		}
	}
	// 保存更新后的标签
	save_user_tags(tags, count);
}

// 列出指定用户的所有标签
void list_user_tags(int user_id) {
	UserTag tags[128]; int count = 0;
	load_user_tags(tags, &count);
	for (int i = 0; i < count; i++) {
		if (tags[i].user_id == user_id) {
			printf("用户ID:%d 标签:%s\n", user_id, tags[i].tags);
			return;
		}
	}
	printf("用户ID:%d 暂无标签\n", user_id);
}

// 分析指定用户的历史行为，自动生成标签画像
void analyze_user_behavior(int user_id) {
	UserHistory histories[128]; int hcount = 0;
	load_user_history(histories, &hcount);
	int idx = -1;
	// 查找目标用户历史数据
	for (int i = 0; i < hcount; i++) {
		if (histories[i].user_id == user_id) { idx = i; break; }
	}
	if (idx == -1) { printf("未找到用户历史数据\n"); return; }
	UserHistory *h = &histories[idx];
	double avg_data = 0, avg_voice = 0, avg_sms = 0;
	// 计算12个月平均流量、通话、短信
	for (int i = 0; i < 12; i++) {
		avg_data += h->data_usage[i];
		avg_voice += h->voice_usage[i];
		avg_sms += h->sms_usage[i];
	}
	avg_data /= 12; avg_voice /= 12; avg_sms /= 12;
	// 根据阈值自动打标签
	if (avg_data > 5000) tag_user(user_id, "重度流量");
	else if (avg_data > 2000) tag_user(user_id, "中度流量");
	else tag_user(user_id, "低流量");
	if (avg_voice > 300) tag_user(user_id, "重度通话");
	else if (avg_voice > 100) tag_user(user_id, "中度通话");
	else tag_user(user_id, "低通话");
	if (avg_sms > 100) tag_user(user_id, "重度短信");
	else if (avg_sms > 30) tag_user(user_id, "中度短信");
	else tag_user(user_id, "低短信");
	printf("已为用户ID:%d 生成标签画像。\n", user_id);
}

