#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define BUFFER_SIZE 1024


typedef struct doubly_node {
    struct doubly_node *prev;
    struct doubly_node *next;
    void *data;
} doubly_node_t;


void free_doubly_linked_list(doubly_node_t *head) {
    while (head != NULL) {
        doubly_node_t *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

doubly_node_t * create_node(void *data) {
    doubly_node_t *node = malloc(sizeof(doubly_node_t));
    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    node->data = data;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

doubly_node_t * get_last_node(doubly_node_t *head) {
    if (head == NULL) {
        return NULL;
    }
    while (head->next != NULL) {
        head = head->next;
    }
    return head;
}

void add_after(doubly_node_t **node, void *data) {
    doubly_node_t *new_node = create_node(data);

    if (*node == NULL) {
        new_node->next = NULL;
        new_node->prev = NULL;
        *node = new_node;
    }
    else {
        new_node->prev = *node;
        new_node->next = (*node)->next;

        if ((*node)->next != NULL) {
            (*node)->next->prev = new_node;
        }

        (*node)->next = new_node;
    }
}

void add_before(doubly_node_t **node,void *data) {
    doubly_node_t *new_node = create_node(data);

    if (*node == NULL) {
        new_node->next = NULL;
        new_node->prev = NULL;
        *node = new_node;
    }
    else {
        new_node->next = *node;
        new_node->prev = (*node)->prev;

        if ((*node)->prev != NULL) {
            (*node)->prev->next = new_node;
        }

        (*node)->prev = new_node;
    }
}

void add_last(doubly_node_t **head, void *data) {
    doubly_node_t *last_node = get_last_node(*head);
    if (last_node == NULL) {
        add_after(head, data);
    }
    else {
        add_after(&last_node, data);
    }
}

void remove_node(doubly_node_t *node) {
    if (node != NULL) {
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
        free(node->data);
        free(node);
    }
}

int list_size(const doubly_node_t *head) {
    int size = 0;
    while (head != NULL) {
        head = head->next;
        size++;
    }
    return size;
}


typedef struct {
    char *identifier;
    char *value;
} var_t;

int varc = 0;
doubly_node_t *vars_list_head = NULL;

void sigchild_handler() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        if (pid > 0) {
            printf("Child process with pid %i terminated\n", pid);
        } else {
            perror("waitpid");
        }
    }
}

void setup_environment() {
    chdir("/");
    if (signal(SIGCHLD, sigchild_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
}

int read_line(char *buffer, const int size) {
    if (size <= 0) {
        errno = ENOBUFS;
        return -1;
    }
    if (fgets(buffer, size, stdin) == NULL) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        }
        perror("read_line");
        return -1;
    }
    buffer[strlen(buffer) - 1] = '\0';
    return 0;
}

void next_token(const char *str, size_t *index, const char *ignore) {
    const size_t ignored_end = strspn(str, ignore);
    *index += ignored_end;
}

char * consume_token(char *str, size_t *index, const char *delimiter) {
    const size_t token_end = strcspn(str, delimiter);
    if (token_end > 0) {
        str[token_end] = '\0';
        *index += token_end + 1;
        return strdup(str);
    }
    return NULL;
}

int parse_command(char *buffer, int *argc, char ***argv) {
    const char* SEPARATOR = " =\t\n\r\f\v";
    size_t offset = 0;
    doubly_node_t *arg_list_head = NULL;

    // Parse command
    next_token(buffer, &offset, SEPARATOR);
    char *command = consume_token(buffer + offset, &offset, SEPARATOR);
    if (command == NULL) {
        // Empty command
        return -1;
    }
    add_last(&arg_list_head, command);

    if (strcmp(arg_list_head->data, "export") == 0) {
        // Parse identifier
        next_token(buffer + offset, &offset, SEPARATOR);
        char *identifier = consume_token(buffer + offset, &offset, SEPARATOR);
        if (identifier == NULL ||
            identifier[0] == '=') {
            // No identifier
            fprintf(stderr, "parse_command: Expected identifier\n");
            return -1;
        }
        add_last(&arg_list_head, identifier);
        next_token(buffer + offset, &offset, SEPARATOR);

        // Parse string value
        if (buffer[offset] == '"') {
            offset++;
            size_t quote_offset = strcspn(buffer + offset, "\"");
            while (buffer[offset + quote_offset] != '"') {
                if (read_line(buffer + offset + quote_offset, BUFFER_SIZE - offset - quote_offset)) {
                    perror("read_line");
                    return -1;
                }
                quote_offset += strcspn(buffer + offset + quote_offset, "\"");
            }
            add_last(&arg_list_head, consume_token(buffer + offset, &offset, "\""));
        }
        else {
            // Parse literal value
            char *literal = consume_token(buffer + offset, &offset, SEPARATOR);
            if (literal == NULL) {
                if ((literal = strdup("")) == NULL) {
                    perror("strdup");
                    return -1;
                }
            }
            add_last(&arg_list_head, literal);
        }
    }
    else if (strcmp(arg_list_head->data, "echo") == 0){
        char *str = consume_token(buffer + offset, &offset, "\n");
        if (str != NULL) {
            add_last(&arg_list_head, str);
        }
    }
    else {
        // Parse arguments
        next_token(buffer + offset, &offset, SEPARATOR);
        char *arg = consume_token(buffer + offset, &offset, SEPARATOR);
        while (arg != NULL) {
            add_last(&arg_list_head, arg);
            next_token(buffer + offset, &offset, SEPARATOR);
            arg = consume_token(buffer + offset, &offset, SEPARATOR);
        }
    }

    // Convert arg list to array
    *argc = list_size(arg_list_head);
    *argv = calloc(*argc + 1, sizeof(char *));

    const doubly_node_t *current = arg_list_head;
    for (int i = 0; i < *argc; i++) {
        (*argv)[i] = strdup(current->data);
        current = current->next;
    }

    free_doubly_linked_list(arg_list_head);
    return 0;
}

char * eval_var(const char *identifier) {
    const doubly_node_t *curr = vars_list_head;
    while (curr != NULL) {
        const var_t *var = curr->data;
        if (strcmp(var->identifier, identifier) == 0) {
            return var->value;
        }
        curr = curr->next;
    }
    return "";
}

void expand_command(const int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '$') {
            const char *var = eval_var(argv[i] + 1);
            if ((argv[i] = strdup(var)) == NULL) {
                perror("strdup");
            }
        }
    }
}

