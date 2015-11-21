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
#include "SET_symid.h"
#include "SET_additions.h"
#include "SET_ast.h"
#include "SET_complaints.h"
#include "SET_symbol.h"

/* -------------------- SYMID MANAGEMENT --------------------
 *
 * variables:	next_symid ellipsis_symid
 *
 * functions:	symid_of_ellipsis assign_symid
 *
 * macros:	SET_symid_of
 *
 * The SYM lines in the IF associate a unique integer identifier with
 * each symbol; those unique identifiers are referred to in this code
 * as symids.  The macro SET_symid_of assigns a symid to a given IL
 * node (using the assign_symid function), if it does not already have
 * one, and returns the IL node's symid as its value.  There is no
 * symbol for ellipsis in the IL, but if a function or catch uses "...",
 * a symid is allocated on the fly (generating the appropriate SYM line)
 * and returned by symid_of_ellipsis.
 */


/* The following value is an error value for symids; it prevents a
 * reference from assigning a new symid, but is guaranteed not to
 * match any real symid (in any practical compilation, at least!).
 */

unsigned long error_symid = 0xfffffff;

/* Joins an entity to a clique.
 * A clique is a set of IL nodes corresponding to the same symbol and name
 * in the IF.  We would be happier if the cliques could all be formed
 * prior to numbering any of them, but that would take another pass.
 * Consequently, the caller needs to be careful not to use a symid
 * of an IL node which is still isolated from its eventual clique.
 *
 * If joiner or clique already has a symid assigned, then respectively the
 * clique or joiner is updated with the same symid assignment.
 * To preserve the invariants of cliques and symids,
 * join_to_clique will not join when distinct symids have already been
 * assigned to joiner and clique.
 *
 * If joiner is already a member of a clique with size > 1,
 * then the two cliques are merged.
 * If joiner is already a member of the other clique, does nothing.
 */
static void join_to_clique(void *joiner, void *clique) {
    void *j = joiner;
    void *c = clique;
    void *pc, *pj, *next_pc, *next_pj;
    an_il_entry_prefix_ptr ce = &il_entry_prefix_of(c);
    an_il_entry_prefix_ptr je = &il_entry_prefix_of(j);
    an_il_entry_prefix_ptr pce, pje;
    if (ce->SET_symid != 0) {
	if (je->SET_symid != 0) {
	    complain_ulong_ulong(error_csev, "Attempt to conflate symbols [$1] and [$2].\n",
				 ce->SET_symid, je->SET_symid);
        }
    }
    else if (il_entry_prefix_of(j).SET_symid != 0) {
	/* Swap so that j is the one with symid unassigned. */
	void *t = j;
	j = c;
	je = ce;
	c = t;
        ce = &il_entry_prefix_of(c);
    }
    /* Find ptr to c. */
    for (pc = c;; pc = next_pc) {
	pce = &il_entry_prefix_of(pc);
	if (pc == j) {
	    /* the cliques are the same already */
	    return;
	}
        next_pc = pce->SET_clique;
        if (next_pc == c) {
	    break;
	}
    }
    /* Find ptr to j; conform joiners to clique. */
    for (pj = j;; pj = next_pj) {
	pje = &il_entry_prefix_of(pj);
	pje->SET_symid = ce->SET_symid;
#if DEBUG
        if (ce->SET_symid != 0 && db_symid) {
            printf("assigning symid [%d] to %lx\n", (int)ce->SET_symid, (long)pj);
	}
#endif
	pje->SET_SYM_written = ce->SET_SYM_written;
        if(generate_multiple_IF_files) {
            /* free existing SET_outbuf_bit_arr */
            if (pje->SET_outbuf_bit_arr && 
                pje->SET_outbuf_bit_arr != ce->SET_outbuf_bit_arr) {
                free(pje->SET_outbuf_bit_arr);
            }
            pje->SET_outbuf_bit_arr = ce->SET_outbuf_bit_arr;
        } 

	next_pj = pje->SET_clique;
	if (next_pj == j) {
	    break;
	}
    }
    pce->SET_clique = j;
    pje->SET_clique = c;
}

/* The following function makes two related entities (e.g., an a_template
 * and its associated a_type) have the same symid, if possible, and
 * complains if it's not possible.  If unsuccessful, emits the
 * message as a complaint or a worry, depending on importance,
 * and returns FALSE.
 */

a_boolean attempt_same_symid(void* entity1, an_il_entry_kind kind1, void* entity2, an_il_entry_kind kind2,
			     const char *message) {
    unsigned long symid1;
    unsigned long symid2;
#if DEBUG
    if (db_same_symid) {
	/* Suppress identification of symbols, and report them different right away. */
	symid1 = SET_symid_of(entity1, kind1);
	symid2 = SET_symid_of(entity2, kind2);
    }
    else
#endif
    {
	symid1 = il_entry_prefix_of(entity1).SET_symid;
	symid2 = il_entry_prefix_of(entity2).SET_symid;
    }
    if (symid1 == 0 || symid2 == 0) {
#if DEBUG
	if (db_symid) {
	    printf("linking IF SYM clique of %s at %lx with %s at %lx\n",
                   il_entry_kind_names[kind1], (long)entity1,
                   il_entry_kind_names[kind2], (long)entity2);
	}
#endif
        join_to_clique(entity1, entity2);
    }
    else if (symid1 != symid2) {
	complain_ulong_ulong(error_csev, message, symid1, symid2);
	return FALSE;
    }
    return TRUE;
}

