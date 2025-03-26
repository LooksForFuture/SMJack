#ifndef _P_MAIN_
#define _P_MAIN_

//init the gameplay system (player, enemies, etc)
void p_init(void);

//shutdown systems and free gameplay related stuff
void p_shutdown(void);

//the main gameplay loop that keeps the program alive
void p_mainloop(void);

#endif
