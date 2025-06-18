#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_RECENT 10
#define MAX_ITEM_LEN 256

// 최근 열람 내역 큐 구조체
typedef struct {
    char items[MAX_RECENT][MAX_ITEM_LEN];
    int front;
    int rear;
    int size;
} RecentQueue;

void init_queue(RecentQueue* q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

void enqueue_recent(RecentQueue* q, const char* item) {
    if (q->size == MAX_RECENT) {
        q->front = (q->front + 1) % MAX_RECENT;
        q->size--;
    }
    q->rear = (q->rear + 1) % MAX_RECENT;
    strncpy(q->items[q->rear], item, MAX_ITEM_LEN - 1);
    q->items[q->rear][MAX_ITEM_LEN - 1] = '\0';
    q->size++;
}

void show_recent(const RecentQueue* q) {
    printf("\n[최근 열람 목록]\n");
    if (q->size == 0) {
        printf("최근 기록이 없습니다.\n");
        return;
    }
    int index = q->front;
    for (int i = 0; i < q->size; ++i) {
        printf("%d. %s\n", i + 1, q->items[index]);
        index = (index + 1) % MAX_RECENT;
    }
}

void send_line(SOCKET sock, const char* msg) {
    send(sock, msg, (int)strlen(msg), 0);
}

void receive_response(SOCKET sock) {
    char buffer[2048];
    while (1) {
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        printf("%s", buffer);
        if (strstr(buffer, "END\n") || strstr(buffer, "BYE\n")) break;
    }
}

void input_hidden(char* buffer, int size) {
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < size - 1) {
        if (ch == '\b') {
            if (i > 0) {
                printf("\b \b");
                i--;
            }
        }
        else {
            buffer[i++] = ch;
            printf("*");
        }
    }
    buffer[i] = '\0';
    printf("\n");
}

int login(SOCKET sock) {
    char id[50], pw[50], msg[128], buffer[128];
    int attempts = 0;

    while (attempts < 3) {
        printf("\n[로그인] 아이디: ");
        fgets(id, sizeof(id), stdin);
        id[strcspn(id, "\n")] = '\0';

        printf("[로그인] 비밀번호: ");
        input_hidden(pw, sizeof(pw));

        sprintf_s(msg, sizeof(msg), "LOGIN//%s//%s", id, pw);
        send_line(sock, msg);

        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) {
            printf("서버 연결 오류.\n");
            return 0;
        }

        buffer[len] = '\0';

        if (strncmp(buffer, "LOGIN_OK", 8) == 0) {
            printf("로그인 성공!\n");
            return 1;
        }
        else {
            printf("로그인 실패. 다시 시도하세요.\n");
            attempts++;
        }
    }

    printf("3회 실패. 프로그램을 종료합니다.\n");
    return 0;
}