void execute_command(int *argc, char *argv[]) {
    if (argv == NULL) return;

    bool foreground = true;
    if (argv[1]) {
        foreground = strcmp(argv[*argc - 1], "&");
        if (!foreground) {
            argv[*argc - 1] = NULL;
            (*argc)--;
        }
    }

    const char *command = argv[0];

    // Check for built-in commands
    if (strcmp(command, "exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    if (strcmp(command, "cd") == 0) {
        if (chdir(argv[1])) {
            perror("chdir");
        }
    }
    else if (strcmp(command, "pwd") == 0) {
        char *cwd = getcwd(NULL, 0);
        if (cwd != NULL) {
            printf("%s\n", cwd);
        }
        else {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
    }
    else if (strcmp(command, "echo") == 0) {
        if (argv[1]) {
            printf("%s\n", argv[1]);
        }
    }
    else if (strcmp(command, "export") == 0) {
        if (!argv[1]) {
            errno = EINVAL;
            perror("export");
            exit(EXIT_FAILURE);
        }
        var_t *v;
        if ((v = malloc(sizeof(var_t))) == NULL ||
            (v->identifier = strdup(argv[1])) == NULL ||
            (v->value = strdup(argv[2])) == NULL
            ) {
            perror("malloc/strdup");
            exit(EXIT_FAILURE);
        }
        add_last(&vars_list_head, v);
    }
    else {
        const pid_t pid = fork();

        if (pid == 0) {
            // Execute external command
            execvp(argv[0], argv);
            perror("shell");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            if (foreground) {
                waitpid(pid, NULL, 0);
            }
        }
        else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
}

int main(void) {
    setup_environment();

    while (true) {
        char buffer[BUFFER_SIZE] = {'\0'};
        char **argv = NULL;
        int argc = 0;

        read_line(buffer, BUFFER_SIZE);
        parse_command(buffer, &argc, &argv);
        expand_command(argc, argv);
        execute_command(&argc, argv);

        for (int i = 0; i < argc; i++) {
            free(argv[i]);
        }
        free(argv);
    }
}