/* The following function makes two related entities (e.g., an a_template
 * and its associated a_type) have the same symid, if possible, and
 * complains if it's not possible.
 */

void ensure_same_symid(void* entity1, an_il_entry_kind kind1, void* entity2, an_il_entry_kind kind2) {
    attempt_same_symid(entity1, kind1, entity2, kind2,
	               "Symbols [$1] and [$2] should be the same but are not.");
}


unsigned long SET_symid_and_write_sym(void* ilp, 
				      an_il_entry_kind il_kind,
				      string_buffer_ptr output_buffer)
{
  if (generate_multiple_IF_files) {
    write_symbol_for_il_node(ilp, il_kind, output_buffer); 
  }

  return SET_symid_of(ilp, il_kind);
}

/* The following function will add il entry and kind to the ast_node_info
 * so that a SYM line can be generated later during walk_ast (if needed
 * to the appropriate file/buffer) for all the references that do
 * not have a SYM line.
 */

unsigned long SET_symid_and_add_to_ast(void* ilp,
				       an_il_entry_kind il_kind,
				       ast_node_info_ptr astp)
{
  if (generate_multiple_IF_files && astp && !astp->entry) {
    a_tagged_pointer * tp = (a_tagged_pointer *)malloc(sizeof(a_tagged_pointer));
    if (tp) {
      tp->ptr = ilp;
      tp->kind = il_kind;
      
      astp->entry = tp;
    }
  }
  return SET_symid_of(ilp, il_kind);
}


/* The following variable is the numeric identifier of the "ellipsis"
 * type, that is, the type that is used for the dummy extra parameter
 * of a function with an ellipsis and for the type caught by
 * "catch(...)".  The default initialization of 0 is used to indicate
 * that it has not been used (and hence not assigned) yet.
 */

static unsigned long ellipsis_symid;

/* The following function returns the ellipsis_symid, initializing
 * it and writing the corresponding SYM and REL line if this is the
 * first use.
 */

unsigned long symid_of_ellipsis(string_buffer_ptr output_buffer) {
    a_boolean previously_initted = (ellipsis_symid != 0);
    if (!previously_initted) {
	ellipsis_symid = next_symid++;
    }
    if (generate_multiple_IF_files || !previously_initted) {
	write_ellipsis_symbol(ellipsis_symid, output_buffer);
    }
    return ellipsis_symid;
}	

/* The following function assigns the next available symid to the
 * specified IL entry, inserting it into its il_entry_prefix; if
 * the entry is from an "uninteresting" file (which presumably
 * only happens if it is referenced from an "interesting" file's
 * entity), a SYM line is inserted into the output.  [NOTE: this
 * processing assumes that output_buf is currently not busy, i.e.,
 * that the generated SYM line won't go into the middle of another
 * IF line.  That should be the case, since only RELs will be
 * using the symids of entities from uninteresting files and the
 * symid is fetched before the REL line is formatted.  If this
 * proves to be a problem, however, some mechanism for deferring
 * the SYM line will be needed.]
 *
 * The IL entry's clique may contain other entries, in which case
 * all associated entries are updated with the new symid.
 */

unsigned long assign_symid(void* ilp, an_il_entry_kind kind) {
    an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
    an_il_entry_prefix_ptr cep, fep = NULL;
    void *cilp, *silp = NULL;
    unsigned long symid = next_symid++;
    for (cilp = ilp;; cilp = cep->SET_clique) {
    	cep = &il_entry_prefix_of(cilp);
	cep->SET_symid = symid;
#if DEBUG
	if (db_symid) {
	    if (cilp == ilp) {
		printf("assigning symid [%d] to %s at %lx\n",
                       (int)symid, il_entry_kind_names[kind], (long)ilp);
	    }
	    else {
	        printf("assigning symid [%d] to %lx\n", (int)symid, (long)cilp);
	    }
	}
#endif
	if (cep->full_SET_dump) {
	    fep = cep;
	}
	else if (cep->SET_info) {
	    silp = cilp;
	}
	if (cep->SET_clique == ilp) {
	    break;
	}
    }
    /* Add to list of expected SYM records. */
    expect_SYM(ilp, kind);
    if (!generate_multiple_IF_files && fep == NULL && silp != NULL) {
	write_summary_sym_info(silp, kind, &output_buf);
    }
    return symid;
}

/* Give the il entry an easily-identifiable bogus symid.
 */
