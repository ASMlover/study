# Subagent Execution Log

Date: 2026-03-08
Repo: Maple (`mslang5`)

## Wave Mapping

- Subagent-A: `T01` `T02` `T03`
- Subagent-B: `T04` `T07` + grammar bridge for `T08/T09`
- Subagent-C: `T05` `T06` `T10` + runtime bridge for `T08/T09`
- Subagent-D: `T11` `T12` `T13`
- Subagent-E: `T08/T09` semantic-upgrade preparation + verification refresh

## Delivery Notes

1. Task IDs
- `T01` Logger
- `T02` Source + location mapping
- `T03` Bytecode chunk + disassembler
- `T04` Token + lexer
- `T05` Value/object/table base
- `T06` Minimal stack VM
- `T07` Pratt-style expression/compiler pipeline
- `T08` Function/closure bridge baseline (compiler/runtime interfaces reserved)
- `T09` Class/inheritance bridge baseline (object/module runtime extension points)
- `T10` GC controller integration hooks
- `T11` `import module` loader + cache
- `T12` `from module import name as alias`
- `T13` CLI/REPL + tests + CTest integration

2. Changed files (summary)
- Build: `CMakeLists.txt`
- Support: `src/support/*`
- Bytecode: `src/bytecode/*`
- Frontend: `src/frontend/*`
- Runtime: `src/runtime/*`
- CLI: `src/cli/*`, `src/main.cc`
- Tests: `tests/unit/*`, `tests/integration/*`, `tests/scripts/*`
- Progress docs: `PLAN.md`, `SUBAGENTS.md`

3. Verification command and result
- `cmake -S . -B build`
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`
- Result: pass (`1/1` test suite)

## Incremental Follow-up (Subagent-E)

1. Task IDs
- `T08` (in progress)
- `T09` (in progress)

2. Changed files
- `src/frontend/token.hh`
- `src/frontend/lexer.cc`
- `src/runtime/object.hh`
- `src/runtime/value.hh`
- `PLAN.md`

3. Behavior change
- Added lexer primitives for closure/class work:
  - symbols: `{`, `}`, `,`, `<`
  - keywords: `fun`, `return`, `class`, `this`, `super`
- Added a generic runtime object carrier:
  - introduced `RuntimeObject` base type
  - extended `Value` to hold `std::shared_ptr<RuntimeObject>`

4. Verification command and result
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`
- Result: pass (`1/1` test suite)

## W13 Execution (Subagents Mode)

Date: 2026-03-09

1. Subagent-A (Batch A: W13-D1)
- Scope:
  - introduced a canonical structured diagnostics model (`phase/code/span/notes`)
  - unified VM error emission and made CLI prefer structured rendering
- Changed files:
  - `src/support/source.hh`
  - `src/support/source.cc`
  - `src/runtime/vm.hh`
  - `src/runtime/vm.cc`
  - `src/cli/app.cc`
  - `src/runtime/module.cc`

2. Subagent-B (Batch B: W13-D2 + W13-D3)
- Scope:
  - added conformance metadata parser/runner for `@id/@spec/@expect/@diag.*`
  - added a diagnostics golden checker for structured diagnostic fields
- Changed files:
  - `tests/integration/test_conformance.cc`
  - `tests/integration/test_diagnostics_golden.cc`
  - `tests/unit/test_main.cc`
  - `tests/diagnostics/samples/runtime_arity_mismatch.golden.json`
  - `tests/diagnostics/samples/module_not_found.golden.json`
  - `tests/integration/test_language_resolver.cc`

3. Subagent-C (Batch C: W13-D4 + W13-D5)
- Scope:
  - wired staged CTest gates and labels (`all/unit/integration/conformance/diagnostics`)
  - updated wave closeout bookkeeping and GAP status
- Changed files:
  - `CMakeLists.txt`
  - `PLAN.md`
  - `SUBAGENTS.md`

