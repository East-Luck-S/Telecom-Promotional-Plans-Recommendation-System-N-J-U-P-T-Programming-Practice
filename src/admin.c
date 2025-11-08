#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h> /* getch() 用于密码输入遮掩，实现无回显输入读取 */

#include "admin.h"
#include "user.h"

int pkgCount=0;			   // 总套餐数量  
int adminCount = 0;		 // 管理员数量
char* tag_buf = NULL;
// 检查输入是否为回退指令（q/Q）
static int isQuitInput(const char* input) {
    return (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0);
}

/*以下为对管理员信息的管理*/
// 从文件加载管理员账号
int load_admin_accounts(AdminAccount admins[], int *count)
{
    if (tag_buf == NULL) {
        tag_buf = (char*)malloc(128 * sizeof(char));  // 大小匹配 UserTag.tags
        if (tag_buf == NULL) {
            fprintf(stderr, "内存分配失败：tag_buf\n");
            exit(1);
        }
    }

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
	printf("                    管理员用户名: ");
	if (scanf("%31s", username) != 1) return 0;
	getchar();
	printf("                      密码: ");
	read_password(password, sizeof(password));
	for (int i = 0; i < count; i++)
	{
		if (strcmp(admins[i].username, username) == 0 && strcmp(admins[i].password, password) == 0)
		{
			*login_idx = i;
			printf("                    登录成功，类型: %s\n", admins[i].is_super ? "全局管理员" : "普通管理员");
			return 1;
		}
	}
	printf("                     用户名或密码错误。\n");
	return 0;
}
// 管理员批量管理（仅全局管理员可用）
void manage_admins(AdminAccount admins[], int *count) {
    while (1) {
        printf("\n---====================== 管理员批量管理 -======================--\n");
        printf("                        1. 列出所有管理员\n");
        printf("                            2.添加管理员\n");
        printf("                            3.删除管理员\n");
        printf("                        4. 修改管理员密码\n");
        printf("                              0. 返回\n");
        printf("                               选择: ");
        char cmdInput[16];
        if (fgets(cmdInput, sizeof(cmdInput), stdin) == NULL) {
            printf("                        输入错误，返回上一级...\n");
            clearInputBuffer();
            return;
        }
        cmdInput[strcspn(cmdInput, "\n")] = '\0';
        if (isQuitInput(cmdInput)) {  // 支持q/Q直接返回
            printf("                          返回上一级菜单...\n");
            return;
        }
        int cmd = atoi(cmdInput);
        if (cmd < 0 || cmd > 4) {
            printf("                          无效选项，请重新输入！\n");
            continue;
        }

        switch (cmd) {
            case 0:
                return;
            case 1:
                printf("\n                        所有管理员：\n");
                for (int i = 0; i < *count; i++) {
                    printf("                   [%d] 用户名:%s 类型:%s\n", i+1, admins[i].username, admins[i].is_super ? "全局" : "普通");
                }
                break;
            case 2: {
                if (*count >= 32) { printf("                                管理员数量已达上限。\n"); break; }
                AdminAccount newa;
                char input[32];

                // 输入用户名（支持取消）
                printf("                        输入新管理员用户名 (输入q/Q取消): ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    printf("                              输入错误，取消添加...\n");
                    clearInputBuffer();
                    break;
                }
                input[strcspn(input, "\n")] = '\0';
                if (isQuitInput(input)) {
                    printf("                        取消添加管理员...\n");
                    break;
                }
                strncpy(newa.username, input, sizeof(newa.username)-1);

                // 输入密码（支持取消）
                printf("                            输入新管理员密码 (输入q/Q取消): ");
                read_password(input, sizeof(input));  // 复用现有密码输入函数
                if (isQuitInput(input)) {
                    printf("                                取消添加管理员...\n");
                    break;
                }
                strncpy(newa.password, input, sizeof(newa.password)-1);

                // 输入类型（支持取消）
                printf("                          类型 (1=全局, 0=普通, 输入q/Q取消): ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    printf("                                输入错误，取消添加...\n");
                    clearInputBuffer();
                    break;
                }
                input[strcspn(input, "\n")] = '\0';
                if (isQuitInput(input)) {
                    printf("                                  取消添加管理员...\n");
                    break;
                }
                newa.is_super = atoi(input);

                // 确认添加
                admins[*count] = newa;
                (*count)++;
                save_admin_accounts(admins, *count);
                printf("                                        添加成功。\n");
                break;
            }
            case 3: {
                char uname[32];
                printf("                            输入要删除的管理员用户名 (输入q/Q取消): ");
                if (fgets(uname, sizeof(uname), stdin) == NULL) {
                    printf("                                    输入错误，取消删除...\n");
                    clearInputBuffer();
                    break;
                }
                uname[strcspn(uname, "\n")] = '\0';
                if (isQuitInput(uname)) {
                    printf("                                    取消删除管理员...\n");
                    break;
                }

                int found = -1;
                for (int i = 0; i < *count; i++) {
                    if (strcmp(admins[i].username, uname) == 0) { found = i; break; }
                }
                if (found == -1) { printf("                             未找到该管理员。\n"); break; }

                // 二次确认（防止误操作）
                printf("                            确定要删除管理员 %s 吗？(y=确认, 其他=取消): ", uname);
                char confirm[8];
                fgets(confirm, sizeof(confirm), stdin);
                confirm[strcspn(confirm, "\n")] = '\0';
                if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0) {
                    printf("                                    已取消删除...\n");
                    break;
                }

                // 执行删除
                for (int i = found; i < *count-1; i++) admins[i] = admins[i+1];
                (*count)--;
                save_admin_accounts(admins, *count);
                printf("                                        删除成功。\n");
                break;
            }
            case 4: {
                char uname2[32];
                printf("                        输入要修改密码的管理员用户名 (输入q/Q取消): ");
                if (fgets(uname2, sizeof(uname2), stdin) == NULL) {
                    printf("                                    输入错误，取消修改...\n");
                    clearInputBuffer();
                    break;
                }
                uname2[strcspn(uname2, "\n")] = '\0';
                if (isQuitInput(uname2)) {
                    printf("                                    取消修改密码...\n");
                    break;
                }

                int found2 = -1;
                for (int i = 0; i < *count; i++) {
                    if (strcmp(admins[i].username, uname2) == 0) { found2 = i; break; }
                }
                if (found2 == -1) { printf("                                    未找到该管理员。\n"); break; }

                // 输入新密码（支持取消）
                printf("                                    输入新密码 (输入q/Q取消): ");
                char newPwd[32];
                read_password(newPwd, sizeof(newPwd));
                if (isQuitInput(newPwd)) {
                    printf("                                    取消修改密码...\n");
                    break;
                }
                strncpy(admins[found2].password, newPwd, sizeof(admins[found2].password)-1);

                save_admin_accounts(admins, *count);
                printf("                                        密码修改成功。\n");
                break;
            }
            default:
                printf("                                        无效选项。\n");
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
	printf("\n                          当前套餐列表（%d）:\n", count);
	for (int i = 0; i < count; i++) {
		Package *p = &pkgs[i];
		printf("ID:%d 名称:%s 资费:%.2f元 流量:%dMB 通话:%d分 短信:%d 合约:%d月 启用:%d\n备注:%s\n",
			p->id, p->name, p->monthly_fee, p->data_mb, p->voice_minutes, p->sms, p->contract_months, p->is_active, p->description);
	}
}
// 添加新套餐
void add_package(Package pkgs[], int *count) {
    if (*count >= MAX_PACKAGES) {
        printf("            已达到最大套餐数 %d，无法添加。\n", MAX_PACKAGES);
        return;
    }

    Package tempPkg;  // 临时存储，避免中途取消污染原数组
    tempPkg.id = (*count == 0) ? 1 : (pkgs[*count - 1].id + 1);
    char input[256];  // 通用输入缓冲区

    // 输入套餐名称（支持取消）
    printf("                       输入套餐名称 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                          输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.name, input, sizeof(tempPkg.name)-1);

    // 输入月资费（支持取消）
    printf("                         月资费(元) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                       取消添加套餐...\n");
        return;
    }
    tempPkg.monthly_fee = atof(input);

    // 输入每月流量（支持取消）
    printf("                         每月流量(MB) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    tempPkg.data_mb = atoi(input);

    // 输入语音分钟数（支持取消）
    printf("                          语音分钟数 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    tempPkg.voice_minutes = atoi(input);

    // 输入短信条数（支持取消）
    printf("                           短信条数 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                          取消添加套餐...\n");
        return;
    }
    tempPkg.sms = atoi(input);

    // 输入合约月数（支持取消）
    printf("                          合约月数(0表示无合约) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                          输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                          取消添加套餐...\n");
        return;
    }
    tempPkg.contract_months = atoi(input);

    // 输入生效日期（支持取消）
    printf("                      生效日期 (YYYY-MM-DD) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                          输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                          取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.start_date, input, sizeof(tempPkg.start_date)-1);

    // 输入终止日期（支持取消）
    printf("                      终止日期 (YYYY-MM-DD, 可空) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.end_date, input, sizeof(tempPkg.end_date)-1);

    // 输入是否启用（支持取消）
    printf("                      是否启用 (1=是,0=否) (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    tempPkg.is_active = atoi(input);

    // 输入备注（支持取消）
    printf("                       备注 (输入q/Q取消添加): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                         输入错误，取消添加...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                         取消添加套餐...\n");
        return;
    }
    strncpy(tempPkg.description, input, sizeof(tempPkg.description)-1);

    // 所有输入完成，确认添加
    pkgs[*count] = tempPkg;
    (*count)++;
    save_packages(pkgs, *count);
    printf("                       添加成功（ID=%d）。\n", tempPkg.id);
}
// 修改或删除套餐
void modify_package(Package pkgs[], int count) {
    if (count == 0) {
        printf("                         没有套餐可供修改。\n");
        return;
    }

    char input[64];
    int id;

    // 输入要修改的套餐ID（支持取消）
    printf("                  输入要修改的套餐 ID (输入q/Q返回上一级): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("              输入错误，返回上一级...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("               返回上一级菜单...\n");
        return;
    }
    id = atoi(input);
    if (id <= 0) {
        printf("               无效的套餐ID！\n");
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
        printf("                未找到ID为 %d 的套餐！\n", id);
        return;
    }

    Package *p = &pkgs[found];
    printf("\n                  当前套餐信息：\n");
    printf("                  ID: %d\n                  名称: %s\n                  月资费: %.2f元\n                  每月流量: %dMB\n"
           "                  语音分钟数: %d\n                  短信条数: %d\n                  合约月数: %d\n"
           "                  生效日期: %s\n                  终止日期: %s\n                  是否启用: %d（1=是，0=否）\n                  备注: %s\n",
           p->id, p->name, p->monthly_fee, p->data_mb, p->voice_minutes,
           p->sms, p->contract_months, p->start_date, p->end_date,
           p->is_active, p->description);

    // 选择操作（修改/删除，支持取消）
    printf("\n                   请选择操作：1-修改  2-删除  (输入q/Q返回上一级): ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("                            输入错误，返回上一级...\n");
        clearInputBuffer();
        return;
    }
    input[strcspn(input, "\n")] = '\0';
    if (isQuitInput(input)) {
        printf("                            返回上一级菜单...\n");
        return;
    }
    int op = atoi(input);
    if (op != 1 && op != 2) {
        printf("                            无效操作！\n");
        return;
    }

    if (op == 1) {  // 修改套餐
        printf("\n========================== 开始修改套餐（输入q/Q可跳过当前项保留原值）==========================\n");

        // 修改名称
        printf("                                输入新名称（原：%s）: ", p->name);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                                输入错误，保留原名称...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {  // 非退出且非空才更新
                strncpy(p->name, input, sizeof(p->name)-1);
            }
        }

        // 修改月资费
        printf("                                输入新月资费（元，原：%.2f）: ", p->monthly_fee);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                              输入错误，保留原资费...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->monthly_fee = atof(input);
            }
        }

        // 其他字段（流量、语音、短信等）采用相同逻辑，支持q/Q跳过
        printf("                                输入新每月流量（MB，原：%d）: ", p->data_mb);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                                输入错误，保留原流量...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->data_mb = atoi(input);
            }
        }

        printf("                                输入新语音分钟数（原：%d）: ", p->voice_minutes);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                                输入错误，保留原语音分钟数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->voice_minutes = atoi(input);
            }
        }

        printf("                                输入新短信条数（原：%d）: ", p->sms);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                                输入错误，保留原短信条数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->sms = atoi(input);
            }
        }

        printf("                            输入新合约月数（0表示无合约，原：%d）: ", p->contract_months);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                            输入错误，保留原合约月数...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->contract_months = atoi(input);
            }
        }

        printf("                            输入新生效日期（YYYY-MM-DD，原：%s）: ", p->start_date);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                            输入错误，保留原生效日期...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->start_date, input, sizeof(p->start_date)-1);
            }
        }

        printf("                            输入新终止日期（YYYY-MM-DD，原：%s）: ", p->end_date);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                            输入错误，保留原终止日期...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->end_date, input, sizeof(p->end_date)-1);
            }
        }

        printf("                            是否启用（1=是，0=否，原：%d）: ", p->is_active);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                            输入错误，保留原状态...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                p->is_active = atoi(input);
            }
        }

        printf("                            输入新备注（原：%s）: ", p->description);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("                                输入错误，保留原备注...\n");
        } else {
            input[strcspn(input, "\n")] = '\0';
            if (!isQuitInput(input) && strlen(input) > 0) {
                strncpy(p->description, input, sizeof(p->description)-1);
            }
        }

        save_packages(pkgs, count);
        printf("                                套餐修改成功！\n");
    } else {  // 删除套餐（支持二次确认）
        printf("                    确定要删除ID为 %d 的套餐吗？(y=确认, 其他=取消): ", id);
        char confirm[8];
        fgets(confirm, sizeof(confirm), stdin);
        confirm[strcspn(confirm, "\n")] = '\0';
        if (strcmp(confirm, "y") != 0 && strcmp(confirm, "Y") != 0) {
            printf("                            已取消删除...\n");
            return;
        }

        // 执行删除
        for (int i = found; i < count - 1; i++) {
            pkgs[i] = pkgs[i + 1];
        }
        pkgCount--;
        save_packages(pkgs, pkgCount);
        printf("                            套餐 ID=%d 已删除！\n", id);
    }
}


