#include "user_management.h"
#include <cstring>
#include <string>

UserManagement::UserManagement(Keypad& keypad, SLCD& slcd) : keypad(keypad), slcd(slcd) {
    init_flash();
    
    // Initialize with default users
    strncpy(users[0].password, "12345678", 8);
    users[0].password[8] = '\0';
    users[0].role = 'a';
    strncpy(users[0].name, "a0", sizeof(users[0].name));
    
    strncpy(users[1].password, "00000000", 8);
    users[1].password[8] = '\0';
    users[1].role = 'u';
    strncpy(users[1].name, "u1", sizeof(users[1].name));
    
    // Clear remaining slots
    for (int i = 2; i < MAX_USERS; i++) {
        memset(&users[i], 0, sizeof(User));
    }
    
    // Save default users to flash
    save_users();
}

void UserManagement::launch_admin() {
    slcd.printf("Admn");
    ThisThread::sleep_for(1s);
    slcd.clear();
    slcd.Home();

    // Menu items
    const char* menu_items[] = {
        "OPEN",   // First screen
        "Add.u",   // Add user
        "dEL.u",   // Delete user
        "Ch.PS",   // Change password
        "EXIT",    // Exit
    };
    const int menu_count = sizeof(menu_items) / sizeof(menu_items[0]);
    
    int current_menu = 0;
    const unsigned long scroll_interval = 2000; // 2 seconds between scrolls
    
    unsigned long last_scroll_time = us_ticker_read() / 1000;
    
    while (true) {
        // Get current time
        unsigned long current_time = us_ticker_read() / 1000;
        
        // Display current menu item
        slcd.clear();
        slcd.Home();
        slcd.printf("%s", menu_items[current_menu]);
        
        // Wait for key input
        char key = keypad.ReadKey();
        if (key != NO_KEY) 
        {
            // Reset scroll timer on key press
            last_scroll_time = current_time;
            
            // Simple navigation logic
            if (key == '*') {
                // Move to next menu item
                current_menu = (current_menu + 1) % menu_count;
            }
            else if (key == '#') {
                // Select current menu item
                switch (current_menu) {
                    case 0: // Open
                        // Implement open functionality
                        slcd.clear();
                        slcd.Home();
                        for (int i = 0; i < 3; i++){
                            slcd.printf("Open");
                            ThisThread::sleep_for(200ms);
                            slcd.clear();
                            ThisThread::sleep_for(200ms);
                        }
                        return; // Exit the function

                    case 1: // Add user
                        // Implement add user functionality
                        reset_input();
                        while(!INPUT_ENTERED){ processInput("password");}
                        if (add_user(entered_password, 'u')) {
                            for (int i = 0; i < 3; i++){
                                slcd.printf("8888");
                                ThisThread::sleep_for(200ms);
                                slcd.clear();
                                ThisThread::sleep_for(200ms);
                            }
                            return;
                        }
                        else {
                            slcd.printf("None");
                            ThisThread::sleep_for(500ms);
                        }
                        break;
                    case 2: { // Delete user
                        // Reset input states
                        id = '\0';
                        INPUT_ENTERED = false;
                        position = 0;
                        slcd.clear();
                        slcd.Home();
                        slcd.printf("ID  ");
                        slcd.Colon(1);
                        ThisThread::sleep_for(500ms);
                        while (!INPUT_ENTERED) {processInput("id");}
                        
                        // Create user name more explicitly
                        char user_name[10];
                        snprintf(user_name, sizeof(user_name), "u%c", id);
                        
                        if (remove_user(user_name)) {
                            for (int i = 0; i < 3; i++){
                                slcd.printf("8888");
                                ThisThread::sleep_for(200ms);
                                slcd.clear();
                                ThisThread::sleep_for(200ms);
                            }
                            return;
                        }
                        else {
                            slcd.printf("None");
                            ThisThread::sleep_for(500ms);
                        }
                        break;
                    }
                    case 3: { // Change password
                        // Reset input states
                        id = '\0';
                        INPUT_ENTERED = false;
                        position = 0;
                        slcd.clear();
                        slcd.Home();
                        slcd.printf("ID  ");
                        slcd.Colon(1);
                        ThisThread::sleep_for(500ms);
                        while (!INPUT_ENTERED) {processInput("id");}
                        reset_input();
                        while(!INPUT_ENTERED){ processInput("password");}

                        // Create user name more explicitly
                        char user_name[10];
                        snprintf(user_name, sizeof(user_name), "u%c", id);

                        if (change_password(user_name, entered_password)) {
                            for (int i = 0; i < 3; i++){
                                slcd.printf("8888");
                                ThisThread::sleep_for(200ms);
                                slcd.clear();
                                ThisThread::sleep_for(200ms);
                            }
                            return;
                        }
                        else {
                            slcd.printf("None");
                            ThisThread::sleep_for(500ms);
                        }
                        break;
                    }
                    case 4: // Exit Admin
                        slcd.clear();
                        slcd.Home();
                        slcd.printf("Bye");
                        ThisThread::sleep_for(1s);
                        return; // Exit the function
                }
            }
        }
        else {
            // Auto-scroll only if enough time has passed
            if (current_time - last_scroll_time >= scroll_interval) {
                current_menu = (current_menu + 1) % menu_count;
                last_scroll_time = current_time;
            }
        }
        
        // Small delay to prevent tight loop
        ThisThread::sleep_for(50ms);
    }
}

void UserManagement::init_flash() {
    // Initialize flash interface and reset wear counters
    flash.init();
    current_sector = FLASH_STORAGE_ADDR;
    memset(wear_count, 0, sizeof(wear_count));
}

