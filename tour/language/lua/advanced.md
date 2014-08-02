# **高级**
***



## **1. 线程和状态**
> ### **1.1 多个线程**
    1) Lua中, 线程的本质就是一个协同程序(协同程序就是一个线程外加一套良好的
       操作接口);
    2) 一个栈拥有一个线程得以继续运行的所有信息, 多个线程就是多个独立的栈;
    3) 只要创建一个Lua状态, 就会自动在这个状态中创建一个新线程——'主线程',
       主线程永远不会回收, 调用lua_close的时候随状态一起释放;
    4) lua_newthread -> 可以在一个状态中创建其他线程; 老线程的栈顶就是这个新
       创建的线程;
    5) lua_xmove -> 可以在两个栈之间移动Lua值;
    6) 使用多线程的目的是实现协同程序;
    7) lua_resume -> 恢复某个协同程序;
    8) lua_yield -> 可以挂起lua调用者;

> ### **1.2 Lua状态**
    1) 每次调用lua_newstate都会创建一个新的Lua状态, 不同的Lua状态是各自独立
       的, 它们之间不共享任何数据;
    2) 对两个状态lua_pushstring(L2, lua_tostring(L1, -1))可以将L1栈顶的字符
       压入L2中;
          typedef struct Proc {
            lua_State* L;
            pthread_t thread;
            pthread_cond_t cond;
            const char* channel;
            struct Proc* prev;
            struct Proc* next;
          } Proc;
          static Proc* g_wait_send = NULL;
          static Proc* g_wait_recv = NULL;
          static pthread_mutex_t g_kernerl_access 
            = PTHREAD_MUTEX_INITIALIZE;
          static Proc* get_self(lua_State* L) {
            Proc* p;
            lua_getfield(L, LUA_REGISTRYINDEX, "_SELF");
            p = (Proc*)lua_touserdata(L, -1);
            lua_pop(L, 1);
            return p;
          }
          static void move_values(lua_State* send, lua_State* recv) {
            int n = lua_gettop(send);
            int i;
            for (i = 2; i <= n; ++i)
              lua_pushstring(recv, lua_tostring(send, i));
          }
          static Proc* search_match(const char* channel, Proc** list) {
            Proc* node = *list;
            if (NULL == node)
              return NULL;
            do {
              if (0 == strcmp(channel, node->channel)) {
                if (*list == node) 
                  *list = (node->next == node) ? NULL : node->next;
                node->prev->next = node->next;
                node->next->prev = node->prev;
                return node;
              }
              node = node->next;
            } while (node != *list);
            return NULL;
          }
          static void wait_on_list(lua_State* L, 
              const char* channel, Proc** list) {
            Proc* p = get_self(L);
            if (NULL == *list) {
              *list = p;
              p->prev = p->next = p;
            }
            else {
              p->prev = (*list)->prev;
              p->next = *list;
              p->prev->next = p->next->prev = p;
            }
            p->channel = channel;
            do {
              pthread_cond_wait(&p->cond, &g_kernerl_access);
            } while (p->channel);
          }
          static int l_send(lua_State* L) {
            Proc* p;
            const char* channel = luaL_checkstring(L, 1);
            pthread_mutex_lock(&g_kernerl_access);
            p = search_match(channel, &g_wait_recv);
            if (p) {
              move_values(L, p->L);
              p->channel = NULL;
              pthread_cond_signal(&p->cond);
            }
            else {
              wait_on_list(L, channel, &g_wait_send);
            }
            pthread_mutex_unlock(&g_kernerl_access);
            return 0;
          }
          static int l_recv(lua_State* L) {
            Proc* p;
            const char* channel = luaL_checkstring(L, 1);
            lua_settop(L, 1);
            pthread_mutex_lock(&g_kernerl_access);
            p = search_match(channel, &g_wait_send);
            if (p) {
              move_values(p->L, L);
              p->channel = NULL;
              pthread_cond_signal(&p->cond);
            }
            else {
              wait_on_list(L, channel, &g_wait_recv);
            }
            pthread_mutex_unlock(&g_kernerl_access);
            return lua_gettop(L) - 1;
          }
          static void* l_thread(void* arg) {
            lua_State* L = (lua_State*)arg;
            luaL_openlibs(L);
            lua_cpcall(L, luaopen_lproc, NULL);
            if (lua_pcall(L, 0, 0, 0) != 0)
              fprintf(stderr, "error ...");
            pthread_cond_destroy(&get_self(L)->cond);
            lua_close(L);
            return NULL;
          }
          static int l_start(lua_State* L) {
            pthread_t thread;
            const char* chunk = luaL_checkstring(L, 1);
            lua_State* L1 = luaL_newstate();
            if (NULL == L1)
              luaL_error(L, "unable ...");
            if (0 != luaL_loadstring(L1, chunk))
              luaL_error(L, "error ...");
            if (pthread_create(&thread, NULL, l_thread, L1) != 0)
              luaL_error(L, "unable ...");
            pthread_detach(thread);
            return 0;
          }
          static int l_exit(lua_State* L) {
            pthread_exit(NULL);
            return 0;
          }
          static const struct luaL_reg funcs[] = {
            {"start", l_start}, 
            {"send", l_send}, 
            {"recv", l_recv}, 
            {"exit", l_exit}, 
            {NULL, NULL},
          };
          int luaopen_lproc(lua_State* L) {
            Proc* self = (Proc*)lua_newuserdata(L, sizeof(Proc));
            lua_setfield(L, LUA_REGISTRYINDEX, "_SELF");
            self->L = L;
            self->thread = pthread_self();
            self->channel = NULL;
            pthread_cond_init(&self->cond, NULL);
            luaL_register(L, "lproc", funcs);
            return 1;
          }



## **2. 内存管理**
> ### **2.1 分配函数**
    1) lua_newstate会以一个默认的分配函数来创建Lua状态;
        lua_State* lua_newstate(lua_Alloc f, void* ud);
        typedef void* (*lua_Alloc)(void* ud, 
            void* ptr, size_t osize, size_t nsize);
    2) lua_newstate使用的标准分配函数如下:
        void* l_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
          if (0 == nsize) {
            free(ptr);
            return NULL;
          }
          else {
            return realloc(ptr, nsize);
          }
        }
    3) 可以调用lua_getallocf来获取一个Lua状态的内存分配状态;
        lua_Alloc lua_getallocf(lua_State* L, void** ud);
    4) lua_setallocf -> 修改一个Lua状态的内存分配函数;
        void lua_setallocf(lua_State* L, lua_Alloc f, void* ud);
