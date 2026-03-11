# Conformance Coverage Matrix (Draft v0.1)

Status: Draft (W12 value/module migration baseline).

This matrix links normative spec clauses to conformance case IDs.

## 1. Legend

Coverage state:

1. `mapped-existing`: covered by existing integration/script cases (not yet in conformance format)
2. `mapped-conformance`: covered by structured conformance `.ms` case files
3. `planned`: case is planned but not yet written
4. `gap`: no case mapped yet

## 2. Matrix

| Spec Clause | Case ID(s) | Current Source | State | Notes |
|---|---|---|---|---|
| `semantics.scope.lexical` | `SEM-SCOPE-BLOCK-001`, `SEM-CLOSURE-INDEPENDENT-001`, `RUN-UNDEF-VAR-001` | `tests/conformance/semantics/scope_block_shadowing_001.ms`, `tests/conformance/semantics/closure_independent_001.ms`, `tests/conformance/semantics/runtime_undefined_variable_001.ms` | mapped-conformance | block shadowing, lexical capture, undefined name |
| `semantics.control.if` | `SEM-CTRL-IF-001` | `tests/conformance/semantics/control_if_else_001.ms` | mapped-conformance | branch execution with boolean and comparison operators |
| `semantics.control.while` | `SEM-CTRL-LOOP-001` | `tests/conformance/semantics/control_while_for_001.ms` | mapped-conformance | while loop condition and mutation semantics |
| `semantics.control.for` | `SEM-CTRL-LOOP-001` | `tests/conformance/semantics/control_while_for_001.ms` | mapped-conformance | for initializer/condition/increment semantics |
| `semantics.operators.comparison` | `SEM-CTRL-IF-001` | `tests/conformance/semantics/control_if_else_001.ms` | mapped-conformance | `<`, `>`, `!=` contracts in VM path |
| `semantics.scope.assignment` | `SEM-SCOPE-BLOCK-001` | `tests/conformance/semantics/scope_block_shadowing_001.ms` | mapped-conformance | assignment updates nearest lexical binding |
| `semantics.closure.capture` | `SEM-CLOSURE-CAPTURE-001`, `SEM-CLOSURE-INDEPENDENT-001`, `RES-THIS-NESTED-OK-001` | `tests/conformance/semantics/closure_capture_001.ms`, `tests/conformance/semantics/closure_independent_001.ms`, `tests/conformance/semantics/resolver_this_in_nested_function_ok_001.ms` | mapped-conformance | mutation persistence and nested capture |
| `semantics.functions.arity` | `RUN-ARITY-001` | `tests/conformance/semantics/runtime_arity_001.ms` | mapped-existing | runtime arity mismatch |
| `semantics.functions.return` | `SEM-FUNC-RETURN-001`, `RES-RETURN-IN-FUNC-OK-001`, `RES-RETURN-OUTSIDE-001`, `RES-RETURN-OUTSIDE-BLOCK-001` | `tests/conformance/semantics/function_basic_return_001.ms`, `tests/conformance/semantics/resolver_return_in_function_ok_001.ms`, `tests/conformance/semantics/resolver_return_outside_001.ms`, `tests/conformance/semantics/resolver_return_outside_block_001.ms` | mapped-conformance | valid and invalid return contexts |
| `semantics.functions.values` | `SEM-FUNC-ANON-001` | `tests/conformance/semantics/function_anonymous_value_001.ms` | mapped-conformance | anonymous function value and nested closure |
| `semantics.class.fields` | `SEM-CLASS-FIELD-001`, `RUN-UNDEF-PROP-001`, `RUN-PROP-NON-INSTANCE-001`, `RUN-SET-NON-INSTANCE-001` | `tests/conformance/semantics/class_fields_001.ms`, `tests/conformance/semantics/runtime_undefined_property_001.ms`, `tests/conformance/semantics/runtime_property_on_non_instance_001.ms`, `tests/conformance/semantics/runtime_set_on_non_instance_001.ms` | mapped-conformance | read/write and invalid receiver paths |
| `semantics.class.this` | `SEM-CLASS-FIELD-001`, `SEM-CLASS-METHOD-BIND-001`, `RES-THIS-NESTED-OK-001`, `RES-THIS-OUTSIDE-001`, `RES-THIS-FREE-FUNC-001` | `tests/conformance/semantics/class_fields_001.ms`, `tests/conformance/semantics/class_method_binding_001.ms`, `tests/conformance/semantics/resolver_this_in_nested_function_ok_001.ms`, `tests/conformance/semantics/resolver_this_outside_001.ms`, `tests/conformance/semantics/resolver_this_in_free_function_001.ms` | mapped-conformance | valid method receiver and resolver guards |
| `semantics.class.methods` | `SEM-CLASS-METHOD-BIND-001` | `tests/conformance/semantics/class_method_binding_001.ms` | mapped-conformance | bound method keeps receiver |
| `semantics.class.initializer` | `SEM-CLASS-INIT-001` | `tests/conformance/semantics/class_initializer_receiver_001.ms` | mapped-conformance | initializer execution and instance state |
| `semantics.class.inheritance` | `SEM-CLASS-SUPER-CHAIN-001`, `RES-SELF-INHERIT-001`, `RUN-SUPERCLASS-NON-CLASS-001` | `tests/conformance/semantics/class_super_chain_001.ms`, `tests/conformance/semantics/resolver_self_inherit_001.ms`, `tests/conformance/semantics/runtime_superclass_not_class_001.ms` | mapped-conformance | valid chain, self-inherit, invalid superclass |
| `semantics.class.super` | `SEM-CLASS-SUPER-CHAIN-001`, `RES-SUPER-SUBCLASS-OK-001`, `RES-SUPER-OUTSIDE-001`, `RES-SUPER-NO-SUPERCLASS-001` | `tests/conformance/semantics/class_super_chain_001.ms`, `tests/conformance/semantics/resolver_super_in_subclass_ok_001.ms`, `tests/conformance/semantics/resolver_super_outside_001.ms`, `tests/conformance/semantics/resolver_super_without_superclass_001.ms` | mapped-conformance | super dispatch and misuse checks |
| `modules.import.cache` | `MOD-IMPORT-CACHE-001` | `tests/conformance/modules/module_import_cache_001.ms` | mapped-conformance | repeated import initializes once |
| `modules.import.path` | `MOD-IMPORT-DOTTED-001` | `tests/conformance/modules/module_import_dotted_001.ms` | mapped-conformance | dotted module path resolution |
| `modules.from_import.alias` | `MOD-FROM-ALIAS-OK-001`, `MOD-FROM-PLAIN-001`, `MOD-SYMBOL-NOT-FOUND-001` | `tests/conformance/modules/module_from_import_alias_ok_001.ms`, `tests/conformance/modules/module_from_import_plain_001.ms`, `tests/conformance/modules/module_symbol_not_found_001.ms` | mapped-conformance | alias/non-alias bind and missing symbol |
| `modules.circular_dependency` | `MOD-CYCLE-001` | `tests/conformance/modules/module_cycle_001.ms` | mapped-existing | circular import rejection |
| `lexical.keywords` | `LEX-KW-001` | `tests/unit/test_lexer.cc` | mapped-existing | keyword recognition |
| `lexical.comments.line` | `LEX-COMMENT-001` | `tests/unit/test_lexer.cc` | mapped-existing | `//` stripping |
| `grammar.assignment` | `GRM-ASSIGN-TARGET-001` | `tests/conformance/grammar/parse_invalid_assignment_target_001.ms` | mapped-conformance | invalid assignment target rejected |
| `semantics.expression.assignment_target` | `GRM-ASSIGN-TARGET-001` | `tests/conformance/grammar/parse_invalid_assignment_target_001.ms` | mapped-conformance | non-lvalue assignment fails in parse phase |
| `errors.MS1002` | `LEX-ERR-STRING-001` | none | planned | unterminated string diagnostic code mapping |
| `errors.MS2003` | `PARSE-ASSIGN-TARGET-001` | none | planned | invalid assignment target |
| `errors.MS3001` | `RES-RETURN-OUTSIDE-001`, `RES-RETURN-OUTSIDE-BLOCK-001` | `tests/conformance/semantics/resolver_return_outside_001.ms`, `tests/conformance/semantics/resolver_return_outside_block_001.ms` | mapped-conformance | resolve-phase return misuse |
| `errors.MS3002` | `RES-THIS-OUTSIDE-001`, `RES-THIS-FREE-FUNC-001` | `tests/conformance/semantics/resolver_this_outside_001.ms`, `tests/conformance/semantics/resolver_this_in_free_function_001.ms` | mapped-conformance | resolve-phase this misuse |
| `errors.MS3003` | `RES-SUPER-OUTSIDE-001`, `RES-SUPER-NO-SUPERCLASS-001` | `tests/conformance/semantics/resolver_super_outside_001.ms`, `tests/conformance/semantics/resolver_super_without_superclass_001.ms` | mapped-conformance | resolve-phase super misuse |
| `errors.MS3004` | `RES-SELF-INHERIT-001` | `tests/conformance/semantics/resolver_self_inherit_001.ms` | mapped-conformance | self-inheritance rejected in resolve phase |
| `errors.MS4001` | `RUN-UNDEF-VAR-001` | `tests/conformance/semantics/runtime_undefined_variable_001.ms` | mapped-existing | undefined variable standardized diagnostics |
| `errors.MS4002` | `RUN-ARITY-001` | `tests/conformance/semantics/runtime_arity_001.ms` | mapped-existing | callable arity mismatch |
| `errors.MS4003` | `RUN-INVALID-OPERAND-001`, `RUN-PROP-NON-INSTANCE-001`, `RUN-SET-NON-INSTANCE-001`, `RUN-SUPERCLASS-NON-CLASS-001`, `GRM-ASSIGN-TARGET-001` | `tests/conformance/semantics/runtime_invalid_operand_001.ms`, `tests/conformance/semantics/runtime_property_on_non_instance_001.ms`, `tests/conformance/semantics/runtime_set_on_non_instance_001.ms`, `tests/conformance/semantics/runtime_superclass_not_class_001.ms`, `tests/conformance/grammar/parse_invalid_assignment_target_001.ms` | mapped-conformance | runtime type misuse and current parse-target mapping |
| `errors.MS4004` | `RUN-UNDEF-PROP-001` | `tests/conformance/semantics/runtime_undefined_property_001.ms` | mapped-existing | undefined instance property |
| `errors.MS4005` | `RUN-NON-CALLABLE-001` | `tests/conformance/semantics/runtime_non_callable_001.ms` | mapped-existing | non-callable invocation |
| `errors.MS5001` | `MOD-NOT-FOUND-001` | `tests/conformance/modules/module_not_found_001.ms` | mapped-existing | module not found |
| `errors.MS5002` | `MOD-SYMBOL-NOT-FOUND-001` | `tests/conformance/modules/module_symbol_not_found_001.ms` | mapped-existing | missing export symbol |
| `errors.MS5003` | `MOD-CYCLE-001` | `tests/conformance/modules/module_cycle_001.ms` | mapped-existing | circular dependency |

## 3. Immediate Backlog (Docs-Only Planning)

Priority order for next conformance-file migration:

1. lexical negative-path canonical cases (unexpected character / unterminated string)
2. parse code normalization to align invalid-assignment with `MS2003`
3. diagnostics-structured matcher rollout (`phase+code+span`)
4. cross-platform deterministic span checks (line/column)
5. optional diagnostics edge cases for nested control-flow parse failures

## 4. Gaps Requiring T16/T19 Prerequisites

The following need additional diagnostic/conformance standardization before strict lock:

1. stable structured diagnostics (`phase + code + span`)
2. column-level source mapping guarantees
