// account.c
#define ACCOUNT_CRYPT_COST 12

#include "account.h"
#include "macros.h"
#include "io.h"       /* for DATA_DIR */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <crypt.h>    /* for crypt_gensalt/crypt */
#include <stdio.h>    /* for FILE*, fopen, fprintf, fgets, snprintf, fclose */
#include <sys/stat.h> /* for mkdir */
#include <errno.h>

/* where account files live */
#define ACCOUNTS_DIR DATA_DIR "/accounts"

/* static helpers */
static char *make_account_filepath(const char *username);
static char *generate_password_hash(const char *password);
static void  ensure_directory(const char *path);

static char *generate_password_hash(const char *password) {
    char *salt = crypt_gensalt("$6$", ACCOUNT_CRYPT_COST, NULL, 0);
    CHECK_MSG(salt, "crypt_gensalt failed");
    char *hash = crypt(password, salt);
    CHECK_MSG(hash, "crypt failed");
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
    CHECK_MSG(password && *password, "Password cannot be empty");

    Account *account = calloc(1, sizeof *account);
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

void account_save(const Account *account) {
    CHECK(account);
    /* ensure data and accounts directories exist */
    ensure_directory(DATA_DIR);
    ensure_directory(ACCOUNTS_DIR);

    char *path = make_account_filepath(account->username);
    FILE *f = fopen(path, "w");
    CHECK_MSG(f, "Failed to open '%s' for writing", path);
    int written = fprintf(f, "%s:%s\n",
                          account->username,
                          account->password_hash);
    CHECK_MSG(written >= 0, "Failed to write account to '%s'", path);
    CHECK_MSG(fclose(f) == 0, "Failed to close '%s'", path);
    free(path);
}

Account *account_load(const char *username) {
    CHECK_MSG(username, "Username is NULL");
    char *path = make_account_filepath(username);
    FILE *f = fopen(path, "r");
    if (!f) {
        free(path);
        return NULL;
    }
    char buf[ACCOUNT_USERNAME_BUF_SIZE + 1 + 512];
    char *line = fgets(buf, sizeof buf, f);
    CHECK_MSG(line, "Failed to read account from '%s'", path);
    fclose(f);
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';
    char *sep = strchr(buf, ':');
    CHECK_MSG(sep, "Malformed account file '%s'", path);
    *sep = '\0';
    char *hash = sep + 1;
    Account *acct = malloc(sizeof *acct);
    CHECK_MSG(acct, "OOM creating Account");
    memcpy(acct->username, username, ACCOUNT_USERNAME_BUF_SIZE);
    acct->username[ACCOUNT_USERNAME_BUF_SIZE-1] = '\0';
    size_t hlen = strlen(hash);
    acct->password_hash = malloc(hlen + 1);
    CHECK_MSG(acct->password_hash, "OOM allocating hash copy");
    memcpy(acct->password_hash, hash, hlen + 1);
    free(path);
    return acct;
}

/*---------------------------------------------------------------*/

static char *make_account_filepath(const char *username) {
    size_t n = strlen(ACCOUNTS_DIR) + 1 + strlen(username) + sizeof(".acct");
    char *p = malloc(n);
    CHECK_MSG(p, "OOM allocating file path");
    int r = snprintf(p, n, "%s/%s.acct", ACCOUNTS_DIR, username);
    CHECK_MSG(r > 0 && (size_t)r < n, "File path truncated");
    return p;
}

static void ensure_directory(const char *path) {
    if (mkdir(path, 0755) != 0) {
        if (errno != EEXIST) {
            CHECK_MSG(false, "mkdir '%s' failed: %s", path, strerror(errno));
        }
    }
}
