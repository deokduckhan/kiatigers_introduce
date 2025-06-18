#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_LINE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <math.h>  // fabs 사용을 위한 include 선언

#pragma comment(lib, "ws2_32.lib")

// 전체 선수단 
typedef struct player {
    int back_no;                 // 등번호 (정렬용)
    char back_no_str[10];        // 등번호 문자열 (출력용: 013 등)
    char name[50];               // 선수명
    char position[20];           // 포지션
    char birthdate[20];          // 생년월일
    char physique[50];           // 체격 (ex: "183cm, 90kg")
    char career[100];            // 경력
    int salary;                  // 연봉 (단위: 만원)
    struct Player* next;
} Player;

// 코칭스태프
typedef struct coach {
    int back_no;                 // 등번호
    char back_no_str[10];        // 등번호 문자열
    char name[50];               // 코치 이름
    char team[50];               // 소속팀
    char position[20];           // 포지션 (ex: 수석코치, 타격코치 등)
    char birthdate[20];          // 생년월일
    char physique[50];           // 체격
    char career[100];            // 경력
    int salary;                  // 연봉 (단위: 만원)
    struct Coach* next;
} Coach;

// 타자 스탯
typedef struct hitter {
    int back_no;                  // 등번호
    char back_no_str[10];         // 등번호 문자열
    char name[50];                // 선수명
    char position[20];            // 포지션
    int g;                        // 경기수
    int ab;                       // 타수
    int h;                        // 안타
    int d2;                       // 2루타
    int d3;                       // 3루타
    int hr;                       // 홈런
    int tb;                       // 루타수
    int bb;                       // 볼넷
    int hp;                       // 사구
    int sf;                       // 희생플라이
    float slg;                    // 장타율
    float obp;                    // 출루율
    float ops;                    // OPS
    float avg;                    // 타율
    struct Hitter* next;
} Hitter;

// 투수 스탯
typedef struct pitcher {
    int back_no;                  // 등번호
    char back_no_str[10];         // 등번호 문자열
    char name[50];                // 선수명
    char position[20];            // 포지션
    int g;                        // 경기수
    int er;                       // 자책점
    float ip;                     // 이닝수
    int bb;                       // 볼넷
    int h;                        // 피안타
    int hr;                       // 피홈런
    float era;                    // 평균자책점
    float whip;                   // WHIP
    struct Pitcher* next;
} Pitcher;

// 로그인
typedef struct user {
    char id[50];                 // 아이디
    char pw[50];                 // 비밀번호
    struct User* next;
} User;


// 문자열 메세지 전송
void send_line(SOCKET client, const char* msg) {
    send(client, msg, (int)strlen(msg), 0);
}

// 코칭스태프 직책 수정을 위한 함수 선언
// 문자열을 토큰 단위로 분리 (구분자는 //)
char* get_token(char** str) {
    if (*str == NULL) return NULL;

    char* token = *str;
    while (*token == ' ' || *token == '\t') token++; // 앞 공백 무시

    char* end = token;
    while (*end && *end != '/' && *(end + 1) != '/') end++;

    if (*end == '/' && *(end + 1) == '/') {
        *end = '\0';
        *str = end + 2; // 다음 토큰 시작점
    }
    else {
        *str = NULL;
    }

    return token;
}



// 로그인 함수
void handle_login(SOCKET client, const char* id, const char* pw) {
    FILE* fp = fopen("users.txt", "r");
    if (!fp) {
        send_line(client, "LOGIN_FAIL\n");
        return;  //void 이므로 return;
    }

    char line[100];
    char file_id[50] = { 0 }, file_pw[50] = { 0 };

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        int matched = sscanf_s(line, "%[^/]//%s",
            file_id, (unsigned)_countof(file_id),
            file_pw, (unsigned)_countof(file_pw));

        if (matched == 2) {
            file_id[sizeof(file_id) - 1] = '\0';
            file_pw[sizeof(file_pw) - 1] = '\0';

            if (strcmp(id, file_id) == 0 && strcmp(pw, file_pw) == 0) {
                fclose(fp);
                send_line(client, "LOGIN_OK\n");
                return;
            }
        }
    }


    fclose(fp);
    send_line(client, "LOGIN_FAIL\n");
}

