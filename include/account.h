// account.h
#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdbool.h>
#include <stddef.h>

#define ACCOUNT_USERNAME_SIZE     32

typedef struct Account {
    char   username[ACCOUNT_USERNAME_SIZE];
    char  *password_hash;   /* malloc’d copy of the crypt() output */
} Account;

/**
 * Create a new Account.  Aborts on OOM or programming errors.
 * Caller must have validated username (≤ ACCOUNT_USERNAME_MAX_LEN
 * and valid chars) and non‐empty password.
 */
Account *account_create(const char *username, const char *password);

/**
 * Zero out and free an Account.
 */
void account_destroy(Account *account);

/**
 * Check a plaintext password against the stored hash.
 */
bool account_check_password(const Account *account, const char *password);

/**
 * Validate a username: 1–ACCOUNT_USERNAME_MAX_LEN chars,
 * letters, digits, '_' or '-'.
 */
bool account_validate_username(const char *username);

/**
 * Save this account to
 *     DATA_DIR "/accounts/<username>.acct"
 * Aborts on any I/O or allocation error.
 */
void account_save(const Account *account);

/**
 * Load an account from
 *     DATA_DIR "/accounts/<username>.acct"
 * Returns NULL if the file doesn't exist.  Aborts on OOM or malformed file.
 */
Account *account_load(const char *username);

#endif // ACCOUNT_H
