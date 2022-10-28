#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

const char *log_file_path = "log/log.html";

int first_img_file_num = 0;

const size_t MAX_COMMAND_SIZE = 30;

enum List_errors {
    INDEX_IS_TOO_LARGE    = 1,
    PUSH_AFTER_FREE_INDEX = 2,
    POP_ZERO_ELEMENT      = 4,
};

struct List {
    Type_t *data;
    size_t *left;
    size_t *right;

    size_t size;
    size_t capacity;

    size_t first_free;
};

#define MIN(a, b) (((a) < (b) )? (a):(b))

#define MAX(a, b) (((a) > (b) )? (a):(b))

#define list_init(list) _list_init((&(list)))

#define list_dec(list) _list_dex((&(list)))

#define check_list(list) 

#ifdef LOG

#define warning(list, message) _log_warning(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define info(list, message)    _log_info(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

#define error(list, message)   _log_error(list, message, __PRETTY_FUNCTION__, __FILE__, __LINE__)

void dump(List *list); //only for Type_t == int

#else 

#define warning(message)

#define info(message)

#define error(message)

#endif

int list_push_front(List *list, Type_t a);

int list_push_back(List *list, Type_t a);

void _list_init(List *list);

void _list_dec(List *list);

int list_resize(List *list, size_t new_size);

int list_push_index(List *list, Type_t a, size_t index);

size_t list_num_to_index(List *list, size_t num);

size_t list_previous(List *list, size_t num);

size_t list_next(List *list, size_t num);

int list_pop_front(List *list);

int list_pop_back(List *list);

int list_pop_index(List *list, size_t num);

#ifdef LOG

