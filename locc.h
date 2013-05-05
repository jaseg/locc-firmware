
#ifndef __LOCC_H__
// setup pin + led + timer0
void loccSetup(void);
// open locc
void loccStartOpening(void);
// you have to call poll within your main loo
void loccPoll(void);

#ifdef LOCC_DEBUG
void loccTicks(void);
void loccPowerDown(void);
void loccPowerUp(void);
#endif /* LOCC_DEBUG */
#endif//__LOCC_H__
