#ifndef USER_MANAGEMENT_H
#define USER_MANAGEMENT_H
#undef __ARM_FP // removes the error on the #include "mbed.h" line
#include "mbed.h"
#include "FlashIAP.h"
#include "keypad.h"
#include "SLCD/SLCD.h"
#include "DigitalOut.h"

// Define User struct for storing passwords and roles
typedef struct {
    char password[9];  // 8 characters + null terminator
    char role;         // 'a' for admin, 'u' for user, etc.
    char name[3];      // 'a0' for admin, 'u1', 'u2', 'u3', ...
} User;

// Constants for flash storage
#define MAX_USERS 10
#define USER_SIZE sizeof(User)
#define FLASH_SECTOR_SIZE 512
#define MAX_WEAR_COUNT 100000
#define FLASH_STORAGE_ADDR 0x0003FC00 // Storage start address

/**
 * @class UserManagement
 * @brief Manages user authentication and persistent storage in flash memory
 * 
 * Provides functionality to add/remove users and authenticate passwords
 * with wear-leveling for flash memory durability
 */
class UserManagement {
private:
    Keypad& keypad;
    SLCD& slcd;
    DigitalOut& led;
    DigitalOut& led2;
    FlashIAP flash;                                // Flash interface
    uint32_t current_sector;                       // Current active sector in flash
    uint32_t wear_count[FLASH_SECTOR_SIZE / USER_SIZE]; // Track sector wear
    User users[MAX_USERS];                         // User array

    // Input stuff
    char showed_password[9] = {'\0'};
    char entered_password[9] = {'\0'}; 
    int position = 0;
    char id = '\0';
    char user_name[3] = {'\0'};
    bool INPUT_ENTERED;

    // Private methods for flash operations
    void init_flash();
    uint32_t find_least_worn_sector();
    void write_to_flash(uint32_t address, const void* data, uint32_t size);
    void processInput(const char* input_type);
    void reset_input();
    void display_message(const char* msg);
    void blink_led(int times, char led_type, std::chrono::milliseconds interval);
    void led_on_for(char led_type, std::chrono::milliseconds duration);

public:
    /**
     * @brief Constructor initializes flash and loads users
     */
    UserManagement(Keypad& keypad, SLCD& slcd, DigitalOut& led, DigitalOut& led2);

    /**
     * @brief Displays Admin Menu, and serves as the launchpad for all admin commands
     */
    void launch_admin();

    /**
     * @brief Load users from flash memory
     */
    void load_users();

    /**
     * @brief Save users to flash memory with wear leveling
     */
    void save_users();

    /**
     * @brief Authenticate a user by password
     * @param password Password to authenticate
     * @return Role character ('a', 'u', 'x' for not found)
     */
    char authenticate(const char* password);

    /**
     * @brief Add a new user
     * @param password User password (8 chars max)
     * @param role User role
     * @return true if successful, false if no space
     */
    bool add_user(const char* password, char role);

    /**
     * @brief Remove a user
     * @param password Password of user to remove
     * @return true if found and removed, false otherwise
     */
    bool remove_user(const char* password);

    /**
    * @brief Change a user's password
    * @param old_password Current password to verify identity
    * @param new_password New password to set
    * @return true if successful, false if old password not found or invalid new password
    */
    bool change_password(const char* user_name, const char* new_password);
};

#endif // USER_MANAGEMENT_H

// For adding a new user (in admin mode)
// userManager.add_user("newpassword", 'u');

// For removing a user (in admin mode)
// userManager.remove_user("oldpassword");

// For changing a password (in admin mode)
// userManager.change_password("oldpassword", "new_password");