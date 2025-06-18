#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <unistd.h>

#define MAX_ORDER_ITEMS 3
#define MAX_THREADS 32
#define TIMEOUT 30

typedef struct {
    int id;
    char padding[28];  // Makes it easier to get a leak :-)
    char special_instructions[100];
    char name[100];
    double price;
} MenuItem;

typedef struct {
    int order_count;
    int id;
    char username[100];
    int is_admin;
    char password_hash[120];
    MenuItem order[MAX_ORDER_ITEMS];
} User;

typedef struct {
    int menu_id;
    char special_instructions[100];
    User *user;
} OrderRequest;

sqlite3 *db;
pthread_t thread_pool[MAX_THREADS];
int threads_used = 0;
char menu_text[256] = {};
char logged_in[100] = {};

int get_number(int min, int max) {
    char buffer[128];
    int number;
    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            clearerr(stdin);
            continue;
        }

        // Check for overflow
        if (buffer[strlen(buffer) - 1] != '\n') {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        // Parse number
        char *endptr;
        number = strtol(buffer, &endptr, 10);

        // Validation: endptr should only contain newline or be at the end
        while (isspace(*endptr)) endptr++;
        if (*endptr == '\0' || *endptr == '\n') {
            if (number >= min && number <= max)
                return number;
            else
                printf("Input must be between %d and %d.\n", min, max);
        } else {
            printf("Invalid number. Please try again.\n");
        }
    }
}

void get_line(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0'; // strip newline
    } else {
        clearerr(stdin);
        buffer[0] = '\0';
    }
}

void hash_password(const char *password, char *output_hash) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output_hash + (i * 2), "%02x", hash[i]);
    output_hash[64] = '\0';
}

void* join_thread(pthread_t t) {
    void *ret;
    pthread_join(t, &ret);
    threads_used--;
    return ret;
}

void join_all_threads() {
    while (threads_used != 0) {
        join_thread(thread_pool[threads_used-1]);
    }
} 

pthread_t start_thread(void *(*start_routine)(void *), void *arg) {
    if (threads_used >= MAX_THREADS) {
        join_all_threads();
    }

    int ret = pthread_create(&thread_pool[threads_used], NULL, start_routine, arg);
    if (ret == 0) {
        threads_used++;
    }
    return thread_pool[threads_used-1];
}

void check_db(int rc, const char *msg) {
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: %s: %s\n", msg, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
}

void connect_db(const char *filename) {
    int rc = sqlite3_open(filename, &db);
    check_db(rc, "Can't open database");
}

void sql_create_user(User *user) {
    char sql[256];
    sqlite3_stmt *stmt;
    time_t now = time(NULL);

    sprintf(sql, "INSERT INTO users (username, password_hash, last_login, is_admin) VALUES (?, ?, ?, ?);");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Prepare failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user->password_hash, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, now);
    sqlite3_bind_int(stmt, 4, user->is_admin);

    int res = sqlite3_step(stmt);
    if (res != SQLITE_DONE) {
        printf("Username '%s' already exists: %s\n", user->username, sqlite3_errstr(res));
        sqlite3_finalize(stmt);
    }

    sqlite3_finalize(stmt);
    strncpy(logged_in, user->username, sizeof(logged_in));
    user->id = (int)sqlite3_last_insert_rowid(db);
}

void create_user(User *user) {
    char password[100];
    
    printf("Choose a username: ");
    get_line(user->username, sizeof(user->username));
    printf("Choose a password: ");
    get_line(password, sizeof(password));
    hash_password(password, user->password_hash);

    pthread_t t = start_thread((void * (*)(void *))sql_create_user, user);
    join_thread(t);
}

