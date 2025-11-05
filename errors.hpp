// errors.hpp
#ifndef ERRORS_HPP
# define ERRORS_HPP

// 🎨 Terminal Colors (ANSI Escape Codes)
# define CLR_RESET    "\033[0m"
# define CLR_RED      "\033[1;31m"
# define CLR_YELLOW   "\033[1;33m"
# define CLR_CYAN     "\033[1;36m"
# define CLR_MAGENTA  "\033[1;35m"
# define CLR_WHITE    "\033[1;37m"

// 🐾 Error Message Prefix
# define ERROR_PREFIX CLR_RED "✖ ERROR: " CLR_RESET

// 💔 Common Errors
# define SOCKET_CREATION_FAILED   ERROR_PREFIX CLR_CYAN "Socket creation failed (｡•́︿•̀｡)\n" CLR_RESET
# define BIND_FAILED       ERROR_PREFIX CLR_CYAN "Bind failed (＞﹏＜)\n" CLR_RESET
# define LISTEN_FAILED     ERROR_PREFIX CLR_CYAN "Listen failed (╥﹏╥)\n" CLR_RESET
# define ERR_ACCEPT_FAILED     ERROR_PREFIX CLR_CYAN "Accept failed (；ω；)\n" CLR_RESET
# define ERR_CONNECT_FAILED    ERROR_PREFIX CLR_CYAN "Connection failed (×﹏×)\n" CLR_RESET
# define ERR_SEND_FAILED       ERROR_PREFIX CLR_CYAN "Send operation failed (╯°□°）╯︵ ┻━┻\n" CLR_RESET
# define ERR_RECV_FAILED       ERROR_PREFIX CLR_CYAN "Receive operation failed (っ- ‸ – ς)\n" CLR_RESET
# define ERR_FILE_NOT_FOUND    ERROR_PREFIX CLR_CYAN "File not found (´･_･`)\n" CLR_RESET
# define ERR_PERMISSION_DENIED ERROR_PREFIX CLR_CYAN "Permission denied (≧ヘ≦ )\n" CLR_RESET
# define ERR_INVALID_CONFIG    ERROR_PREFIX CLR_CYAN "Invalid configuration file (・_・;)\n" CLR_RESET
# define MALLOC_FAIL      ERROR_PREFIX CLR_CYAN "Memory allocation failed (⊙_☉)\n" CLR_RESET
# define ERR_FORK_FAILED       ERROR_PREFIX CLR_CYAN "Process fork failed (◎﹏◎;)\n" CLR_RESET
# define ERR_PIPE_FAILED       ERROR_PREFIX CLR_CYAN "Pipe creation failed (⚆_⚆)\n" CLR_RESET
# define ERR_DUP_FAILED        ERROR_PREFIX CLR_CYAN "File descriptor duplication failed (T_T)\n" CLR_RESET
# define ERR_STAT_FAILED       ERROR_PREFIX CLR_CYAN "File stat failed (╥﹏╥)\n" CLR_RESET
# define ERR_DIR_FAILED        ERROR_PREFIX CLR_CYAN "Directory operation failed (◞‸◟；)\n" CLR_RESET
# define TOO_FEW_ARGS      ERROR_PREFIX CLR_CYAN "Call webserv with './webserv [config_path]' (ง'̀-'́)ง\n" CLR_RESET
#define MEMORY_CORRUPTION ERROR_PREFIX CLR_CYAN "Memory corruption detected (⊙_☉)\n" CLR_RESET
#define CONFIG_OPEN_FAILED ERROR_PREFIX CLR_CYAN "Config file can't be opened! (╥﹏╥)\n" CLR_RESET
#define CONFIG_STAT_FAILED ERROR_PREFIX CLR_CYAN "Cannot stat config file! (◞‸◟；)\n" CLR_RESET
#define CONFIG_READ_FAILED ERROR_PREFIX CLR_CYAN "Failed to read config file! (；ω；)\n" CLR_RESET
#define NO_SERVERS ERROR_PREFIX CLR_CYAN "No servers have been configured! (╯°□°）╯︵ ┻━┻\n" CLR_RESET
#define NO_LOCATIONS    ERROR_PREFIX CLR_CYAN#define ERR_NO_LOCATIONS    ERROR_PREFIX CLR_ORANGE "Server has no locations configured!\n" CLR_RESET
#define NO_LISTEN_BINDING    ERROR_PREFIX CLR_RED "Server has no listen binding!\n" CLR_RESET
#define INVALID_LOCATION    ERROR_PREFIX CLR_MAGENTA "Location without a valid path!\n" CLR_RESET


#endif // ERRORS_HPP

