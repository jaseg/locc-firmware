
#ifndef __LOCC_H__
// setup pin + led + timer0
void loccSetup(void);
// open locc
void loccStartOpening(void);
// you have to call poll within your main loo
void loccPoll(void);

void loccTicks(void);
void loccPowerDown(void);
void loccPowerUp(void);
#endif//__LOCC_H__
