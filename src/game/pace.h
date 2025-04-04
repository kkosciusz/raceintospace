#ifndef RIS_PACE_H
#define RIS_PACE_H

#include <string>

#include <SDL.h>

#include "raceintospace_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#include "fake_unistd.h"
#endif

void delay(int millisecs);
void FadeIn(char wh, int steps, int val, char mode);
void FadeOut(char wh, int steps, int val, char mode);
int PCX_D(const char *src, char *dest, unsigned src_size);
int brandom(int limit);
int RLED_img(const char *src, char *dest, unsigned int src_size,
             int w, int h);
char *seq_filename(int seq, int mode);
void idle_loop_secs(double secs);
int getch(void);
char DoModem(int sel);
void MesCenter(void);
void StopAudio(char mode);
void stop_voice(void);
void NGetVoice(char plr, char val);
void PlayVoice(void);
ssize_t load_audio_file(const char *, char **data, size_t *size, bool music);
void idle_loop(int ticks);
void play_audio(std::string str, int mode);
void bzdelay(int ticks);
int RLED(const char *src, char *dest, unsigned int src_size);
char MPrefs(char mode);
int put_serial(unsigned char n);
int bioskey(int wait);

extern int show_intro_flag;
extern char *letter_data;


#endif // RIS_PACE_H
