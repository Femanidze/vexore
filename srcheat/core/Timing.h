#ifndef VEXORE_TIMING_H
#define VEXORE_TIMING_H

long long TimingNowNs(void);
void TimingSleepUntilNs(long long targetNs);
void TimingSleepDeltaNs(long long deltaNs);

#endif