/*以下为对用户所享受套餐政策，优惠的管理*/
int load_user_tags(UserTag tags[], int* count) {
    *count = 0; // 初始化计数
    FILE* fp = fopen(USER_TAG_FILE, "r");
    if (!fp) {
        perror("                        打开标签文件失败");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        trimStr(line); // 调用user.c中的trimStr去除首尾空格
        if (isStrEmpty(line)) continue; // 跳过空行

        // 解析格式：user_id,标签1,标签2...
        char* token = strtok(line, ",");
        if (!token) continue; // 格式错误，跳过该行

        // 提取用户ID
        int uid = atoi(token);
        // 提取标签（剩余部分）
        char* tags_str = strtok(NULL, "\n"); // 标签部分可能包含逗号，直接取到行尾
        if (!tags_str) continue;

        // 存入tags数组
        tags[*count].user_id = uid;
        strncpy(tags[*count].tags, tags_str, sizeof(tags[*count].tags) - 1);
        tags[*count].tags[sizeof(tags[*count].tags) - 1] = '\0';
        (*count)++;

        if (*count >= 100) break; // 不超过数组最大容量
    }

    fclose(fp);
    return 1;
}
// 保存用户标签数据
int save_user_tags(UserTag tags[], int count) {
    FILE* fp = fopen(USER_TAG_FILE, "w"); // 关键：用"w"模式覆盖原文件
    if (!fp) {
        perror("                        打开标签文件失败");
        return 0;
    }

    // 按格式写入所有标签（与user_tags.txt格式一致：user_id,标签1,标签2...）
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d,%s\n", tags[i].user_id, tags[i].tags);
    }

    fclose(fp);
    return 1;
}
//为所有用户添加标签并保存
void generate_user_tag(User* user, Demand* demand, char* tag_buf) {
    memset(tag_buf, 0, 128);  // 清空缓冲区
    // 基于用户星级
    if (user->userStar >= 4) {
        strcat(tag_buf, "高星级,");
    } else if (user->userStar >= 2) {
        strcat(tag_buf, "中星级,");
    } else {
        strcat(tag_buf, "低星级,");
    }

    // 基于使用年限
    if (user->useYears >= 5) {
        strcat(tag_buf, "老用户,");
    } else if (user->useYears >= 1) {
        strcat(tag_buf, "忠诚用户,");
    } else {
        strcat(tag_buf, "新用户,");
    }

    // 基于需求（从user_demand.txt读取）
    if (demand && demand->valid) {
        if (demand->data_mb >= 10000) {
            strcat(tag_buf, "大流量,");
        } else if (demand->data_mb >= 2000) {
            strcat(tag_buf, "中流量,");
        } else {
            strcat(tag_buf, "小流量,");
        }

        if (demand->voice_minutes >= 1000) {
            strcat(tag_buf, "高通话,");
        } else if (demand->voice_minutes >= 300) {
            strcat(tag_buf, "中通话,");
        } else {
            strcat(tag_buf, "低通话,");
        }
    }

    // 去除末尾逗号
    if (tag_buf[strlen(tag_buf)-1] == ',') {
        tag_buf[strlen(tag_buf)-1] = '\0';
    }
}
void show_user_tag(int user_id) {
    UserTag tags[100];
    int tag_count;
    if (!load_user_tags(tags, &tag_count)) {
        printf("                        未找到用户标签数据\n");
        return;
    }

    for (int i = 0; i < tag_count; i++) {
        if (tags[i].user_id == user_id) {
            printf("                        用户ID:%d 的标签: %s\n", user_id, tags[i].tags);
            return;
        }
    }
    printf("                        未找到用户ID:%d 的标签\n", user_id);
}
// 标签匹配逻辑（基于实际标签类型：星级/用户类型/流量/通话）
void match_packages_by_tag(int user_id, Package matched[], int* matched_count) {
    UserTag tags[100];  // 存储所有用户标签
    int total_tags = 0;
    *matched_count = 0;  // 初始化匹配计数

    // 加载用户标签文件数据
    if (!load_user_tags(tags, &total_tags)) {
        printf("                        用户标签数据加载失败\n");
        return;
    }

    // 提取目标用户的标签字符串（如"中星级,忠诚用户,小流量,中通话"）
    char user_tags[128] = "";
    for (int i = 0; i < total_tags; i++) {
        if (tags[i].user_id == user_id) {
            strncpy(user_tags, tags[i].tags, sizeof(user_tags) - 1);
            break;
        }
    }
    if (strlen(user_tags) == 0) {
        printf("                        未找到用户ID:%d的标签\n", user_id);
        return;
    }

    // 遍历所有套餐，进行标签匹配
    for (int i = 0; i < pkgCount; i++) {
        int match_count = 0;  // 记录当前套餐匹配的标签数量
        char tags_copy[128];  // 复制用户标签用于拆分（避免修改原字符串）

        // 复制并确保字符串结束符
        strncpy(tags_copy, user_tags, sizeof(tags_copy) - 1);
        tags_copy[sizeof(tags_copy) - 1] = '\0';

        // 按逗号拆分用户标签（如拆分出"中星级"、"忠诚用户"等单个标签）
        char* single_tag = strtok(tags_copy, ",");
        while (single_tag != NULL) {
            // 检查套餐描述是否包含当前标签
            if (strstr(allPackages[i].description, single_tag) != NULL) {
                match_count++;  // 匹配到一个标签则计数+1
            }
            single_tag = strtok(NULL, ",");  // 取下一个标签
        }

        // 只要匹配到至少1个标签，就加入推荐列表
        if (match_count > 0) {
            // 控制推荐列表最大容量（假设最多10个）
            if (*matched_count < 10) {
                matched[*matched_count] = allPackages[i];
                (*matched_count)++;
            } else {
                break;  // 达到最大容量则停止匹配
            }
        }
    }
}
// 修改用户标签（返回1成功，0失败）
int modify_user_tag(int user_id, const char* new_tags) {
    UserTag tags[100]; // 假设最大支持100个用户标签
    int tag_count = 0;

    // 先加载现有标签数据（关键：必须基于文件数据修改）
    if (!load_user_tags(tags, &tag_count)) {
        printf("                        加载标签数据失败，无法修改\n");
        return 0;
    }

    // 查找目标用户ID
    for (int i = 0; i < tag_count; i++) {
        if (tags[i].user_id == user_id) {
            // 清空原有标签，复制新标签（确保不越界）
            memset(tags[i].tags, 0, sizeof(tags[i].tags));
            strncpy(tags[i].tags, new_tags, sizeof(tags[i].tags) - 1);
            tags[i].tags[sizeof(tags[i].tags) - 1] = '\0'; // 强制终止符

            // 保存修改到文件（覆盖原文件）
            if (save_user_tags(tags, tag_count)) {
                return 1; // 修改并保存成功
            } else {
                printf("                        标签保存到文件失败\n");
                return 0;
            }
        }
    }

    // 未找到用户ID
    printf("                        未找到ID为%d的用户标签\n", user_id);
    return 0;
}