void sql_login_user(User *user) {
    char sql[256];
    sqlite3_stmt *stmt;

    sprintf(sql, "SELECT id, password_hash, last_login, is_admin FROM users WHERE username = ?;");
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, user->username, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *stored_hash = (const char *)sqlite3_column_text(stmt, 1);
        int last_login = sqlite3_column_int(stmt, 2);
        time_t now = time(NULL);

        if ((now - last_login) < 30) {
            printf("Login cooldown active. Try again in %ld seconds.\n", 30 - (now - last_login));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            exit(1);
        }

        if (strcmp(stored_hash, user->password_hash) == 0) {
            user->id = sqlite3_column_int(stmt, 0);
            user->is_admin = sqlite3_column_int(stmt, 3);
            sqlite3_finalize(stmt);
            strncpy(logged_in, user->username, sizeof(logged_in));

            // Update last_login to now
            sqlite3_stmt *update_stmt;
            sprintf(sql, "UPDATE users SET last_login = ? WHERE id = ?;");
            sqlite3_prepare_v2(db, sql, -1, &update_stmt, NULL);
            sqlite3_bind_int(update_stmt, 1, now);
            sqlite3_bind_int(update_stmt, 2, user->id);
            sqlite3_step(update_stmt);
            sqlite3_finalize(update_stmt);
            return;
        }
    }

    sqlite3_finalize(stmt);
    printf("Invalid username or password.\n");
}

void login_user(User *user) {
    char input_pass[100];

    printf("Username: ");
    get_line(user->username, sizeof(user->username));
    printf("Password: ");
    get_line(input_pass, sizeof(input_pass));
    hash_password(input_pass, user->password_hash);

    pthread_t t = start_thread((void * (*)(void *))sql_login_user, user);
    join_thread(t);
}

void sql_logout_user(char *username) {
    sqlite3_stmt *update_stmt;
    sqlite3_prepare_v2(db, "UPDATE users SET last_login = ? WHERE username = ?;", -1, &update_stmt, NULL);
    sqlite3_bind_int(update_stmt, 1, 0);
    sqlite3_bind_text(update_stmt, 2, username, -1, SQLITE_STATIC);
    sqlite3_step(update_stmt);
    sqlite3_finalize(update_stmt);
    *logged_in = '\x00';  // Mark user as logged out
}

void sql_load_order(User *user) {
    char sql[256];
    sqlite3_stmt *stmt;
    user->order_count = 0;

    sprintf(sql, "SELECT menu.id, menu.name, orders.special_instructions, menu.price FROM orders "
                 "JOIN menu ON menu.id = orders.menu_id WHERE user_id = ?;");
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, user->id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        user->order[user->order_count].id = sqlite3_column_int(stmt, 0);
        char const *name = sqlite3_column_text(stmt, 1);
        if (*name) {
            strncpy(user->order[user->order_count].name, name, 99);
        }
        char const *special_instructions = sqlite3_column_text(stmt, 2);
        if (*special_instructions) {
            strncpy(user->order[user->order_count].special_instructions, special_instructions, 99);
        }
        user->order[user->order_count].price = sqlite3_column_double(stmt, 3);
        user->order_count++;
    }

    sqlite3_finalize(stmt);
}

void show_menu() {
    printf("\n--- Segfault Supper Menu ---\n%s", menu_text);
}

void sql_place_order(OrderRequest *request) {
    char sql[128];
    sqlite3_stmt *stmt;

    sprintf(sql, "INSERT OR IGNORE INTO orders (user_id, menu_id, special_instructions) VALUES (?, ?, ?);");
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, request->user->id);
    sqlite3_bind_int(stmt, 2, request->menu_id);
    sqlite3_bind_text(stmt, 3, request->special_instructions, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
        printf("Failed to add item.\n");

    sqlite3_finalize(stmt);
    request->user->order_count += 1;
    free(request);
}

