#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
char timezone[32] = "America/Los_Angeles";
int money = 1330;
//int donuts = 0xcafebabe;
int donuts = 1;


void buy() {
    printf("You have %d dollars\n", money);
    puts("How many donuts would you like to buy?");
    int order;
    printf("> ");
    scanf("%d", &order);
    if (order <= 0) {
        puts("Invalid amount of donuts!");
        return;
    }
    if (money-50*order < 0) {
        puts("Not enough money!");
        return;
    }
    donuts += order; 
    money -= 50*order;
    printf("Bought %d donuts. Your balance is now %d.\n", donuts, money);
}
void earn() {
    int target;
    getrandom(&target, 4, 0);
    //printf("%d\n",target);
    puts("What is your guess?");
    printf("> ");
    int guess;
    scanf("%d", &guess);
    if (guess == target) {
        puts("Correct! You get $50");
        money += 50;
    } else {
        puts("Oops, you lost half of your money!");
        money /= 2;
    }
}
void maintenance() {
    if (donuts != 0xcafebabe) {
        puts("You aren't authorized to access this!");
        return;
    }
    puts("Welcome to the admin panel!");
    puts("Date:");
    char cmd[100];
    snprintf(cmd, 100, "date --date='TZ=\"%s\"'", timezone);
    system(cmd);
    puts("What would you like to set your balance to?");
    printf("> ");
    scanf("%d", &money);
    puts("Balance set!");
}
void menu() {
    puts("Options:");
    puts("1. Buy a donut");
    puts("2. Earn money to use in the shop");
    puts("3. Maintenance");
    puts("4. Exit");
}
int main()
{
    donuts = 0;
    puts("Welcome to the donut shop!");
    puts("Please enter your timezone so that we can tailor your experience for today:");
    printf("> ");
    gets(timezone);
    printf("Timezone set to %s!\n", timezone);
    while (1) {
        int choice;
	menu();
        printf("> ");
        scanf("%d", &choice);
        if (choice == 1) buy();
        else if (choice == 2) earn();
        else if (choice == 3) maintenance();
        else if (choice == 4) break;
        else {
            puts("Unknown choice!");
        }
    }    
    return 0;
}