void assign_error_symid(void* ilp) {
    an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
    an_il_entry_prefix_ptr cep;
    void *cilp;
    if (ep->SET_symid != 0) {
	complain_ulong(error_csev, "Assigning error symid to symbol [$1].\n", ep->SET_symid);
    }
    for (cilp = ilp;; cilp = cep->SET_clique) {
	cep = &il_entry_prefix_of(cilp);
	cep->SET_symid = error_symid;
	/* Consider it written for consistency with expected_sym_count. */
	cep->SET_SYM_written = TRUE;
	if (cep->SET_clique == ilp) {
	    break;
	}
    }
}

#if DEBUG
/* If -d-xsym is used (setting db_expected_syms),
 * track SYM ids which have been used, but for which SYM records
 * have not yet been emitted.  This checks the normal cases in
 * which an error could occur due to a failure of fragile assumptions.
 * Other cases are not checked, e.g. file symbols, macro symbols
 * (which are checked in another way), and symbols for ellipsis
 * parameters.
 */

/* This code was disabled and superseded, because it was found
 * that the number of expected symbols could grow enormously.
 * In a program with about 50K symbols, the list of expected
 * symbols grew to 2K, and averaged 200.
 */
typedef struct expected_SYM {
    unsigned long symid;
    void *entry;
    an_il_entry_kind kind;
    struct expected_SYM *next;
} expected_SYM;

expected_SYM *free_expected_SYMs = NULL;
expected_SYM *expected_SYMs = NULL;
#endif /* DEBUG */

long expected_sym_count = 0;

/* Account for a symid being allocated.  This deals with ordinary symbols,
 * not macro or file symbols.  A problem occurs if a symid is allocated,
 * and no SYM line is subsequently written.
 */
void expect_SYM(void *ep, an_il_entry_kind kind) {
#if DEBUG
    if (db_expected_syms) {
	expected_SYM *es;
	if (free_expected_SYMs != NULL) {
	    es = free_expected_SYMs;
	    free_expected_SYMs = es->next;
	}
	else {
	    es = (expected_SYM *)malloc(sizeof(expected_SYM));
	}
	es->symid = il_entry_prefix_of(ep).SET_symid;
	es->entry = ep;
	es->kind = kind;
	es->next = expected_SYMs;
	expected_SYMs = es;
    }
#endif /* DEBUG */
    expected_sym_count += 1;
}

/* Account for a SYM line being written.  This deals with ordinary symbols,
 * not macro or file symbols.  All il nodes associated with the same
 * symid are marked as written.
 */
void fulfill_SYM(void *ilp) {
    an_il_entry_prefix_ptr ep = &il_entry_prefix_of(ilp);
#if DEBUG
    if (db_expected_syms) {
	unsigned long id = ep->SET_symid;
	expected_SYM *prev = NULL;
	expected_SYM *es;
	/* Check the id rather than the ep pointer, because it's OK
	 * to define the SYM using a different il entry than the one
	 * for which it was originally assigned.
	 */
	for (es = expected_SYMs; es != NULL; prev = es, es = es->next) {
	    if (es->symid == id) {
		break;
	    }
	}
	if (es == NULL) {
	    /* did not expect this sym */
	}
	else {
	    /* Remove from expected list. */
	    if (prev == NULL) {
		expected_SYMs = es->next;
	    }
	    else {
		prev->next = es->next;
	    }
	    /* Add to free list. */
	    es->next = free_expected_SYMs;
	    free_expected_SYMs = es;
	}
    }
#endif /* DEBUG */
    if (!ep->SET_SYM_written) {
	an_il_entry_prefix_ptr p;
	for (p = ep;; p = &il_entry_prefix_of(p->SET_clique)) {
	    p->SET_SYM_written = TRUE;
	    if (p->SET_clique == ilp) {
		break;
	    }
	}
	expected_sym_count -= 1;
    }
    else {
	/* Not serious, because ifext removes duplicates. */
	worry_ulong("Multiple SYM lines for symbol [$1].", ep->SET_symid);
    }
}

/* Deal with any SYM ids which were assigned but did not have
 * SYM records emitted for them.  Complain.
 */
void check_expected_SYMs(string_buffer_ptr output_buffer) {
    if (generate_multiple_IF_files) {
        /* Disable this functionality in multiple IF generation mode. Need to
           come up with a mechanism that checks for each IF file.*/
        return;
    }
#if DEBUG
    if (db_expected_syms && output_buffer != NULL) {
	expected_SYM *es;
	for (es = expected_SYMs; es != NULL; es = es->next) {
	    complain_ulong(error_csev, "Missing SYM line for symbol [$1].", es->symid);
            /* write SYM */
	    write_referenced_symbol_info(es->entry, es->kind, 
					 output_buffer, TRUE);
	}
    }
#endif /* DEBUG */
    if (expected_sym_count > 0) {
	if (expected_sym_count == 1) {
	    complain(error_csev, "SYM line for 1 symbol omitted.");
	}
	else {
	    complain_ulong(error_csev, "SYM lines for $1 symbols omitted.", expected_sym_count);
	}
    }
    if (expected_sym_count < 0) {
	complain_ulong(error_csev, "SYM lines miscount by $1", -expected_sym_count);
    }
}

