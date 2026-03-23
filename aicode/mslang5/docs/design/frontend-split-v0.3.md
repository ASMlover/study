# Maple Frontend Split Design (v0.3, P3-03)

Status: Draft (non-behavioral architecture plan)  
Date: 2026-03-24  
Scope: Design only. No runtime/language behavior change in this document.

## 1. Problem Statement

Current frontend responsibilities are concentrated in `src/frontend/compiler.cc`:

1. Parsing and statement/expression control flow.
2. Resolver-like static semantic checks and lexical capture decisions.
3. Bytecode emission and constant/jump patching.

This coupling increases change risk and makes parser recovery, resolver diagnostics, and lowering evolution difficult to validate independently.

## 2. Goals

1. Split frontend into explicit phases: `parser`, `resolver`, `lowering`.
2. Keep language behavior and VM bytecode semantics unchanged during the split.
3. Define phase ownership and stable data contracts for future work.
4. Enable phase-local tests and diagnostics accountability.

## 3. Non-Goals

1. No type system or optimization pipeline in v0.3.
2. No diagnostics code renumbering in this task.
3. No AST format freeze for external tooling.

## 4. Target Phase Architecture

Pipeline:

1. `Lexer` -> token stream.
2. `Parser` -> AST + parse diagnostics (`MS2xxx`).
3. `Resolver` -> resolved AST + symbol metadata + resolve diagnostics (`MS3xxx`).
4. `Lowering` -> `Chunk`/`FunctionPrototype` graph + compile-time lowering diagnostics (`MS4xxx` where applicable).

Compatibility anchor:

1. Existing behavior contract remains `phase + code`.
2. Message text may evolve with compatibility notes.

## 5. Phase Responsibilities

## 5.1 Parser Phase

Owned concerns:

1. Grammar-driven syntax structure only.
2. Statement boundary handling (semicolon/newline policy).
3. Recovery and synchronization at statement/block boundaries.

Must not:

1. Choose local/upvalue/global storage strategy.
2. Emit bytecode directly.

Outputs:

1. `AstModule` (root node and child declarations/statements).
2. `ParseDiagnostics` (line/column spans where available).

## 5.2 Resolver Phase

Owned concerns:

1. Lexical scope stack and binding lifecycle (`declare`/`define`/`capture`).
2. Contextual semantic checks (`return`, `this`, `super`, self-inherit, etc.).
3. Symbol resolution metadata attachment for variable accesses and assignments.

Must not:

1. Emit bytecode/jump offsets.
2. Re-parse source text.

Outputs:

1. `ResolutionTable` keyed by AST node id.
2. `ResolveDiagnostics` (`MS3xxx`).

## 5.3 Lowering Phase

Owned concerns:

1. Map resolved AST into bytecode operations and constant table entries.
2. Build nested `FunctionPrototype` objects.
3. Patch control-flow jumps and closure upvalue operands.

Must not:

1. Perform grammar recovery.
2. Invent semantic rules that belong to resolver.

Outputs:

1. `CompileResult.chunk` and compiled nested function constants.
2. `LoweringDiagnostics` (phase `compile`/`lowering`, canonical code mapping aligned with diagnostics spec).

## 6. Core Data Contracts

## 6.1 AST Contract (Parser -> Resolver)

Required properties:

1. Stable node ids within one compile invocation.
2. Span metadata per node (line required, column optional baseline).
3. Node kinds covering current language features (declaration, statement, expression, class/function/module forms).

## 6.2 Resolution Contract (Resolver -> Lowering)

Per identifier expression/assignment:

1. `binding_kind`: `local | upvalue | global | synthetic_this | synthetic_super`.
2. `depth`: lexical depth when non-global.
3. `slot`: resolved local/upvalue slot index when applicable.

Per function/class context:

1. function kind: `script | function | method | initializer`.
2. class context: `none | class | subclass`.

## 6.3 Diagnostics Contract

1. Parser emits only parse-phase diagnostics.
2. Resolver emits only resolve-phase diagnostics.
3. Lowering emits only lowering/compile diagnostics.
4. Aggregation order follows source order first, then phase-local stable tie-breaker.

## 7. Incremental Migration Plan

## Step 1: Introduce AST Without Behavior Drift

1. Add AST node model and parser entry point returning `AstModule`.
2. Keep legacy compile path as reference until parity checks pass.
3. Add snapshot tests for parser output shape on representative scripts.

## Step 2: Extract Resolver Pass

1. Move resolver-like logic from `CompilerImpl` into dedicated resolver unit.
2. Emit `ResolutionTable` and resolve diagnostics.
3. Gate lowering on resolve error presence using existing compile failure behavior.

## Step 3: Extract Lowering Pass

1. Rework bytecode emission to consume `AstModule + ResolutionTable`.
2. Preserve opcode sequence parity for baseline scripts.
3. Keep runtime behavior unchanged under existing conformance suites.

## Step 4: Retire Monolithic Compiler Path

1. Remove duplicated monolithic code after parity/stability gates are green.
2. Keep compile public API stable (`compile_to_chunk`) to avoid VM call-site churn.

## 8. Acceptance Criteria for P3-03 Deliverable

1. Explicit phase boundary document exists and is reviewable.
2. Each phase has clear owned concerns and forbidden concerns.
3. Data contracts between phases are defined.
4. Migration order is incremental and behavior-preserving.
5. `docs/improve_03.md` progress entry references this design artifact.

## 9. Risks and Mitigations

1. Risk: hidden semantic coupling during extraction.  
   Mitigation: keep legacy path until parity tests are stable.
2. Risk: diagnostics phase drift.  
   Mitigation: enforce phase ownership in diagnostics contract and tests.
3. Risk: AST churn delays resolver/lowering integration.  
   Mitigation: start with minimal AST surface required by current grammar.

## 10. Deliverable Mapping

P3-03 maps to Stream B / Milestone M2:

1. Design artifact: this document.
2. No code behavior change in this task.