void add_player(SOCKET sock) {
    char back_no[10], name[50], team[50], position[20];
    char birth[20], physique[50], career[100], salary[20];
    char msg[1024];

    printf("[선수 추가]\n");
    printf("등번호: "); fgets(back_no, sizeof(back_no), stdin); back_no[strcspn(back_no, "\n")] = '\0';
    printf("이름: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = '\0';
    printf("팀: "); fgets(team, sizeof(team), stdin); team[strcspn(team, "\n")] = '\0';
    printf("포지션: "); fgets(position, sizeof(position), stdin); position[strcspn(position, "\n")] = '\0';
    printf("생년월일: "); fgets(birth, sizeof(birth), stdin); birth[strcspn(birth, "\n")] = '\0';
    printf("체격: "); fgets(physique, sizeof(physique), stdin); physique[strcspn(physique, "\n")] = '\0';
    printf("경력: "); fgets(career, sizeof(career), stdin); career[strcspn(career, "\n")] = '\0';
    printf("연봉(만원): "); fgets(salary, sizeof(salary), stdin); salary[strcspn(salary, "\n")] = '\0';

    sprintf_s(msg, sizeof(msg), "ADD_PLAYER//%s %s %s %s %s %s %s %s",
        back_no, name, team, position, birth, physique, career, salary);
    send_line(sock, msg);
    receive_response(sock);
}

void delete_player(SOCKET sock) {
    char back_no[10], msg[64];
    printf("[선수 삭제] 등번호 입력: ");
    fgets(back_no, sizeof(back_no), stdin); back_no[strcspn(back_no, "\n")] = '\0';
    sprintf_s(msg, sizeof(msg), "DEL_PLAYER//%s", back_no);
    send_line(sock, msg);
    receive_response(sock);
}

void modify_player(SOCKET sock) {
    char back_no[10], name[50], team[50], position[20], birth[20], physique[50], career[100], salary[20], msg[512];
    printf("[선수 정보 수정]\n");
    printf("등번호: "); fgets(back_no, sizeof(back_no), stdin); back_no[strcspn(back_no, "\n")] = '\0';
    printf("이름: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = '\0';
    printf("팀: "); fgets(team, sizeof(team), stdin); team[strcspn(team, "\n")] = '\0';
    printf("포지션: "); fgets(position, sizeof(position), stdin); position[strcspn(position, "\n")] = '\0';
    printf("생년월일: "); fgets(birth, sizeof(birth), stdin); birth[strcspn(birth, "\n")] = '\0';
    printf("체격: "); fgets(physique, sizeof(physique), stdin); physique[strcspn(physique, "\n")] = '\0';
    printf("경력: "); fgets(career, sizeof(career), stdin); career[strcspn(career, "\n")] = '\0';
    printf("연봉(만원): "); fgets(salary, sizeof(salary), stdin); salary[strcspn(salary, "\n")] = '\0';

    sprintf_s(msg, sizeof(msg), "MODIFY_PLAYER//%s %s %s %s %s %s %s %s",
        back_no, name, team, position, birth, physique, career, salary);
    send_line(sock, msg);
    receive_response(sock);
}

// 코칭스태프 정보 수정 기능
void modify_coach(SOCKET sock) {
    char back_no[10], name[50], position[50], msg[256];

    printf("등번호 입력 : ");
    fgets(back_no, sizeof(back_no), stdin);
    back_no[strcspn(back_no, "\n")] = '\0';

    printf("이름 입력 : ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("직책 수정 : ");
    fgets(position, sizeof(position), stdin);
    position[strcspn(position, "\n")] = '\0';

    sprintf_s(msg, sizeof(msg), "MODIFY_COACH//%s//%s//%s", back_no, name, position);
    send_line(sock, msg);
    receive_response(sock);
}

// 타자 스탯 수정 기능
void modify_hitter(SOCKET sock) {
    char back_no[10], name[50], position[20], stats[256], msg[512];
    printf("[타자 스탯 수정]\n");
    printf("등번호: "); fgets(back_no, sizeof(back_no), stdin); back_no[strcspn(back_no, "\n")] = '\0';
    printf("이름: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = '\0';
    printf("포지션: "); fgets(position, sizeof(position), stdin); position[strcspn(position, "\n")] = '\0';
    printf("스탯 입력 (G AB H 2B 3B HR TB BB HP SF SLG OBP OPS AVG) Tab으로 구분:\n");
    fgets(stats, sizeof(stats), stdin); stats[strcspn(stats, "\n")] = '\0';
    sprintf_s(msg, sizeof(msg), "MODIFY_HITTER//%s %s %s %s", back_no, name, position, stats);
    send_line(sock, msg);
    receive_response(sock);
}

// 투수 스탯 수정 기능
void modify_pitcher(SOCKET sock) {
    char back_no[10], name[50], position[20], stats[256], msg[512];
    printf("[투수 스탯 수정]\n");
    printf("등번호: "); fgets(back_no, sizeof(back_no), stdin); back_no[strcspn(back_no, "\n")] = '\0';
    printf("이름: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\n")] = '\0';
    printf("포지션: "); fgets(position, sizeof(position), stdin); position[strcspn(position, "\n")] = '\0';
    printf("스탯 입력 (G W L SV HLD ER IP BB H HR ERA WHIP) Tab으로 구분:\n");
    fgets(stats, sizeof(stats), stdin); stats[strcspn(stats, "\n")] = '\0';
    sprintf_s(msg, sizeof(msg), "MODIFY_PITCHER//%s %s %s %s", back_no, name, position, stats);
    send_line(sock, msg);
    receive_response(sock);
}

// 자동 스탯 계산 기능
void calc_stats(SOCKET sock) {
    char mode[10], input[256], msg[512];
    printf("스탯 계산 모드 선택 (H = 타자, P = 투수): ");
    fgets(mode, sizeof(mode), stdin); mode[strcspn(mode, "\n")] = '\0';

    if (mode[0] == 'H' || mode[0] == 'h') {
        printf("\n[타자 스탯 계산]\n");
        printf("입력 항목: AB\tH\t2B\t3B\tHR\tTB\tBB\tHP\tSF\n");
        printf("입력 예시: 100\t30\t10\t2\t3\t150\t10\t2\t5\n");
        printf("항목은 반드시 [Tab] 키로 구분해 주세요.\n");
        printf("스탯 입력: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        sprintf_s(msg, sizeof(msg), "CALC_STATS//H %s", input);
    }
    else if (mode[0] == 'P' || mode[0] == 'p') {
        printf("\n[투수 스탯 계산]\n");
        printf("입력 항목: ER\tIP\tBB\tH\tHR\n");
        printf("입력 예시: 12\t45.2\t10\t50\t5\n");
        printf("항목은 반드시 [Tab] 키로 구분해 주세요.\n");
        printf("스탯 입력: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        sprintf_s(msg, sizeof(msg), "CALC_STATS//P %s", input);
    }
    else {
        printf("잘못된 입력입니다.\n");
        return;
    }

    send_line(sock, msg);
    receive_response(sock);
}

int show_user_menu(SOCKET sock, RecentQueue* recent) {
    char choice[10];
    while (1) {
        printf("\n=== 사용자 메뉴 ===\n");
        printf("1. 구단 소개\n");
        printf("2. 코치진\n");
        printf("3. 선수단 (이름 순)\n");
        printf("4. 선수단 (연봉 높은 순)\n");
        printf("5. 타자 스탯 열람\n");
        printf("6. 투수 스탯 열람\n");
        printf("7. 최근 열람 내역 보기\n");
        printf("f. 메인메뉴로 돌아가기\n");
        printf("0. 종료\n선택: ");
        fgets(choice, sizeof(choice), stdin);

        if (choice[0] == '1') {
            send_line(sock, "SEND_BASIC");
            enqueue_recent(recent, "구단 소개 열람");
        }
        else if (choice[0] == '2') {
            send_line(sock, "SEND_COACHES");
            enqueue_recent(recent, "코치진 열람");
        }
        else if (choice[0] == '3') {
            send_line(sock, "SEND_ALL_PLAYERS//a");
            enqueue_recent(recent, "선수단 (연봉 높은 순) 열람");
        }
        else if (choice[0] == '4') {
            send_line(sock, "SEND_ALL_PLAYERS//b");
            enqueue_recent(recent, "선수단 (이름 순) 열람");
        }
        else if (choice[0] == '5') {
            send_line(sock, "SEND_HITTERS");
            enqueue_recent(recent, "타자 스탯 열람");
        }
        else if (choice[0] == '6') {
            send_line(sock, "SEND_PITCHERS");
            enqueue_recent(recent, "투수 스탯 열람");
        }
        else if (choice[0] == '7') {
            show_recent(recent);
            continue;
        }
        else if (choice[0] == 'f') {
            return 1;
        }
        else if (choice[0] == '0') {
            return 0;
        }
        else {
            printf("잘못된 입력입니다.\n");
            continue;
        }

        receive_response(sock);
    }
}

int show_admin_menu(SOCKET sock) {
    char choice[10];
    while (1) {
        printf("\n=== 관리자 메뉴 ===\n");
        printf("1. 전체 선수단 파일에서 특정 선수 정보 수정\n");
        printf("2. 코칭스태프 직책 수정\n");
        printf("3. 특정 타자 스탯 수정\n");
        printf("4. 특정 투수 스탯 수정\n");
        printf("5. 전체 선수단 파일에서 선수 추가\n");
        printf("6. 전체 선수단 파일에서 선수 삭제\n");
        printf("7. 자동 스탯 계산 기능 (타자/투수)\n");
        printf("f. 메인메뉴로 돌아가기\n");
        printf("0. 종료\n선택: ");
        fgets(choice, sizeof(choice), stdin);

        if (choice[0] == '1') {
            modify_player(sock);
        }
        else if (choice[0] == '2') {
            modify_coach(sock);
        }
        else if (choice[0] == '3') {
            modify_hitter(sock);
        }
        else if (choice[0] == '4') {
            modify_pitcher(sock);
        }
        else if (choice[0] == '5') {
            add_player(sock);
        }
        else if (choice[0] == '6') {
            delete_player(sock);
        }
        else if (choice[0] == '7') {
            calc_stats(sock);
        }
        else if (choice[0] == 'f') {
            return 1;
        }
        else if (choice[0] == '0') {
            return 0;
        }
        else {
            printf("잘못된 입력\n");
        }
    }
}


// 메인 함수
int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    RecentQueue recent;

    init_queue(&recent);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(12345);
    server.sin_addr.s_addr = inet_addr("192.168.70.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("서버 연결 실패\n");
        return 1;
    }

    printf("서버에 연결됨.\n");

    while (1) {
        char mode[10];
        printf("\n모드를 선택하세요 (a: 사용자, b: 관리자, 0: 종료): ");
        fgets(mode, sizeof(mode), stdin);

        if (mode[0] == 'a') {
            if (!show_user_menu(sock, &recent)) break;
        }
        else if (mode[0] == 'b') {
            if (login(sock)) {
                if (!show_admin_menu(sock)) break;
            }
            else {
                printf("로그인 실패\n");
            }
        }
        else if (mode[0] == '0') {
            break;
        }
        else {
            printf("잘못된 입력\n");
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
