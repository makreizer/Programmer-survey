#ifndef CONSTANTS_H
#define CONSTANTS_H

// Базовые пути
#define DIR_TEMPLATES "./templates/"
#define DIR_CSS "./css/"

// Пути к HTML-шаблонам
#define PATH_LOGIN_HTML DIR_TEMPLATES "login.html"
#define PATH_SUCCESS_HTML DIR_TEMPLATES "success.html"
#define PATH_SURVEY_HTML DIR_TEMPLATES "survey.html"
#define PATH_RESULTS_HTML DIR_TEMPLATES "results.html"
#define PATH_ERROR_HTML DIR_TEMPLATES "error.html"

// Путь к CSS
#define PATH_CSS_STYLES DIR_CSS "styles.css"

// Content-Type заголовки
#define CONTENT_TYPE_HTML "Content-Type: text/html; charset=utf-8\r\n"
#define CONTENT_TYPE_CSS "Content-Type: text/css; charset=utf-8\r\n"

// Определения маршрутов
#define ROUTE_STYLES "/styles.css"
#define ROUTE_LOGOUT "/logout"
#define ROUTE_LOGIN "/login"
#define ROUTE_SURVEY "/survey"
#define ROUTE_SUBMIT_SURVEY "/submit-survey"

#endif
