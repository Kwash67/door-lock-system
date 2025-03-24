#include "user_management.h"
#include <cstring>

UserManagement::UserManagement() {
    init_flash();
    
    // Initialize with default users
    strncpy(users[0].password, "12345678", 8);
    users[0].password[8] = '\0';
    users[0].role = 'a';
    
    strncpy(users[1].password, "00000000", 8);
    users[1].password[8] = '\0';
    users[1].role = 'u';
    
    // Clear remaining slots
    for (int i = 2; i < MAX_USERS; i++) {
        memset(&users[i], 0, sizeof(User));
    }
    
    // Save default users to flash
    save_users();
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
            strncpy(users[i].password, password, 8);
            users[i].password[8] = '\0'; // Ensure null termination
            users[i].role = role;
            save_users();
            return true;
        }
    }
    return false; // No space available
}

bool UserManagement::remove_user(const char* password) {
    // Find and remove user with matching password
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(password, users[i].password) == 0) {
            memset(&users[i], 0, sizeof(User));
            save_users();
            return true;
        }
    }
    return false; // User not found
}

bool UserManagement::change_password(const char* old_password, const char* new_password) {
    // Validate new password (must be 8 characters)
    size_t new_pass_len = strlen(new_password);
    if (new_pass_len == 0 || new_pass_len > 8) {
        return false; // Invalid password length
    }
    
    // Find user with matching old password
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(old_password, users[i].password) == 0) {
            // Store old role
            char role = users[i].role;
            
            // Update password
            strncpy(users[i].password, new_password, 8);
            users[i].password[8] = '\0'; // Ensure null termination
            users[i].role = role;        // Preserve original role
            
            // Save changes to flash
            save_users();
            return true;
        }
    }
    
    return false; // Old password not found
}
