/* nv_pstate:
   Change pstate of nvidia gfx card.
   This program uses nouveau driver interface exposed trough kernel debugfs:
   /sys/kernel/debug/dri/<card_number>/pstate

   nv_pstate is intended to be used in start-up scripts for games or
   other gfx-intensive programs - and therefore it must have suid bit set

   WARNING: support for changing pstates is experimental - use at Your own risk!

   Author : Tomasz Pawlak
   e-mail : tomasz.pawlak@wp.eu
   License: GPLv3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>

#define ERR_PRINT(...) \
   fprintf(stderr, __VA_ARGS__)

#define LOG_PRINT(...) \
   fprintf(stdout, __VA_ARGS__)

#define _PROGNAME_  "nv_pstate"
#define MAX_CARD_NR 255
#define MAX_PSTATE  255

const char VERSION   [] = "1.0";
const char PROG_NAME [] = _PROGNAME_;
const char HLP_UINFO [] = "Use " _PROGNAME_ " --help or -h to display usage info\n";
const char BAD_PSTATE[] = "[E!] Invalid/missing pstate value!\n";

/* cmd line args */
struct option long_options[] = {
   {"card   " , required_argument, NULL, 'c'},
   {"pstate " , required_argument, NULL, 's'},
   {"verbose" , no_argument      , NULL, 'V'},
   {"help   " , no_argument      , NULL, 'h'},
   {"version" , no_argument      , NULL, 'v'},
   {0, 0, 0, 0}
};
#define SHORT_OPTS "c:s:Vhv"

#define PATH_SZ  48
#define PATH_MAX (PATH_SZ-1)

typedef struct {
   int  card   ;
   int  verbose;
   int  pstate ;
   char path   [PATH_SZ];
} config_t;

void cfg_print(const config_t *const cfg) {
   LOG_PRINT("Configuration:\n"
             "card : %d\n"
             "path : %s\n"
             "pstate: 0x%02X\n",
             cfg->card,
             cfg->path,
             cfg->pstate
            );
}

int get_cmdline_opts(int inargc, char* const* inargv, config_t *const cfg) {

   static const char BUILD[] = "build: " __DATE__ ", " __TIME__;

   static const char HELP[] =
   "Options:\n"
   "   -c --card      gfx card number\n"
   "   -s --pstate    pstate value\n"
   "   -V --verbose   verbose mode\n"
   "   -h --help      *this* short usage info\n"
   "   -v --version   display program version\n";

   int     opt_rv; /* 0 = continue main(), 1 = exit_ok, -1 = exit_error */
   int     tmpi;
   long    tmpl;
   char   *endptr;

   opt_rv = 0;

   while ((tmpi = getopt_long(inargc, inargv, SHORT_OPTS, long_options, NULL)) != EOF)
   {
      errno = 0;

      switch (tmpi) {
         case 'c': /* card */
            tmpl = strtol(optarg, &endptr, 0);
            if ( (errno != 0) || (tmpl < 0) || (tmpl > MAX_CARD_NR) ) {
               ERR_PRINT( "[E!] Invalid card number: \'%s\'\n", optarg);
               opt_rv = -1;
            }
            cfg->card = (int) tmpl;
            break;
         case 's': /* pstate */
            tmpl = strtol(optarg, &endptr, 0);
            if ( (errno != 0) || (tmpl < 0) || (tmpl > MAX_PSTATE) ) {
               ERR_PRINT( BAD_PSTATE );
               opt_rv = -1;
            }
            cfg->pstate = (int) tmpl;
            break;
         case 'V': /* verbose */
            cfg->verbose = 1;
            break;
         case 'h': /* help */
            LOG_PRINT( HELP );
            opt_rv = 1;
            break;
         case 'v': /* version */
            LOG_PRINT("%s version %s, %s\n", PROG_NAME, VERSION, BUILD);
            opt_rv = 1;
            break;
         case '?': /* error */
            opt_rv = -1;
            break;
      }
      if (opt_rv != 0) return opt_rv;
   }
   return opt_rv;
}

int write_pstate(const config_t *const cfg) {
   char str_pstate[8];
   int  fds;
   int  slen;
   int  wlen;
   int  retv = 0;

   if (cfg->verbose) cfg_print(cfg);

   if (cfg->pstate < 0) {
      ERR_PRINT( BAD_PSTATE );
      return -1;
   }

   slen = snprintf(str_pstate, 8, "%x%c", cfg->pstate, 0);
   if (slen < 1) {
      ERR_PRINT( "[E!] Failed conversion: pstate -> string.\n");
      return -1;
   }

   errno = 0;
   fds   = open(cfg->path, O_WRONLY);
   if (fds < 0) {
      ERR_PRINT( "[E!] Failed to open file:\n\"%s\"\n%s\n", cfg->path, strerror(errno) );
      return -1;
   }

   slen  ++ ; //++ NULL byte
   errno  = 0;
   wlen   = write(fds, str_pstate, slen);
   if (wlen < slen) {
      ERR_PRINT( "[E!] Failed writing pstate:\n\"%s\"\n%s\n", cfg->path, strerror(errno) );
      retv = -1;
   }

   close(fds);

   if ( (retv == 0) && (cfg->verbose) ) LOG_PRINT("[i] pstate changed to 0x%02X\n", cfg->pstate);

   return retv;
}

int main( int argc, char *argv[] )
{
   /* default debugfs path for nouveau driver */
   static const char def_path[] = "/sys/kernel/debug/dri/%u/pstate";

   config_t cfg;
   int      retv;

   /* init config */
   memset(&cfg, 0, sizeof(config_t) );
   cfg.card   = -1;
   cfg.pstate = -1;

   retv = get_cmdline_opts(argc, argv, &cfg);
   if (0 != retv) {
      if (0 > retv) goto exit_err;
      goto exit;
   }

   /* If card number is given, use the default debugfs path */
   if (cfg.card >= 0) {
      snprintf(cfg.path, PATH_MAX, def_path, cfg.card);
      retv = write_pstate(&cfg);
      return retv;
   }

exit_err:
   ERR_PRINT( "[E!] Incorrect and/or missing parameters.\n%s", HLP_UINFO );
exit:
   if (cfg.verbose != 0) cfg_print(&cfg);
   return retv;
}
