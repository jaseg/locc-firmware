
#ifndef __LOCC_H__
// setup pin + led + timer0
void loccSetup(void);
// open locc
void loccStartOpening(void);
// you have to call poll within your main loo
void loccPoll(void);

void loccPowerDown(void);
void loccPowerUp(void);

static bool do_next_locc_step(void);

static void do_locc_step(int);

static bool timer_is_expired(void);
#endif//__LOCC_H__
