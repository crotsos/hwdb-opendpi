/*
 * logdefs.h - global #defines used for logging, debugging, warning, and error
 *             messages
 *
 * define these symbols in CFLAGS to change the behavior
 */

#ifndef _LOGDEFS_H_DEFINED_
#define _LOGDEFS_H_DEFINED_

/*
 * defined symbols that control logging and debugging information from
 * the running system - these will typically be changed by specifying
 * -D<symbol> in the CFLAGS definition in the Makefile 
 *
 * DEBUG - if defined, debugf() statements are printed on stdout preceded
 *         by "DEBUG> "; otherwise, debugf() and debugvf() statements are
 *         compiled out of the code
 * VDEBUG - if defined and DEBUG, all debugvf() statements are printed
 *          on stdout preceded by "DEBUGV> "
 * LOG - if defined, logf() statements are printed on stdout preceded by "LOG> "
 *       otherwise, all occurrences of logf() and logvf() statements are
 *       compiled out of the code
 * VLOG - if defined and LOG, all logvf() statements are printed on stdout
 *        preceded by "LOGV> "
 * WARNING - if defined, warningf() statements are printed on stdout preceded
 *           by "WARNING> "; otherwise, warningf() statements are compiled
 *           out of the code
 */
#ifdef DEBUG
#define debugf printf("DEBUG> "); printf
#ifndef LOG
#define LOG
#endif /* LOG */
#ifdef VDEBUG
#define debugvf printf("DEBUGV> "); printf
#else /* !VDEBUG */
#define debugvf(...) ((void)0)
#endif /* VDEBUG */
#else /* DEBUG */
#define debugf(...) ((void)0)
#define debugvf(...) ((void)0)
#endif /* DEBUG */

#ifdef LOG
#define logf printf("LOG> "); printf
#ifdef VLOG
#define logvf printf("LOGV> "); printf
#else /* !VLOG */
#define logvf(...) ((void)0)
#endif /* VLOG */
#else /* LOG */
#define logf(...) ((void)0)
#define logvf(...) ((void)0)
#endif /* LOG */

#ifdef WARNING
#define warningf printf("WARNING> "); printf
#else /* WARNING */
#define warningf(...) ((void)0)
#endif /* WARNING */

/*
 * errorf() statements are printed on stdout preceded by "ERROR> "
 */
#define errorf printf("ERROR> "); printf

#endif /* _LOGDEFS_H_DEFINED_ */