// 야구단 소개
void handle_send_basic(SOCKET client) {
    FILE* fp;
    errno_t err = fopen_s(&fp, "basic.txt", "r");
    if (err != 0 || fp == NULL) {
        send_line(client, "파일 열기 실패.\nEND\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        send(client, line, (int)strlen(line), 0);
    }

    fclose(fp);
    send_line(client, "END\n");
}

// 코칭스태프 소개
void handle_print_all_coaches(SOCKET client) {
    FILE* fp;
    errno_t err = fopen_s(&fp, "kia_coach.txt", "r");
    if (err != 0 || fp == NULL) {
        send_line(client, "코치진 파일 열기 실패.\nEND\n");
        return;
    }

    char line[256];
    send_line(client, "코치진 목록\n등번호\t이름\t직책\n");

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        char* context = NULL;
        char* back_no = strtok_s(line, "\t", &context);
        char* name = strtok_s(NULL, "\t", &context);
        char* position = strtok_s(NULL, "\t", &context);

        if (back_no && name && position) {
            char buf[256];
            sprintf_s(buf, sizeof(buf), "%s\t%s\t%s\n", back_no, name, position);
            send_line(client, buf);
        }
    }

    fclose(fp);
    send_line(client, "END\n");
}




// 전체 선수단 정렬 및 출력 (연봉순 또는 이름순)
void handle_all_players_sorted(SOCKET client, char mode) {
    FILE* fp;
    errno_t err = fopen_s(&fp, "kia_players_all.txt", "r");
    if (err != 0 || fp == NULL) {
        send_line(client, "선수단 파일 열기 실패.\nEND\n");
        return;
    }

    typedef struct {
        char back_no[10], name[50], team[20], position[20];
        char birthdate[20], physique[50], career[100];
        int salary;
    } PlayerData;

    PlayerData* players = (PlayerData*)malloc(sizeof(PlayerData) * 200);
    if (!players) {
        fclose(fp);
        send_line(client, "메모리 할당 실패\nEND\n");
        return;
    }

    int count = 0;
    char line[512];
    char* token;
    char* context;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0'; // 개행 제거

        PlayerData p;
        context = NULL;

        token = strtok_s(line, "\t", &context); if (!token) continue;
        strcpy_s(p.back_no, sizeof(p.back_no), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.name, sizeof(p.name), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.team, sizeof(p.team), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.position, sizeof(p.position), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.birthdate, sizeof(p.birthdate), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.physique, sizeof(p.physique), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        strcpy_s(p.career, sizeof(p.career), token);

        token = strtok_s(NULL, "\t", &context); if (!token) continue;
        p.salary = atoi(token);

        if (count < 200)
            players[count++] = p;
    }

    fclose(fp);

    // 정렬
    if (mode == 'b') { // 연봉 높은 순
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (players[i].salary < players[j].salary) {
                    PlayerData temp = players[i];
                    players[i] = players[j];
                    players[j] = temp;
                }
            }
        }
    }
    else if (mode == 'c') { // 이름 순
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (strcmp(players[i].name, players[j].name) > 0) {
                    PlayerData temp = players[i];
                    players[i] = players[j];
                    players[j] = temp;
                }
            }
        }
    }

    // 출력
    char* response = (char*)malloc(8192);
    if (!response) {
        free(players);
        send_line(client, "메모리 할당 실패\nEND\n");
        return;
    }

    strcpy_s(response, 8192, "선수단 목록\n등번호\t선수명\t소속팀\t포지션\t생년월일\t체격\t\t경력\t\t\t\t연봉\n");

    char buf[512];
    for (int i = 0; i < count; i++) {
        sprintf_s(buf, sizeof(buf), "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%d만원\n",
            players[i].back_no, players[i].name, players[i].team, players[i].position,
            players[i].birthdate, players[i].physique, players[i].career, players[i].salary);
        strcat_s(response, 8192, buf);
    }

    strcat_s(response, 8192, "END\n");
    send_line(client, response);

    free(response);
    free(players);
}




