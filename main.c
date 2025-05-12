#include "./mongoose/mongoose.h"
#include "./input/input.h"
#include "./constants/constants.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <alloca.h>

enum {
    ERR_OK = 0,
    ERR_MISSING_ENV = 1,
    ERR_INVALID_CREDENTIALS = 2,
    ERR_FILE_NOT_FOUND = 3
};

static bool authenticated = false;

static void handle_styles(struct mg_connection *c) {
    char *response = read_file(PATH_CSS_STYLES);
    if (response) {
        mg_http_reply(c, 200, CONTENT_TYPE_CSS, "%s", response);
        free(response);
    } else {
        mg_http_reply(c, 500, "", "Internal Server Error");
    }
}

static void handle_logout(struct mg_connection *c) {
    authenticated = false;
    char *response = read_file(PATH_LOGIN_HTML);
    mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
    free(response);
}

static void handle_login(struct mg_connection *c, struct mg_http_message *hm) {
    char username[100], password[100];
    const char *expected_user = getenv("LOGIN_USER");
    const char *expected_pass = getenv("LOGIN_PASS");

    if (!expected_user || !expected_pass) {
        char *response = read_file(PATH_ERROR_HTML);
        mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
        free(response);
        return;
    }

    mg_http_get_var(&hm->body, "username", username, sizeof(username));
    mg_http_get_var(&hm->body, "password", password, sizeof(password));

    if (strcmp(username, expected_user) == 0 && strcmp(password, expected_pass) == 0) {
        authenticated = true;
        char *response = read_file(PATH_SUCCESS_HTML);
        mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
        free(response);
    } else {
        char *response = read_file(PATH_ERROR_HTML);
        mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
        free(response);
    }
}

static void handle_survey_submission(struct mg_connection *c, struct mg_http_message *hm) {
    char q1[100], q2[100];
    mg_http_get_var(&hm->body, "q1", q1, sizeof(q1));
    mg_http_get_var(&hm->body, "q2", q2, sizeof(q2));

    char *response = read_file(PATH_RESULTS_HTML);
    if (response) {
        char *placeholder;
        
        placeholder = strstr(response, "%q1%");
        if (placeholder) {
            memmove(placeholder + strlen(q1), placeholder + 4, 
                    strlen(placeholder + 4) + 1);
            memcpy(placeholder, q1, strlen(q1));
        }
        
        placeholder = strstr(response, "%q2%");
        if (placeholder) {
            memmove(placeholder + strlen(q2), placeholder + 4, 
                    strlen(placeholder + 4) + 1);
            memcpy(placeholder, q2, strlen(q2));
        }
        
        mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
        free(response);
    } else {
        mg_http_reply(c, 500, "", "Internal Server Error");
    }
}

static void main_fun(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    (void)fn_data;
    
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Обработка CSS файлов
        if (mg_strcmp(hm->uri, mg_str(ROUTE_STYLES)) == 0) {
            handle_styles(c);
            return;
        }

        // Обработка выхода из системы
        if (mg_strcmp(hm->uri, mg_str(ROUTE_LOGOUT)) == 0) {
            handle_logout(c);
            return;
        }

        // Проверка аутентификации
        if (!authenticated) {
            if (mg_strcmp(hm->uri, mg_str(ROUTE_LOGIN)) == 0 && 
                mg_strcmp(hm->method, mg_str("POST")) == 0) {
                handle_login(c, hm);
            } else {
                char *response = read_file(PATH_LOGIN_HTML);
                mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
                free(response);
            }
            return;
        }

        // Обработка действий с опросом
        if (mg_strcmp(hm->uri, mg_str(ROUTE_SUBMIT_SURVEY)) == 0) {
            handle_survey_submission(c, hm);
        } else if (mg_strcmp(hm->uri, mg_str(ROUTE_SURVEY)) == 0) {
            char *response = read_file(PATH_SURVEY_HTML);
            mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
            free(response);
        } else {
            char *response = read_file(PATH_SUCCESS_HTML);
            mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
            free(response);
        }
    }
}

int main(void) {
    const char *server_address = "http://localhost:8088";
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    
    struct mg_http_serve_opts opts = {0};
    mg_http_listen(&mgr, server_address, main_fun, &opts);
    
    printf("Сервер запущен на %s\n", server_address);
    for (;;) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
    return 0;
}
