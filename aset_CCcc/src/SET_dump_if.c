/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#include "basic_hdrs.h"
#include "fe_common.h"
#include "symbol_ref.h"
#include "cmd_line.h"
#include "SET_dump_if.h"
#include "SET_ast.h"
#include "il_to_str.h"
#include "il_walk.h"
#include "SET_symid.h"
#include "SET_preprocess.h"
#include "SET_process_entry.h"
#include "SET_additions.h"
#include "SET_symbol.h"
#include "SET_dump_if.h"
#include "SET_names.h"
#include "SET_complaints.h"
#include "SET_multiple_iffs.h"

void (*walk_entry_and_subtree_for_IF)(char*, an_il_entry_kind) = SET_walk_entry_and_subtree;

/* The following variable is the file to which the IF information is */
/* to be dumped. */

FILE* IF_file;

/* The following variable controls whether to ignore or process the
 * notification of block entry (dumping the SET information is not the
 * only use of IL tree walking).
 */

a_boolean doing_IF_dump;
    
/* The following function dumps each reference from the saved xref. */

static void dump_xref() {
    an_xref_entry_ptr ep;
    an_xref_entry_ptr end_loc;
    init_xref_entry_iterator();
    while (next_xref_entry(&ep, &end_loc)) {
	void *id_entry = NULL;
        an_il_entry_kind id_kind;
	string_buffer_ptr output_buffer = get_buffer_from_source_seq(ep->source_position.seq);
	if (output_buffer == NULL) {
	    continue;
	}
	if (ep->source_position.seq) {
	    if (ep->srk_flags & SRK_LITERAL_REF) {
		/* not a symbol ptr, it's an a_constant_ptr */
		a_constant_ptr constant = (a_constant_ptr) ep->sym_ptr;
                if (constant->from_huge_init)
                    continue;
		if (!il_entry_prefix_of(constant).SET_symid) {
		    /* oops, missed this one during the IL walk --
		     * we have to go back and handle it specially.
		     */
		    write_symbol_of_entry((char*) constant, iek_constant, FALSE, TRUE);
		}
		id_entry = constant;
                id_kind = iek_constant;
	    }
	    else switch (ep->sym_ptr->kind) {
	    case sk_undefined:
	    case sk_macro:
	    case sk_parameter:
		continue;	/* ignore */

	    case sk_keyword:
		break;	/* completely handled below */
		
	    case sk_constant: {
		a_constant_ptr cp = ep->sym_ptr->variant.constant;
                if( cp->from_huge_init )
                    continue;

		if (!il_entry_prefix_of(cp).SET_symid) {
		    /* Missed this constant during the tree walk.  See similar
		     * code for types in next case for explanatory comments.
		     */
		    if (interesting_node(cp)) {
			insert_sym_prefix("constant", cp, iek_constant,
					  /*compiler_generated=*/FALSE,
					  FALSE, output_buffer);
			form_name(&cp->source_corresp, iek_constant,
				  ocb_of_string_buf(output_buffer));
			add_1_char_to_string('\"', output_buffer);
			terminate_string(output_buffer);
			if (cp->kind != ck_template_param) {
			    complain_ulong(error_csev, "Missed constant [$1] during tree walk.",
					   SET_symid_and_write_sym(cp, iek_constant, output_buffer));
			}
		    }
		}
		id_entry = cp;
                id_kind = iek_constant;
	    }
		break;

	    case sk_type: {
		a_type_ptr type = ep->sym_ptr->variant.type.ptr;
		if (!il_entry_prefix_of(type).SET_symid) {
		    /* We didn't see this type in the il walk.  That's
		     * legitimate for template type parameters and for types
		     * from "uninteresting" files, but we should complain
		     * otherwise.  (Types from "uninteresting" files will
		     * automatically get a SYM line generated when
		     * SET_symid_of() assigns a symid, so no overt action
		     * here is required.
		     */
		    if (interesting_node(type)) {
			insert_sym_prefix("type", type, iek_type,
					  /*compiler_generated=*/FALSE,
					  FALSE, output_buffer);
			form_name(&type->source_corresp, iek_type,
				  ocb_of_string_buf(output_buffer));
			add_1_char_to_string('\"', output_buffer);
			terminate_string(output_buffer);
			if (type->kind != tk_template_param) {
			    complain_ulong(error_csev, "Missed type [$1] during tree walk.",
					   SET_symid_and_write_sym(type, iek_type, output_buffer));
			}
		    }
		}
		id_entry = type;
                id_kind = iek_type;
	    }
		break;

	    case sk_enum_tag:
		id_entry = ep->sym_ptr->variant.enumeration.type;
		id_kind = iek_type;
		break;

	    case sk_class_or_struct_tag:
	    case sk_union_tag: {
		a_type_ptr tp = ep->sym_ptr->variant.class_struct_union.type;
		if (!il_entry_prefix_of(tp).SET_symid) {
                    write_symbol_of_entry((char *)tp, iek_type, FALSE, TRUE);
		}
		id_entry = tp;
		id_kind = iek_type;
	    }
		break;

	    case sk_variable:
		id_entry = ep->sym_ptr->variant.variable.ptr;
		id_kind = iek_variable;
		break;

	    case sk_static_data_member: {
		if (ep->sym_ptr->variant.static_data_member.instance_ptr &&
		    ep->sym_ptr->variant.static_data_member.instance_ptr->template_info &&
		    ep->sym_ptr->variant.static_data_member.instance_ptr->template_info->
		    il_template_entry) {
		    id_entry = ep->sym_ptr->variant.static_data_member.
				   instance_ptr->template_info->il_template_entry;
		    id_kind = iek_template;
		}
		else {
		    id_entry = ep->sym_ptr->variant.static_data_member.variable;
		    id_kind = iek_variable;
		}
	    }
		break;

	    case sk_field:
		id_entry = ep->sym_ptr->variant.field.ptr;
		id_kind = iek_field;
		break;

	    case sk_routine:
	    case sk_member_function:
		if (ep->sym_ptr->variant.routine.instance_ptr &&
		    ep->sym_ptr->variant.routine.instance_ptr->template_info &&
		    ep->sym_ptr->variant.routine.instance_ptr->template_info->
		    il_template_entry) {
		    id_entry = ep->sym_ptr->variant.routine.instance_ptr->
				   template_info->il_template_entry;
		    id_kind = iek_template;
		}
		else id_entry = ep->sym_ptr->variant.routine.ptr;
		id_kind = iek_routine;
		break;

	    case sk_label:
		id_entry = ep->sym_ptr->variant.label.ptr;
		id_kind = iek_label;
		break;

	    case sk_projection:
		/* TBD: do these occur in the xref? */
 		break;

	    case sk_overloaded_function:
		/* TBD: do these occur in the xref? */
		break;

	    case sk_class_template:
	    case sk_function_template:
		id_entry = ep->sym_ptr->variant.template_info->
			       il_template_entry;
		id_kind = iek_template;
		break;

	    case sk_namespace:
		id_entry = ep->sym_ptr->variant.namespace_info.ptr;
		id_kind = iek_namespace;
		break;

	    case sk_namespace_projection:
		/* TBD */
		break;

	    default:
		break;
	    }
	}
	if ((ep->srk_flags & SRK_IMPLICIT) == 0) {
	    a_boolean is_keyword = (ep->srk_flags & SRK_LITERAL_REF) == 0
				   && ep->sym_ptr->kind == sk_keyword;
	    unsigned long symid = id_entry != NULL ? SET_symid_and_write_sym(id_entry, id_kind, output_buffer)
                                                   : error_symid;
	    a_boolean good_loc = TRUE;
	    const char *loc = NULL;
	    if (end_loc) {
		good_loc = add_pos_to_string(&ep->source_position, &trial_buf)
		        && add_pos_to_string(&end_loc->source_position, &trial_buf);
	    }
	    else {
		good_loc = add_pos_and_len_to_string(&ep->source_position, &trial_buf);
	    }
   	    loc = terminate_string(&trial_buf);
	    if ((is_keyword || symid != error_symid) && good_loc) {
		add_3_chars_to_string("SMT", output_buffer);
		if (is_keyword) {
		    add_to_string(" keyword \"", output_buffer);
		    add_to_string(ep->sym_ptr->header->identifier, output_buffer);
		    add_1_char_to_string('\"', output_buffer);
		}
		else {
		    add_symid_to_string(symid, output_buffer);
		}
		add_to_string(loc, output_buffer);
		terminate_string(output_buffer);
	    }
	}
    } /* end of while */
}

