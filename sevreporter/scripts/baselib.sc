<root>
  <folder>
    <name>Java</name>
    <tag>java</tag>
    <folder>
      <name>Critical Style</name>
      <tag>critical_style</tag>
      <query>
        <name>Clone method does not call super.clone</name>
        <cmd>java:clone_super</cmd>
        <tag>clone_super</tag>
      </query>

      <query>
        <name>Comparison with NaN constant</name>
        <cmd>java:compare_nan</cmd>
        <tag>compare_nan</tag>
      </query>

      <query>
        <tag>equals_class</tag>
        <name>Equals method does not check for class equality using getClass</name>
        <cmd>java:equals_class</cmd>
      </query>

      <query>
        <tag>equals_null</tag>
        <name>Equals method does not check if the object is equal to null</name>
        <cmd>java:equals_null</cmd>
      </query>

      <query>
        <tag>equals_this</tag>
        <name>Equals method does not check if the object is equal to this</name>
        <cmd>java:equals_this</cmd>
      </query>

      <query>
        <tag>equals_no_super</tag>
        <name>Equals method invokes direct reference comparison via a call to super.equals</name>
        <cmd>java:equals_no_super</cmd>
      </query>

      <query>
        <tag>equals_super</tag>
        <name>Equals method should call super.equals if a superclass defines it</name>
        <cmd>java:equals_super</cmd>
      </query>

      <query>
        <tag>impl_equals_hash</tag>
        <name>Class defines equals or hashCode, but not both</name>
        <cmd>java:impl_equals_hash</cmd>
      </query>

      <query>
        <tag>field_hides</tag>
        <name>Field hides base class field</name>
        <cmd>java:field_hides</cmd>
      </query>

      <query>
        <tag>method_hides</tag>
        <name>Method hides a parent method</name>
        <cmd>java:method_hides</cmd>
      </query>

      <query>
        <tag>obj_equality</tag>
        <name>Objects compared using direct reference comparison (== or !=)</name>
        <cmd>java:obj_equality</cmd>
      </query>

      <query>
        <tag>overload_similar</tag>
        <name>Method overloaded with arguments that are related by inheritance</name>
        <cmd>java:overload_similar</cmd>
      </query>

      <query>
        <tag>shift_max</tag>
        <name>Shift amount out of range</name>
        <cmd>java:shift_max</cmd>
      </query>

      <query>
        <tag>static_field_ref</tag>
        <name>Static field accessed through object instead of class</name>
        <cmd>java:static_field_ref</cmd>
      </query>
    </folder>
    <folder>
      <tag>declaration</tag>
      <name>Declaration Style</name>
      <query>
        <tag>line_separator</tag>
        <name>Hardcoded newline character(s)</name>
        <cmd>java:line_separator</cmd>
      </query>

      <query>
        <tag>numeric_octal</tag>
        <name>Use of octal literal</name>
        <cmd>java:numeric_octal</cmd>
      </query>
    </folder>
    <folder>
      <tag>errors</tag>
      <name>Possible Errors</name>
      <query>
        <tag>atflv</tag>
        <name>Assignment To For-Loop Variables</name>
        <cmd>java:atflv</cmd>
      </query>

      <query>
        <name>Constant expression as conditional(all hits)</name>
        <cmd>java:const_cond</cmd>
        <tag>const_cond</tag>
      </query>

      <query>
        <name>Constant expression as conditional(except infinite loops)</name>
        <cmd>java:const_cond_except_loops</cmd>
        <tag>const_cond</tag>
      </query>

      <query>
        <tag>finalize_super</tag>
        <name>Finalize method does not call super.finalize</name>
        <cmd>java:finalize_super</cmd>
      </query>

      <query>
        <tag>label_dup</tag>
        <name>Label with duplicate name</name>
        <cmd>java:label_dup</cmd>
      </query>

      <query>
        <tag>conv_num_narrow</tag>
        <name>Narrowing primitive conversion</name>
        <cmd>java:conv_num_narrow</cmd>
      </query>
    </folder>
    <folder>
      <tag>superfluous</tag>
      <name>Superfluous Code</name>
      <query>
        <tag>did</tag>
        <name>Duplicate Import Declarations</name>
        <cmd>java:did</cmd>
      </query>

      <query>
        <tag>dipsfbt</tag>
        <name>Don't Import the Package the Source File Belongs To</name>
        <cmd>java:dipsfbt</cmd>
      </query>

      <query>
        <tag>eiojlc</tag>
        <name>Explicit Import Of the java.lang Classes</name>
        <cmd>java:eiojlc</cmd>
      </query>

      <query>
        <tag>label_unused</tag>
        <name>Unused label</name>
        <cmd>java:label_unused</cmd>
      </query>

      <query>
        <tag>unused_local</tag>
        <name>Unused local variable</name>
        <cmd>java:unused_local</cmd>
      </query>

      <query>
        <tag>unused_private_field</tag>
        <name>Unused private field</name>
        <cmd>java:unused_private_field</cmd>
      </query>

      <query>
        <tag>unused_private_method</tag>
        <name>Unused private method</name>
        <cmd>java:unused_private_method</cmd>
      </query>
    </folder>
    <folder>
      <tag>gc</tag>
      <name>Garbage Collection</name>
      <query>
        <tag>gcasc</tag>
        <name>Avoid Static Collections - Vector, Hashtable</name>
        <cmd>javasupp:gcasc</cmd>
      </query>

      <query>
        <tag>gcautcpts</tag>
        <name>Avoid unnecessary temporaries when convert primitive types to java.lang.String</name>
        <cmd>javasupp:gcautcpts</cmd>
      </query>

      <query>
        <tag>gcfmul</tag>
        <name>Method finalize() should not unregister listeners</name>
        <cmd>javasupp:gcfmul</cmd>
      </query>

      <query>
        <tag>gcgcb</tag>
        <name>Method getClipBounds() should not be called too often</name>
        <cmd>javasupp:gcgcb</cmd>
      </query>

      <query>
        <tag>gcncfe</tag>
        <name>Never Call finalize() method Explicitly</name>
        <cmd>javasupp:gcncfe</cmd>
      </query>

      <query>
        <tag>gcoskr</tag>
        <name>Object Stream Keep References</name>
        <cmd>javasupp:gcoskr</cmd>
      </query>
    </folder>
    <folder>
      <tag>javaperf</tag>
      <name>Performance</name>
      <query>
        <tag>advil</tag>
        <name>Avoid Declaring Variables Inside Loops</name>
        <cmd>javaperf:advil</cmd>
      </query>

      <query>
        <tag>atswl</tag>
        <name>Append To String Within a Loop</name>
        <cmd>javaperf:atswl</cmd>
      </query>

      <query>
        <tag>cle</tag>
        <name>Complex Loop Expression</name>
        <cmd>javaperf:cle</cmd>
      </query>

      <query>
        <tag>dis</tag>
        <name>Define Initial capacity for StringBuffer</name>
        <cmd>javaperf:dis</cmd>
      </query>

      <query>
        <tag>dicfvh</tag>
        <name>Define Initial capacity for Vector and Hashtable collections</name>
        <cmd>javaperf:dicfvh</cmd>
      </query>

      <query>
        <tag>dntefnp</tag>
        <name>Do Not throw Exceptions for Normal Processing</name>
        <cmd>javaperf:dntefnp</cmd>
      </query>

      <query>
        <tag>muc</tag>
        <name>Minimize Use of Casting</name>
        <cmd>javaperf:muc</cmd>
      </query>

      <query>
        <tag>mui</tag>
        <name>Minimize Use of instanceof</name>
        <cmd>javaperf:mui</cmd>
      </query>

      <query>
        <tag>ptcol</tag>
        <name>Place Try/Catch blocks Outside of Loops</name>
        <cmd>javaperf:ptcol</cmd>
      </query>

      <query>
        <tag>uarrcpy</tag>
        <name>Use 'System.arraycopy ()' for arrays copying</name>
        <cmd>javaperf:uarrcpy</cmd>
      </query>

      <query>
        <tag>ucis</tag>
        <name>Use charAt() instead startsWith()</name>
        <cmd>javaperf:ucis</cmd>
      </query>

      <query>
        <tag>ucisfsc</tag>
        <name>Use char instead of java.lang.String for one character string concatenation</name>
        <cmd>javaperf:ucisfsc</cmd>
      </query>
    </folder>
  </folder>
  <folder>
    <name>General</name>
    <tag>gen</tag>

    <query>
      <name>Assignment in conditional</name>
      <cmd>lang:ass_cond</cmd>
      <tag>ass_cond</tag>
    </query>

    <query>
      <name>Empty statement body</name>
      <cmd>lang:empty_body</cmd>
      <tag>empty_body</tag>
    </query>

    <query>
      <name>Use of goto statement</name>
      <cmd>lang:goto</cmd>
      <tag>goto</tag>
    </query>

    <query>
      <name>Use of break or continue statement in loop</name>
      <cmd>lang:break_cont</cmd>
      <tag>break_cont</tag>
    </query>

    <query>
      <name>Constant expression as conditional</name>
      <cmd>lang:const_cond</cmd>
      <tag>const_cond</tag>
    </query>

    <query>
      <name>Defaultless switch</name>
      <cmd>lang:sw_default</cmd>
      <tag>sw_default</tag>
    </query>

    <query>
      <name>Control flows past end of case</name>
      <cmd>lang:sw_fall</cmd>
      <tag>sw_fall</tag>
    </query>

    <query>
      <name>Dangling else-if</name>
      <cmd>lang:dang_ei</cmd>
      <tag>dang_ei</tag>
    </query>

    <query>
      <name>Unused local variable</name>
      <cmd>lang:unused_local</cmd>
      <tag>unused_local</tag>
    </query>

    <query>
      <name>Unused static global variable</name>
      <cmd>lang:unused_static</cmd>
      <tag>unused_static</tag>
    </query>

    <query>
      <name>Declaration of type or external symbol at function level</name>
      <cmd>lang:nest_fun_decl</cmd>
      <tag>nest_fun_decl</tag>
    </query>

    <query>
      <name>Questionable comparison of unsigned</name>
      <cmd>lang:uns_cmp</cmd>
      <tag>uns_cmp</tag>
    </query>

    <query>
      <name>Discarded return value, internal function</name>
      <cmd>lang:discard_ret</cmd>
      <tag>discard_ret</tag>
    </query>

    <query>
      <name>Discarded return value, external function</name>
      <cmd>lang:discard_ret_external</cmd>
      <tag>discard_ret_external</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Uninitialized persistent variable</name>
      <cmd>lang:init_glob</cmd>
      <tag>init_glob</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Conversion narrows numerical precision</name>
      <cmd>lang:conv_num_narrow</cmd>
      <tag>conv_num_narrow</tag>
    </query>

    <query>
      <name>Conversion between pointers narrows integral precision</name>
      <cmd>lang:conv_intptr_narrow</cmd>
      <tag>conv_intptr_narrow</tag>
    </query>

    <query>
      <name>Conversion between pointers widens integral precision</name>
      <cmd>lang:conv_intptr_widen</cmd>
      <tag>conv_intptr_widen</tag>
    </query>

    <query>
      <name>Conversion between pointers to incompatible numerical types</name>
      <cmd>lang:conv_numptr_compat</cmd>
      <tag>conv_numptr_compat</tag>
    </query>

    <query>
      <name>Conversion loses const qualifier</name>
      <cmd>lang:conv_const</cmd>
      <tag>conv_const</tag>
    </query>

    <query>
      <name>Conversion loses volatile qualifier</name>
      <cmd>lang:conv_vol</cmd>
      <tag>conv_vol</tag>
    </query>

    <query>
      <name>Conversion between signed and unsigned types</name>
      <cmd>lang:conv_uns</cmd>
      <tag>conv_uns</tag>
    </query>

    <query>
      <name>Conversion between pointer and non-pointer types</name>
      <cmd>lang:conv_ptr_nonptr</cmd>
      <tag>conv_ptr_nonptr</tag>
    </query>

    <query>
      <name>Conversion between pointer types of unequal depth</name>
      <cmd>lang:conv_ptr_depth</cmd>
      <tag>conv_ptr_depth</tag>
    </query>

    <query>
      <name>Conversion between unrelated pointer types</name>
      <cmd>lang:conv_ptr_rel</cmd>
      <tag>conv_ptr_rel</tag>
    </query>

    <query>
      <name>Expression uses operand of side-effect more than once</name>
      <cmd>lang:side_effect_usage</cmd>
      <tag>side_effect_usage</tag>
    </query>

    <query>
      <name>Using type size in pointer arithmetic calculation</name>
      <cmd>lang:arith_sizeof</cmd>
      <tag>arith_sizeof</tag>
    </query>

    <query>
      <name>Function takes a variable number of arguments</name>
      <cmd>lang:def_vararg</cmd>
      <tag>def_vararg</tag>
    </query>

    <query>
      <name>Non-virtual destructor in class hierarchy</name>
      <cmd>lang:nv_dtor</cmd>
      <tag>nv_dtor</tag>
    </query>

    <query>
      <name>Class hierarchy downcast</name>
      <cmd>lang:downcast</cmd>
      <tag>downcast</tag>
    </query>

    <query>
      <name>Class hierarchy sidecast</name>
      <cmd>lang:sidecast</cmd>
      <tag>sidecast</tag>
    </query>

    <query>
      <name>Assignment operator returns inappropriate type</name>
      <cmd>lang:ass_op_ret</cmd>
      <tag>ass_op_ret</tag>
    </query>

    <query>
      <name>Assignment operator does not check for self-assignment</name>
      <cmd>lang:ass_op_self</cmd>
      <tag>ass_op_self</tag>
    </query>

    <query>
      <name>Parameter of assignment operator/copy constructor is not a const reference</name>
      <cmd>lang:const_ref_param</cmd>
      <tag>const_ref_param</tag>
    </query>

    <query>
      <name>Faulty initialization in constructor</name>
      <cmd>lang:init_order</cmd>
      <tag>init_order</tag>
    </query>

    <query>
      <name>Virtual function call in constructor/destructor</name>
      <cmd>lang:virt_call</cmd>
      <tag>virt_call</tag>
    </query>

    <query>
      <name>Class inherits from multiple concrete base classes</name>
      <cmd>lang:mi_concrete</cmd>
      <tag>mi_concrete</tag>
    </query>

    <query>
      <name>Function returns object by value</name>
      <cmd>lang:ret_by_val</cmd>
      <tag>ret_by_val</tag>
    </query>

    <query>
      <name>Function parameter takes object by value</name>
      <cmd>lang:pass_by_val</cmd>
      <tag>pass_by_val</tag>
    </query>

    <query>
      <name>Expression creates temporary object</name>
      <cmd>lang:temp_obj</cmd>
      <tag>temp_obj</tag>
    </query>

    <query>
      <name>Mismatch of function parameters</name>
      <cmd>lang:fun_mismatch</cmd>
      <tag>fun_mismatch</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Entity case clash</name>
      <cmd>lang:entity_case_clash</cmd>
      <tag>entity_case_clash</tag>
    </query>

    <query>
      <name>Statements without braces</name>
      <cmd>lang:without_braces</cmd>
      <tag>without_braces</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Operand of modulus or division operator is signed</name>
      <cmd>lang:divmod_signed</cmd>
      <tag>divmod_signed</tag>
    </query>

    <query>
      <name>Virtual function not declared explicitly with virtual keyword</name>
      <cmd>lang:explicit_virtual</cmd>
      <tag>explicit_virtual</tag>
    </query>

    <query>
      <name>Class inherits from multiple base classes</name>
      <cmd>lang:nomi</cmd>
      <tag>nomi</tag>
    </query>

    <query>
      <name>Use of C-style allocators in C++ code</name>
      <cmd>lang:nomalloc</cmd>
      <tag>nomalloc</tag>
    </query>

    <query>
      <name>Struct defined in C++ code</name>
      <cmd>lang:nostruct</cmd>
      <tag>nostruct</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Union defined in C++ code</name>
      <cmd>lang:nounion</cmd>
      <tag>nounion</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Nested class defined</name>
      <cmd>lang:nonest</cmd>
      <tag>nonest</tag>
    </query>

    <query>
      <name>Template defined</name>
      <cmd>lang:notemplate</cmd>
      <tag>notemplate</tag>
    </query>

    <query>
      <name>Class defined with friends</name>
      <cmd>lang:nofriend</cmd>
      <tag>nofriend</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Use of exceptions</name>
      <cmd>lang:noexcept</cmd>
      <tag>noexcept</tag>
    </query>

    <query>
      <name>Multiple declarations of macros</name>
      <cmd>lang:multiple_macros</cmd>
      <tag>multiple_macros</tag>
    </query>

    <folder>
      <name>Data Flow Analysis</name>
      <tag>dfa</tag>
      <enabled>0</enabled>

      <query>
        <name>Function exports pointer to local stack</name>
        <cmd>lang:stackret</cmd>
        <tag>stackret</tag>
      </query>

      <query>
        <name>Potential memory leak</name>
        <cmd>lang:memleak</cmd>
        <tag>memleak</tag>
      </query>

    </folder>

  </folder>

  <folder>
    <name>Portability</name>
    <tag>port</tag>

    <query>
      <name>Call to foreign function</name>
      <cmd>port:foreign_call</cmd>
      <tag>foreign_call</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Inline assembly</name>
      <cmd>port:inline_asm</cmd>
      <tag>inline_asm</tag>
    </query>

    <query>
      <name>Character subscript</name>
      <cmd>port:char_sub</cmd>
      <tag>char_sub</tag>
      <enabled>0</enabled>
    </query>

    <query>
      <name>Conversion may result in improper alignment</name>
      <cmd>port:alignment</cmd>
      <tag>alignment</tag>
    </query>

    <query>
      <name>Right shift of signed quantity</name>
      <cmd>port:rshift</cmd>
      <tag>rshift</tag>
    </query>

    <query>
      <name>Testing equality of floating-point values</name>
      <cmd>port:float_eq</cmd>
      <tag>float_eq</tag>
    </query>

    <folder>
      <name>ANSI Implementation Quantities</name>
      <tag>ansilim</tag>

      <query>
        <name>String size exceeds limit</name>
        <cmd>port:string_lim</cmd>
        <tag>string_lim</tag>
      </query>

      <query>
        <name>Field count exceeds limit</name>
        <cmd>port:field_lim</cmd>
        <tag>field_lim</tag>
      </query>

      <query>
        <name>Enumeration size exceeds limit</name>
        <cmd>port:enum_lim</cmd>
        <tag>enum_lim</tag>
      </query>

      <query>
        <name>Level of nested control exceeds limit</name>
        <cmd>port:nested_lim</cmd>
        <tag>nested_lim</tag>
      </query>

      <query>
        <name>Number of block identifiers limit</name>
        <cmd>port:block_lim</cmd>
        <tag>block_lim</tag>
      </query>

    </folder>

  </folder>

  <folder>
    <name>Globalization</name>
    <tag>glob</tag>
    <enabled>0</enabled>

    <folder>
      <name>String-Based Pointer Arithmetic</name>
      <tag>str_arith</tag>

      <query>
        <name>Pointer addition</name>
        <cmd>glob:arith_add</cmd>
        <tag>arith_add</tag>
      </query>

      <query>
        <name>Pointer subtraction</name>
        <cmd>glob:arith_sub</cmd>
        <tag>arith_sub</tag>
      </query>

      <query>
        <name>Postfix increment</name>
        <cmd>glob:arith_post_incr</cmd>
        <tag>arith_post_incr</tag>
      </query>

      <query>
        <name>Postfix decrement</name>
        <cmd>glob:arith_post_decr</cmd>
        <tag>arith_post_decr</tag>
      </query>

      <query>
        <name>Prefix increment</name>
        <cmd>glob:arith_pre_incr</cmd>
        <tag>arith_pre_incr</tag>
      </query>

      <query>
        <name>Prefix decrement</name>
        <cmd>glob:arith_pre_decr</cmd>
        <tag>arith_pre_decr</tag>
      </query>

      <query>
        <name>Character array reference</name>
        <cmd>glob:arith_array</cmd>
        <tag>arith_array</tag>
      </query>

    </folder>

    <query>
      <name>Character constant</name>
      <cmd>glob:char_const</cmd>
      <tag>char_const</tag>
    </query>

    <query>
      <name>String literal</name>
      <cmd>glob:stringl</cmd>
      <tag>stringl</tag>
    </query>

    <query>
      <name>String literal not wrapped in macro</name>
      <cmd>glob:stringl_nomac</cmd>
      <tag>stringl_nomac</tag>
      <enabled>0</enabled>
    </query>

  </folder>

  <folder>
    <name>Structure</name>
    <tag>struct</tag>

    <folder>
      <name>File Metrics</name>
      <tag>file_met</tag>

      <query>
        <name>File LOC exceeds limit</name>
        <cmd>met:file_tloc</cmd>
        <tag>file_tloc</tag>
      </query>

      <query>
        <name>File ULOC exceeds limit</name>
        <cmd>met:file_uloc</cmd>
        <tag>file_uloc</tag>
      </query>

      <query>
        <name>File XLOC exceeds limit</name>
        <cmd>met:file_xloc</cmd>
        <tag>file_xloc</tag>
      </query>

      <query>
        <name>Number of includes exceeds limit</name>
        <cmd>met:file_incl</cmd>
        <tag>file_incl</tag>
      </query>

    </folder>

    <folder>
      <name>Function Metrics</name>
      <tag>fun_met</tag>

      <query>
        <name>Function LOC exceeds limit</name>
        <cmd>met:fun_tloc</cmd>
        <tag>fun_tloc</tag>
      </query>

      <query>
        <name>Function ULOC exceeds limit</name>
        <cmd>met:fun_uloc</cmd>
        <tag>fun_uloc</tag>
      </query>

      <query>
        <name>Function XLOC exceeds limit</name>
        <cmd>met:fun_xloc</cmd>
        <tag>fun_xloc</tag>
      </query>

      <query>
        <name>Inline function XLOC exceeds limit</name>
        <cmd>met:fun_inline_xloc</cmd>
        <tag>fun_inline_xloc</tag>
      </query>

      <query>
        <name>Cyclomatic complexity exceeds limit</name>
        <cmd>met:fun_cc</cmd>
        <tag>fun_cc</tag>
      </query>

      <query>
        <name>Myers' complexity exceeds limit</name>
        <cmd>met:fun_mc</cmd>
        <tag>fun_mc</tag>
      </query>

      <query>
        <name>Function depth exceeds limit</name>
        <cmd>met:fun_nest</cmd>
        <tag>fun_nest</tag>
      </query>

      <query>
        <name>Number of returns exceeds limit</name>
        <cmd>met:fun_ret</cmd>
        <tag>fun_ret</tag>
      </query>

      <query>
        <name>Number of loops exceeds limit</name>
        <cmd>met:fun_loop</cmd>
        <tag>fun_loop</tag>
      </query>

      <query>
        <name>Number of switches exceeds limit</name>
        <cmd>met:fun_sw</cmd>
        <tag>fun_sw</tag>
      </query>

      <query>
        <name>Number of parameters exceeds limit</name>
        <cmd>met:fun_params</cmd>
        <tag>fun_params</tag>
      </query>

    </folder>

    <folder>
      <name>Class Metrics</name>
      <tag>class_met</tag>

      <query>
        <name>Number of sub-classes exceeds limit</name>
        <cmd>met:class_subs</cmd>
        <tag>class_subs</tag>
      </query>

      <query>
        <name>Number of super-classes exceeds limit</name>
        <cmd>met:class_supers</cmd>
        <tag>class_supers</tag>
      </query>

      <query>
        <name>Number of member functions exceeds limit</name>
        <cmd>met:class_funs</cmd>
        <tag>class_funs</tag>
      </query>

      <query>
        <name>Number of member variables exceeds limit</name>
        <cmd>met:class_vars</cmd>
        <tag>class_vars</tag>
      </query>

      <query>
        <name>Constructor XLOC exceeds limit</name>
        <cmd>met:class_ctor_xloc</cmd>
        <tag>class_ctor_xloc</tag>
      </query>

      <query>
        <name>Destructor XLOC exceeds limit</name>
        <cmd>met:class_dtor_xloc</cmd>
        <tag>class_dtor_xloc</tag>
      </query>

    </folder>

  </folder>

  <mail_dtl></mail_dtl>
  <mail_stl></mail_stl>
  <mail_dtu>1</mail_dtu>
  <mail_stu>0</mail_stu>

</root>