// 타자 스탯 열람 기능
void handle_print_all_hitters(SOCKET client) {
    FILE* fp;
    errno_t err = fopen_s(&fp, "kia_hitters.txt", "r");
    if (err != 0 || fp == NULL) {
        send_line(client, "타자 파일 열기 실패.\nEND\n");
        return;
    }

    // 맨 위 제목 라인 출력
    send_line(client, "등번호\t이름\t포지션\tG\tAB\tH\t2B\t3B\tHR\tTB\tBB\tHP\tSF\tSLG\tOBP\tOPS\tAVG\n");

    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strcspn(buf, "\n")] = '\0';  // 개행 제거
        send_line(client, buf);
        send_line(client, "\n");        // 줄 간격
    }

    fclose(fp);
    send_line(client, "END\n");
}

// 투수 스탯 열람 기능
void handle_print_all_pitchers(SOCKET client) {
    FILE* fp;
    errno_t err = fopen_s(&fp, "kia_pitchers.txt", "r");
    if (err != 0 || fp == NULL) {
        send_line(client, "투수 파일 열기 실패.\nEND\n");
        return;
    }

    send_line(client, "등번호\t이름\t포지션\tG\tW\tL\tSV\tHLD\tER\tIP\tBB\tH\tHR\tERA\tWHIP\n");

    char buf[512];
    while (fgets(buf, sizeof(buf), fp)) {
        buf[strcspn(buf, "\n")] = '\0';
        send_line(client, buf);
        send_line(client, "\n");
    }

    fclose(fp);
    send_line(client, "END\n");
}


// 전체 선수단 수정 기능
void handle_modify_player(SOCKET client,
    const char* back_no,
    const char* name,
    const char* team,
    const char* position,
    const char* birth,
    const char* physique,
    const char* career,
    const char* salary) {
    FILE* fp = fopen("kia_players_all.txt", "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        send_line(client, "MODIFY_FAIL\n");
        send_line(client, "END\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    char line[MAX_LINE];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char current_back_no[10] = { 0 };
        sscanf_s(line, "%[^\t]", current_back_no, (unsigned)_countof(current_back_no));

        if (strcmp(current_back_no, back_no) == 0) {
            // 대상 등번호와 일치 -> 수정된 정보 작성
            fprintf(temp, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
                back_no, name, team, position, birth, physique, career, salary);
            found = 1;
        }
        else {
            fputs(line, temp); // 그대로 복사
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove("kia_players_all.txt") != 0 || rename("temp.txt", "kia_players_all.txt") != 0) {
        send_line(client, "MODIFY_FAIL\n");
        send_line(client, "END\n");
        return;
    }

    send_line(client, found ? "MODIFY_OK\n" : "MODIFY_FAIL\n");
    send_line(client, "END\n");
}


// 전체 선수단 추가 기능
void handle_add_player(SOCKET client, const char* back_no, const char* name, const char* team, 
    const char* position, const char* birth, const char* physique, const char* career, const char* salary) {

    FILE* fp = fopen("kia_players_all.txt", "r");
    char line[1024];

    // 1. 마지막 줄 개행 보정
    int has_lines = 0;
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            has_lines = 1;
        }

        if (has_lines) {
            fseek(fp, -1, SEEK_END);
            char ch;
            fread(&ch, 1, 1, fp);
            fclose(fp);

            if (ch != '\n') {
                FILE* fp_fix = fopen("kia_players_all.txt", "a");
                if (fp_fix) {
                    fputc('\n', fp_fix);
                    fclose(fp_fix);
                }
            }
        }
        else {
            fclose(fp);
        }
    }

    // 2. 추가 모드로 열기
    fp = fopen("kia_players_all.txt", "a");
    if (!fp) {
        send_line(client, "ADD_PLAYER_FAIL\n");
        send_line(client, "END\n");
        return;
    }

    // 3. 선수 정보 추가 (입력받은 등번호 사용)
    fprintf(fp, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
        back_no, name, team, position, birth, physique, career, salary);

    fclose(fp);
    send_line(client, "ADD_PLAYER_OK\n");
    send_line(client, "END\n");
}



