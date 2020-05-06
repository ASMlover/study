#include <vector>
#include <Tadpole/Lexer.hh>
#include <Tadpole/Value.hh>
#include <Tadpole/Chunk.hh>
#include <Tadpole/VM.hh>
#include <Tadpole/Compiler.hh>

namespace _mevo::tadpole {

enum class Precedence {
  NONE,

  ASSIGNMENT, // =
  TERM,       // + -
  FACTOR,     // * /
  CALL,       // ()

  PRIMARY,
};

inline Precedence operator+(Precedence a, int b) noexcept {
  return as_type<Precedence>(as_type<int>(a) + b);
}

struct ParseRule {
  using ParseFn = std::function<void (GlobalParser&, bool)>;

  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

struct LocalVar {
  Token name;
  int depth{};
  bool is_upvalue{};

  LocalVar(const Token& _name, int _depth = -1, bool _is_upvalue = false) noexcept
    : name(_name), depth(_depth), is_upvalue(_is_upvalue) {
  }
};

struct Upvalue {
  u8_t index{};
  bool is_local{};

  Upvalue(u8_t _index = 0, bool _is_local = false) noexcept
    : index(_index), is_local(_is_local) {
  }
};

enum class FunType {
  FUNCTION,
  TOPLEVEL,
};

class Compiler final : private UnCopyable {
  using ErrorFn = std::function<void (const str_t&)>;

  Compiler* enclosing_{};
  FunctionObject* fn_{};
  FunType fn_type_{};

  int scope_depth_{};

  std::vector<LocalVar> locals_;
  std::vector<Upvalue> upvalues_;
public:
  Compiler() noexcept {}

  Compiler(Compiler* enclosing,
    FunctionObject* fn, FunType fn_type, int scope_depth = 0) noexcept
    : enclosing_(enclosing), fn_(fn), fn_type_(fn_type), scope_depth_(scope_depth) {
  }

  inline void set_compiler(Compiler* enclosing,
    FunctionObject* fn, FunType fn_type, int scope_depth = 0) noexcept {
    enclosing_ = enclosing;
    fn_ = fn;
    fn_type_ = fn_type;
    scope_depth_ = scope_depth;
  }

  inline Compiler* enclosing() const noexcept { return enclosing_; }
  inline FunctionObject* fn() const noexcept { return fn_; }
  inline FunType fn_type() const noexcept { return fn_type_; }
  inline int scope_depth() const noexcept { return scope_depth_; }
  inline void set_scope_depth(int depth) noexcept { scope_depth_ = depth; }
  inline int locals_count() const noexcept { return as_type<int>(locals_.size()); }
  inline LocalVar& get_local(sz_t i) noexcept { return locals_[i]; }
  inline const LocalVar& get_local(sz_t i) const noexcept { return locals_[i]; }
  inline LocalVar& peek_local() noexcept { return locals_.back(); }
  inline const LocalVar& peek_local() const noexcept { return locals_.back(); }
  inline void append_local(const LocalVar& lvar) noexcept { locals_.push_back(lvar); }
  inline Upvalue& get_upvalue(sz_t i) noexcept { return upvalues_[i]; }
  inline const Upvalue& get_upvalue(sz_t i) const noexcept { return upvalues_[i]; }
  inline void append_upvalue(const Upvalue& u) noexcept { upvalues_.push_back(u); }

  void enter_scope() noexcept { ++scope_depth_; }
  template <typename Fn> void leave_scope(Fn&& visitor) {
    --scope_depth_;
    while (!locals_.empty() && locals_.back().depth > scope_depth_) {
      visitor(locals_.back());
      locals_.pop_back();
    }
  }

  int resolve_local(const Token& name, const ErrorFn& errfn) {
    for (int i = locals_count() - 1; i >= 0; --i) {
      auto& local = locals_[i];
      if (local.name == name) {
        if (local.depth == -1)
          errfn("cannot load local variable `" + name.as_string() + "` in its own initializer");
        return i;
      }
    }
    return -1;
  }

  int add_upvalue(u8_t index, bool is_local) {
    for (int i = 0; i < fn_->upvalues_count(); ++i) {
      auto& upvalue = upvalues_[i];
      if (upvalue.index == index && upvalue.is_local == is_local)
        return i;
    }

    upvalues_.push_back(Upvalue(index, is_local));
    return fn_->inc_upvalues_count();
  }

