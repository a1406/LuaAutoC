#include "../lautoc.h"

typedef struct {
  char* name;
  int num_wings;
} birdie;

birdie test_birdie;

birdie* get_instance_ptr(lua_State* L) {
  return &test_birdie;
}

int birdie_index(lua_State* L) {
  const char* membername = lua_tostring(L, -1);
  birdie* self = get_instance_ptr(L);

  lua_pushstring(L, "b");
  lua_gettable(L, -3);
  int b = lua_tointeger(L, -1);
  lua_pop(L, 1);
  printf("b = %d\n", b);

  return luaA_struct_push_member_name(L, birdie, membername, self);
}

int birdie_newindex(lua_State* L) {
  const char* membername = lua_tostring(L, -2);
  birdie* self = get_instance_ptr(L);
  luaA_struct_to_member_name(L, birdie, membername, self, -1);
  return 0;
}

static int docall (lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  signal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  signal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  return status;
}
static void l_message (const char *pname, const char *msg) {
  if (pname) printf("%s: ", pname);
  printf("%s\n", msg);
}
static int report (lua_State *L, int status) {
  if (status != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    l_message("", msg);
    lua_pop(L, 1);  /* remove message */
  }
  return status;
}


int main(int argc, char **argv) {
  
  test_birdie.name = "MrFlingly";
  test_birdie.num_wings = 2;
  
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  luaA_open(L);
  
  luaA_struct(L, birdie);
  luaA_struct_member(L, birdie, name, char*);
  luaA_struct_member(L, birdie, num_wings, int);
  
  lua_register(L, "birdie_index", birdie_index);
  lua_register(L, "birdie_newindex", birdie_newindex);

/* (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0)) */
  luaL_loadstring(L, ""
    "Birdie = {}\n"
    "setmetatable(Birdie, Birdie)\n"
    "function Birdie.__call()\n"
    "  local self = {a = 10, b = 20, c = 30}\n"
    "  setmetatable(self, Birdie)\n"
    "  return self\n"
    "end\n"
    "Birdie.__index = birdie_index\n"
    "Birdie.__newindex = birdie_newindex\n"
    "\n"
    "bird = Birdie()\n"
    "print(bird.name)\n"
    "print(bird.num_wings)\n"
    "bird.num_wings = 3\n"
    "print(bird.num_wings)\n"
    "\n");

  int status = docall(L, 0, LUA_MULTRET);
  report(L, status);
  luaA_close(L);
  lua_close(L);
  
  return 0;
  
}