/* The following function creates ERR lines for each saved message. */

static void dump_err_msgs()
{
    string_buffer_ptr output_buffer = (generate_multiple_IF_files) ? 
                                       NULL : &output_buf;
    a_saved_message_ptr msg;
    init_saved_message_iterator();
    while (msg = next_saved_message()) {
       if (generate_multiple_IF_files) {
           /* Place the ERR node in the file that the error message 
            * originally came from. 
            */
           output_buffer = get_buffer_from_source_seq(msg->pos.seq); 
       }
       if (output_buffer) {
	    a_boolean good_loc = add_sym_def_pos_to_string(&msg->pos, &trial_buf);
	    const char *loc = terminate_string(&trial_buf);
	    if (good_loc) {
		add_3_chars_to_string("ERR", output_buffer);
		add_to_string(loc, output_buffer);
		add_quoted_str_to_string(msg->text, /*add_quotes=*/TRUE,
					 output_buffer);
		terminate_string(output_buffer);
	    }
	}
    }
}

/* Always adds at least 1 space char; always begins and ends with a space.
 */
static a_boolean add_pos_from_ast_node_to_string(ast_node_info_ptr astp,
                                                 string_buffer_ref output_buffer)
{
    a_boolean ret = FALSE;
    if (!astp->use_len) {
        if (add_pos_to_string(&astp->start_pos, output_buffer)) {
            add_pos_to_string(&astp->end_pos, output_buffer);
            ret = TRUE;
        }
    }
    else {
        ret = add_pos_and_len_to_string(&astp->start_pos, output_buffer);
    }
    return ret;
}

