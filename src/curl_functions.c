#include <stdio.h>
#include "curl/curl.h"
#include <stdlib.h>
#include <string.h>
struct string {
    char *ptr;
    size_t len;
};

static void init_string(struct string *s){
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        controller_log("malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        controller_log("realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

int curlPostJson(const char *url, const char *userName, const char *password, const char *json, char **result)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        controller_log("curl initialization failure");
        return 128;
    }

    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

    if (userName != NULL)
        curl_easy_setopt(curl, CURLOPT_USERNAME, userName);
    if (password != NULL)
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    struct curl_slist *slist = NULL;
    slist = curl_slist_append(slist, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        controller_log("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    *result = strdup(s.ptr);

    free(s.ptr);

    curl_easy_cleanup(curl);

    return 0;
}

int curlGet(const char *url, const char *userName, const char *password, char **result)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        controller_log("curl initialization failure");
        return 128;
    }

    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    if (userName != NULL)
        curl_easy_setopt(curl, CURLOPT_USERNAME, userName);
    if (password != NULL)
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

    res = curl_easy_perform(curl);

    //*result = malloc(s.len + 2);
    //memset(*result, '\0', s.len + 2);
    //memcpy(*result, s.ptr, s.len);
    *result = strdup(s.ptr);

    free(s.ptr);

    curl_easy_cleanup(curl);

    return 0;
}