uint32_t UserManagement::find_least_worn_sector() {
    // Find sector with lowest write count for wear leveling
    uint32_t min_wear = UINT32_MAX;
    uint32_t sector = current_sector;

    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE / USER_SIZE; i++) {
        if (wear_count[i] < min_wear) {
            min_wear = wear_count[i];
            sector = FLASH_STORAGE_ADDR + i * USER_SIZE;
        }
    }

    return sector;
}

void UserManagement::write_to_flash(uint32_t address, const void* data, uint32_t size) {
    // Write data to flash and increment wear counter
    flash.program(data, address, size);
    wear_count[(address - FLASH_STORAGE_ADDR) / USER_SIZE]++;
}

void UserManagement::load_users() {
    // Load user data from current flash sector
    flash.read(users, current_sector, sizeof(users));
}

void UserManagement::save_users() {
    // Find optimal sector and save user data
    uint32_t new_sector = find_least_worn_sector();
    if (new_sector != current_sector) {
        flash.erase(new_sector, FLASH_SECTOR_SIZE);
    }
    write_to_flash(new_sector, users, sizeof(users));
    current_sector = new_sector;
}

char UserManagement::authenticate(const char* password) {
    // Check if password matches any user
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(password, users[i].password) == 0) {
            return users[i].role;
        }
    }
    return 'x'; // Not found
}

bool UserManagement::add_user(const char* password, char role) {
    // Find empty slot and add new user
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].password[0] == '\0') {
            // Copy password
            strncpy(users[i].password, password, 8);
            users[i].password[8] = '\0'; // Ensure null termination
            
            // Set role
            users[i].role = role;
            
            // Set name based on role and index
            if (role == 'a') {
                // First admin is always 'a0', subsequent are 'a1', 'a2', etc.
                if (i == 0) {
                    strncpy(users[i].name, "a0", sizeof(users[i].name));
                } else {
                    snprintf(users[i].name, sizeof(users[i].name), "a%d", i);
                }
            } else {
                // User names start from 'u1'
                snprintf(users[i].name, sizeof(users[i].name), "u%d", i);
            }
            
            save_users();
            slcd.clear();
            slcd.Home();
            slcd.printf("%s", users[i].name);
            ThisThread::sleep_for(1s);
            return true;
        }
    }
    return false; // No space available
}

bool UserManagement::remove_user(const char* user_name) {
    // Find user with matching name
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(user_name, users[i].name) == 0) {
            memset(&users[i], 0, sizeof(User));
            // Save changes to flash
            save_users();
            return true;
        }
    }
    return false; // User not found
}

bool UserManagement::change_password(const char* user_name, const char* new_password) {
    // Validate new password (must be 8 characters)
    size_t new_pass_len = strlen(new_password);
    if (new_pass_len == 0 || new_pass_len > 8) {
        return false; // Invalid password length
    }
    
    // Find user with matching name
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(user_name, users[i].name) == 0) {
            // Update password
            strncpy(users[i].password, new_password, 8);
            users[i].password[8] = '\0'; // Ensure null termination
            
            // Save changes to flash
            save_users();
            return true;
        }
    }
    
    return false; // User not found
}


void UserManagement::processInput(const char* input_type) {
    if(strcmp(input_type, "id") == 0) {
        char key = keypad.ReadKey();
        if (key != NO_KEY) {
            if(position < 1 && key != '*' && key != '#') { // Limit to single digit
                slcd.clear();
                slcd.Home();
                id = key; // Store the key directly
                position++;
                slcd.printf("ID %c", id);
                slcd.Colon(1);
            }
            else if (key == '*') // Clear
            {
                id = '\0';
                INPUT_ENTERED = false;
                position = 0;
                slcd.clear();
                slcd.Home();
                slcd.printf("ID  ");
                slcd.Colon(1);
            }
            else if (key == '#') //Confirm
            {
                if (position > 0) { // Ensure an ID was entered
                    slcd.clear();
                    slcd.Home();
                    INPUT_ENTERED = true;
                }
            }
        }
    }
    else {
        char key = keypad.ReadKey();
        if (key != NO_KEY) 
        {
            if (position < 8 && key != '*' && key != '#') 
            {
                slcd.clear(); // Removes the Entr Message from screen
                slcd.Home();
                entered_password[8] = '\0';
                entered_password[position] = key;

                position++;

                // Always show last 4 characters from left to right
                char showed_password[9] = {0};
                if (position <= 4) {
                    // If less than 4 characters entered, show all entered characters
                    strncpy(showed_password, entered_password, position);
                } else {
                    // If more than 4 characters entered, show last 4 from left to right
                    strncpy(showed_password, entered_password + (position - 4), 4);
                }

                slcd.printf("%s", showed_password);
            }

            else if (key == '*') // Clear
            {
                reset_input();
            }

            else if (key == '#') //Confirm
            {
                slcd.clear();
                slcd.Home();
                entered_password[8] = '\0'; // Ensure null termination
                
                // Check if password is too short
                if (position < 8) {
                    slcd.printf("INSF");  // Display "INSF"
                    ThisThread::sleep_for(2s);
                    reset_input();
                }

                else {
                    INPUT_ENTERED = true;
                }
            }
        }
    }
}

void UserManagement::reset_input() {
    INPUT_ENTERED = false;
    position = 0;
    memset(entered_password, 0, sizeof(entered_password));
    memset(showed_password, 0, sizeof(showed_password));
    slcd.clear();
    slcd.Home();
    slcd.printf("Pswd");
    ThisThread::sleep_for(300ms);
}