void modify_user_tag_menu() {
    while(1){
    printf("\n-------------------------- 修改用户标签 --------------------------\n");
    printf("                        1. 查看用户当前标签\n");
    printf("                        2. 编辑用户标签\n");
    printf("                        0. 返回上一级\n");
    printf("                        请选择操作: ");
    
    int sub_choice;
    scanf("%d", &sub_choice);
    clearInputBuffer();
    
    int uid;
    char new_tags[128];
    UserTag tags[100];
    int tag_count;
    load_user_tags(tags, &tag_count);  // 加载现有标签
    
    switch (sub_choice) {
        case 1:  // 查看用户当前标签
            printf("                        请输入要查询的用户ID: ");
            scanf("%d", &uid);
            clearInputBuffer();
            show_user_tag(uid);  // 调用显示标签函数
            break;
            
        case 2:  // 编辑用户标签
            printf("                        请输入要修改的用户ID: ");
            int uid;
            // 校验用户ID输入格式（必须是整数）
            if (scanf("%d", &uid) != 1) {
                printf("                        输入错误！用户ID必须是整数\n");
            clearInputBuffer();
                break;
            }
            clearInputBuffer();

            printf("                    请输入新标签(逗号分隔，如'高星级,大流量'): ");
            char new_tags[128];
            if (fgets(new_tags, sizeof(new_tags), stdin) == NULL) {
            printf("                        输入标签失败\n");
                break;
            }
            new_tags[strcspn(new_tags, "\n")] = '\0';  // 去除换行符
            if (isStrEmpty(new_tags)) {
                printf("                        标签不能为空\n");
                break;
            }

            // 调用修改函数
            if (modify_user_tag(uid, new_tags)) {
                printf("                        用户标签修改成功\n");
            } else {
                printf("                        用户标签修改失败\n");
            }
            break;            
        case 0:  // 返回上一级
            printf("                        返回管理员菜单...\n");
            return;
            
        default:
            printf("                        无效选项，请重新输入\n");
    }
}
}