void place_order(User *user) {
    if (!user->is_admin && user->order_count >= MAX_ORDER_ITEMS) {
        printf("You already have 3 items in your order.\n");
        return;
    }
    OrderRequest *req = malloc(sizeof(OrderRequest));

    show_menu();
    printf("\nEnter menu item ID to order: ");
    req->menu_id = get_number(1, 9);
    if (req->menu_id == 9 && user->is_admin) {
        FILE* f = fopen("flag.txt", "r");
        fgets(req->special_instructions, sizeof(req->special_instructions), f);
        printf("Thank you for your business! Here is the flag:\n%s\n", req->special_instructions);
        fclose(f);
    }
    else {
        if (req->menu_id == 9) {
            printf("Sorry, that item is too expensive.\nEnter menu item ID to order: ");
            req->menu_id = get_number(1, 8);
        }
        printf("If you have special instructions, please enter them here:\n");
        get_line(req->special_instructions, sizeof(req->special_instructions));
    }
    req->user = user;
    start_thread((void * (*)(void *))sql_place_order, req);
}

void view_order(User *user) {
    pthread_t t = start_thread((void * (*)(void *))sql_load_order, user);
    join_thread(t);
    printf("\n--- Your Order ---\n");
    if (user->order_count == 0) {
        printf("No items ordered.\n");
        return;
    }
    for (int i = 0; i < user->order_count; i++) {
        printf("%s - $%.2f\n", user->order[i].name, user->order[i].price);
        if (user->order[i].special_instructions[0]) {
            printf("  %s\n", user->order[i].special_instructions);
        }
    }
}

void sql_delete_account(User *user) {
    char sql[128];
    sqlite3_stmt *stmt;

    sprintf(sql, "DELETE FROM users WHERE id = ?;");
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, user->id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    printf("Account and orders deleted.\n");
}

void sql_load_menu() {
    sqlite3_stmt *stmt;
    char sql[] = "SELECT id, name, price FROM menu;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    char *menu_ptr = menu_text;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        menu_ptr += sprintf(menu_ptr, "%d. %s - $%.2f\n", sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_double(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

void load_menu() {
    pthread_t t = start_thread((void * (*)(void *))sql_load_menu, NULL);
    join_thread(t);
}

int run_order_cli() {
    pthread_t t;
    int choice;
    User current_user = {};
    while (! *logged_in) {
        printf("1. Register\n2. Login\nChoice: ");
        choice = get_number(1, 2);

        if (choice == 1) {
            create_user(&current_user);
            printf("Registration successful!\n");
        } else if (choice == 2) {
            login_user(&current_user);
        } else {
            printf("Invalid choice.\n");
            return 1;
        }

        if (*logged_in) {
            printf("Welcome to Segfault Supper, %s!\n", current_user.username);
            view_order(&current_user);
        }

        while (*logged_in) {
            printf("\n1. View Menu\n2. Place Order\n3. View Order\n4. Delete Account\n5. Exit\nChoice: ");
            choice = get_number(1,5);
            if (choice == 5) break;
            switch (choice) {
                case 1: show_menu(); break;
                case 2: place_order(&current_user); break;
                case 3: view_order(&current_user); break;
                case 4:
                    t = start_thread((void * (*)(void *))sql_delete_account, &current_user);
                    join_thread(t);
                    *logged_in = '\x00';
                    break;
                default: printf("Invalid choice.\n");
            }
        }
    }
    start_thread((void * (*)(void *))sql_logout_user, current_user.username);
    join_all_threads();
    printf("Thank you for choosing Segfault Supper. Have a nice day!\n");
    return 0;
}

void handle_alarm() {
    if (*logged_in != '\x00') {
        start_thread((void * (*)(void *))sql_logout_user, logged_in);
    }
    join_all_threads();
    sqlite3_close(db);
    printf("Thank you for choosing Segfault Supper. Have a nice day!\n");
    exit(0);
}

int main() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    signal(SIGALRM, handle_alarm);
    alarm(30);
    connect_db("./db/food.db");
    load_menu();
    int res = run_order_cli();
    sqlite3_close(db);
    return res;
}
