#include "test_common.hh"

#include <string>
#include <vector>

#include "frontend/lexer.hh"
#include "frontend/resolver_pass.hh"

namespace {

bool HasCode(const std::vector<std::string>& errors, const std::string& code) {
  for (const std::string& error : errors) {
    if (error.find(code) != std::string::npos) {
      return true;
    }
  }
  return false;
}

}  // namespace

int RunResolverPassTests() {
  {
    ms::Lexer lexer("{\n var x = 1\n { var y = 2 }\n}\n", true);
    ms::ResolverPassResult pass = ms::run_resolver_pass(lexer.scan_all_tokens());
    Expect(pass.metadata.max_scope_depth >= 2,
           "resolver pass should track nested scope depth metadata");
    Expect(pass.metadata.scope_transition_count >= 2,
           "resolver pass should track scope transitions");
  }

  {
    ms::ResolverPass resolver;
    resolver.begin_scope();
    const bool duplicate = resolver.declare_local("captured");
    Expect(!duplicate, "first local declaration should not be duplicate");
    resolver.mark_initialized();

    resolver.begin_function(ms::ResolverFunctionType::kFunction);
    const std::optional<ms::ResolverBinding> binding = resolver.resolve_get_binding("captured");
    Expect(binding.has_value(), "resolver should return a binding for captured local");
    Expect(binding.has_value() && binding->kind == ms::ResolverBindingKind::kUpvalue,
           "captured local should resolve to upvalue in nested function");
    const std::vector<ms::ResolverUpvalue> upvalues = resolver.end_function();
    Expect(upvalues.size() == 1, "nested function should persist one upvalue metadata entry");
    Expect(upvalues.size() == 1 && upvalues[0].is_local,
           "captured variable should be marked as local upvalue capture");
  }

  {
    ms::ResolverPass resolver;
    resolver.check_return_allowed(1);
    resolver.check_this_allowed(2);
    (void)resolver.check_super_allowed(3);

    resolver.begin_class();
    resolver.set_current_class_superclass("Self", "Self", 4);
    resolver.end_class();

    const std::vector<std::string>& errors = resolver.errors();
    Expect(HasCode(errors, "MS3001"), "resolver should report return-outside-function");
    Expect(HasCode(errors, "MS3002"), "resolver should report this-outside-class");
    Expect(HasCode(errors, "MS3003"), "resolver should report super-outside-subclass");
    Expect(HasCode(errors, "MS3004"), "resolver should report self-inheritance");
  }

  {
    ms::ResolverPass resolver;
    resolver.begin_class();
    resolver.set_current_class_superclass("Leaf", "Base", 10);
    const bool valid_super = resolver.check_super_allowed(11);
    Expect(valid_super, "resolver should accept super usage in subclass context");
    resolver.end_class();
  }

  return 0;
}
