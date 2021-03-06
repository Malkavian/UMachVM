/*
 * collect_data.c
 * 
 * functions to collect variables defined by the user
 * 
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "uasm.h"
#include "collect_data.h"
#include "symbols.h"

static GSList *data_list = NULL;

GSList *get_data_list() {
    return data_list;
}

// insert a string variable into the data_list
void insert_string_data(char *label, char *value) {
    data_t *data = malloc(sizeof(data_t));
    
    data->type = DATATYPE_STRING;
    data->string_data.label = g_strdup(label);
    // convert all escape sequences into the matching 1-byte ASCII code
    data->string_data.value = g_strcompress(value);

    data_list = g_slist_prepend(data_list, data);
}

// insert a integer variable into the data_list
void insert_int_data(char *label, int32_t value) {
    data_t *data = malloc(sizeof(data_t));
   
    data->type = DATATYPE_INT;
    data->int_data.label = g_strdup(label);
    data->int_data.value = value;

    data_list = g_slist_prepend(data_list, data);
}

// insert all collected variables into the symbol table
int insert_data_symbols(asm_context_t *cntxt) {
    data_list = g_slist_reverse(data_list);

    for (GSList *l = data_list; l != NULL; l = g_slist_next(l)) {
        data_t *data = l->data;

        symbol_t *sym = malloc(sizeof(symbol_t));
        
        switch (data->type) {
        case DATATYPE_STRING:
            sym->sym_name = strdup(data->string_data.label);
            sym->sym_type = SYMTYPE_STRDAT;
            sym->sym_addr = cntxt->current_addr;

            if (!insert_symbol(sym)) {
                print_error(cntxt, "String constant <%s> already defined", sym->sym_name);
                free(sym->sym_name);
                free(sym);
                return FALSE;
            }

            // find next ALIGNED address
            cntxt->current_addr += (strlen(data->string_data.value) + 1);
            while (cntxt->current_addr % 4 != 0)
                cntxt->current_addr++;
                
            break;
            
        case DATATYPE_INT:
            sym->sym_name = strdup(data->int_data.label);
            sym->sym_type = SYMTYPE_INTDAT;
            sym->sym_addr = cntxt->current_addr;

            if (!insert_symbol(sym)) {
                print_error(cntxt, "Integer constant <%s> already defined", sym->sym_name);
                free(sym->sym_name);
                free(sym);
                return FALSE;
            }

            cntxt->current_addr += 4;
            break;
        }
    }

    return TRUE;
}

static void free_data_helper(gpointer data) {
    data_t *d = data;

    switch (d->type) {
    case DATATYPE_INT:
        g_free(d->int_data.label);
        free(d);
        break;
    case DATATYPE_STRING:
        g_free(d->string_data.label);
        g_free(d->string_data.value);
        free(d);
        break;
    }
}

void free_data() {
    g_slist_free_full(data_list, free_data_helper);
    data_list = NULL;
}
