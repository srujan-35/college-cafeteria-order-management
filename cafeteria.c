
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ORDERS 100
#define NAME_SIZE 50
#define FOOD_COUNT 5

typedef struct {
    int id;
    char customer[NAME_SIZE];
    int foodIndex;
    int quantity;
    double total;
} Order;

const char *foodNames[FOOD_COUNT] = {
    "Veg Burger",
    "Masala Dosa",
    "Sandwich",
    "Veg Fried Rice",
    "Tea"
};

const double foodPrices[FOOD_COUNT] = {
    60.0, 50.0, 40.0, 80.0, 15.0
};

/* Circular queue for pending orders */
Order pending[MAX_ORDERS];
int front = 0;
int pendingCount = 0;

/* Array-based stack for completed orders */
Order completed[MAX_ORDERS];
int top = -1;

int nextOrderId = 101;

/* Read a complete line safely */
void readLine(const char *prompt, char *buffer, size_t size)
{
    int ch;

    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        printf("\nInput ended. Exiting program.\n");
        exit(0);
    }

    if (strchr(buffer, '\n') == NULL) {
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Discard excess characters */
        }
    } else {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/* Read and validate an integer */
int readInt(const char *prompt, int min, int max)
{
    char line[100];
    char *end;
    long value;

    for (;;) {
        readLine(prompt, line, sizeof(line));

        char *start = line;
        while (isspace((unsigned char)*start)) {
            start++;
        }

        if (*start == '\0') {
            printf("Please enter a number.\n");
            continue;
        }

        value = strtol(start, &end, 10);

        while (isspace((unsigned char)*end)) {
            end++;
        }

        if (end == start || *end != '\0' ||
            value < min || value > max) {
            printf("Invalid input. Enter a number from %d to %d.\n",
                   min, max);
            continue;
        }

        return (int)value;
    }
}

/* Display food menu */
void showFoodMenu(void)
{
    int i;

    printf("\n============== FOOD MENU ==============\n");
    printf("%-6s %-22s %s\n", "No.", "Food Item", "Price");

    for (i = 0; i < FOOD_COUNT; i++) {
        printf("%-6d %-22s Rs. %.2f\n",
               i + 1, foodNames[i], foodPrices[i]);
    }

    printf("=======================================\n");
}

/* Circular queue: insert at rear */
int enqueue(Order order)
{
    int rear;

    if (pendingCount == MAX_ORDERS) {
        return 0;
    }

    rear = (front + pendingCount) % MAX_ORDERS;
    pending[rear] = order;
    pendingCount++;

    return 1;
}

/* Circular queue: remove from front */
int dequeue(Order *order)
{
    if (pendingCount == 0) {
        return 0;
    }

    *order = pending[front];
    front = (front + 1) % MAX_ORDERS;
    pendingCount--;

    if (pendingCount == 0) {
        front = 0;
    }

    return 1;
}

/* Circular queue: insert at front for undo */
int enqueueFront(Order order)
{
    if (pendingCount == MAX_ORDERS) {
        return 0;
    }

    front = (front - 1 + MAX_ORDERS) % MAX_ORDERS;
    pending[front] = order;
    pendingCount++;

    return 1;
}

/* Stack: push a completed order */
int push(Order order)
{
    if (top == MAX_ORDERS - 1) {
        return 0;
    }

    completed[++top] = order;
    return 1;
}

/* Stack: pop the most recently completed order */
int pop(Order *order)
{
    if (top == -1) {
        return 0;
    }

    *order = completed[top--];
    return 1;
}

/* Print one order */
void printOrder(Order order)
{
    printf("%-7d %-18s %-19s %-7d Rs. %.2f\n",
           order.id,
           order.customer,
           foodNames[order.foodIndex],
           order.quantity,
           order.total);
}

/* Place a new order */
void placeOrder(void)
{
    Order order;
    char name[NAME_SIZE];
    int foodChoice;
    int quantity;

    if (pendingCount == MAX_ORDERS) {
        printf("\nPending queue is full. Cannot accept more orders.\n");
        return;
    }

    readLine("Enter student/customer name: ",
             name, sizeof(name));

    if (name[0] == '\0') {
        printf("Customer name cannot be empty.\n");
        return;
    }

    showFoodMenu();

    foodChoice = readInt("Choose food item (1-5): ",
                         1, FOOD_COUNT);

    quantity = readInt("Enter quantity (1-100): ", 1, 100);

    order.id = nextOrderId++;
    strcpy(order.customer, name);
    order.foodIndex = foodChoice - 1;
    order.quantity = quantity;
    order.total = foodPrices[order.foodIndex] * quantity;

    if (enqueue(order)) {
        printf("\nOrder placed successfully!\n");
        printf("Order ID : %d\n", order.id);
        printf("Customer : %s\n", order.customer);
        printf("Food     : %s\n", foodNames[order.foodIndex]);
        printf("Quantity : %d\n", order.quantity);
        printf("Bill     : Rs. %.2f\n", order.total);
    } else {
        printf("Could not place order. Queue is full.\n");
    }
}

/* Display pending orders */
void viewPending(void)
{
    int i;
    int index;

    printf("\n========== PENDING ORDERS (FIFO) ==========\n");

    if (pendingCount == 0) {
        printf("No pending orders.\n");
        return;
    }

    printf("%-7s %-18s %-19s %-7s %s\n",
           "ID", "Customer", "Food", "Qty", "Bill");

    for (i = 0; i < pendingCount; i++) {
        index = (front + i) % MAX_ORDERS;
        printOrder(pending[index]);
    }

    printf("Total pending orders: %d\n", pendingCount);
}

/* Prepare the oldest pending order */
void prepareOrder(void)
{
    Order order;

    if (pendingCount == 0) {
        printf("\nNo pending orders to prepare.\n");
        return;
    }

    if (top == MAX_ORDERS - 1) {
        printf("\nCompleted-order stack is full.\n");
        printf("Cannot complete another order.\n");
        return;
    }

    dequeue(&order);

    if (push(order)) {
        printf("\nOrder prepared and marked completed!\n");
        printf("Order ID : %d\n", order.id);
        printf("Customer : %s\n", order.customer);
        printf("Food     : %s\n", foodNames[order.foodIndex]);
        printf("Bill     : Rs. %.2f\n", order.total);
    }
}

/* Display completed orders, newest first */
void viewCompleted(void)
{
    int i;

    printf("\n========== COMPLETED ORDERS (STACK) ==========\n");

    if (top == -1) {
        printf("No completed orders yet.\n");
        return;
    }

    printf("%-7s %-18s %-19s %-7s %s\n",
           "ID", "Customer", "Food", "Qty", "Bill");

    for (i = top; i >= 0; i--) {
        printOrder(completed[i]);
    }

    printf("Total completed orders: %d\n", top + 1);
}

/* Undo the most recently completed order */
void undoCompletion(void)
{
    Order order;

    if (top == -1) {
        printf("\nNo completed order available to undo.\n");
        return;
    }

    if (pendingCount == MAX_ORDERS) {
        printf("\nPending queue is full. Cannot undo right now.\n");
        return;
    }

    /* Read the top order first; remove it only after reinsertion works */
    order = completed[top];

    if (enqueueFront(order)) {
        top--;

        printf("\nLast completion undone successfully!\n");
        printf("Order ID : %d\n", order.id);
        printf("Customer : %s\n", order.customer);
        printf("The order has returned to the front of the queue.\n");
    }
}

/* Search pending and completed orders by ID */
void searchOrder(void)
{
    int id;
    int i;
    int index;
    int found = 0;

    id = readInt("Enter order ID to search: ", 1, 1000000000);

    for (i = 0; i < pendingCount; i++) {
        index = (front + i) % MAX_ORDERS;

        if (pending[index].id == id) {
            printf("\nOrder found: PENDING\n");
            printf("%-7s %-18s %-19s %-7s %s\n",
                   "ID", "Customer", "Food", "Qty", "Bill");
            printOrder(pending[index]);
            found = 1;
            break;
        }
    }

    if (found) {
        return;
    }

    for (i = 0; i <= top; i++) {
        if (completed[i].id == id) {
            printf("\nOrder found: COMPLETED\n");
            printf("%-7s %-18s %-19s %-7s %s\n",
                   "ID", "Customer", "Food", "Qty", "Bill");
            printOrder(completed[i]);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("\nOrder ID %d was not found.\n", id);
    }
}

/* Show counts and completed sales */
void cafeteriaSummary(void)
{
    int i;
    int completedCount = top + 1;
    double sales = 0.0;
    double pendingValue = 0.0;
    int index;

    for (i = 0; i <= top; i++) {
        sales += completed[i].total;
    }

    for (i = 0; i < pendingCount; i++) {
        index = (front + i) % MAX_ORDERS;
        pendingValue += pending[index].total;
    }

    printf("\n========== CAFETERIA SUMMARY ==========\n");
    printf("Pending orders       : %d\n", pendingCount);
    printf("Completed orders     : %d\n", completedCount);
    printf("Total orders in use  : %d\n",
           pendingCount + completedCount);
    printf("Pending order value  : Rs. %.2f\n", pendingValue);
    printf("Completed sales      : Rs. %.2f\n", sales);
    printf("=======================================\n");
}

/* Sales report for the current program session */
void dailySalesReport(void)
{
    int i;
    int foodCounts[FOOD_COUNT] = {0};
    double foodSales[FOOD_COUNT] = {0.0};
    double totalSales = 0.0;
    int completedCount = top + 1;

    for (i = 0; i <= top; i++) {
        int food = completed[i].foodIndex;

        foodCounts[food] += completed[i].quantity;
        foodSales[food] += completed[i].total;
        totalSales += completed[i].total;
    }

    printf("\n========== DAILY SALES REPORT ==========\n");
    printf("Completed orders: %d\n", completedCount);
    printf("%-22s %-12s %s\n",
           "Food Item", "Qty Sold", "Sales");

    for (i = 0; i < FOOD_COUNT; i++) {
        printf("%-22s %-12d Rs. %.2f\n",
               foodNames[i], foodCounts[i], foodSales[i]);
    }

    printf("----------------------------------------\n");
    printf("TOTAL SALES: Rs. %.2f\n", totalSales);
    printf("Note: Report covers completed orders in this session.\n");
    printf("========================================\n");
}

/* Main menu */
int main(void)
{
    int choice;

    printf("Welcome to the College Cafeteria!\n");

    for (;;) {
        printf("\n============================================\n");
        printf("     COLLEGE CAFETERIA ORDER MANAGER\n");
        printf("============================================\n");
        printf(" 1. Place a new order\n");
        printf(" 2. View pending orders (Queue)\n");
        printf(" 3. Prepare next order\n");
        printf(" 4. View completed orders (Stack)\n");
        printf(" 5. Undo last completion\n");
        printf(" 6. Search order by ID\n");
        printf(" 7. Cafeteria summary\n");
        printf(" 8. Daily sales report\n");
        printf(" 9. View food menu and prices\n");
        printf(" 0. Exit\n");
        printf("============================================\n");

        choice = readInt("Enter your choice (0-9): ", 0, 9);

        switch (choice) {
            case 1:
                placeOrder();
                break;
            case 2:
                viewPending();
                break;
            case 3:
                prepareOrder();
                break;
            case 4:
                viewCompleted();
                break;
            case 5:
                undoCompletion();
                break;
            case 6:
                searchOrder();
                break;
            case 7:
                cafeteriaSummary();
                break;
            case 8:
                dailySalesReport();
                break;
            case 9:
                showFoodMenu();
                break;
            case 0:
                printf("\nThank you for using the College Cafeteria Order Manager!\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
}
