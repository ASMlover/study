// @id: RES-SUPER-NO-SUPERCLASS-001
// @spec: semantics.class.super
// @spec: errors.MS3003
// @expect: compile_error
// @diag.phase: resolve
// @diag.code: MS3003
// @diag.message_contains: cannot use 'super' outside of a subclass
class Solo {
  ping() {
    return super.ping()
  }
}

print Solo().ping()
