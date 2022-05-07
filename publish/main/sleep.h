#ifndef SLEEP_H
#define SLEEP_H

void handle_deep_sleep_wakeup();

void set_wakeup_timer(int wakeup_time_sec);

void start_deep_sleep();

#endif /* SLEEP_H */