// 전체 선수단 삭제 기능
void handle_delete_player(SOCKET client, const char* back_no) {
    FILE* fp = fopen("kia_players_all.txt", "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        send_line(client, "DELETE_FAIL\n");
        send_line(client, "END\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        // line에서 등번호 부분 추출
        char* tab_ptr = strchr(line, '\t');
        if (tab_ptr) {
            size_t len = tab_ptr - line;
            if (strncmp(line, back_no, len) == 0 && strlen(back_no) == len) {
                found = 1;  // 해당 등번호 줄을 건너뜀 (삭제)
                continue;
            }
        }
        fputs(line, temp);  // 삭제 대상이 아니면 임시 파일에 저장
    }

    fclose(fp);
    fclose(temp);

    if (remove("kia_players_all.txt") != 0) {
        send_line(client, "DELETE_FAIL\n");
        send_line(client, "END\n");
        return;
    }

    if (rename("temp.txt", "kia_players_all.txt") != 0) {
        send_line(client, "DELETE_FAIL\n");
        send_line(client, "END\n");
        return;
    }

    send_line(client, found ? "DELETE_OK\n" : "DELETE_FAIL\n");
    send_line(client, "END\n");
}


// 문자열 앞뒤 공백 제거 함수
void trim(char* str) {
    if (str == NULL || str[0] == '\0') return;

    // 앞쪽 공백 제거
    char* start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') start++;

    // 뒤쪽 공백 제거
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        *end-- = '\0';

    // 정리된 결과를 원래 str에 복사
    if (start != str)
        memmove(str, start, strlen(start) + 1);
}



// 코칭스태프 직책 수정 기능
void handle_modify_coach(SOCKET client,
    const char* back_no,
    const char* name,
    const char* new_position)
{
    FILE* fp = fopen("kia_coach.txt", "r");
    FILE* temp = fopen("temp.txt", "w");

    if (!fp || !temp) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    int found = 0;
    char line[512];

    char back_no_trim[20], name_trim[50], pos_trim[50];
    strncpy_s(back_no_trim, sizeof(back_no_trim), back_no, _TRUNCATE);
    strncpy_s(name_trim, sizeof(name_trim), name, _TRUNCATE);
    strncpy_s(pos_trim, sizeof(pos_trim), new_position, _TRUNCATE);
    trim(back_no_trim);
    trim(name_trim);
    trim(pos_trim);

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        char* token;
        char* context = NULL;
        char file_no[20] = "", file_name[50] = "", file_pos[50] = "";

        token = strtok_s(line, "\t", &context);
        if (token) { strncpy_s(file_no, sizeof(file_no), token, _TRUNCATE); trim(file_no); }
        token = strtok_s(NULL, "\t", &context);
        if (token) { strncpy_s(file_name, sizeof(file_name), token, _TRUNCATE); trim(file_name); }
        token = strtok_s(NULL, "\t", &context);
        if (token) { strncpy_s(file_pos, sizeof(file_pos), token, _TRUNCATE); trim(file_pos); }

        // 디버깅 로그
        printf("[DEBUG] file_no=[%s] vs input_no=[%s]\n", file_no, back_no_trim);
        printf("[DEBUG] file_name=[%s] vs input_name=[%s]\n", file_name, name_trim);

        if (strcmp(file_no, back_no_trim) == 0 &&
            strcmp(file_name, name_trim) == 0)
        {
            fprintf(temp, "%s\t%s\t%s\n", file_no, file_name, pos_trim);
            found = 1;
        }
        else {
            fprintf(temp, "%s\t%s\t%s\n", file_no, file_name, file_pos);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove("kia_coach.txt") != 0 || rename("temp.txt", "kia_coach.txt") != 0) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        return;
    }

    send_line(client, found ? "MODIFY_OK\n" : "MODIFY_FAIL\n");
    send_line(client, "END\n");
}