// 修改用户享受套餐
void admin_modify_user_package() {
    // 加载用户和套餐数据（确保操作基于最新数据）
    if (!loadUsersFromText()) {
        printf("                加载用户数据失败，无法修改套餐\n");
        return;
    }
    if (!load_packages(allPackages, &pkgCount)) {
        printf("                加载套餐数据失败，无法修改套餐\n");
        return;
    }

    // 加载用户标签（用于显示）
    UserTag tags[100];
    int tag_count = 0;
    load_user_tags(tags, &tag_count);

    // 显示所有用户信息（明确标注“已选套餐ID”）
    printf("\n================== 所有用户列表 ==================\n");
    printf("            ID\t用户名\t已选套餐ID\t星级\t标签\n");  // 修正表头提示
    for (int i = 0; i < totalUsers; i++) {
        printf("            %s\t%s\t%s\t\t%d星\t", 
               userList[i].userId,
               userList[i].userName,
               userList[i].selectedPkg,  // 此处应显示套餐ID
               userList[i].userStar);
        
        // 显示标签
        int has_tag = 0;
        for (int j = 0; j < tag_count; j++) {
            if (tags[j].user_id == atoi(userList[i].userId)) {
                printf("%s", tags[j].tags);
                has_tag = 1;
                break;
            }
        }
        if (!has_tag) printf("无");
        printf("\n");
    }

    // 输入目标用户ID
    char user_id_str[20];
    printf("\n          请输入要修改套餐的用户ID: ");
    scanf("%s", user_id_str);
    clearInputBuffer();  // 清空输入缓存，避免后续读取异常
    User* target_user = findUser(user_id_str);
    if (!target_user) {
        printf("                用户不存在\n");
        return;
    }

    // 基于标签推荐套餐（显示套餐ID和名称，引导用户输入ID）
    Package recommended[10];
    int rec_count = 0;
    match_packages_by_tag(atoi(user_id_str), recommended, &rec_count);

    printf("\n================== 推荐套餐 (基于用户标签) ==================\n");
    printf("  ID\t名称\t\t月费\t流量(MB)\t通话(分钟)\n");  // 明确显示ID
    if (rec_count == 0) {
        printf("                无匹配推荐套餐，显示所有套餐\n");
        // 显示所有套餐（带ID）
        for (int i = 0; i < pkgCount; i++) {
            printf("  %d\t%s\t%.2f元\t%d\t\t%d\n",
                   allPackages[i].id,
                   allPackages[i].name,
                   allPackages[i].monthly_fee,
                   allPackages[i].data_mb,
                   allPackages[i].voice_minutes);
        }
    } else {
        // 显示推荐套餐（带ID）
        for (int i = 0; i < rec_count; i++) {
            printf("  %d\t%s\t%.2f元\t%d\t\t%d\n",
                   recommended[i].id,
                   recommended[i].name,
                   recommended[i].monthly_fee,
                   recommended[i].data_mb,
                   recommended[i].voice_minutes);
        }
    }

    // 输入并验证新套餐ID
    char new_pkg_id_str[20];
    int new_pkg_id;
    int is_valid = 0;
    while (!is_valid) {
        printf("                        请输入新套餐ID（参考上方列表）: ");
        if (scanf("%s", new_pkg_id_str) != 1) {
            printf("                输入格式错误，请重新输入\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        // 校验是否为数字（套餐ID是整数）
        new_pkg_id = atoi(new_pkg_id_str);
        if (new_pkg_id == 0 && strcmp(new_pkg_id_str, "0") != 0) {  // 排除非数字输入
            printf("                套餐ID必须是数字，请重新输入\n");
            continue;
        }

        // 校验套餐ID是否存在
        for (int i = 0; i < pkgCount; i++) {
            if (allPackages[i].id == new_pkg_id) {
                is_valid = 1;
                break;
            }
        }
        if (!is_valid) {
            printf("                套餐ID不存在，请重新输入\n");
        }
    }

    // 执行修改（存储套餐ID字符串）
    strncpy(target_user->selectedPkg, new_pkg_id_str, sizeof(target_user->selectedPkg) - 1);
    target_user->selectedPkg[sizeof(target_user->selectedPkg) - 1] = '\0';  // 确保终止符

    // 保存到文件
    if (saveUsersToText()) {
        printf("                    用户套餐修改成功（已保存套餐ID: %s）\n", new_pkg_id_str);
    } else {
        printf("                    用户套餐修改失败（保存文件出错）\n");
    }
}
void modify_user_package_menu() {
    while(1){
        printf("\n-------------------- 修改用户享受套餐 --------------------\n");
        printf("                    1. 查看用户当前套餐\n");
        printf("                    2. 手动修改用户套餐\n");
        printf("                    3. 根据标签推荐并修改套餐\n");
        printf("                    0. 返回上一级\n");
        printf("                    请选择操作: ");
        
        int sub_choice;
        scanf("%d", &sub_choice);
        clearInputBuffer();
        switch (sub_choice) {
            case 1:  // 查看用户当前套餐
            {
                char user_id[20];
                printf("                    请输入要查询的用户ID: ");
                scanf("%s", user_id);
                clearInputBuffer();
                User* user = findUser(user_id);
                if (user) {
                    printf("                    用户 %s 当前套餐ID: %s\n", user->userId, user->selectedPkg);
                    // 额外显示套餐名称（增强可读性）
                    int pkg_id = atoi(user->selectedPkg);
                    for (int i = 0; i < pkgCount; i++) {
                        if (allPackages[i].id == pkg_id) {
                            printf("                    对应套餐名称: %s\n", allPackages[i].name);
                            break;
                        }
                    }
                } else {
                    printf("                    未找到该用户\n");
                }
            }
            break;
            
            case 2:  // 手动修改用户套餐
                admin_modify_user_package();  // 调用原有修改函数（已确保存储ID）
                break;
                
            case 3:  // 根据标签推荐并修改套餐
        {
            int uid;
            printf("                        请输入用户ID: ");
            // 增强输入校验，确保正确读取用户ID
            if (scanf("%d", &uid) != 1) {
                printf("                    输入错误，用户ID必须是数字！\n");
                clearInputBuffer();
                break;
            }
            clearInputBuffer();
            // 打印确认信息，避免用户输入错误
            printf("                    已选择用户ID: %d\n", uid);
    
            // 转换用户ID为字符串（匹配User结构体的userId格式）
            char user_id_str[20];
            sprintf(user_id_str, "%d", uid);
            // 提前查找用户，确认存在性
            User* target_user = findUser(user_id_str);
            if (target_user == NULL) {
                printf("                    错误：未找到ID为%d的用户\n", uid);
                break;
            }
    
            // 获取推荐套餐
            Package matched[10];
            int matched_count = 0;
            match_packages_by_tag(uid, matched, &matched_count);  // 调用标签匹配函数
    
            if (matched_count > 0) {
                printf("                        为用户推荐以下套餐:\n");
                // 明确显示套餐序号、ID和名称，避免混淆
                for (int i = 0; i < matched_count; i++) {
                    printf("                        %d. [ID:%d] %s (月费: %.2f元)\n", 
                        i+1,  // 选项序号
                        matched[i].id,  // 套餐ID（关键：明确显示）
                        matched[i].name, 
                        matched[i].monthly_fee);
                }
                printf("                    请选择要为用户设置的套餐(1-%d): ", matched_count);
                int pkg_choice;
                if (scanf("%d", &pkg_choice) != 1) {
                    printf("                    输入错误，套餐选择必须是数字！\n");
                    clearInputBuffer();
                    break;
                }
                clearInputBuffer();
        
                // 校验套餐选择范围
                if (pkg_choice < 1 || pkg_choice > matched_count) {
                    printf("                    无效的套餐选择，范围应为1-%d\n", matched_count);
                    break;
                }
        
                // 获取选中的套餐ID（关键修正：确保取到正确的ID）
                int selected_pkg_id = matched[pkg_choice - 1].id;  // 选项1对应索引0
                char selected_pkg_id_str[20];
                sprintf(selected_pkg_id_str, "%d", selected_pkg_id);
        
                // 打印调试信息，确认选中的套餐ID
                printf("                    您选择的套餐ID: %d\n", selected_pkg_id);
        
                // 更新用户套餐（存储套餐ID）
                strncpy(target_user->selectedPkg, selected_pkg_id_str, sizeof(target_user->selectedPkg) - 1);
                target_user->selectedPkg[sizeof(target_user->selectedPkg) - 1] = '\0';
        
                // 保存修改到文件，并验证结果
                if (saveUsersToText()) {
                    // 显示正确的用户ID和套餐信息
                    printf("                    已成功为用户[%d]设置套餐:\n", uid);
                    printf("                    套餐ID: %s, 套餐名称: %s\n", 
                        selected_pkg_id_str, 
                        matched[pkg_choice - 1].name);
                } else {
                    printf("                    套餐修改失败：保存到文件出错\n");
                }
            } else {
                printf("                    未找到匹配的推荐套餐\n");
            }
        }
        break;
            
            case 0:  // 返回上一级
                printf("                    返回管理员菜单...\n");
                return;
                
            default:
                printf("                    无效选项，请重新输入\n");
        }
    }
}