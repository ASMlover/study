# Maple Script Fixtures

This directory contains runnable `.ms` fixtures for existing Maple language features.
The files are intentionally grouped by capability and include both happy-path and
error-path examples.

## Language Scripts (`tests/scripts/language`)

- `expr_arithmetic_precedence.ms`: arithmetic precedence, grouping, unary minus.
- `expr_string_concat.ms`: string concatenation and mixed print sequences.
- `var_scope_block.ms`: global/block shadowing and assignment visibility.
- `function_basic_return.ms`: named function call and `nil` return.
- `function_anonymous_value.ms`: anonymous function values and closure-style parameter usage.
- `closure_independent_counters.ms`: multiple closure instances with isolated captured state.
- `class_method_binding.ms`: method extraction, bound receiver, field mutation.
- `class_initializer_receiver.ms`: initializer return semantics and constructed instance state.
- `class_super_chain.ms`: multi-level inheritance and `super` method dispatch.
- `error_parse_invalid_assignment.ms`: parse error for invalid assignment target.
- `error_parse_self_inherit.ms`: resolve error (`MS3004`) for class self-inheritance.
- `error_runtime_top_level_return.ms`: resolve error (`MS3001`) for top-level `return`.
- `error_runtime_this_outside_class.ms`: resolve error (`MS3002`) for `this` outside class context.
- `error_runtime_super_outside_subclass.ms`: resolve error (`MS3003`) for `super` outside subclass context.
- `error_resolve_top_level_return_in_block.ms`: resolve error (`MS3001`) for `return` inside top-level block.
- `error_resolve_this_in_free_function.ms`: resolve error (`MS3002`) for `this` inside a free function.
- `error_resolve_super_without_superclass_method.ms`: resolve error (`MS3003`) for `super` inside class with no superclass.
- `error_runtime_undefined_variable.ms`: runtime error for undefined variable lookup.
- `error_runtime_call_non_callable.ms`: runtime error for calling non-callable values.
- `error_runtime_property_on_non_instance.ms`: runtime error for property get on non-instance.
- `error_runtime_set_on_non_instance.ms`: runtime error for property set on non-instance.
- `error_runtime_superclass_not_class.ms`: runtime error for invalid superclass expression.
- `resolver_ok_return_in_function.ms`: valid nested-function returns to verify resolver function context tracking.
- `resolver_ok_this_in_nested_function.ms`: valid `this` capture from nested function inside class method.
- `resolver_ok_super_in_subclass.ms`: valid multi-level `super` dispatch under subclass resolver context.

## Module Scripts (`tests/scripts/module`)

- `import_cache_and_alias.ms`: repeated `import` cache behavior + `from ... import ... as ...`.
- `import_dotted_name.ms`: dotted module import (`pkg.math`) and alias read.
- `error_missing_module.ms`: module-not-found error path.
- `error_missing_symbol.ms`: missing export symbol error path.
- `error_cycle_entry.ms`: circular dependency error path.
- `cycle_a.ms` / `cycle_b.ms`: cycle pair used by `error_cycle_entry.ms`.
- `pkg/math.ms`: nested module fixture for dotted-name import.

## Existing Baseline Fixtures

- `closure_capture.ms`
- `closure_lexical.ms`
- `closure_arity_error.ms`
- `class_fields.ms`
- `class_inherit.ms`
- `class_super_error.ms`
- `side.ms`
- `util.ms`
- `cli_ok.ms`
