#include "./mongoose/mongoose.h"
#include "./input/input.h"
#include "./constants/constants.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

enum {
    ERR_OK                  = 0,
    ERR_MISSING_ENV         = 1,
    ERR_INVALID_CREDENTIALS = 2,
    ERR_FILE_NOT_FOUND      = 3
};

static bool authenticated = false;

static void handle_styles(struct mg_connection *c) {

    char       *response = read_file(PATH_CSS_STYLES);
    int         status   = 200;
    const char *content  = CONTENT_TYPE_CSS;
    const char *body     = "";

    if (response) {

        body = response;

    } else {

        status  = 500;
        content = "";
        body    = "Internal Server Error";

    }

    mg_http_reply(c, status, content, "%s", body);

    if (response) free(response);

}

static void handle_logout(struct mg_connection *c) {

    authenticated  = false;

    char *response = read_file(PATH_LOGIN_HTML);

    mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
    free(response);

}

static void handle_login(struct mg_connection *c,
                         struct mg_http_message *hm) {

    char username[100], password[100];

    const char *expected_user = getenv("LOGIN_USER");
    const char *expected_pass = getenv("LOGIN_PASS");

    char *response  = NULL;
    bool  success   = false;

    if (expected_user && expected_pass) {

        mg_http_get_var(&hm->body, "username", username, sizeof(username));
        mg_http_get_var(&hm->body, "password", password, sizeof(password));

        success = (strcmp(username, expected_user) == 0 &&
                   strcmp(password, expected_pass) == 0);
    }

    if (success) {

        authenticated = true;
        response      = read_file(PATH_SUCCESS_HTML);

    } else response = read_file(PATH_ERROR_HTML);

    mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
    free(response);

}

static char *str_replace(const char *orig,
                         const char *old,
                         const char *new) {

    char *result = NULL;
    
    if (orig && old && new) {

        size_t  write_pos   = 0;
        size_t  occurrences = 0;
        size_t  new_len     = strlen(new);
        size_t  old_len     = strlen(old);

        for (write_pos = 0;
             orig[write_pos] != '\0';
             write_pos++) {

            if (strstr(&orig[write_pos], old) ==
                &orig[write_pos]) {

                occurrences++;
                write_pos += old_len - 1;

            }
        }

        result = (char *)malloc(write_pos +
                                occurrences * (new_len - old_len) + 1);

        if (result) {

            write_pos = 0;

            while (*orig) {
                if (strstr(orig, old) == orig) {

                    strcpy(&result[write_pos], new);
                    write_pos += new_len;
                    orig      += old_len;

                } else result[write_pos++] = *orig++;
               
            }

            result[write_pos] = '\0';

        }
    }
    
    return result;
}

static void handle_survey_submission(struct mg_connection *c,
                                     struct mg_http_message *hm) {

    char q1[100]       = {0},
         q2[100]       = {0},
         q1_other[100] = {0};

    char       *response       = read_file(PATH_RESULTS_HTML);
    char       *temp           = NULL;
    char       *final_response = NULL;
    const char *q1_value       = NULL;

    if (response) {

        mg_http_get_var(&hm->body, "q1", q1, sizeof(q1));
        mg_http_get_var(&hm->body, "q2", q2, sizeof(q2));
        mg_http_get_var(&hm->body, "q1-other", q1_other, sizeof(q1_other));

        q1_value = (strcmp(q1, "Другой") == 0 &&
                    strlen(q1_other) > 0) ? q1_other : q1;

        temp = str_replace(response, "%q1%", q1_value);
        
        if (temp) {

            final_response = str_replace(temp, "%q2%", q2);

            if (final_response) mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", final_response);
        }
    }

    if (!final_response) mg_http_reply(c, 500, "", "Internal Server Error");

    free(response);
    free(temp);
    free(final_response);

}

static void main_fun(struct mg_connection *c,
                     int    ev,
                     void   *ev_data, 
                     void   *fn_data) {
    (void)fn_data;
    
    if (ev == MG_EV_HTTP_MSG) {

        struct  mg_http_message *hm = (struct mg_http_message *) ev_data;
        char   *response            = NULL;
        bool    handled             = false;

        if (mg_strcmp(hm->uri, mg_str(ROUTE_STYLES)) == 0) {

            handle_styles(c);

            handled = true;

        }

        if (!handled && mg_strcmp(hm->uri, mg_str(ROUTE_LOGOUT)) == 0) {

            handle_logout(c);

            handled = true;
        }

        if (!handled && !authenticated) {

            if (mg_strcmp(hm->uri, mg_str(ROUTE_LOGIN)) == 0 && 
                mg_strcmp(hm->method, mg_str("POST")) == 0) {

                handle_login(c, hm);

            } else {
                response = read_file(PATH_LOGIN_HTML);

                mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
                free(response);
            }

            handled = true;

        }

        if (!handled) {
            if (mg_strcmp(hm->uri, mg_str(ROUTE_SUBMIT_SURVEY)) == 0) {

                handle_survey_submission(c, hm);

            } else if (mg_strcmp(hm->uri, mg_str(ROUTE_SURVEY)) == 0) {

                response = read_file(PATH_SURVEY_HTML);

                mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
                free(response);

            } else {

                response = read_file(PATH_SUCCESS_HTML);

                mg_http_reply(c, 200, CONTENT_TYPE_HTML, "%s", response);
                free(response);
            }
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