4. Verification command and result
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`
- Result: pass (`5/5` tests)

## W11 Execution (Subagents Mode)

Date: 2026-03-08

1. Subagent-A (Batch A: W11-D1)
- Scope:
  - added an explicit resolver phase seam in `ScriptInterpreter::Execute` between parse and runtime
  - moved self-inheritance checks from parse-time to resolve-time policy
- Changed files:
  - legacy interpreter source

2. Subagent-B (Batch B: W11-D2 + W11-D3)
- Scope:
  - implemented lexical depth metadata (`local_depths`) and depth-aware environment lookup/update (`GetAt/AssignAt`)
  - enforced resolve diagnostics for `return/this/super/self-inherit` with `MS3001~MS3004`
  - aligned compile/runtime category mapping by classifying resolve errors as compile-like
- Changed files:
  - legacy interpreter source
  - `tests/integration/test_language_resolver.cc`
  - `tests/unit/test_main.cc`
  - `CMakeLists.txt`

3. Subagent-C (Batch C: W11-D4 + W11-D5)
- Scope:
  - added resolver conformance fixtures and matrix mapping
  - updated PLAN GAP status and wave closeout notes
- Changed files:
  - `tests/conformance/semantics/resolver_return_outside_001.ms`
  - `tests/conformance/semantics/resolver_this_outside_001.ms`
  - `tests/conformance/semantics/resolver_super_outside_001.ms`
  - `tests/conformance/semantics/resolver_self_inherit_001.ms`
  - `tests/conformance/MATRIX.md`
  - `tests/scripts/README.md`
  - `PLAN.md`
  - `SUBAGENTS.md`

4. Verification command and result
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`

## W10 Execution (Subagents Mode)

Date: 2026-03-08

1. Subagent-A (Batch A: W10-D1 + W10-D2)
- Scope:
  - switched `Vm::ExecuteSource` to VM-first normative routing
  - introduced explicit source execution mode and last-route guard state
  - normalized compile/runtime error category mapping across VM and compatibility fallback
- Changed files:
  - `src/runtime/vm.hh`
  - `src/runtime/vm.cc`
  - legacy interpreter interface
  - legacy interpreter source
  - `tests/unit/test_vm_compiler.cc`

2. Subagent-B (Batch B: W10-D3)
- Scope:
  - locked closure/class integration behavior across execution routes
- Changed files:
  - `tests/integration/test_language_closure.cc`
  - `tests/integration/test_language_class.cc`

3. Subagent-C (Batch C: W10-D4 + W10-D5)
- Scope:
  - aligned execution-mode policy docs with ADR and PLAN GAP closure tracking
  - captured verification evidence
- Changed files:
  - `docs/adr/ADR-001-execution-model.md`
  - `PLAN.md`
  - `SUBAGENTS.md`

4. Verification command and result
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`
- Result: pass (`1/1` test suite)

## W12 Execution (Subagents Mode)

Date: 2026-03-09

1. Subagent-A (Batch A: W12-D1 + W12-D2)
- Scope:
  - aligned runtime error-family emission for value/operator/callability contracts (`MS4001~MS4005`) across VM and compatibility interpreter paths
  - updated integration/unit assertions to lock code-bearing diagnostics for arity/property/undefined-variable failures
- Changed files:
  - `src/runtime/vm.cc`
  - legacy interpreter source
  - `tests/unit/test_vm_compiler.cc`
  - `tests/integration/test_language_closure.cc`
  - `tests/integration/test_language_class.cc`

2. Subagent-B (Batch B: W12-D3)
- Scope:
  - aligned module loader lifecycle diagnostics and cache-state handling to `MS5001~MS5004`
  - added module failure memory to represent failed-state behavior without treating it as successful cache
- Changed files:
  - `src/runtime/module.hh`
  - `src/runtime/module.cc`
  - `src/runtime/vm.cc`
  - `tests/unit/test_module.cc`

3. Subagent-C (Batch C: W12-D4 + W12-D5)
- Scope:
  - migrated value/module error clauses into standalone conformance assets and refreshed matrix mappings
  - updated wave closeout and GAP status bookkeeping
- Changed files:
  - `tests/conformance/MATRIX.md`
  - `tests/conformance/semantics/runtime_arity_001.ms`
  - `tests/conformance/semantics/runtime_invalid_operand_001.ms`
  - `tests/conformance/semantics/runtime_non_callable_001.ms`
  - `tests/conformance/semantics/runtime_undefined_property_001.ms`
  - `tests/conformance/semantics/runtime_undefined_variable_001.ms`
  - `tests/conformance/modules/module_not_found_001.ms`
  - `tests/conformance/modules/module_symbol_not_found_001.ms`
  - `tests/conformance/modules/module_cycle_001.ms`
  - `PLAN.md`
  - `SUBAGENTS.md`

4. Verification command and result
- `cmake --build build --config Debug`
- `ctest --test-dir build --output-on-failure -C Debug`
- Result: pass (`1/1` test suite)
