# Segfault Supper Writeup

> Our blazing fast CLI-based food ordering system uses advanced technology to perform database operations in the background, delivering the fastest and most responsive food ordering experience ever! The service is so fast that you'll never need more than 30 seconds to order your food.

The provided binary implements a CLI-based food ordering service. When connecting to the server or starting the binary, we're given the option to create a new account or log in to an existing one. After creating a new account, the following options are available:

```
❯ nc 127.0.0.1 1337
1. Register
2. Login
Choice: 1
Choose a username: bob
Choose a password: bob
Registration successful!
Welcome to Segfault Supper, bob!

--- Your Order ---
No items ordered.

1. View Menu
2. Place Order
3. View Order
4. Delete Account
5. Exit
Choice:
```

The menu lists 9 options, the last (and most expensive) of which is the flag. We can add up to three of the items to our order, but we are not allowed to add the flag:
```
Choice: 2

--- Segfault Supper Menu ---
1. Binary Burrito - $5.99
2. Null Pointer Nachos - $6.49
3. Rootkit Ramen - $10.49
4. DDoS Donuts - $4.99
5. Packet Pizza - $7.49
6. Trojan Tacos - $4.75
7. Firewall Fries - $3.50
8. Exploit Espresso - $2.99
9. Fabulous Flag Falafel - $999999999.99

Enter menu item ID to order: 1
If you have special instructions, please enter them here:
none

...

Enter menu item ID to order: 9
Sorry, that item is too expensive.
Enter menu item ID to order: 9
Input must be between 1 and 8.

...

Choice: 3

--- Your Order ---
Binary Burrito - $5.99
  none
Exploit Espresso - $2.99
  asdf
Rootkit Ramen - $10.49
  hello

1. View Menu
2. Place Order
3. View Order
4. Delete Account
5. Exit
Choice: 2
You already have 3 items in your order.
```

The application also times out after 30 seconds.

## Race Condition
As stated in the description, database operations are indeed done in a background thread. In fact, when ordering, a new thread is spun up for each item ordered. This means we can bypass the 3 item limit by sending several order requests in quick succession, because the number of orders is a global and only gets incremented after a database transaction is successful.

## Buffer Overflow
Orders are stored in a static array on the stack, so exploiting the race condition and then viewing the order (which loads the orders from the database) results in a buffer overflow where we control the "special instructions" field of the order struct.

## Program Address Leak
To exploit the buffer overflow, we need to break ASLR. Luckily, when no "special instructions" are specified, that field in the struct is left uninitialized, so when we overflow the buffer, it ends up pointing to the saved return address. When we view the order, the raw bytes are printed out to us!

```
--- Your Order ---
Null Pointer Nachos - $6.49
Null Pointer Nachos - $6.49
Null Pointer Nachos - $6.49
Null Pointer Nachos - $6.49
  "{sGQe
```

## Stack Corruption
The check preventing us from buying the Fabulous Flag Falafel simply checks if the `is_admin` flag in the user struct is nonzero. If we can execute the `place_order` function with the stack in a state where that value is nonzero, we will be allowed to purchase the flag. One method of doing this is to overwrite the return address of the `run_order_cli` function with an address a few bytes after the prologue of the function, bypassing the code that initializes the user struct to all nulls.

## Final Exploit Flow:
1. Create a randomized user
2. Order something 8 times without waiting for a response and without specifying any special instructions
3. Exit (disconnect), then connect and log in again to reload the orders and get the leaks
4. Delete the account and register a new one in the same session
5. Order something 4 times without waiting for a response, specifying the desired return address as the special instructions
6. View the order and exit to trigger the overflow
7. Instead of exiting, the program returns to the order menu
8. Order the flag, which is now possible because the stack pointer is not where the program expects and the `is_admin` field is nonzero

See `solve.py` for full details.
