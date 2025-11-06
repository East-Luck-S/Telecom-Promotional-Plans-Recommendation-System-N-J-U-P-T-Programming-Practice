#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h> /* getch() 用于密码输入遮掩，实现无回显输入读取 */

#include "admin.h"

int pkgCount=0;			   // 总套餐数量  
int adminCount = 0;		 // 管理员数量

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
		int cmd;
		if (scanf("%d", &cmd) != 1) { getchar(); continue; }
		getchar();
		switch (cmd) {
			case 0:
				return;
			case 1:
					printf("\n所有管理员：\n");
					for (int i = 0; i < *count; i++) {
						printf("[%d] 用户名:%s 类型:%s\n", i+1, admins[i].username, admins[i].is_super ? "全局" : "普通");
					}
					break;
			case 2:
					if (*count >= 32) { printf("管理员数量已达上限。\n"); break; }
					AdminAccount newa;
					printf("输入新管理员用户名: ");
					scanf("%31s", newa.username); getchar();
					printf("输入新管理员密码: ");
					read_password(newa.password, sizeof(newa.password));
					printf("类型 (1=全局, 0=普通): ");
					scanf("%d", &newa.is_super); getchar();
					admins[*count] = newa;
					(*count)++;
					save_admin_accounts(admins, *count);
					printf("添加成功。\n");
					break;
			case 3:
					printf("输入要删除的管理员用户名: ");
					char uname[32];
					scanf("%31s", uname); getchar();
					int found = -1;
					for (int i = 0; i < *count; i++) {
						if (strcmp(admins[i].username, uname) == 0) { found = i; break; }
					}
					if (found == -1) { printf("未找到该管理员。\n"); break; }
					for (int i = found; i < *count-1; i++) admins[i] = admins[i+1];
					(*count)--;
					save_admin_accounts(admins, *count);
					printf("删除成功。\n");
					break;
			case 4:
					printf("输入要修改密码的管理员用户名: ");
					char uname2[32];
					scanf("%31s", uname2); getchar();
					int found2 = -1;
					for (int i = 0; i < *count; i++) {
						if (strcmp(admins[i].username, uname2) == 0) { found2 = i; break; }
					}
					if (found2 == -1) { printf("未找到该管理员。\n"); break; }
					printf("输入新密码: ");
					read_password(admins[found2].password, sizeof(admins[found2].password));
					save_admin_accounts(admins, *count);
					printf("密码修改成功。\n");
					break;
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
	Package *p = &pkgs[*count];
	p->id = (*count == 0) ? 1 : (pkgs[*count - 1].id + 1);
	printf("输入套餐名称: ");
	fgets(p->name, sizeof(p->name), stdin);
	p->name[strcspn(p->name, "\n")] = '\0';
	printf("月资费(元): ");
	scanf("%lf", &p->monthly_fee);
	printf("每月流量(MB): ");
	scanf("%d", &p->data_mb);
	printf("语音分钟数: ");
	scanf("%d", &p->voice_minutes);
	printf("短信条数: ");
	scanf("%d", &p->sms);
	printf("合约月数(0表示无合约): ");
	scanf("%d", &p->contract_months);
	getchar();
	printf("生效日期 (YYYY-MM-DD): ");
	fgets(p->start_date, sizeof(p->start_date), stdin);
	p->start_date[strcspn(p->start_date, "\n")] = '\0';
	printf("终止日期 (YYYY-MM-DD, 可空): ");
	fgets(p->end_date, sizeof(p->end_date), stdin);
	p->end_date[strcspn(p->end_date, "\n")] = '\0';
	printf("是否启用 (1=是,0=否): ");
	scanf("%d", &p->is_active);
	getchar();
	printf("备注: ");
	fgets(p->description, sizeof(p->description), stdin);
	p->description[strcspn(p->description, "\n")] = '\0';

	(*count)++;
	save_packages(pkgs, *count);
	printf("添加成功（ID=%d）。\n", p->id);
}
// 修改套餐
// void modify_package(Package pkgs[], int count) {
// 	if (count == 0) {
// 		printf("没有套餐可供修改。\n");
// 		return;
// 	}
// 	int id;
// 	printf("输入要修改的套餐 ID: ");
// 	scanf("%d", &id); getchar();
// 	int found = -1;
// 	for (int i = 0; i < count; i++) if (pkgs[i].id == id) { found = i; break; }
// 	if (found == -1) {
// 		printf("未找到 ID=%d 的套餐。\n", id);
// 		return;
// 	}
// 	Package *p = &pkgs[found];
// 	printf("当前名称: %s\n输入新名称(回车保持不变): ", p->name);
// 	char buf[128];
// 	fgets(buf, sizeof(buf), stdin);
// 	if (buf[0] != '\n') { buf[strcspn(buf, "\n")]='\0'; strncpy(p->name, buf, sizeof(p->name)); }
// 	printf("当前月资费: %.2f\n输入新资费(0保持不变): ", p->monthly_fee);
// 	double d; scanf("%lf", &d); if (d > 0) p->monthly_fee = d; getchar();
// 	save_packages(pkgs, count);
// 	printf("修改保存。\n");
// }

