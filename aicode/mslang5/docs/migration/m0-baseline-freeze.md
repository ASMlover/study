# M0 Baseline Freeze And Migration Guardrails

Date: 2026-03-11  
Scope: `docs/improve.md` milestone `M0`

## 1. Unified Migration Checklist

Use this checklist for every PR touching runtime/frontend/tests during M0-M6.

1. Interpreter freeze policy:
   - `ScriptInterpreter` is transitional only.
   - No new language capability may be added on interpreter path.
   - Any intentional interpreter change must update:
     - `docs/migration/interpreter_freeze.sha256`
     - this document with rationale and affected milestone.
2. VM-first feature policy:
   - New capability must land in VM pipeline (`lexer/parser/resolver/compiler/vm`).
   - New behavior tests must include VM-route assertion where feasible.
3. Fallback debt tracking:
   - Fallback-dependent cases must be listed in Section 3.
   - CI must emit fallback debt metrics via `maple_tests_migration_debt`.
4. Planning sync:
   - Any M0-M6 status update must be reflected in both `docs/improve.md` and `PLAN.md` on the same day.

## 2. VM Capability Gap Matrix (Syntax -> VM support -> fallback trigger)

| Capability | VM Native Status | Fallback Trigger Today | Representative Tests | Debt Milestone |
|---|---|---|---|---|
| Arithmetic expressions | Supported | none | `tests/unit/test_vm_compiler.cc` | - |
| `var` define/get/set (global) | Supported | none | `tests/unit/test_vm_compiler.cc` | - |
| `import <module>` | Supported | none | `tests/unit/test_module.cc`, `tests/conformance/modules/module_import_cache_001.ms` | - |
| `from <module> import <name> as <alias>` | Supported | none | `tests/conformance/modules/module_from_import_alias_ok_001.ms` | - |
| Closures and lexical capture | Not native | VM compile failure -> legacy execution | `tests/integration/test_language_closure.cc` | M2 |
| Class fields/methods/inheritance | Not native | VM compile failure -> legacy execution | `tests/integration/test_language_class.cc` | M3 |
| Resolver-only semantic checks (`return/this/super/self-inherit`) | Not native in VM frontend path | VM compile failure -> legacy execution | `tests/integration/test_language_resolver.cc` | M1/M3 |

Notes:
1. This matrix is intentionally execution-oriented and tied to concrete tests.
2. When a capability migrates to VM-native behavior, update this table and remove its case from Section 3 debt registry.

## 3. Fallback Migration Debt Registry (CI-labeled)

The following cases are tracked in dedicated migration debt suite:

1. `tests/scripts/language/closure_capture.ms`
2. `tests/scripts/language/closure_lexical.ms`
3. `tests/scripts/language/class_fields.ms`
4. `tests/scripts/language/class_inherit.ms`
5. `tests/scripts/language/resolver_ok_return_in_function.ms`
6. `tests/scripts/language/resolver_ok_this_in_nested_function.ms`
7. `tests/scripts/language/resolver_ok_super_in_subclass.ms`
8. `tests/scripts/language/error_runtime_top_level_return.ms`
9. `tests/scripts/language/error_resolve_this_in_free_function.ms`
10. `tests/scripts/language/error_resolve_super_without_superclass_method.ms`
11. `tests/scripts/language/error_parse_self_inherit.ms`

CI / local commands:

1. `ctest --test-dir build --output-on-failure -C Debug -L migration_debt`
2. `ctest --test-dir build --output-on-failure -C Debug -R maple_guard_interpreter_freeze`

Metric emitted by suite:

1. `fallback_rate = fallback_cases / total_cases`
