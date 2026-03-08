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
- `error_parse_self_inherit.ms`: parse error for class self-inheritance.
- `error_runtime_top_level_return.ms`: runtime error for top-level `return`.
- `error_runtime_this_outside_class.ms`: runtime error for `this` outside class context.
- `error_runtime_super_outside_subclass.ms`: runtime error for `super` outside subclass context.
- `error_runtime_undefined_variable.ms`: runtime error for undefined variable lookup.
- `error_runtime_call_non_callable.ms`: runtime error for calling non-callable values.
- `error_runtime_property_on_non_instance.ms`: runtime error for property get on non-instance.
- `error_runtime_set_on_non_instance.ms`: runtime error for property set on non-instance.
- `error_runtime_superclass_not_class.ms`: runtime error for invalid superclass expression.

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