  int resolve_upvalue(const Token& name, const ErrorFn& errfn) {
    if (enclosing_ == nullptr)
      return -1;

    if (int local = enclosing_->resolve_local(name, errfn); local != -1) {
      enclosing_->locals_[local].is_upvalue = true;
      return add_upvalue(as_type<u8_t>(local), true);
    }
    if (int upvalue = enclosing_->resolve_upvalue(name, errfn); upvalue != -1)
      return add_upvalue(as_type<u8_t>(upvalue), false);

    return -1;
  }

  void declare_localvar(const Token& name, const ErrorFn& errfn) {
    if (scope_depth_ == 0)
      return;

    for (auto it = locals_.rbegin(); it != locals_.rend(); ++it) {
      if (it->depth != -1 && it->depth < scope_depth_)
        break;

      if (it->name == name)
        errfn("name `" + name.as_string() + "` is redefined");
    }
    locals_.push_back(LocalVar(name, -1, false));
  }
};

class GlobalParser final : private UnCopyable {
  static constexpr int kMaxArguments = 8;

  VM& vm_;
  Lexer& lex_;
  Token prev_;
  Token curr_;

  bool had_error_{};
  bool panic_mode_{};

  Compiler* curr_compiler_{};

  void error_at(const Token& tok, const str_t& msg) noexcept {
    if (panic_mode_)
      return;
    panic_mode_ = true;

    std::cerr
      << "SyntaxError:" << std::endl
      << "  [LINE: " << tok.lineno() << "] ERROR ";
    if (tok.kind() == TokenKind::TK_EOF)
      std::cerr << "at end";
    else if (tok.kind() == TokenKind::TK_ERR)
      (void)0;
    else
      std::cerr << "at `" << tok.literal() << "` ";
    std::cerr << ": " << msg << std::endl;

    had_error_ = true;
  }

  inline void error_at_current(const str_t& msg) noexcept { error_at(curr_, msg); }
  inline void error(const str_t& msg) noexcept { error_at(prev_, msg); }
  inline Chunk* curr_chunk() const noexcept { return curr_compiler_->fn()->chunk(); }
  inline bool check(TokenKind kind) const noexcept { return curr_.kind() == kind; }

  void advance() {
    prev_ = curr_;

    for (;;) {
      curr_ = lex_.next_token();
      if (!check(TokenKind::TK_ERR))
        break;

      error_at_current(curr_.as_string());
    }
  }

  void consume(TokenKind kind, const str_t& msg) {
    if (check(kind))
      advance();
    else
      error_at_current(msg);
  }

  bool match(TokenKind kind) {
    if (check(kind)) {
      advance();
      return true;
    }
    return false;
  }

  template <typename T> inline void emit_byte(T byte) noexcept {
    curr_chunk()->write(byte, prev_.lineno());
  }

  template <typename T, typename U> inline void emit_bytes(T b1, U b2) noexcept {
    emit_byte(b1);
    emit_byte(b2);
  }

  inline void emit_return() noexcept {
    emit_bytes(Code::NIL, Code::RETURN);
  }

  inline void emit_constant(const Value& v) noexcept {
    emit_bytes(Code::CONSTANT, curr_chunk()->add_constant(v));
  }

  void init_compiler(Compiler* compiler, int scope_depth, FunType fn_type) {
    StringObject* func_name{};
    if (fn_type == FunType::FUNCTION)
      func_name = StringObject::create(vm_, prev_.as_string());

    compiler->set_compiler(
      curr_compiler_,
      FunctionObject::create(vm_, func_name),
      fn_type,
      scope_depth);
    curr_compiler_ = compiler;

    curr_compiler_->append_local(LocalVar(Token::make(""), curr_compiler_->scope_depth(), false));
  }

  FunctionObject* finish_compiler() {
    emit_return();

    FunctionObject* fn = curr_compiler_->fn();
    if (!had_error_)
      curr_chunk()->dis(fn->name_asstr());
    curr_compiler_ = curr_compiler_->enclosing();

    return fn;
  }
};

FunctionObject* GlobalCompiler::compile(VM& vm, const str_t& source_bytes) {
  return nullptr;
}

void GlobalCompiler::mark_compiler() {
}

}