// 修改或删除套餐（整合功能）
void modify_package(Package pkgs[], int count) {
    if (count == 0) {
        printf("没有套餐可供操作。\n");
        return;
    }

    // 1. 先显示所有套餐，方便管理员选择
    list_packages(pkgs, count);

    // 2. 输入要操作的套餐ID
    int id;
    printf("\n请输入要操作的套餐 ID: ");
    if (scanf("%d", &id) != 1) {
        printf("输入格式错误！\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    // 3. 查找套餐
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

    // 4. 显示当前套餐信息，供参考
    Package *p = &pkgs[found];
    printf("\n当前套餐信息：\n");
    printf("ID: %d\n", p->id);
    printf("名称: %s\n", p->name);
    printf("月资费: %.2f元\n", p->monthly_fee);
    printf("每月流量: %dMB\n", p->data_mb);
    printf("语音分钟数: %d\n", p->voice_minutes);
    printf("短信条数: %d\n", p->sms);
    printf("合约月数: %d\n", p->contract_months);
    printf("生效日期: %s\n", p->start_date);
    printf("终止日期: %s\n", p->end_date);
    printf("是否启用: %d（1=是，0=否）\n", p->is_active);
    printf("备注: %s\n", p->description);

    // 5. 选择操作（修改/删除）
    int op;
    printf("\n请选择操作：1-修改  2-删除  （输入数字）: ");
    if (scanf("%d", &op) != 1 || (op != 1 && op != 2)) {
        printf("无效操作！\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (op == 1) {
        // 6. 执行修改（覆盖所有字段）
        printf("\n===== 开始修改套餐 =====");
        printf("\n输入新名称（原：%s）: ", p->name);
        fgets(p->name, sizeof(p->name), stdin);
        p->name[strcspn(p->name, "\n")] = '\0';  // 去除换行符

        printf("输入新月资费（元，原：%.2f）: ", p->monthly_fee);
        scanf("%lf", &p->monthly_fee);
        clearInputBuffer();

        printf("输入新每月流量（MB，原：%d）: ", p->data_mb);
        scanf("%d", &p->data_mb);
        clearInputBuffer();

        printf("输入新语音分钟数（原：%d）: ", p->voice_minutes);
        scanf("%d", &p->voice_minutes);
        clearInputBuffer();

        printf("输入新短信条数（原：%d）: ", p->sms);
        scanf("%d", &p->sms);
        clearInputBuffer();

        printf("输入新合约月数（0表示无合约，原：%d）: ", p->contract_months);
        scanf("%d", &p->contract_months);
        clearInputBuffer();

        printf("输入新生效日期（YYYY-MM-DD，原：%s）: ", p->start_date);
        fgets(p->start_date, sizeof(p->start_date), stdin);
        p->start_date[strcspn(p->start_date, "\n")] = '\0';

        printf("输入新终止日期（YYYY-MM-DD，原：%s）: ", p->end_date);
        fgets(p->end_date, sizeof(p->end_date), stdin);
        p->end_date[strcspn(p->end_date, "\n")] = '\0';

        printf("是否启用（1=是，0=否，原：%d）: ", p->is_active);
        scanf("%d", &p->is_active);
        clearInputBuffer();

        printf("输入新备注（原：%s）: ", p->description);
        fgets(p->description, sizeof(p->description), stdin);
        p->description[strcspn(p->description, "\n")] = '\0';

        save_packages(pkgs, count);
        printf("套餐修改成功！\n");
    } else {
        // 7. 执行删除（将后续套餐前移，覆盖当前位置）
        for (int i = found; i < count - 1; i++) {
            pkgs[i] = pkgs[i + 1];  // 数组元素前移
        }
        // 更新全局套餐数量（注意：这里需要修改外部变量pkgCount）
        pkgCount--;  // 因为count是传参的副本，需直接操作全局变量
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