/* The following function adds a specified number of blanks to the
 * output buffer.
 */

void indent_to_depth(int depth, string_buffer_ref output_buffer) {
    int i;
    char* sixtyfour_spaces =
	    "                                                                ";
           /*         1         2         3         4         5         6    */
           /*1234567890123456789012345678901234567890123456789012345678901234*/
    for (i = 0; i < depth / 64; i++) {
	add_to_string_with_len(sixtyfour_spaces, 64, output_buffer);
    }
    add_to_string_with_len(sixtyfour_spaces, depth % 64, output_buffer);
}

/* The following function recursively dumps a given ast_node_info
 * object, its children, and its siblings.  The depth passed in is
 * used to control the indentation of the output.
 */

static void walk_ast(ast_node_info_ptr np, int depth, 
		     string_buffer_ptr output_buffer) {
    if (output_buffer == NULL) {
	return;
    }
    while (np) {
	if (np->is_leaf) {
	    if (indent_AST) {
		indent_to_depth(depth, output_buffer);
	    }
	    add_to_string(np->text, output_buffer);
            add_pos_from_ast_node_to_string(np, output_buffer);
	    terminate_string(output_buffer);
	}
	else if (!np->suppress_subtree) {
	    if (!np->suppress_node) {
		if (indent_AST) {
		    indent_to_depth(depth, output_buffer);
		}
		if (GNU_compatible_AST) {
		    add_to_string(node_name_str_gnu[np->node_name],
				  output_buffer);
		}
		else add_to_string(node_name_str[np->node_name],
				   output_buffer);
		add_to_string(np->text, output_buffer);
                add_pos_from_ast_node_to_string(np, output_buffer);
		add_1_char_to_string('{', output_buffer);
		if (np->first_child) {
		    terminate_string(output_buffer);
		    walk_ast(np->first_child, depth + 1, output_buffer);
		    if (indent_AST) {
			indent_to_depth(depth, output_buffer);
		    }
		}
		add_1_char_to_string('}', output_buffer);
		terminate_string(output_buffer);
	    }
	    else walk_ast(np->first_child, depth, output_buffer);
	}
        if (np == np->next_sibling) {
            complain(error_csev, "Encountered loop in the ast structure.");
            break;
        }
	np = np->next_sibling;
    }
}

/* The following function walks the list of top-level ast_node_info
 * objects, calling walk_ast for each to dump the entire ast_node_info
 * hierarchy.
 */

static void dump_ast() {
    int i;
    intr_info_ref iip;

    if (intr_info_array) { 
        for (i = 0; i < intr_info_count; i++) {
	    iip = intr_info_array[i];
	    if (iip->first_ast_node || iip->pp_ast) {
		string_buffer_ref output_buffer
			= get_buffer_from_intr_info(iip);

		add_to_string("AST {", output_buffer);
		terminate_string(output_buffer);

		if (generate_multiple_IF_files) {
		  add_to_string("ast_root {", output_buffer);
		  terminate_string(output_buffer);
		}
		walk_ast(iip->first_ast_node, 1, output_buffer);
		if (generate_multiple_IF_files) {
		  add_1_char_to_string('}', output_buffer);
		  terminate_string(output_buffer);
		}

		/* write preprocessor AST here if pp_ast is set in
		   write_pp_IF_for */
		if (iip->pp_ast) {
		  write_pp_AST_info(iip->pp_ast, output_buffer);
		}

		add_1_char_to_string('}', output_buffer);
		terminate_string(output_buffer);
	    }
	}
    }
}

/* The following function traverses ast tree to write a SYM line
 * and associated relations if necessary.
 */
  
static void write_symbol_for_ast_node(ast_node_info_ptr astp,
				      string_buffer_ref output_buffer)
{
  if (generate_multiple_IF_files) {
    while (astp) {
        if (!astp->suppress_subtree) {
            if (!astp->suppress_node) {
                /* Write SYM line, etc. */
                if (astp->entry) {
	            write_symbol_for_il_node(astp->entry->ptr, 
					     astp->entry->kind, 
					     output_buffer); 
                }
	    }
            /* Recursively call this function for its children */
            if (astp->first_child) {
	        write_symbol_for_ast_node(astp->first_child, output_buffer);
            }
        }
        if (astp == astp->next_sibling) {
            complain(error_csev, "Encountered loop in the ast structure.");
            break;
        }      
        astp = astp->next_sibling;
      }
  }
}

