#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "frontend/token.hh"

namespace ms {

enum class ResolverFunctionType { kScript, kFunction, kMethod, kInitializer };

enum class ResolverBindingKind { kLocal, kUpvalue, kGlobal };

struct ResolverBinding {
  ResolverBindingKind kind = ResolverBindingKind::kGlobal;
  std::uint8_t slot = 0;
};

struct ResolverUpvalue {
  std::uint8_t index = 0;
  bool is_local = false;
};

struct ScopeExitAction {
  bool is_captured = false;
};

struct ResolverMetadata {
  std::size_t max_scope_depth = 0;
  std::size_t scope_transition_count = 0;
};

struct ResolverPassResult {
  std::vector<Token> tokens;
  ResolverMetadata metadata;
  std::vector<std::string> errors;
};

ResolverPassResult run_resolver_pass(std::vector<Token> tokens);

class ResolverPass {
 public:
  ResolverPass();

  void begin_scope();
  std::vector<ScopeExitAction> end_scope();

  bool is_global_scope() const;
  ResolverFunctionType current_function_type() const;

  bool declare_local(const std::string& name);
  void mark_initialized();

  std::optional<ResolverBinding> resolve_get_binding(const std::string& name);
  std::optional<ResolverBinding> resolve_set_binding(const std::string& name);

  void begin_function(ResolverFunctionType type);
  std::vector<ResolverUpvalue> end_function();

  void begin_class();
  void end_class();
  void set_current_class_superclass(const std::string& class_name, const std::string& super_name,
                                    std::size_t line);
  bool current_class_has_superclass() const;
  std::string current_class_superclass_name() const;

  void check_return_allowed(std::size_t line);
  void check_this_allowed(std::size_t line);
  bool check_super_allowed(std::size_t line);

  std::size_t max_scope_depth() const;
  std::size_t scope_transition_count() const;
  const std::vector<std::string>& errors() const;

 private:
  struct Local {
    std::string name;
    int depth = -1;
    bool is_captured = false;
  };

  struct FunctionContext {
    std::vector<Local> locals;
    std::vector<ResolverUpvalue> upvalues;
    int scope_depth = 0;
    ResolverFunctionType type = ResolverFunctionType::kScript;
    FunctionContext* enclosing = nullptr;
  };

  struct ClassContext {
    ClassContext* enclosing = nullptr;
    bool has_superclass = false;
    std::string superclass_name;
  };

  void report_resolve_error(std::size_t line, const std::string& code, const std::string& message);
  std::optional<std::uint8_t> resolve_local(FunctionContext* context, const std::string& name) const;
  std::uint8_t add_upvalue(FunctionContext* context, std::uint8_t index, bool is_local);
  std::optional<std::uint8_t> resolve_upvalue(FunctionContext* context, const std::string& name);

  std::vector<std::string> errors_;
  std::vector<std::unique_ptr<FunctionContext>> function_contexts_;
  FunctionContext* current_ = nullptr;
  std::vector<std::unique_ptr<ClassContext>> class_contexts_;
  ClassContext* current_class_ = nullptr;
  std::size_t max_scope_depth_ = 0;
  std::size_t scope_transition_count_ = 0;
};

}  // namespace ms