// 타자 스탯 수정 기능
void handle_modify_hitter(SOCKET client,
    const char* back_no,
    const char* name,
    const char* position,
    const char* stats) // "G AB H 2B 3B HR TB BB HP SF SLG OBP OPS AVG"
{
    FILE* fp = fopen("kia_hitters.txt", "r");
    FILE* temp = fopen("temp.txt", "w");

    if (!fp || !temp) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char current_back_no[10] = { 0 };
        sscanf_s(line, "%[^\t]", current_back_no, (unsigned)_countof(current_back_no));

        if (strcmp(current_back_no, back_no) == 0) {
            fprintf(temp, "%s\t%s\t%s\t%s\n", back_no, name, position, stats);
            found = 1;
        }
        else {
            fputs(line, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove("kia_hitters.txt") != 0 || rename("temp.txt", "kia_hitters.txt") != 0) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        return;
    }

    send_line(client, found ? "MODIFY_OK\n" : "MODIFY_FAIL\n");
    send_line(client, "END\n");
}


// 투수 스탯 수정 기능
void handle_modify_pitcher(SOCKET client,
    const char* back_no,
    const char* name,
    const char* position,
    const char* stats) // "G W L SV HLD ER IP BB H HR ERA WHIP"
{
    FILE* fp = fopen("kia_pitchers.txt", "r");
    FILE* temp = fopen("temp.txt", "w");

    if (!fp || !temp) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char current_back_no[10] = { 0 };
        sscanf_s(line, "%[^\t]", current_back_no, (unsigned)_countof(current_back_no));

        if (strcmp(current_back_no, back_no) == 0) {
            fprintf(temp, "%s\t%s\t%s\t%s\n", back_no, name, position, stats);
            found = 1;
        }
        else {
            fputs(line, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove("kia_pitchers.txt") != 0 || rename("temp.txt", "kia_pitchers.txt") != 0) {
        send_line(client, "MODIFY_FAIL\nEND\n");
        return;
    }

    send_line(client, found ? "MODIFY_OK\n" : "MODIFY_FAIL\n");
    send_line(client, "END\n");
}


// 자동 스탯 계산 기능
void handle_calc_stats(SOCKET client, const char* cmd);

void calculate_hitter_stats(Hitter* h) {
    if (h->ab == 0) h->avg = h->slg = h->obp = h->ops = 0.0f;
    else {
        h->avg = (float)h->h / h->ab;
        h->slg = (float)(h->d2 + 2 * h->d3 + 3 * h->hr + h->tb - h->d2 - h->d3 - h->hr) / h->ab;
        h->obp = (float)(h->h + h->bb + h->hp) / (h->ab + h->bb + h->hp + h->sf);
        h->ops = h->obp + h->slg;
    }
}

void calculate_pitcher_stats(Pitcher* p) {
    if (p->ip == 0) p->era = p->whip = 0.0f;
    else {
        p->era = (float)p->er * 9 / p->ip;
        p->whip = (float)(p->bb + p->h + p->hr) / p->ip;
    }
}

void handle_calc_stats(SOCKET client, const char* cmd) {
    char response[256];
    char mode[10] = { 0 };
    char raw[512] = { 0 };

    // C6328 경고 해결: 크기 명시적 캐스팅 필요
    int parsed = sscanf_s(cmd, "CALC_STATS//%9s %[^\n]", mode, (unsigned int)sizeof(mode), raw, (unsigned int)sizeof(raw));
    if (parsed < 2) {
        send_line(client, "입력 오류: 명령 파싱 실패\nEND\n");
        return;
    }

    if (strcmp(mode, "H") == 0) {  // 타자
        int ab, h, d2, d3, hr, tb, bb, hp, sf;
        int matched = sscanf_s(raw, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
            &ab, &h, &d2, &d3, &hr, &tb, &bb, &hp, &sf);

        if (matched != 9 || ab <= 0) {
            send_line(client, "입력 오류: 타자 스탯 항목 부족 또는 AB가 0 이하입니다.\nEND\n");
            return;
        }

        float avg = (float)h / ab;
        float slg = (float)(tb) / ab;
        float obp = (float)(h + bb + hp) / (ab + bb + hp + sf);
        float ops = slg + obp;

        sprintf_s(response, sizeof(response),
            "AVG: %.3f\nSLG: %.3f\nOBP: %.3f\nOPS: %.3f\nEND\n",
            avg, slg, obp, ops);
        send_line(client, response);
    }

    else if (strcmp(mode, "P") == 0) {  // 투수
        int er, bb, h, hr;
        float ip_input;
        int matched = sscanf_s(raw, "%d\t%f\t%d\t%d\t%d", &er, &ip_input, &bb, &h, &hr);

        if (matched != 5 || ip_input <= 0.0f) {
            send_line(client, "입력 오류: 투수 스탯 항목 부족 또는 IP가 0 이하입니다.\nEND\n");
            return;
        }

        // 이닝 보정 처리
        int ip_int = (int)ip_input;
        float ip_decimal = ip_input - ip_int;
        float ip_corrected = (float)ip_int;

        if (fabs(ip_decimal - 0.1f) < 0.01f)
            ip_corrected += 1.0f / 3;
        else if (fabs(ip_decimal - 0.2f) < 0.01f)
            ip_corrected += 2.0f / 3;
        else if (ip_decimal >= 0.3f) {
            send_line(client, "입력 오류: IP는 .0, .1, .2만 허용됩니다 (예: 45.0, 45.1, 45.2)\nEND\n");
            return;
        }

        float era = (float)er * 9 / ip_corrected;
        float whip = (float)(bb + h + hr) / ip_corrected;

        sprintf_s(response, sizeof(response),
            "ERA: %.2f\nWHIP: %.2f\nEND\n", era, whip);
        send_line(client, response);
    }

    else {
        send_line(client, "입력 오류: 모드 식별 실패 (H 또는 P만 가능)\nEND\n");
    }
}





int main() {
    WSADATA wsa;
    SOCKET server, client;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(12345);

    if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("bind 실패\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    listen(server, 5);
    printf("서버 대기 중...\n");

    while (1) {
        client = accept(server, (struct sockaddr*)&client_addr, &client_len);
        if (client == INVALID_SOCKET) {
            printf("클라이언트 연결 실패\n");
            continue;
        }

        printf("클라이언트 연결됨\n");

        char buf[2048];
        while (1) {
            int len = recv(client, buf, sizeof(buf) - 1, 0);
            if (len <= 0) break;
            buf[len] = '\0';

            // 명령 파싱
            if (strncmp(buf, "LOGIN//", 7) == 0) {
                char* data = buf + 7;
                char* id = strtok(data, "//");
                char* pw = strtok(NULL, "//");
                if (id && pw) handle_login(client, id, pw);
                else send_line(client, "LOGIN_FAIL\n");

            }
            else if (strcmp(buf, "SEND_BASIC") == 0) {
                handle_send_basic(client);

            }
            else if (strcmp(buf, "SEND_COACHES") == 0) {
                handle_print_all_coaches(client);

            }
            else if (strcmp(buf, "SEND_HITTERS") == 0) {
                handle_print_all_hitters(client);

            }
            else if (strcmp(buf, "SEND_PITCHERS") == 0) {
                handle_print_all_pitchers(client);

            }
            else if (strncmp(buf, "SEND_ALL_PLAYERS//", 18) == 0) {
                handle_all_players_sorted(client, buf[18]);

            }
            else if (strncmp(buf, "MODIFY_PLAYER//", 15) == 0) {
                char* data = buf + 15;
                char back_no[10], name[50], team[50], position[20], birth[20], physique[50], career[100], salary[20];
                sscanf_s(data, "%s %s %s %s %s %s %s %s",
                    back_no, (unsigned)_countof(back_no),
                    name, (unsigned)_countof(name),
                    team, (unsigned)_countof(team),
                    position, (unsigned)_countof(position),
                    birth, (unsigned)_countof(birth),
                    physique, (unsigned)_countof(physique),
                    career, (unsigned)_countof(career),
                    salary, (unsigned)_countof(salary));
                handle_modify_player(client, back_no, name, team, position, birth, physique, career, salary);

            }

            else if (strncmp(buf, "MODIFY_COACH//", 14) == 0) {
                char* data = buf + 14;
                char* back_no = get_token(&data);
                char* name = get_token(&data);
                char* position = get_token(&data);

                if (back_no && name && position)
                    handle_modify_coach(client, back_no, name, position);
                else
                    send_line(client, "MODIFY_FAIL\nEND\n");
            }

            else if (strncmp(buf, "MODIFY_HITTER//", 15) == 0) {
                char* data = buf + 15;
                char back_no[10], name[50], position[20];
                char stats[512];
                sscanf_s(data, "%s %s %s %[^\n]",
                    back_no, (unsigned)_countof(back_no),
                    name, (unsigned)_countof(name),
                    position, (unsigned)_countof(position),
                    stats, (unsigned)_countof(stats));
                handle_modify_hitter(client, back_no, name, position, stats);

            }
            else if (strncmp(buf, "MODIFY_PITCHER//", 16) == 0) {
                char* data = buf + 16;
                char back_no[10], name[50], position[20];
                char stats[512];
                sscanf_s(data, "%s %s %s %[^\n]",
                    back_no, (unsigned)_countof(back_no),
                    name, (unsigned)_countof(name),
                    position, (unsigned)_countof(position),
                    stats, (unsigned)_countof(stats));
                handle_modify_pitcher(client, back_no, name, position, stats);

            }
            else if (strncmp(buf, "ADD_PLAYER//", 12) == 0) {
                char* data = buf + 12;
                char* back_no = strtok(data, " ");
                char* name = strtok(NULL, " ");
                char* team = strtok(NULL, " ");
                char* position = strtok(NULL, " ");
                char* birth = strtok(NULL, " ");
                char* physique = strtok(NULL, " ");
                char* career = strtok(NULL, " ");
                char* salary = strtok(NULL, " ");

                if (back_no && name && team && position && birth && physique && career && salary) {
                    handle_add_player(client, back_no, name, team, position, birth, physique, career, salary);
                }
                else {
                    send_line(client, "ADD_PLAYER_FAIL\nEND\n");
                }
            }

            else if (strncmp(buf, "DEL_PLAYER//", 12) == 0) {
                char* back_no = buf + 12;
                handle_delete_player(client, back_no);

            }
      
            else if (strncmp(buf, "CALC_STATS//", 12) == 0) {
                handle_calc_stats(client, buf);
                }

            else {
                send_line(client, "UNKNOWN_COMMAND\nEND\n");
            }
        }

        closesocket(client);
        printf("클라이언트 연결 종료\n");
    }

    closesocket(server);
    WSACleanup();
    return 0;
}