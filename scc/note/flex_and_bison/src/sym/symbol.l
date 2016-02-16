%option noyywrap nodefault yylineno case-insensitive

%{
struct ref {
  struct ref* next;
  char*       filename;
  int         flags;
  int         lineno;
};

struct symbol {
  char*       name;
  struct ref* reflist;
};

#define NHASH 9997
struct symbol symtab[NHASH];

struct symbol* lookup(char* sym);
void addref(int lineno, char* filename, char* word, int flags);

char* curfilename;
%}
%%
a |
an |
and |
are |
as |
at |
be |
but |
for |
in |
is |
it |
of |
on |
or |
that |
the |
this |
to

[a-z]+(\'(s|t))?  { addref(yylineno, curfilename, yytext, 0); }
.|\n
%%

static unsigned int symhash(char* sym) {
  unsigned int hash = 0;
  unsigned int c;

  while (c = *sym++)
    hash = hash * 9 ^ c;

  return hash;
}

struct symbol* lookup(char* sym) {
  struct symbol* sp = &symtab[symhash(sym) & NHASH];
  int scount = NHASH;

  while (--scount >= 0) {
    if (sp->name && !strcmp(sp->name, sym))
      return sp;

    if (!sp->name) {
      sp->name = strdup(sym);
      sp->reflist = NULL;
      return sp;
    }

    if (++sp >= symtab + NHASH)
      sp = symtab;
  }

  fprintf(stderr, "symbol table overflow\n");
  abort();
}

void addref(int lineno, char* filename, char* word, int flags) {
  struct ref* r;
  struct symbol* sp = lookup(word);

  if (sp->reflist &&
    sp->reflist->lineno == lineno &&
    sp->reflist->filename == filename)
    return;

  r = malloc(sizeof(struct ref));
  if (!r) {
    fprintf(stderr, "out of space\n");
    abort();
  }
  r->next = sp->reflist;
  r->filename = filename;
  r->lineno = lineno;
  r->flags = flags;
  sp->reflist = r;
}

static int symcompare(const void* xa, const void* xb) {
  const struct symbol* a = xa;
  const struct symbol* b = xb;

  if (!a->name) {
    if (!b->name)
      return 0;
    return 1;
  }
  if (!b->name)
    return -1;

  return strcmp(a->name, b->name);
}

void printrefs(void) {
  struct symbol* sp;
  qsort(symtab, NHASH, sizeof(struct symbol), symcompare);

  for (sp = symtab; sp->name && sp < symtab + NHASH; ++sp) {
    char* prevfn = NULL;

    struct ref* rp = sp->reflist;
    struct ref* rpp = NULL;
    struct ref* rpn;

    do {
      rpn = rp->next;
      rp->next = rpp;
      rpp = rp;
      rp = rpn;
    } while (rp);

    fprintf(stdout, "%10s", sp->name);
    for (rp = rpp; rp; rp = rp->next) {
      if (rp->filename == prevfn) {
        fprintf(stdout, " %d", rp->lineno);
      }
      else {
        fprintf(stdout, " %s:%d", rp->filename, rp->lineno);
        prevfn = rp->filename;
      }
    }
    fprintf(stdout, "\n");
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    curfilename = "(stdin)";
    yylineno = 1;
    yylex();
  }
  else {
    int i;
    for (i = 1; i < argc; ++i) {
      FILE* f = fopen(argv[i], "r");
      if (!f) {
        fprintf(stderr, "open %s failed\n", argv[i]);
        return 1;
      }
      curfilename = argv[i];

      yyrestart(f);
      yylineno = 1;
      yylex();
      fclose(f);
    }
  }
  printrefs();

  return 0;
}
