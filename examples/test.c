#include "../lautoc.h"


typedef struct {
  char* name;
  int num_wings;
} birdie;

birdie test_birdie;

birdie* get_instance_ptr(lua_State* L) {
  return &test_birdie;
}

int create_birdie(lua_State* L)
{
	birdie *ret = malloc(sizeof(birdie));
	lua_pushlightuserdata(L, ret);
	return 1;
}

int birdie_index(lua_State* L) {
  const char* membername = lua_tostring(L, -1);

  lua_pushstring(L, "cdata");
  lua_gettable(L, -3);
  birdie* self = lua_touserdata(L, -1);
  lua_pop(L, 1);

//  lua_pushstring(L, "b");
//  lua_gettable(L, -3);
//  int b = lua_tointeger(L, -1);
//  lua_pop(L, 1);
//  printf("b = %d\n", b);

  return luaA_struct_push_member_name(L, birdie, membername, self);
}

int birdie_newindex(lua_State* L) {
  const char* membername = lua_tostring(L, -2);
//  birdie* self = get_instance_ptr(L);
  lua_pushstring(L, "cdata");
  lua_gettable(L, -4);
  birdie* self = lua_touserdata(L, -1);
  lua_pop(L, 1);
  
  luaA_struct_to_member_name(L, birdie, membername, self, -1);
  return 0;
}

static int msghandler (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  if (msg == NULL) {  /* is error object not a string? */
    if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      return 1;  /* that is the message */
    else
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  return 1;  /* return the traceback */
}

static int docall (lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
//  globalL = L;  /* to be available to 'laction' */
//  signal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
//  signal(SIGINT, SIG_DFL); /* reset C-signal handler */
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

int C(lua_State* L) {
  return luaA_call_name(L, lua_tostring(L, 1));
}

int main(int argc, char **argv) {
  
  test_birdie.name = "MrFlingly";
  test_birdie.num_wings = 2;
  
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  luaA_open(L);

  lua_register(L, "create_birdie", create_birdie);

  luaA_struct(L, birdie);
  luaA_struct_member(L, birdie, name, char*);
  luaA_struct_member(L, birdie, num_wings, int);
  
  lua_register(L, "birdie_index", birdie_index);
  lua_register(L, "birdie_newindex", birdie_newindex);

  lua_register(L, "C", C);  

    /* "C('hello_world')\n" */
  luaL_loadstring(L, ""
    "Birdie = {}\n"
    "setmetatable(Birdie, Birdie)\n"
    "function Birdie.__call()\n"
    "  local self = {cdata = nil, a = 10, b = 20, c = 30}\n"
	"  self.cdata = create_birdie()\n"
    "  setmetatable(self, Birdie)\n"
    "  return self\n"
    "end\n"
    "Birdie.__index = birdie_index\n"
    "Birdie.__newindex = birdie_newindex\n"
    "\n"
    "bird1 = Birdie()\n"
    "bird1.name = \"woshishui111\"\n"
    "bird1.num_wings = 3\n"
    "bird2 = Birdie()\n"
    "bird2.name = \"woshishui222\"\n"
    "bird2.num_wings = 22222\n"
    "-- bird2.ttt = 22222\n"	  
	  
    "print(bird1.name)\n"
    "print(bird1.num_wings)\n"
    "print(bird2.name)\n"
    "print(bird2.num_wings)\n"
    "\n");

  int status = docall(L, 0, LUA_MULTRET);
  report(L, status);
  luaA_close(L);
  lua_close(L);
  
  return 0;
  
}