static void write_sym_info_of_ast_reference(void)
{
    if (generate_multiple_IF_files && intr_info_array) {
        int i;
        for (i = 0; i < intr_info_count; i++) {
	    intr_info_ref iip = intr_info_array[i];
	    string_buffer_ref output_buffer = get_buffer_from_intr_info(iip);
            /* Write SYM lines, etc. for symbol references in ast nodes. */
	    write_symbol_for_ast_node(iip->first_ast_node,
				      output_buffer);
	}
    }
}

/* The following function dumps all the information collected during
 * this parse to the IF file specified on the command line.  If the
 * specified file cannot be opened, it prints a complaining message on
 * stderr and returns; otherwise, it walks the IL tree, dumping
 * information about declarations as it goes and bulding the parallel
 * ast_node_info tree, then dumps the accumulated cross reference
 * information, and finally dumps the ast_node_info tree.
 */

void dump_SET_IF() {
    char psf_iff[MAX_FILENAME_SIZE];
#if PROTOTYPE_INSTANTIATIONS_IN_IL
    if (iek_last !=   iek_static_assertion + 1) {
	complain(catastrophe_csev, "Entry kinds have been added, switch must be updated.");
        /* switches include the giant one in SET_symbol.c and one in
         * SET_ast.c: process_ast_of_entry().  There may also be one
         * in walk_entry.h
         */
    }
#else /* !PROTOTYPE_INSTANTIATIONS_IN_IL */
    if (iek_last != iek_switch_case_entry + 1) {
	complain(catastrophe_csev, "Entry kinds have been added, switch must be updated.");
    }
#endif /* PROTOTYPE_INSTANTIATIONS_IN_IL */
    validate_ast_structures();
    if (alternate_IF_walk) {
        /* This option uses the original EDG tree walk while dumping IF.
         * It is not tested, or even intended for anything but debugging.
	 */
	walk_entry_and_subtree_for_IF = &walk_entry_and_subtree;
    }
#if DEBUG
    init_db();
#endif /* DEBUG */
    if (IF_file_name) {
    	if (strcmp(IF_file_name, "-") == 0) {
	    IF_file = stdout;
	}
        else {
#ifdef _WIN32
	    IF_file = fopen(IF_file_name, "ab");
#else
	    IF_file = fopen(IF_file_name, "a");
#endif
	    if (!IF_file) {
	        complain_str(catastrophe_csev, "Cannot open IF file $1.", IF_file_name);
	    }
	}
    }
    if (IF_file || generate_multiple_IF_files) {
        a_memory_region_number region_number;
	string_buffer_ptr output_buffer = (generate_multiple_IF_files) ?
	                                      get_primary_source_buffer() :
					      &output_buf;
        if (!generate_multiple_IF_files) {
#ifdef _WIN32
	    set_string_buffer_terminator('\r', &output_buf);
#endif       
	    add_SMT_language_to_string(&output_buf);
	    add_to_string("SMT language \"cpp\"", &output_buf);
	    terminate_string(&output_buf);
	}
	write_pp_IF_for(il_header.primary_source_file);
	if (!generate_multiple_IF_files) {
	    write_syms_for_undefined_macros();
	    add_SMT_language_to_string(&output_buf);
	    write_smt_file("", &output_buf);	/* TBD: "output" name */
        }

	reset_meters_after_preprocessing();
	doing_IF_dump = TRUE;
	walk_file_scope_il(&process_entry, NULL, NULL, NULL, NULL, 0);
	for (region_number = FILE_SCOPE_REGION_NUMBER + 1;
	    region_number <= highest_used_region_number;
	    region_number++) {
	    if (il_header.region_scope_entry[region_number] != NULL) {
		walk_routine_scope_il(region_number, &process_entry,
				      NULL, NULL, NULL, NULL, 0);
	    }
	}
	check_ast_consistent();
	dump_xref();
	retry_process_symbol();
	write_sym_info_of_ast_reference();
	check_expected_SYMs(output_buffer);
	dump_err_msgs();
	if (!AST_is_corrupted) {
	    dump_ast();
	}
	if (generate_multiple_IF_files) {
	    flush_all_output_buffers();
	}
	else {
	    handle_output_overflow(&output_buf);	/* flush */
	    fclose(IF_file);
	}
	if (SET_memory_metering) {
	    dump_memory_meters();
	}
	if (SET_dump_file_table) {
	   file_table_dump_hash_table(interesting_file_table, stdout);
	}
    }
}
