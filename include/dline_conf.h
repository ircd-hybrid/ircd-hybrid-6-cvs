/* dline_conf.h  -- lets muse over dlines, shall we?
 *
 * $Id: dline_conf.h,v 1.5 2001/07/18 01:37:05 lusky Exp $ 
 */

#ifndef INCLUDED_dline_conf_h
#define INCLUDED_dline_conf_h

struct Client;
struct ConfItem;

extern void clear_Dline_table();
extern void zap_Dlines();
extern void add_Dline(struct ConfItem *);
extern void add_ip_Kline(struct ConfItem *);
extern void add_ip_Eline(struct ConfItem *);


extern void add_dline(struct ConfItem *);

extern struct ConfItem *match_Dline(unsigned long);
extern struct ConfItem *match_ip_Kline(unsigned long, const char *);

extern void report_dlines(struct Client *);
extern void report_ip_Klines(struct Client *);
extern void report_ip_Ilines(struct Client *);

#endif /* INCLUDED_dline_conf_h */