void _log_info(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_error(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_warning(List *list, const char *message, const char *func, const char *file, const size_t line);

void _log_warning(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"orange\">warning:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

void _log_info(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"green\">info:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

void _log_error(List *list, const char *message, const char *func, const char *file, const size_t line) {
    assert(message != nullptr);

    assert(log_file_path != nullptr);
    FILE *file_ptr = fopen(log_file_path, "a");
    assert(file_ptr != nullptr);

    fprintf(file_ptr, "<font color=\"red\">info:\"%s\" in file:%s in func:%s in line:%zu</font>\n\n", message, file, func, line);

    fclose(file_ptr);

    dump(list);
}

#endif

int list_pop_index(List *list, size_t num) {
    check_list(list);
    info(list, "start pop index");

    if (num == 0) {
        return POP_ZERO_ELEMENT;
    }

    if (list->data[num] == FREE) {
        warning(list, "maybe pop free");
    }

    list->right[list->left[num]] = list->right[num];
    list->left[list->right[num]] = list->left[num];

    list->data[num]  = FREE;
    list->left[num]  = 0;
    list->right[num] = list->first_free;

    list->first_free = num;

    check_list(list);
    info(list, "end pop index");

    return 0;
}

int list_resize(List *list, size_t new_size) {
    check_list(list);
    info(list, "start resize");

    {
        size_t index = 0;

        do {
            new_size = MAX(new_size, index + 1);
            index = list->right[index];
        } while (index);
    }

    Type_t *new_data  = (Type_t *)calloc(sizeof(Type_t), new_size);
    size_t *new_left  = (size_t *)calloc(sizeof(size_t), new_size);
    size_t *new_right = (size_t *)calloc(sizeof(size_t), new_size);
    assert(new_data  != nullptr);
    assert(new_left  != nullptr);
    assert(new_right != nullptr);

    {
        while(list->first_free >= new_size) {
            list->first_free = list->right[list->first_free];
        }

        size_t free_index = list->first_free;

        while (free_index) {
            while (list->right[free_index] >= new_size) {
                list->right[free_index] = list->right[list->right[free_index]];
            }
            free_index = list->right[free_index];
        }
    }

    for (size_t i = 0; i < MIN(list->capacity, new_size); i++) {
        new_data[i]  = list->data[i];
        new_left[i]  = list->left[i];
        new_right[i] = list->right[i];
    }

    free(list->data);
    free(list->left);
    free(list->right);

    list->data  = new_data;
    list->left  = new_left;
    list->right = new_right;

    for (size_t i = MIN(list->capacity, new_size); i < new_size; i++) {
        list->data[i]  = FREE;
        list->left[i]  = 0;
        list->right[i] = (i + 1) % new_size;
    }

    if (list->first_free == 0) {
        list->first_free = MIN(list->capacity, new_size) % new_size;
    }
    else{
        size_t free_index = list->first_free;
        while (list->right[free_index]) {
            free_index = list->right[free_index];
        }
        list->right[free_index] = MIN(list->capacity, new_size) % new_size;
    }

    list->capacity = new_size;

    check_list(list);
    info(list, "end resize");

    return 0;
}

int list_push_index(List *list, Type_t a, size_t index) {
    check_list(list);
    info(list, "start push index");

    if (list->first_free == 0) {
        list_resize(list, list->capacity * 2);
    }

    if (index >= list->capacity) {
        return INDEX_IS_TOO_LARGE;
    }

    if (list->data[index] == FREE) {
        return PUSH_AFTER_FREE_INDEX;
    }

    size_t first_free = list->first_free;
    list->first_free  = list->right[first_free];

    list->data[ first_free] = a;
    list->left[ first_free] = index;
    list->right[first_free] = list->right[index];

    list->right[index] = first_free;

    list->left[list->right[first_free]] = first_free;

    check_list(list);
    info(list, "end pop index");

    return 0;
}

void _list_init(List *list) {
    assert(list != nullptr);

    FILE *file = fopen(log_file_path, "w");
    assert(file != nullptr);
    fprintf(file, "<pre>\n\n");
    fclose(file);

    list->data = (Type_t *)calloc(sizeof(Type_t), 1);
    assert(list->data != nullptr);
    list->data[0] = POISON;

    list->left = (size_t *)calloc(sizeof(size_t), 1);
    assert(list->left != nullptr);
    list->left[0] = 0;

    list->right = (size_t *)calloc(sizeof(size_t), 1);
    assert(list->right != nullptr);
    list->right[0] = 0;

    list->size = 0;
    list->capacity = 1;

    list->first_free = 0;
}

void _list_dec(List *list) {
    check_list(list);

    free(list->data);
    free(list->left);
    free(list->right);

    list->capacity = 0;
    list->size     = 0;
}

int list_push_back(List *list, Type_t a) {
    check_list(list);
    info(list, "start push back");

    return list_push_index(list, a, list->left[0]);
}

int list_push_front(List *list, Type_t a) {
    check_list(list);
    info(list, "start push front");

    return list_push_index(list, a, 0);    
}

int list_pop_back(List *list) {
    check_list(list);
    info(list, "start pop back");

    return list_pop_index(list, list->left[0]);
}

int list_pop_front(List *list) {
    check_list(list);
    info(list, "start pop front");

    return list_pop_index(list, list->right[0]);
}

size_t list_num_to_index(List *list, size_t num) {
    check_list(list);

    size_t index = 0;
    for (size_t i = 0; i < num; i++) {
        index = list->left[index];
    }
    return index;
}

size_t list_next(List *list, size_t num) {
    check_list(list);

    if (num >= list->capacity) {
        return 0;
    }

    return list->right[num];
}

size_t list_previous(List *list, size_t num) {
    check_list(list);

    if (num >= list->capacity) {
        return 0;
    }

    return list->left[num];
}

#ifdef LOG

#define GEN_SVC

#define ADD_SVC_TO_LOG_HTML

void dump(List *list) {
    GEN_SVC {
        FILE *file = fopen("log/input.dot", "w");

        fprintf(file, "digraph G {rankdir=LR;style=filled;graph [splines = headport splines=ortho];\n");
        for (size_t i = 0; i < list->capacity; i++) {
            if (list->data[i] == FREE) {
                fprintf(file, "VERTEX%zu[label=\"%zu | free | l = %zu | r = %zu\", shape=\"Mrecord\", style = filled, fillcolor = \"#c0ffee\"]\n", i, i, list->left[i], list->right[i]);
            }
            else if (list->data[i] == POISON) {
                fprintf(file, "VERTEX%zu[label=\"%zu | poison | l = %zu | r = %zu\", shape=\"Mrecord\"]\n", i, i, list->left[i], list->right[i]);
            }
            else{
                fprintf(file, "VERTEX%zu[label=\"%zu | data = %d | l = %zu | r = %zu\", shape=\"Mrecord\", style = filled, fillcolor = \"#decade\"]\n", i, i, list->data[i], list->left[i], list->right[i]);
            }
            
        }
        for (size_t i = 0; i < list->capacity - 1; i++) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[style=\"invis\", weight = 100]\n", i, i + 1);
        }
        for (size_t i = 0; i < list->capacity; i++) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[color=\"green\"]\n", i, list->right[i]);
        }
        for (size_t i = 0; i < list->capacity; i++ ) {
            fprintf(file, "VERTEX%zu->VERTEX%zu[color=\"red\"]\n", i, list->left[i]);
        }
        fprintf(file, "}");

        fclose(file);
    }

    ADD_SVC_TO_LOG_HTML {
        char *command = (char *)calloc(sizeof(char), MAX_COMMAND_SIZE);
        assert(command != nullptr);

        sprintf(command, "dot -Tsvg log/input.dot > log/img%d.svg", first_img_file_num);

        system(command);

        free(command);

        FILE *file = fopen(log_file_path, "a");
        assert(file != nullptr);

        fprintf(file, "<img src=\"img%d.svg\" height=\"300px\" width=\"400px\"/>\n\n", first_img_file_num);

        fclose(file);

        first_img_file_num++;
    }
}

#undef GEN_SVC

#undef ADD_SVC_TO_LOG_HTML

#endif