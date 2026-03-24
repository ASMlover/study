#include "frontend/resolver_pass.hh"

#include <sstream>

namespace ms {

ResolverPassResult run_resolver_pass(std::vector<Token> tokens) {
  ResolverPass resolver;
  for (const Token& token : tokens) {
    if (token.type == TokenType::kLeftBrace) {
      resolver.begin_scope();
      continue;
    }
    if (token.type == TokenType::kRightBrace && !resolver.is_global_scope()) {
      (void)resolver.end_scope();
    }
  }

  ResolverPassResult result;
  result.tokens = std::move(tokens);
  result.metadata.max_scope_depth = resolver.max_scope_depth();
  result.metadata.scope_transition_count = resolver.scope_transition_count();
  result.errors = resolver.errors();
  return result;
}

ResolverPass::ResolverPass() {
  auto script = std::make_unique<FunctionContext>();
  script->type = ResolverFunctionType::kScript;
  current_ = script.get();
  function_contexts_.push_back(std::move(script));
}

void ResolverPass::begin_scope() {
  ++current_->scope_depth;
  ++scope_transition_count_;
  if (static_cast<std::size_t>(current_->scope_depth) > max_scope_depth_) {
    max_scope_depth_ = static_cast<std::size_t>(current_->scope_depth);
  }
}

std::vector<ScopeExitAction> ResolverPass::end_scope() {
  --current_->scope_depth;
  ++scope_transition_count_;
  std::vector<ScopeExitAction> actions;
  while (!current_->locals.empty() && current_->locals.back().depth > current_->scope_depth) {
    actions.push_back(ScopeExitAction{current_->locals.back().is_captured});
    current_->locals.pop_back();
  }
  return actions;
}

bool ResolverPass::is_global_scope() const { return current_->scope_depth == 0; }

ResolverFunctionType ResolverPass::current_function_type() const { return current_->type; }

bool ResolverPass::declare_local(const std::string& name) {
  if (current_->scope_depth == 0) {
    return false;
  }

  bool duplicate = false;
  for (int i = static_cast<int>(current_->locals.size()) - 1; i >= 0; --i) {
    const Local& local = current_->locals[static_cast<std::size_t>(i)];
    if (local.depth != -1 && local.depth < current_->scope_depth) {
      break;
    }
    if (local.name == name) {
      duplicate = true;
      break;
    }
  }

  current_->locals.push_back(Local{name, -1, false});
  return duplicate;
}

void ResolverPass::mark_initialized() {
  if (current_->scope_depth == 0 || current_->locals.empty()) {
    return;
  }
  current_->locals.back().depth = current_->scope_depth;
}

std::optional<ResolverBinding> ResolverPass::resolve_get_binding(const std::string& name) {
  if (const auto local = resolve_local(current_, name); local.has_value()) {
    return ResolverBinding{ResolverBindingKind::kLocal, *local};
  }
  if (const auto upvalue = resolve_upvalue(current_, name); upvalue.has_value()) {
    return ResolverBinding{ResolverBindingKind::kUpvalue, *upvalue};
  }
  return ResolverBinding{ResolverBindingKind::kGlobal, 0};
}

std::optional<ResolverBinding> ResolverPass::resolve_set_binding(const std::string& name) {
  return resolve_get_binding(name);
}

void ResolverPass::begin_function(const ResolverFunctionType type) {
  auto nested = std::make_unique<FunctionContext>();
  nested->scope_depth = 1;
  nested->type = type;
  nested->enclosing = current_;
  const std::string first_local =
      (type == ResolverFunctionType::kMethod || type == ResolverFunctionType::kInitializer)
          ? "this"
          : "";
  nested->locals.push_back(Local{first_local, 0, false});
  current_ = nested.get();
  function_contexts_.push_back(std::move(nested));
  if (max_scope_depth_ < 1) {
    max_scope_depth_ = 1;
  }
}

std::vector<ResolverUpvalue> ResolverPass::end_function() {
  const std::vector<ResolverUpvalue> upvalues = current_->upvalues;
  current_ = current_->enclosing;
  return upvalues;
}

void ResolverPass::begin_class() {
  auto klass = std::make_unique<ClassContext>();
  klass->enclosing = current_class_;
  current_class_ = klass.get();
  class_contexts_.push_back(std::move(klass));
}

void ResolverPass::end_class() {
  if (current_class_ == nullptr) {
    return;
  }
  current_class_ = current_class_->enclosing;
}

void ResolverPass::set_current_class_superclass(const std::string& class_name,
                                                const std::string& super_name,
                                                const std::size_t line) {
  if (super_name == class_name) {
    report_resolve_error(line, "MS3004", "a class cannot inherit from itself");
  }
  if (current_class_ != nullptr) {
    current_class_->has_superclass = true;
    current_class_->superclass_name = super_name;
  }
}

bool ResolverPass::current_class_has_superclass() const {
  return current_class_ != nullptr && current_class_->has_superclass;
}

std::string ResolverPass::current_class_superclass_name() const {
  if (current_class_ == nullptr) {
    return {};
  }
  return current_class_->superclass_name;
}

void ResolverPass::check_return_allowed(const std::size_t line) {
  if (current_->type == ResolverFunctionType::kScript) {
    report_resolve_error(line, "MS3001", "cannot return from top-level code");
  }
}

void ResolverPass::check_this_allowed(const std::size_t line) {
  if (current_class_ == nullptr) {
    report_resolve_error(line, "MS3002", "cannot use 'this' outside of a class");
  }
}

bool ResolverPass::check_super_allowed(const std::size_t line) {
  const bool valid_super = current_class_ != nullptr && current_class_->has_superclass;
  if (!valid_super) {
    report_resolve_error(line, "MS3003", "cannot use 'super' outside of a subclass");
  }
  return valid_super;
}

std::size_t ResolverPass::max_scope_depth() const { return max_scope_depth_; }

std::size_t ResolverPass::scope_transition_count() const { return scope_transition_count_; }

const std::vector<std::string>& ResolverPass::errors() const { return errors_; }

void ResolverPass::report_resolve_error(const std::size_t line, const std::string& code,
                                        const std::string& message) {
  std::ostringstream out;
  out << "[line " << line << "] resolve error (" << code << "): " << message;
  errors_.push_back(out.str());
}

std::optional<std::uint8_t> ResolverPass::resolve_local(FunctionContext* context,
                                                        const std::string& name) const {
  for (int i = static_cast<int>(context->locals.size()) - 1; i >= 0; --i) {
    if (context->locals[static_cast<std::size_t>(i)].name == name) {
      return static_cast<std::uint8_t>(i);
    }
  }
  return std::nullopt;
}

std::uint8_t ResolverPass::add_upvalue(FunctionContext* context, const std::uint8_t index,
                                       const bool is_local) {
  for (std::size_t i = 0; i < context->upvalues.size(); ++i) {
    const ResolverUpvalue& upvalue = context->upvalues[i];
    if (upvalue.index == index && upvalue.is_local == is_local) {
      return static_cast<std::uint8_t>(i);
    }
  }
  context->upvalues.push_back(ResolverUpvalue{index, is_local});
  return static_cast<std::uint8_t>(context->upvalues.size() - 1);
}

std::optional<std::uint8_t> ResolverPass::resolve_upvalue(FunctionContext* context,
                                                          const std::string& name) {
  if (context->enclosing == nullptr) {
    return std::nullopt;
  }

  if (const auto local = resolve_local(context->enclosing, name); local.has_value()) {
    context->enclosing->locals[*local].is_captured = true;
    return add_upvalue(context, *local, true);
  }

  if (const auto upvalue = resolve_upvalue(context->enclosing, name); upvalue.has_value()) {
    return add_upvalue(context, *upvalue, false);
  }

  return std::nullopt;
}

}  // namespace ms
