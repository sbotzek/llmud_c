// account.c
#define ACCOUNT_CRYPT_COST 12

#include "account.h"
#include "macros.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <crypt.h>    /* Fedora’s libxcrypt provides crypt_gensalt/crypt */

static char *generate_password_hash(const char *password) {
    /* generate a salt using the chosen cost */
    char *salt = crypt_gensalt("$6$", ACCOUNT_CRYPT_COST, NULL, 0);
    CHECK_MSG(salt, "crypt_gensalt failed");

    /* compute the hash */
    char *hash = crypt(password, salt);
    CHECK_MSG(hash, "crypt failed");

    /* strdup isn’t in C11 without feature tests, so do it by hand */
    size_t hlen = strlen(hash);
    char *copy = malloc(hlen + 1);
    CHECK_MSG(copy, "OOM allocating password hash copy");
    memcpy(copy, hash, hlen + 1);
    return copy;
}

Account *account_create(const char *username, const char *password) {
    size_t ulen = strlen(username);
    CHECK_MSG(ulen > 0 && ulen <= ACCOUNT_USERNAME_MAX_LEN,
              "Username length out of bounds (%zu)", ulen);
    CHECK_MSG(password && *password,
              "Password cannot be empty");

    Account *account = malloc(sizeof(*account));
    CHECK_MSG(account, "OOM creating Account");

    memcpy(account->username, username, ulen + 1);
    account->password_hash = generate_password_hash(password);

    return account;
}

bool account_check_password(const Account *account, const char *password) {
    CHECK(account && account->password_hash);
    char *calc = crypt(password, account->password_hash);
    CHECK_MSG(calc, "crypt failed");
    return strcmp(calc, account->password_hash) == 0;
}

void account_destroy(Account *account) {
    CHECK(account != NULL);
    if (account->password_hash) {
        memset(account->password_hash, 0, strlen(account->password_hash));
        free(account->password_hash);
    }
    free(account);
}

bool account_validate_username(const char *username) {
    if (!username || !*username) return false;
    size_t len = strlen(username);
    if (len == 0 || len > ACCOUNT_USERNAME_MAX_LEN) return false;
    for (const char *p = username; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}
