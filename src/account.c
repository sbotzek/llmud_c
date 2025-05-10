// account.c
#define ACCOUNT_CRYPT_COST 12

#include "account.h"
#include "macros.h"
#include "file_chunk.h"
#include "strutil.h"
#include "log.h"
#include "io.h"       /* for DATA_DIR */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <crypt.h>    /* for crypt_gensalt/crypt */
#include <stdio.h>    /* for FILE*, fopen, fprintf, fgets, snprintf, fclose */

/* where account files live */
#define ACCOUNTS_DIR DATA_DIR "/accounts"

/* static helpers */
static char *make_account_filepath(const char *username);
static char *generate_password_hash(const char *password);

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

Account *account_new(const char *username, const char *password) {
    size_t ulen = strlen(username);
    CHECK_MSG(ulen > 0 && ulen < ACCOUNT_USERNAME_SIZE - 1,
              "Username length out of bounds (%zu)", ulen);
    CHECK_MSG(password && *password, "Password cannot be empty");

    Account *account = calloc(1, sizeof *account);
    CHECK_MSG(account, "OOM creating Account");

    memcpy(account->username, username, ulen + 1);
    account->password_hash = generate_password_hash(password);
    buffer_init(&account->character_names, 0);

    return account;
}

bool account_check_password(const Account *account, const char *password) {
    CHECK(account && account->password_hash);
    char *calc = crypt(password, account->password_hash);
    CHECK_MSG(calc, "crypt failed");
    return strcmp(calc, account->password_hash) == 0;
}

void account_free(Account *account) {
    CHECK(account != NULL);
    if (account->password_hash) {
        memset(account->password_hash, 0, strlen(account->password_hash));
        free(account->password_hash);
    }
    buffer_cleanup(&account->character_names);
    free(account);
}

bool account_validate_username(const char *username) {
    if (!username || !*username) return false;
    size_t len = strlen(username);
    if (len == 0 || len >= ACCOUNT_USERNAME_SIZE) return false;
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

    ensure_directory(DATA_DIR);
    ensure_directory(ACCOUNTS_DIR);

    char *path = make_account_filepath(account->username);
    FILE *f = fopen(path, "w");
    CHECK_MSG(f, "Failed to open '%s' for writing", path);

    file_chunk_write_field(f, "username", account->username);
    file_chunk_write_field(f, "password_hash", account->password_hash);
    file_chunk_write_field(f, "character_names", account->character_names.data);

    CHECK_MSG(fclose(f) == 0, "Failed to close '%s'", path);
    free(path);
}

Account *account_load(const char *username) {
    CHECK(username);

    char *path = make_account_filepath(username);
    FILE *f = fopen(path, "r");
    if (!f) {
        free(path);
        return NULL;
    }

    FileChunkReader r;
    file_chunk_reader_init(&r, f);

    Account *acc = calloc(1, sizeof(Account));
    buffer_init(&acc->character_names, 0);
    CHECK(acc);

    while (file_chunk_read(&r)) {
        if (r.chunk.type != FILE_CHUNK_FIELD) {
            // Stop on first non-field (e.g., section header like #character ...)
            break;
        }

        if (strcmp(r.chunk.tag.data, "username") == 0) {
            strncpy(acc->username, r.chunk.value.data, ACCOUNT_USERNAME_SIZE - 1);
            acc->username[ACCOUNT_USERNAME_SIZE - 1] = '\0';
        } else if (strcmp(r.chunk.tag.data, "password_hash") == 0) {
            acc->password_hash = str_copy(r.chunk.value.data);
            CHECK_MSG(acc->password_hash, "OOM loading password hash");
        } else if (strcmp(r.chunk.tag.data, "character_names") == 0) {
            buffer_append_str(&acc->character_names, r.chunk.value.data);
        } else {
            log_warn("Unknown account field '%s' at line %d", r.chunk.tag.data, r.line_number);
        }
    }

    file_chunk_reader_cleanup(&r);
    fclose(f);
    free(path);

    CHECK_MSG(acc->username[0], "Missing username field");
    CHECK_MSG(acc->password_hash, "Missing password_hash field");

    return acc;
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

void account_add_character(Account *account, const char *name) {
    CHECK(account != NULL);
    CHECK(name != NULL);
    if (account->character_names.length > 0)
        buffer_append_str(&account->character_names, " ");
    buffer_append_str(&account->character_names, name);
    str_to_lower(account->character_names.data);
}

void account_remove_character(Account *account, const char *name) {
    CHECK(account != NULL);
    CHECK(name != NULL);
    char *start = account->character_names.data;
    char *match = strstr(start, name);
    CHECK_MSG(match != NULL, "Character not found in account_remove_character");

    size_t len = strlen(name);
    if (match[len] == ' ') len++;  // remove trailing space
    memmove(match, match + len, strlen(match + len) + 1);
}

bool account_has_character(Account *account, const char *name) {
    CHECK(account != NULL);
    CHECK(name != NULL);
    return str_token_contains(account->character_names.data, name);
}
