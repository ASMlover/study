# Conformance Coverage Matrix (Draft v0.1)

Status: Draft (W12 value/module migration baseline).

This matrix links normative spec clauses to conformance case IDs.

## 1. Legend

Coverage state:

1. `mapped-existing`: covered by existing integration/script cases (not yet in conformance format)
2. `planned`: case is planned but not yet written
3. `gap`: no case mapped yet

## 2. Matrix

| Spec Clause | Case ID(s) | Current Source | State | Notes |
|---|---|---|---|---|
| `semantics.scope.lexical` | `SEM-SCOPE-LEX-001` | `tests/scripts/language/closure_lexical.ms` | mapped-existing | lexical capture visibility |
| `semantics.closure.capture` | `SEM-CLOSURE-001` | `tests/scripts/language/closure_capture.ms` | mapped-existing | captured var mutation persists |
| `semantics.functions.arity` | `RUN-ARITY-001` | `tests/conformance/semantics/runtime_arity_001.ms` | mapped-existing | runtime arity mismatch |
| `semantics.class.fields` | `SEM-CLASS-FIELD-001` | `tests/scripts/language/class_fields.ms` | mapped-existing | instance field read/write |
| `semantics.class.inheritance` | `SEM-CLASS-INHERIT-001` | `tests/scripts/language/class_inherit.ms` | mapped-existing | override + super chain output |
| `semantics.class.super` | `RUN-SUPER-001` | `tests/scripts/language/class_super_error.ms` | mapped-existing | missing super method error |
| `modules.import.cache` | `MOD-IMPORT-001` | `tests/unit/test_module.cc` + module scripts | mapped-existing | one-time init / cache reuse |
| `modules.from_import.alias` | `MOD-FROM-001` | `tests/unit/test_module.cc` + module scripts | mapped-existing | symbol alias binding |
| `modules.circular_dependency` | `MOD-CYCLE-001` | `tests/conformance/modules/module_cycle_001.ms` | mapped-existing | circular import rejection |
| `lexical.keywords` | `LEX-KW-001` | `tests/unit/test_lexer.cc` | mapped-existing | keyword recognition |
| `lexical.comments.line` | `LEX-COMMENT-001` | `tests/unit/test_lexer.cc` | mapped-existing | `//` stripping |
| `grammar.assignment` | `GRM-ASSIGN-001` | `tests/unit/test_vm_compiler.cc` | mapped-existing | assignment expression statement |
| `errors.MS1002` | `LEX-ERR-STRING-001` | none | planned | unterminated string diagnostic code mapping |
| `errors.MS2003` | `PARSE-ASSIGN-TARGET-001` | none | planned | invalid assignment target |
| `errors.MS3001` | `RES-RETURN-OUTSIDE-001` | `tests/conformance/semantics/resolver_return_outside_001.ms` | mapped-existing | resolve-phase diagnostic code enforced |
| `errors.MS3002` | `RES-THIS-OUTSIDE-001` | `tests/conformance/semantics/resolver_this_outside_001.ms` | mapped-existing | resolve-phase diagnostic code enforced |
| `errors.MS3003` | `RES-SUPER-OUTSIDE-001` | `tests/conformance/semantics/resolver_super_outside_001.ms` | mapped-existing | resolve-phase diagnostic code enforced |
| `errors.MS3004` | `RES-SELF-INHERIT-001` | `tests/conformance/semantics/resolver_self_inherit_001.ms` | mapped-existing | self-inheritance rejected in resolve phase |
| `errors.MS4001` | `RUN-UNDEF-VAR-001` | `tests/conformance/semantics/runtime_undefined_variable_001.ms` | mapped-existing | undefined variable standardized diagnostics |
| `errors.MS4002` | `RUN-ARITY-001` | `tests/conformance/semantics/runtime_arity_001.ms` | mapped-existing | callable arity mismatch |
| `errors.MS4003` | `RUN-INVALID-OPERAND-001` | `tests/conformance/semantics/runtime_invalid_operand_001.ms` | mapped-existing | invalid operand kinds |
| `errors.MS4004` | `RUN-UNDEF-PROP-001` | `tests/conformance/semantics/runtime_undefined_property_001.ms` | mapped-existing | undefined instance property |
| `errors.MS4005` | `RUN-NON-CALLABLE-001` | `tests/conformance/semantics/runtime_non_callable_001.ms` | mapped-existing | non-callable invocation |
| `errors.MS5001` | `MOD-NOT-FOUND-001` | `tests/conformance/modules/module_not_found_001.ms` | mapped-existing | module not found |
| `errors.MS5002` | `MOD-SYMBOL-NOT-FOUND-001` | `tests/conformance/modules/module_symbol_not_found_001.ms` | mapped-existing | missing export symbol |
| `errors.MS5003` | `MOD-CYCLE-001` | `tests/conformance/modules/module_cycle_001.ms` | mapped-existing | circular dependency |

## 3. Immediate Backlog (Docs-Only Planning)

Priority order for next conformance-file migration:

1. `SEM-CLOSURE-001` and `SEM-SCOPE-LEX-001`
2. `SEM-CLASS-INHERIT-001` and `RUN-SUPER-001`
3. `MOD-IMPORT-001` and `MOD-FROM-001`
4. lexical and parse negative-path canonical cases
5. diagnostics-structured matcher rollout (`phase+code+span`)

## 4. Gaps Requiring T16/T19 Prerequisites

The following need additional diagnostic/conformance standardization before strict lock:

1. stable structured diagnostics (`phase + code + span`)
2. column-level source mapping guarantees
